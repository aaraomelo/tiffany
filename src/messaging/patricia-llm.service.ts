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

    // Load conversation history from session metadata
    const session = await this.gateway.getOrCreateSession(channel, target);
    const history: Array<{ role: string; content: string }> = (session.metadata as any)?.history || [];

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

    // Load memories: core (always) + relevant long_term/short_term (by message)
    const memoryContext = await this.memory.getContext(inbound.text);

    // Group context: recent messages from the group
    const groupSection = inbound.groupContext
      ? `\n\n## Mensagens recentes do grupo\n${inbound.groupContext}`
      : '';

    // Build system prompt with dynamic context + memories + group context
    const systemPrompt = `${PATRICIA_SYSTEM_PROMPT}\n\n${memoryContext}${groupSection}\n\n## Contexto atual da conversa\n${context}\n\nRemetente: ${inbound.displayName} (${inbound.senderPhone})`;

    try {
      // Call Claude with tools
      let response = await this.client.messages.create({
        model: MODEL,
        max_tokens: 1024,
        system: systemPrompt,
        tools: PATRICIA_TOOLS as any,
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

      // Save history
      history.push({ role: 'user', content: inbound.text });
      history.push({ role: 'assistant', content: finalText });
      const trimmedHistory = history.slice(-MAX_HISTORY * 2);

      await this.prisma.conversationSession.update({
        where: { id: session.id },
        data: {
          metadata: { ...(session.metadata as any || {}), history: trimmedHistory },
          lastUserMessage: inbound.text,
          lastActionAt: new Date(),
        },
      });

      return finalText;
    } catch (err) {
      this.logger.error(`LLM error: ${err.message}`);
      return 'Desculpe, tive um problema ao processar sua mensagem. Tente novamente em alguns instantes.';
    }
  }
}
