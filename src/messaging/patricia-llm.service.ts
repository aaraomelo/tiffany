import { Injectable, Logger } from '@nestjs/common';
import type Anthropic from '@anthropic-ai/sdk';
import { PrismaService } from '../prisma.service';
import { PatriciaGatewayService } from '../patricia-gateway.service';
import { ProfileService } from '../profile.service';
import { MemoryService } from './memory.service';
import { PATRICIA_TOOLS, PATRICIA_SYSTEM_PROMPT } from './patricia-tools';
import { InboundMessage } from './dto/inbound-message.dto';
import { bridgeCall } from '../worker/bridge-client';

const MAX_HISTORY = 15;

@Injectable()
export class PatriciaLlmService {
  private readonly logger = new Logger('PatriciaLLM');

  constructor(
    private prisma: PrismaService,
    private gateway: PatriciaGatewayService,
    private profileService: ProfileService,
    private memory: MemoryService,
  ) {}

  private async getCurrentModel(personId?: string): Promise<string> {
    return this.profileService.getPersonModel(personId);
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

    // Check privacy activation by keyword (don't trust LLM to call the tool)
    const lower = inbound.text.toLowerCase();
    if (!meta.privacyMode && (lower.includes('modo privado') || lower.includes('privacidade') || lower.includes('incógnito') || lower.includes('incognito') || lower.includes('sandbox'))) {
      if (inbound.channelType === 'telegram') {
        // Telegram: only send Mini App button, activation happens via initData
        try {
          const { TelegramService } = await import('./telegram.service');
          const telegram = new TelegramService();
          const appUrl = `https://${process.env.APP_DOMAIN || 'patria.patriatechnology.com'}/sandbox.html`;
          await telegram.sendWithWebApp(target, '🔒 Toque pra ativar o modo privado:', 'Ativar Modo Privado', appUrl);
        } catch (err) {
          this.logger.error(`Mini App button failed: ${err.message}`);
        }
        return '';
      }
      // WhatsApp/other: activate directly
      await this.gateway.executeAction('toggle_privacy', channel, target, { enabled: true });
      return 'Modo privado ativado. Zero rastro ao sair.';
    }

    // Sandbox: check deactivation keywords or Mini App status
    if (meta.privacyMode) {
      const deactivateKeywords = ['sai do modo privado', 'desativa privacidade', 'encerrar modo', 'sair do privado', 'desativar privacidade', 'encerrar privacidade'];
      if (deactivateKeywords.some(k => lower.includes(k))) {
        // Close Mini App FIRST (before deactivating, so SSE is still alive)
        if (inbound.channelType === 'telegram') {
          const { SandboxController } = await import('./sandbox.controller');
          SandboxController.closeMiniApp(target);
          await new Promise(r => setTimeout(r, 500)); // wait for close event to arrive
        }
        await this.gateway.executeAction('toggle_privacy', channel, target, { enabled: false });
        return '🔓 Modo privado encerrado.';
      }
      // Telegram: verify Mini App is still open
      if (inbound.channelType === 'telegram') {
        const { SandboxController } = await import('./sandbox.controller');
        if (!SandboxController.isMiniAppAlive(target)) {
          await this.gateway.executeAction('toggle_privacy', channel, target, { enabled: false });
          return '🔓 Modo privado encerrado. O app de privacidade foi fechado.';
        }
      }
    }

    // Check simulation mode
    if (meta.simulationActive && meta.simulationPerson) {
      const exitPhrases = ['sai da simulação', 'para de simular', 'encerra simulação', 'volta ao normal', 'pode parar', 'para a simulação', 'sair', 'fecha simulação'];
      if (exitPhrases.some(p => inbound.text.toLowerCase().includes(p))) {
        await this.prisma.conversationSession.update({
          where: { id: session.id },
          data: { metadata: { ...meta, simulationActive: false, simulationPerson: null, simulationHistory: [] } },
        });
        return 'Simulação encerrada. Voltei ao modo normal, Aarão.';
      }
      return this.processSimulationMessage(inbound, session, meta);
    }



    // Load conversation history
    const MAX_HISTORY_BYTES = 8000;
    let history: Array<{ role: string; content: string }> = [];
    const isPrivacyMode = meta?.privacyMode === true;

    if (isPrivacyMode) {
      // Stateless: fetch from client device
      if (inbound.channelType === 'whatsapp') {
        // WhatsApp: via Baileys bridge
        try {
          const waUrl = process.env.WA_BRIDGE_URL || 'http://host.docker.internal:8089';
          const waKey = process.env.WA_BRIDGE_KEY || 'wa_bridge_2026';
          const res = await fetch(`${waUrl}/messages`, {
            method: 'POST',
            headers: { 'Content-Type': 'application/json', 'X-API-Key': waKey },
            body: JSON.stringify({ chat: inbound.remoteId, limit: 30 }),
            signal: AbortSignal.timeout(5_000),
          });
          if (res.ok) {
            const data = await res.json();
            history = (data.messages || []).map((m: any) => ({ role: m.role, content: m.content }));
          }
        } catch {}
      } else if (inbound.channelType === 'telegram') {
        // Telegram: load existing history from DB (read-only) + sandbox messages from RAM
        if (person) {
          try {
            const msgs = await this.prisma.personMessage.findMany({
              where: { personId: person.id },
              orderBy: { createdAt: 'desc' },
              take: 15,
              select: { role: true, content: true },
            });
            history = msgs.reverse();
          } catch {}
        }
        const sandboxStore = (global as any).__sandboxStore?.get(inbound.remoteId);
        if (sandboxStore?.length) {
          history = [...history, ...sandboxStore.map((m: any) => ({ role: m.role, content: m.content }))];
        }
      } else {
        // Fallback: ephemeral metadata
        history = meta.sandboxHistory || [];
      }
    } else if (person) {
      // Normal: load from person_messages DB
      try {
        const msgs = await this.prisma.personMessage.findMany({
          where: { personId: person.id },
          orderBy: { createdAt: 'desc' },
          take: 30,
          select: { role: true, content: true },
        });
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
    const memoryAccess = this.profileService.getMemoryAccess(profile);
    const memoryContext = await this.memory.getContextForPerson(inbound.text, person?.id, memoryAccess);

    // Group context
    const groupSection = inbound.groupContext
      ? `\n\n## Mensagens recentes do grupo\n${inbound.groupContext}`
      : '';

    // === BUILD DYNAMIC CORE ===
    const systemPrompt = this.buildDynamicPrompt({
      person, profile, inbound, memoryContext, groupSection,
      context, isPrivacyMode, isFirstMessage: history.length === 0,
    });

    try {
      // Filter tools by profile — NO PROFILE = NO TOOLS (security)
      const tools = this.profileService.getTools(profile);

      // Call LLM via bridge — model comes from person or global config
      const currentModel = await this.getCurrentModel(person?.id);
      let response = await bridgeCall<any>('/llm/chat', {
        model: currentModel,
        max_tokens: 1024,
        system: systemPrompt,
        tools: tools.length > 0 ? tools : undefined,
        messages,
      }, 60_000);

      // Handle tool use loop (multi-turn)
      response = await this.processToolLoop(response, messages, {
        model: currentModel, system: systemPrompt, tools, channel, target,
        allowedToolNames: profile?.allowedTools || [],
      });

      // Extract final text response
      const textBlocks = response.content.filter((b) => b.type === 'text');
      let finalText = textBlocks.map((b) => (b as any).text).join('\n') || 'Entendido.';

      // Save messages — sandbox uses session.metadata only, normal saves to DB
      const freshSession = await this.prisma.conversationSession.findUnique({ where: { id: session.id } });
      const freshMeta = (freshSession?.metadata as any) || {};

      if (isPrivacyMode) {
        if (inbound.channelType === 'telegram') {
          // Telegram: store in memory (Mini App syncs to client localStorage)
          if (!(global as any).__sandboxStore) (global as any).__sandboxStore = new Map();
          const store = (global as any).__sandboxStore.get(inbound.remoteId) || [];
          store.push({ role: 'user', content: inbound.text });
          store.push({ role: 'assistant', content: finalText });
          (global as any).__sandboxStore.set(inbound.remoteId, store.slice(-100));
        }
        // WhatsApp: zero storage (bridge has it)
        // All channels: update session timestamp only
        await this.prisma.conversationSession.update({
          where: { id: session.id },
          data: { lastActionAt: new Date(), metadata: freshMeta },
        });
      } else {
        // Normal: save to person_messages DB
        if (person) {
          await this.prisma.personMessage.createMany({
            data: [
              { personId: person.id, channel: inbound.channelType, role: 'user', content: inbound.text },
              { personId: person.id, channel: inbound.channelType, role: 'assistant', content: finalText, model: currentModel },
            ],
          });
          await this.prisma.person.update({
            where: { id: person.id },
            data: { context: { lastChannel: inbound.channelType, lastMessageAt: new Date().toISOString() } },
          });
        }
        await this.prisma.conversationSession.update({
          where: { id: session.id },
          data: { lastUserMessage: inbound.text, lastActionAt: new Date(), metadata: freshMeta },
        });

        // Auto-save memory only in normal mode (never in sandbox)
        if (person && profile) {
          this.autoSaveMemory(person, profile, inbound.text, finalText).catch(() => {});
        }
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

    // 4. Sandbox mode — minimal indicator, don't change behavior
    if (isPrivacyMode) {
      parts.push('(modo privado ativo)');
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

  private async processToolLoop(
    response: any, initialMessages: any[],
    opts: { model: string; system: string; tools: any[]; channel: string; target: string; allowedToolNames: string[] },
  ): Promise<any> {
    const toolMessages = [...initialMessages];
    let iterations = 0;

    while (response.stop_reason === 'tool_use' && iterations < 5) {
      iterations++;
      const assistantContent = response.content;
      toolMessages.push({ role: 'assistant', content: assistantContent });

      const toolResults: any[] = [];
      for (const block of assistantContent) {
        if (block.type !== 'tool_use') continue;

        this.logger.log(`Tool call: ${block.name}(${JSON.stringify(block.input).substring(0, 100)})`);

        if (opts.allowedToolNames.length > 0 && !opts.allowedToolNames.includes(block.name)) {
          this.logger.warn(`Blocked tool ${block.name}`);
          toolResults.push({ type: 'tool_result', tool_use_id: block.id, content: JSON.stringify({ error: `Ação "${block.name}" não disponível.` }), is_error: true });
          continue;
        }

        try {
          const result = await this.gateway.executeAction(block.name, opts.channel, opts.target, block.input);
          const r = result as any;

          // Handle special action: send Telegram Mini App button
          if (r?.action === 'send_webapp_button' && opts.channel === 'telegram') {
            try {
              const { TelegramService } = await import('./telegram.service');
              const telegram = new TelegramService();
              await telegram.sendWithWebApp(opts.target, r._summary || 'Ative o modo privado:', r.buttonText, r.webAppUrl);
              this.logger.log(`Sent Mini App button to ${opts.target}`);
            } catch (webAppErr) {
              this.logger.error(`Mini App button failed: ${webAppErr.message}`);
            }
          }

          const { formatToolResult } = await import('../gateway-format');
          const summary = r?._summary || '';
          const data = { ...r };
          delete data._summary; delete data.allowed; delete data.sessionState; delete data.action; delete data.buttonText; delete data.webAppUrl;
          const formatted = summary ? formatToolResult(summary, data) : JSON.stringify(result);
          toolResults.push({ type: 'tool_result', tool_use_id: block.id, content: formatted });
        } catch (err) {
          toolResults.push({ type: 'tool_result', tool_use_id: block.id, content: JSON.stringify({ error: err.message }), is_error: true });
        }
      }

      toolMessages.push({ role: 'user', content: toolResults });
      response = await bridgeCall<any>('/llm/chat', {
        model: opts.model,
        max_tokens: 1024,
        system: opts.system,
        tools: opts.tools.length > 0 ? opts.tools : undefined,
        messages: toolMessages,
      }, 60_000);
    }

    return response;
  }

  private async processSimulationMessage(inbound: InboundMessage, session: any, meta: any): Promise<string> {
    const sim = meta.simulationPerson;
    const simHistory: Array<{ role: string; content: string }> = meta.simulationHistory || [];

    // Load memories as the simulated person
    const memoryContext = await this.memory.getContextForPerson(
      inbound.text, sim.id, sim.memoryAccess,
    );

    // Build prompt as if the simulated person is talking
    const parts = [PATRICIA_SYSTEM_PROMPT];
    parts.push(`## Quem está falando\n${sim.name} (${sim.role})\n— ${sim.description || 'sem descrição'}`);
    if (sim.profilePrompt) parts.push(sim.profilePrompt);
    parts.push(`⚠️ MODO SIMULAÇÃO: O diretor está testando como você responderia para ${sim.name}. Responda naturalmente como se ${sim.name} estivesse falando com você.`);
    if (memoryContext) parts.push(memoryContext);

    // Filter tools by simulated profile
    const allowedTools: string[] = sim.allowedTools || [];
    const tools = allowedTools.length > 0
      ? PATRICIA_TOOLS.filter((t) => allowedTools.includes(t.name))
      : [];

    // Build messages from simulation history
    const messages: any[] = [];
    for (const msg of simHistory.slice(-MAX_HISTORY)) {
      messages.push({ role: msg.role === 'user' ? 'user' : 'assistant', content: msg.content });
    }
    messages.push({ role: 'user', content: inbound.text });

    // Call LLM with simulated person's model
    let response = await bridgeCall<any>('/llm/chat', {
      model: sim.model,
      max_tokens: 1024,
      system: parts.join('\n\n'),
      tools: tools.length > 0 ? tools : undefined,
      messages,
    }, 60_000);

    // Handle tool use loop (shared with normal processing)
    response = await this.processToolLoop(response, messages, {
      model: sim.model, system: parts.join('\n\n'), tools, channel: session.channel, target: session.target,
      allowedToolNames: allowedTools,
    });

    const finalText = response.content
      .filter((b: any) => b.type === 'text')
      .map((b: any) => b.text)
      .join('\n') || 'Entendido.';

    // Save to simulation history (NOT to real person_messages)
    simHistory.push({ role: 'user', content: inbound.text });
    simHistory.push({ role: 'assistant', content: finalText });

    await this.prisma.conversationSession.update({
      where: { id: session.id },
      data: { metadata: { ...meta, simulationHistory: simHistory.slice(-30) } },
    });

    return `[SIM:${sim.name}] ${finalText}`;
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
      const currentModel = await this.getCurrentModel();
      const response = await bridgeCall<any>('/llm/chat', {
        model: currentModel,
        max_tokens: 2048,
        system: systemPrompt,
        messages,
      }, 60_000);

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
