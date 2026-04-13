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

    // Resolve person + profile FIRST (needed for history, tools, prompt)
    let person: any = null;
    let profile: any = null;
    try {
      const contact = await this.prisma.messagingContact.findFirst({
        where: { channelType: inbound.channelType as any, remoteId: inbound.remoteId },
        include: { person: { include: { profile: true } } },
      });
      if (contact?.person) {
        person = contact.person;
        profile = contact.person.profile;
      }
    } catch {}

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

    // Load conversation history centralized by PERSON — max 5KB
    const MAX_HISTORY_BYTES = 5000;
    let history: Array<{ role: string; content: string }> = [];
    if (person) {
      try {
        const msgs = await this.prisma.personMessage.findMany({
          where: { personId: person.id },
          orderBy: { createdAt: 'desc' },
          take: 30, // fetch more, then trim by size
          select: { role: true, content: true },
        });
        // Trim to fit 5KB, keeping most recent
        let totalBytes = 0;
        const trimmed: typeof msgs = [];
        for (const m of msgs) {
          totalBytes += m.content.length;
          if (totalBytes > MAX_HISTORY_BYTES) break;
          trimmed.push(m);
        }
        history = trimmed.reverse();
      } catch {}
    } else {
      // Unknown person: use session history as fallback
      history = meta.history || [];
    }

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

    // person + profile already resolved above

    // Load memories filtered by access level
    const memoryAccess = profile?.memoryAccess || 'own';
    const memoryContext = await this.memory.getContextForPerson(inbound.text, person?.id, memoryAccess);

    // Group context
    const groupSection = inbound.groupContext
      ? `\n\n## Mensagens recentes do grupo\n${inbound.groupContext}`
      : '';

    // Privacy mode
    const freshSession = await this.prisma.conversationSession.findUnique({ where: { id: session.id } });
    const isPrivacyMode = (freshSession?.metadata as any)?.privacyMode === true;

    // === BUILD DYNAMIC CORE ===
    const systemPrompt = this.buildDynamicPrompt({
      person, profile, inbound, memoryContext, groupSection,
      context, isPrivacyMode, isFirstMessage: history.length === 0,
    });

    try {
      // Filter tools by profile — NO PROFILE = NO TOOLS (security)
      const allowedToolNames: string[] = profile?.allowedTools || [];
      const tools = allowedToolNames.length > 0
        ? PATRICIA_TOOLS.filter((t) => allowedToolNames.includes(t.name))
        : []; // Unknown person gets ZERO tools — conversation only

      // Classify intent via LLM to force correct tool
      const toolChoice = await this.classifyIntent(inbound.text, tools);

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

          // Block tool calls not in profile's allowed list
          const allowedNames = profile?.allowedTools || [];
          if (allowedNames.length > 0 && !allowedNames.includes(toolName)) {
            this.logger.warn(`Blocked tool ${toolName} — not in profile ${profile?.slug}`);
            toolResults.push({
              type: 'tool_result',
              tool_use_id: block.id,
              content: JSON.stringify({ error: `Ação "${toolName}" não disponível no seu perfil.` }),
              is_error: true,
            });
            continue;
          }

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

      // Save messages centralized by person (skip if privacy mode)
      if (person && !isPrivacyMode) {
        await this.prisma.personMessage.createMany({
          data: [
            { personId: person.id, channel: inbound.channelType, role: 'user', content: inbound.text },
            { personId: person.id, channel: inbound.channelType, role: 'assistant', content: finalText },
          ],
        });
        // Update person context
        await this.prisma.person.update({
          where: { id: person.id },
          data: { context: { lastChannel: inbound.channelType, lastMessageAt: new Date().toISOString() } },
        });
      }

      // Update session metadata (preserve gateway flags like specialistActive)
      const freshSession = await this.prisma.conversationSession.findUnique({ where: { id: session.id } });
      const freshMeta = (freshSession?.metadata as any) || {};
      await this.prisma.conversationSession.update({
        where: { id: session.id },
        data: { lastUserMessage: inbound.text, lastActionAt: new Date(), metadata: freshMeta },
      });

      // Auto-save memory based on profile rules (async, non-blocking)
      if (person && !isPrivacyMode && profile) {
        this.autoSaveMemory(person, profile, inbound.text, finalText).catch(() => {});
      }

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

  private buildDynamicPrompt(opts: {
    person: any; profile: any; inbound: InboundMessage;
    memoryContext: string; groupSection: string;
    context: string; isPrivacyMode: boolean; isFirstMessage: boolean;
  }): string {
    const { person, profile, inbound, memoryContext, groupSection, context, isPrivacyMode, isFirstMessage } = opts;
    const parts: string[] = [];

    // 1. Core identity (from SOUL.md — just name + 10 principles)
    parts.push(PATRICIA_SYSTEM_PROMPT);

    // 2. Who is talking
    if (person) {
      parts.push(`## Quem está falando
${person.name} (${person.role})${person.description ? ' — ' + person.description : ''}
Canal: ${inbound.channelType}`);
    } else {
      parts.push(`## Quem está falando
${inbound.displayName} — pessoa desconhecida`);
    }

    // 3. Profile — define TUDO: tom, regras, permissões
    if (profile?.systemPrompt) {
      parts.push(profile.systemPrompt);
    }

    // 4. Privacy
    if (isPrivacyMode) {
      parts.push('🔒 MODO PRIVADO ATIVO');
    }

    // 5. First message
    if (isFirstMessage) {
      parts.push(`Primeira mensagem de ${person?.name || inbound.displayName}. Apresente-se.`);
    }

    // 6. Memories
    if (memoryContext) parts.push(memoryContext);

    // 7. Group context
    if (groupSection) parts.push(groupSection);

    // 8. Gateway context
    if (context) parts.push(context);

    return parts.join('\n\n');
  }

  // Keyword triggers per profile slug for auto-save
  private static MEMORY_TRIGGERS: Record<string, Array<{ terms: RegExp; category: string; priority: string }>> = {
    gestora: [
      { terms: /\b(decid|decidimos|decidiu|foco|prioridade|estratégia|objetivo|meta|prazo|deadline)\b/i, category: 'decision', priority: 'long_term' },
      { terms: /\b(prefir|prefere|gosto|não gosto|sempre|nunca faça|evite)\b/i, category: 'preference', priority: 'long_term' },
      { terms: /\b(concluí|finalizou|lançou|deployou|promoveu|migrou|implementou)\b/i, category: 'project', priority: 'short_term' },
      { terms: /\b(bug|erro|problema|falha|quebrou|caiu|travou)\b/i, category: 'technical', priority: 'short_term' },
      { terms: /\b(contrat|demit|entrou|saiu|parceiro|cliente|fornecedor)\b/i, category: 'person', priority: 'long_term' },
    ],
    amiga: [
      { terms: /\b(aniversário|nasceu|data|evento|casamento|formatura|festa)\b/i, category: 'person', priority: 'long_term' },
      { terms: /\b(gost[oa]|amo|odeio|prefir|favorit|hobby|paixão)\b/i, category: 'preference', priority: 'long_term' },
      { terms: /\b(famíli|pai|mãe|irmã|irmão|filh[oa]|esposa|marido|namorad)\b/i, category: 'person', priority: 'long_term' },
      { terms: /\b(trabalh|emprego|profissão|formação|faculdade|curso|estudo)\b/i, category: 'person', priority: 'long_term' },
      { terms: /\b(saúde|médico|doença|remédio|hospital|dor|cirurgia|tratamento)\b/i, category: 'preference', priority: 'long_term' },
      { terms: /\b(igreja|religião|deus|oração|culto|daime|espiritual|fé)\b/i, category: 'preference', priority: 'long_term' },
      { terms: /\b(medo|ansiedade|triste|depressão|preocupad|angústia|sozinho)\b/i, category: 'preference', priority: 'long_term' },
      { terms: /\b(sonho|plano|quero|vou|pretendo|projeto de vida|futuro)\b/i, category: 'preference', priority: 'long_term' },
      { terms: /\b(mora|mudei|cidade|bairro|casa|apartamento|endereço)\b/i, category: 'person', priority: 'long_term' },
    ],
    juridica: [
      { terms: /\b(contrato|cláusula|lei|artigo|lgpd|processo|ação judicial|multa)\b/i, category: 'decision', priority: 'long_term' },
    ],
    mentora: [
      { terms: /\b(negócio|startup|produto|mercado|receita|lucro|investimento|meta|kpi|métrica)\b/i, category: 'decision', priority: 'long_term' },
    ],
    assistente: [
      { terms: /\b(lembr|agendar|marcar|compromisso|reunião|horário|prazo|entregar)\b/i, category: 'preference', priority: 'short_term' },
    ],
  };

  private async autoSaveMemory(person: any, profile: any, userText: string, assistantText: string): Promise<void> {
    const slug = profile.slug || '';
    const triggers = PatriciaLlmService.MEMORY_TRIGGERS[slug];
    if (!triggers) return;

    for (const trigger of triggers) {
      if (trigger.terms.test(userText)) {
        // Extract first sentence as title, full text as content
        const title = userText.substring(0, 80).replace(/[.!?].*/, '') || userText.substring(0, 80);
        const visibility = profile.memoryAccess === 'all' ? 'global' : 'private';

        await this.memory.save(
          trigger.category,
          title,
          userText.substring(0, 500),
          trigger.priority,
          person.id,
          visibility,
        );
        this.logger.log(`Auto-saved: [${trigger.category}] "${title.substring(0, 40)}" for ${person.name}`);
        return; // Save once per message
      }
    }
  }

  private async classifyIntent(text: string, availableTools: any[]): Promise<any> {
    const toolNames = availableTools.map((t) => t.name).join(', ');
    try {
      const res = await this.client.messages.create({
        model: MODEL,
        max_tokens: 30,
        system: `Classifique a intenção da mensagem. Responda APENAS com o nome da tool ou "none".

Tools disponíveis: ${toolNames}

Regras:
- Enviar mensagem pra alguém → send_message
- Salvar/adicionar contato → add_contact
- Atualizar contato/descrição → update_contact
- Consultar contato/quem é/telefone → check_contact
- O que mandei/enviei pra alguém → check_sent
- Ela respondeu/o que disse → check_contact
- Status/relatório → status
- Criar tarefa → create_task
- Criar projeto → create_project
- Pergunta pro técnico → ask_specialist
- Chama o especialista → open_specialist
- Salvar memória → save_memory
- Qual erro/detalhe da tarefa/o que aconteceu → task_detail
- Detalhe do projeto → project_detail
- Retenta/tenta de novo/roda de novo → retry_task
- Conversa casual, saudação, opinião → none`,
        messages: [{ role: 'user', content: text }],
      });

      const answer = (res.content[0] as any)?.text?.trim().toLowerCase() || 'none';
      const matchedTool = availableTools.find((t) => answer.includes(t.name));

      if (matchedTool) {
        this.logger.log(`Intent: "${text.substring(0, 40)}" → ${matchedTool.name}`);
        return { type: 'tool', name: matchedTool.name };
      }
      // If classifier suggested a tool not in available list, don't force anything
      // This prevents profile leakage (e.g. amiga calling status)
      this.logger.log(`Intent: "${text.substring(0, 40)}" → none (${answer} not available)`);
      return undefined;
    } catch (err) {
      this.logger.warn(`Intent classification failed: ${err.message}`);
      return undefined;
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
