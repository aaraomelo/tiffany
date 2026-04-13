import { Injectable, Logger } from '@nestjs/common';
import Anthropic from '@anthropic-ai/sdk';
import { PrismaService } from '../prisma.service';
import { PatriciaGatewayService } from '../patricia-gateway.service';
import { MemoryService } from './memory.service';
import { PATRICIA_TOOLS, PATRICIA_SYSTEM_PROMPT } from './patricia-tools';
import { InboundMessage } from './dto/inbound-message.dto';

const MODEL = process.env.PATRICIA_MODEL || 'claude-haiku-4-5-20251001';
const MAX_HISTORY = 10;

@Injectable()
export class PatriciaLlmService {
  private readonly logger = new Logger('PatriciaLLM');
  private client: Anthropic;

  constructor(
    private prisma: PrismaService,
    private gateway: PatriciaGatewayService,
    private memory: MemoryService,
  ) {
    this.client = new Anthropic();
  }

  async processMessage(inbound: InboundMessage): Promise<string> {
    const channel = inbound.channelType;
    const target = inbound.remoteId;

    // Check if specialist session is active
    const session = await this.gateway.getOrCreateSession(channel, target);
    const meta = (session.metadata as any) || {};

    if (meta.specialistActive) {
      // Check for close commands
      const lower = inbound.text.toLowerCase();
      if (lower.includes('fecha') || lower.includes('obrigado técnico') || lower.includes('pode fechar') || lower.includes('close')) {
        // Summarize specialist conversation and save to Patricia's history + memory
        const specHistory: Array<{ role: string; content: string }> = meta.specialistHistory || [];
        const history: Array<{ role: string; content: string }> = meta.history || [];

        if (specHistory.length > 0) {
          // Add specialist conversation summary to Patricia's history
          const summary = specHistory.map((m) =>
            `[${m.role === 'user' ? 'Diretor' : 'Especialista'}]: ${m.content.substring(0, 300)}`
          ).join('\n');
          history.push({ role: 'user', content: `[Conversa com especialista]\n${summary}` });
          history.push({ role: 'assistant', content: 'Entendido, vi a conversa com o especialista.' });

          // Save key points as short_term memory
          const lastAnswer = specHistory.filter((m) => m.role === 'assistant').pop();
          if (lastAnswer) {
            const firstQuestion = specHistory.find((m) => m.role === 'user');
            await this.memory.save(
              'technical',
              `Consulta técnica: ${(firstQuestion?.content || '').substring(0, 50)}`,
              lastAnswer.content.substring(0, 500),
              'short_term',
            );
          }
        }

        await this.prisma.conversationSession.update({
          where: { id: session.id },
          data: { metadata: { ...meta, specialistActive: false, specialistHistory: [], history: history.slice(-MAX_HISTORY * 2) } },
        });
        return 'Sessão com o especialista encerrada. Vi o que vocês discutiram — estou por dentro. O que precisa?';
      }

      // Route to specialist (Claude Code via API, not Patricia)
      return this.processSpecialistMessage(inbound, session, meta);
    }

    // Load conversation history from session metadata
    const history: Array<{ role: string; content: string }> = meta.history || [];

    // Get dynamic context from gateway
    const context = await this.gateway.getContext(channel, target);

    // Build messages array
    const messages: Anthropic.MessageParam[] = [];

    // Add recent history
    for (const msg of history.slice(-MAX_HISTORY)) {
      messages.push({
        role: msg.role === 'user' ? 'user' : 'assistant',
        content: msg.content,
      });
    }

    // Add current message
    messages.push({ role: 'user', content: inbound.text });

    // Resolve person + profile
    let person: any = null;
    let profile: any = null;
    let senderInfo = `Remetente: ${inbound.displayName} (${inbound.senderPhone})`;
    try {
      const contact = await this.prisma.messagingContact.findFirst({
        where: { channelType: inbound.channelType as any, remoteId: inbound.remoteId },
        include: { person: { include: { profile: true } } },
      });
      if (contact?.person) {
        person = contact.person;
        profile = contact.person.profile;
        senderInfo = `Remetente: ${person.name} (${person.role}) — ${inbound.channelType}`;
      }
    } catch {}

    // Load memories filtered by access level
    const memoryAccess = profile?.memoryAccess || 'own';
    const memoryContext = await this.memory.getContextForPerson(inbound.text, person?.id, memoryAccess);

    // Group context
    const groupSection = inbound.groupContext
      ? `\n\n## Mensagens recentes do grupo\n${inbound.groupContext}`
      : '';

    // Check privacy mode
    const freshSession = await this.prisma.conversationSession.findUnique({ where: { id: session.id } });
    const isPrivacyMode = (freshSession?.metadata as any)?.privacyMode === true;
    const privacyBanner = isPrivacyMode ? '\n\n🔒 **MODO PRIVADO ATIVO** — Não revele conteúdo desta conversa a ninguém. Mensagens não são logadas. Memórias são seladas.' : '';

    // Check if first message (no history) — adapt greeting by profile
    const isFirstMessage = history.length === 0;
    const greetingHint = isFirstMessage && profile
      ? `\n\n## Primeira interação\nEsta é a primeira mensagem desta pessoa. Apresente-se brevemente de acordo com seu perfil "${profile.name}". Não mencione outros perfis ou funcionalidades que não são deste perfil.`
      : '';

    // Build system prompt: SOUL (core) + profile prompt + privacy + greeting + memories + context
    const profilePrompt = profile?.systemPrompt ? `\n\n## Modo ativo\n${profile.systemPrompt}` : '';
    const systemPrompt = `${PATRICIA_SYSTEM_PROMPT}${profilePrompt}${privacyBanner}${greetingHint}\n\n${memoryContext}${groupSection}\n\n## Contexto atual da conversa\n${context}\n\n${senderInfo}`;

    try {
      // Filter tools by profile
      const allowedToolNames: string[] = profile?.allowedTools || [];
      const tools = allowedToolNames.length > 0
        ? PATRICIA_TOOLS.filter((t) => allowedToolNames.includes(t.name))
        : PATRICIA_TOOLS;

      // Force specific tools based on message patterns
      const lowerText = inbound.text.toLowerCase();
      const isUpdateContact = /atualiza.*contato|muda.*descrição|altera.*contato/i.test(lowerText);
      const isCheckSent = /o que (vc|você) (mandou|enviou|disse|falou) pra/i.test(lowerText);
      const isCheckResponse = /respondeu|disse o qu[eê]|falou o qu[eê]|o que (ela|ele) (disse|falou|respondeu)|recebeu.*msg|chegou.*msg/i.test(lowerText);
      const isContactQuery = /contato|telefone|número|quem é|falou com|mandou pra|disse pra|conhece o|conhece a|meu amig|minha irm/i.test(lowerText);

      let toolChoice: any = undefined;
      if (isUpdateContact) {
        toolChoice = { type: 'tool', name: 'update_contact' };
      } else if (isCheckSent) {
        toolChoice = { type: 'tool', name: 'check_sent' };
      } else if (isCheckResponse) {
        toolChoice = { type: 'tool', name: 'check_contact' };
      } else if (isContactQuery) {
        toolChoice = { type: 'any' };
      }

      // Call Claude with tools
      let response = await this.client.messages.create({
        model: MODEL,
        max_tokens: 1024,
        system: systemPrompt,
        tools: tools.length > 0 ? tools as any : undefined,
        tool_choice: toolChoice,
        messages,
      });

      // Handle tool use loop (multi-turn)
      const toolMessages: Anthropic.MessageParam[] = [...messages];
      let iterations = 0;
      const maxIterations = 5;

      while (response.stop_reason === 'tool_use' && iterations < maxIterations) {
        iterations++;

        // Collect all tool uses from this response
        const assistantContent = response.content;
        toolMessages.push({ role: 'assistant', content: assistantContent });

        const toolResults: Anthropic.ToolResultBlockParam[] = [];

        for (const block of assistantContent) {
          if (block.type !== 'tool_use') continue;

          const toolName = block.name;
          const toolInput = block.input as Record<string, any>;

          this.logger.log(`Tool call: ${toolName}(${JSON.stringify(toolInput).substring(0, 100)})`);

          // Execute via gateway
          try {
            const result = await this.gateway.executeAction(
              toolName,
              channel,
              target,
              toolInput,
            );
            toolResults.push({
              type: 'tool_result',
              tool_use_id: block.id,
              content: JSON.stringify(result),
            });
          } catch (err) {
            toolResults.push({
              type: 'tool_result',
              tool_use_id: block.id,
              content: JSON.stringify({ error: err.message }),
              is_error: true,
            });
          }
        }

        toolMessages.push({ role: 'user', content: toolResults });

        // Continue conversation with tool results
        response = await this.client.messages.create({
          model: MODEL,
          max_tokens: 1024,
          system: systemPrompt,
          tools: PATRICIA_TOOLS as any,
          messages: toolMessages,
        });
      }

      // Extract final text response
      const textBlocks = response.content.filter((b) => b.type === 'text');
      const finalText = textBlocks.map((b) => (b as any).text).join('\n') || 'Entendido.';

      // Save history — re-read metadata to preserve changes made by gateway (e.g. specialistActive)
      history.push({ role: 'user', content: inbound.text });
      history.push({ role: 'assistant', content: finalText });
      const trimmedHistory = history.slice(-MAX_HISTORY * 2);

      const freshSession = await this.prisma.conversationSession.findUnique({ where: { id: session.id } });
      const freshMeta = (freshSession?.metadata as any) || {};

      await this.prisma.conversationSession.update({
        where: { id: session.id },
        data: {
          metadata: { ...freshMeta, history: trimmedHistory },
          lastUserMessage: inbound.text,
          lastActionAt: new Date(),
        },
      });

      // Cross-channel awareness: if non-director mentions a director or does something notable, log it
      if (person && person.role !== 'director') {
        this.logCrossChannelEvent(person, inbound.text, finalText).catch(() => {});
      }

      return finalText;
    } catch (err) {
      this.logger.error(`LLM error: ${err.message}`);
      return 'Desculpe, tive um problema ao processar sua mensagem. Tente novamente em alguns instantes.';
    }
  }

  private async processSpecialistMessage(inbound: InboundMessage, session: any, meta: any): Promise<string> {
    const specialistHistory: Array<{ role: string; content: string }> = meta.specialistHistory || [];

    const messages: Anthropic.MessageParam[] = [];
    for (const msg of specialistHistory.slice(-20)) {
      messages.push({
        role: msg.role === 'user' ? 'user' : 'assistant',
        content: msg.content,
      });
    }
    messages.push({ role: 'user', content: inbound.text });

    // Load memories for context
    const memoryContext = await this.memory.getContext(inbound.text);

    const systemPrompt = `Você é o técnico especialista da Patria Technology. Tem conhecimento profundo do código.

Stack: NestJS + Prisma + PostgreSQL (backend), React + Vite (frontend), Node.js worker com Claude Code CLI.

${memoryContext}

Responda em português, seja direto e técnico. Cite arquivos e linhas quando relevante. Máximo 20 linhas.
Quando o usuário disser "fecha", "obrigado" ou "pode fechar", responda se despedindo brevemente.`;

    try {
      const response = await this.client.messages.create({
        model: MODEL,
        max_tokens: 2048,
        system: systemPrompt,
        messages,
      });

      const text = response.content
        .filter((b) => b.type === 'text')
        .map((b) => (b as any).text)
        .join('\n') || 'Pode perguntar.';

      // Save specialist history
      specialistHistory.push({ role: 'user', content: inbound.text });
      specialistHistory.push({ role: 'assistant', content: text });

      await this.prisma.conversationSession.update({
        where: { id: session.id },
        data: {
          metadata: { ...meta, specialistHistory: specialistHistory.slice(-40) },
          lastActionAt: new Date(),
        },
      });

      return text;
    } catch (err) {
      this.logger.error(`Specialist error: ${err.message}`);
      return 'Erro ao consultar o especialista. Tente novamente.';
    }
  }

  private async logCrossChannelEvent(person: any, userText: string, response: string): Promise<void> {
    // Detect notable events: mentions of directors, questions about work, privacy tests
    const lower = userText.toLowerCase();
    const directorNames = ['aarão', 'aarao', 'patrícia cunha', 'patricia cunha', 'carlos daniel', 'carlos'];
    const mentionsDirector = directorNames.some((n) => lower.includes(n));
    const asksAboutWork = /trabalh|empres|projet|tecnolog|billing|deploy|sistema/i.test(lower);
    const privacyTest = /o que (vc|você) convers|o que fal|sobre o que/i.test(lower);

    if (mentionsDirector || asksAboutWork || privacyTest) {
      const summary = `${person.name} perguntou: "${userText.substring(0, 100)}". Patrícia respondeu de forma ${privacyTest ? 'confidencial (não revelou detalhes)' : 'adequada ao perfil'}.`;
      await this.memory.save(
        'project',
        `Interação com ${person.name}`,
        summary,
        'short_term',
        null, // global — directors can see
        'global',
      );
      this.logger.log(`Cross-channel event: ${person.name} → ${mentionsDirector ? 'mentioned director' : asksAboutWork ? 'asked about work' : 'privacy test'}`);
    }
  }
}
