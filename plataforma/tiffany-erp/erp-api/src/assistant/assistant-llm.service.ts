import Anthropic from '@anthropic-ai/sdk';
import { Injectable, Logger, BadRequestException } from '@nestjs/common';
import { UserRole } from '@prisma/client';
import {
  getTenantContext,
  requireTenantId,
} from '../common/tenant-context/tenant-context';
import { PrismaService } from '../prisma/prisma.service';
import { AssistantConfigService } from './assistant-config.service';
import { AssistantMemoryService } from './assistant-memory.service';
import { AssistantProfileService } from './assistant-profile.service';
import { AssistantToolRunnerService } from './assistant-tool-runner.service';
import { ASSISTANT_TOOLS } from './assistant-tools';
import { DEFAULT_SOUL_PROMPT } from './prompts/soul';

const MAX_HISTORY = 20;
const MAX_TOOL_ITERATIONS = 5;
const MAX_TOKENS = 1500;

export interface ChatInput {
  conversationId?: string;
  text: string;
}

export interface ChatResult {
  conversationId: string;
  reply: string;
  model: string;
  toolCalls: Array<{ name: string; summary: string }>;
}

@Injectable()
export class AssistantLlmService {
  private readonly logger = new Logger(AssistantLlmService.name);

  constructor(
    private readonly prisma: PrismaService,
    private readonly config: AssistantConfigService,
    private readonly profiles: AssistantProfileService,
    private readonly memory: AssistantMemoryService,
    private readonly tools: AssistantToolRunnerService,
  ) {}

  async chat(input: ChatInput): Promise<ChatResult> {
    const tenantId = requireTenantId();
    const { userId, role } = getTenantContext();
    if (!userId || !role) throw new BadRequestException('Usuário não autenticado');

    const apiKey = await this.config.resolveApiKey();
    if (!apiKey) {
      throw new BadRequestException(
        'API key do assistente não configurada. Configure em Configurações > Assistente IA.',
      );
    }

    // 1. Resolve conversa
    let conversation = input.conversationId
      ? await this.prisma.assistantConversation.findFirst({
          where: { id: input.conversationId, tenantId, userId },
        })
      : null;
    if (!conversation) {
      conversation = await this.prisma.assistantConversation.create({
        data: {
          tenantId,
          userId,
          title: input.text.slice(0, 60),
          channel: 'web',
        },
      });
    }

    // 2. Resolve perfil do role
    const profile = await this.profiles.findByRole(role as UserRole);
    if (!profile.active) {
      throw new BadRequestException(`Assistente desativado para o papel ${role}`);
    }

    // 3. Histórico
    const history = await this.prisma.assistantMessage.findMany({
      where: { conversationId: conversation.id },
      orderBy: { createdAt: 'desc' },
      take: MAX_HISTORY,
      select: { role: true, content: true, toolCalls: true, toolResult: true },
    });
    history.reverse();

    // 4. Memória contextual (top-K relevantes + core)
    const memoryContext = await this.memory.buildContext(
      input.text,
      profile.memoryAccess,
      8,
    );

    // 5. Soul prompt (config ou default)
    const soul = (await this.config.resolveSoulPrompt()) ?? DEFAULT_SOUL_PROMPT;

    // 6. Identidade do usuário
    const userMeta = await this.prisma.tenantUser.findFirst({
      where: { id: userId, tenantId },
      include: { tenant: { select: { name: true } } },
    });
    const whoSection = userMeta
      ? `## Quem está falando\n${userMeta.name} — ${userMeta.role} em ${userMeta.tenant.name}`
      : '';

    // 7. Build system prompt
    const systemPrompt = [
      soul,
      whoSection,
      profile.systemPrompt,
      memoryContext,
    ].filter(Boolean).join('\n\n');

    // 8. Tools filtradas
    const allowed = profile.allowedTools;
    const tools = ASSISTANT_TOOLS
      .filter((t) => allowed.includes(t.name))
      .map((t) => ({
        name: t.name,
        description: t.description,
        input_schema: t.input_schema,
      })) as Anthropic.Messages.Tool[];

    // 9. Persist user message
    const userMsg = await this.prisma.assistantMessage.create({
      data: { conversationId: conversation.id, role: 'user', content: input.text },
    });
    // ignore embedding here for speed; could backfill async

    // 10. Build messages array
    const messages: Anthropic.Messages.MessageParam[] = [];
    for (const h of history) {
      if (h.role === 'tool') continue; // tool results não viram message direta
      messages.push({
        role: h.role === 'assistant' ? 'assistant' : 'user',
        content: h.content,
      });
    }
    messages.push({ role: 'user', content: input.text });

    // 11. Chamada inicial
    const client = new Anthropic({ apiKey });
    const model = await this.config.resolveModel();
    let response = await client.messages.create({
      model,
      max_tokens: MAX_TOKENS,
      system: systemPrompt,
      messages,
      ...(tools.length > 0 ? { tools } : {}),
    });

    const toolCallsLog: Array<{ name: string; summary: string }> = [];

    // 12. Tool use loop
    let iter = 0;
    while (response.stop_reason === 'tool_use' && iter < MAX_TOOL_ITERATIONS) {
      iter++;
      const assistantBlocks = response.content;
      messages.push({ role: 'assistant', content: assistantBlocks });

      const toolResults: Anthropic.Messages.ToolResultBlockParam[] = [];
      for (const block of assistantBlocks) {
        if (block.type !== 'tool_use') continue;
        if (!allowed.includes(block.name)) {
          toolResults.push({
            type: 'tool_result',
            tool_use_id: block.id,
            content: JSON.stringify({ error: `Tool ${block.name} não permitida pra esse papel.` }),
            is_error: true,
          });
          continue;
        }
        const result = await this.tools.run(block.name, block.input as Record<string, unknown>);
        const summary = result.summary ?? (result.ok ? 'ok' : (result.error ?? 'erro'));
        toolCallsLog.push({ name: block.name, summary });
        await this.prisma.assistantMessage.create({
          data: {
            conversationId: conversation.id,
            role: 'tool',
            content: summary,
            toolCalls: [{ id: block.id, name: block.name, input: block.input }] as never,
            toolResult: result as never,
          },
        });
        toolResults.push({
          type: 'tool_result',
          tool_use_id: block.id,
          content: JSON.stringify(result.data ?? result),
          is_error: !result.ok,
        });
      }

      messages.push({ role: 'user', content: toolResults });
      response = await client.messages.create({
        model,
        max_tokens: MAX_TOKENS,
        system: systemPrompt,
        messages,
        ...(tools.length > 0 ? { tools } : {}),
      });
    }

    // 13. Texto final
    const finalText = response.content
      .filter((b) => b.type === 'text')
      .map((b) => (b as Anthropic.Messages.TextBlock).text)
      .join('\n') || 'Entendido.';

    // 14. Persist assistant message
    await this.prisma.assistantMessage.create({
      data: {
        conversationId: conversation.id,
        role: 'assistant',
        content: finalText,
        model,
      },
    });

    // 15. Touch conversation
    await this.prisma.assistantConversation.update({
      where: { id: conversation.id },
      data: { updatedAt: new Date() },
    });

    return {
      conversationId: conversation.id,
      reply: finalText,
      model,
      toolCalls: toolCallsLog,
    };
  }

  async listConversations() {
    const tenantId = requireTenantId();
    const { userId } = getTenantContext();
    return this.prisma.assistantConversation.findMany({
      where: { tenantId, userId: userId ?? undefined },
      orderBy: { updatedAt: 'desc' },
      take: 50,
      select: { id: true, title: true, createdAt: true, updatedAt: true },
    });
  }

  async listMessages(conversationId: string) {
    const tenantId = requireTenantId();
    const { userId } = getTenantContext();
    const conv = await this.prisma.assistantConversation.findFirst({
      where: { id: conversationId, tenantId, userId: userId ?? undefined },
    });
    if (!conv) throw new BadRequestException('Conversa não encontrada');
    return this.prisma.assistantMessage.findMany({
      where: { conversationId },
      orderBy: { createdAt: 'asc' },
      select: { id: true, role: true, content: true, model: true, createdAt: true, toolCalls: true, toolResult: true },
    });
  }

  async deleteConversation(conversationId: string) {
    const tenantId = requireTenantId();
    const { userId } = getTenantContext();
    const conv = await this.prisma.assistantConversation.findFirst({
      where: { id: conversationId, tenantId, userId: userId ?? undefined },
    });
    if (!conv) throw new BadRequestException('Conversa não encontrada');
    await this.prisma.assistantConversation.delete({ where: { id: conversationId } });
    return { ok: true };
  }
}
