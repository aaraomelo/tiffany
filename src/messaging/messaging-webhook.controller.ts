import { Controller, Post, Req, Res, Logger } from '@nestjs/common';
import { Request, Response } from 'express';
import { MessagingService } from './messaging.service';
import { PatriciaLlmService } from './patricia-llm.service';
import { PrismaService } from '../prisma.service';
import { InboundMessage } from './dto/inbound-message.dto';

const BOT_JID = process.env.WHATSAPP_BOT_JID || '559584227029@s.whatsapp.net';
const TELEGRAM_BOT_USERNAME = process.env.TELEGRAM_BOT_USERNAME || 'PatriaTechnologyBot';
const GROUP_CONTEXT_LIMIT = parseInt(process.env.GROUP_CONTEXT_LIMIT || '20');

@Controller('api/webhooks')
export class MessagingWebhookController {
  private readonly logger = new Logger('MessagingWebhook');

  constructor(
    private messaging: MessagingService,
    private llm: PatriciaLlmService,
    private prisma: PrismaService,
  ) {}

  @Post('evolution')
  async handleEvolution(@Req() req: Request, @Res() res: Response) {
    const payload = req.body;

    if (payload?.event !== 'messages.upsert') {
      return res.status(200).json({ ok: true, skipped: true });
    }

    const data = payload.data;
    const key = data?.key;
    if (!key || key.fromMe) {
      return res.status(200).json({ ok: true, skipped: true });
    }

    const remoteJid = key.remoteJid;
    const text = data.message?.conversation
      || data.message?.extendedTextMessage?.text
      || '';

    if (!text.trim()) {
      return res.status(200).json({ ok: true, skipped: true });
    }

    const isGroup = remoteJid.endsWith('@g.us');
    const senderPhone = isGroup
      ? (key.participant || '').replace('@s.whatsapp.net', '')
      : remoteJid.replace('@s.whatsapp.net', '');
    const displayName = data.pushName || senderPhone;

    // Always log inbound (group or DM)
    await this.messaging.logInbound('whatsapp', remoteJid, `${displayName}: ${text}`, displayName, key.id);

    // Group: check if bot was mentioned
    if (isGroup) {
      const mentioned = data.message?.extendedTextMessage?.contextInfo?.mentionedJid || [];
      const botMentioned = mentioned.some((jid: string) =>
        jid === BOT_JID || jid.includes('559584227029'),
      );
      const textMention = text.toLowerCase().includes('patrícia') || text.toLowerCase().includes('patricia');
      if (!botMentioned && !textMention) {
        // Not mentioned — just stored, no LLM processing
        return res.status(200).json({ ok: true, stored: true });
      }
    }

    this.logger.log(`Inbound ${isGroup ? 'group' : 'DM'}: ${remoteJid} → "${text.substring(0, 60)}"`);

    // Load group context if group message
    let groupContext = '';
    if (isGroup) {
      groupContext = await this.getGroupContext('whatsapp', remoteJid, GROUP_CONTEXT_LIMIT);
    }

    const inbound: InboundMessage = {
      channelType: 'whatsapp',
      remoteId: remoteJid,
      senderPhone: `+${senderPhone}`,
      displayName,
      text,
      isGroup,
      groupContext,
      timestamp: new Date(data.messageTimestamp * 1000),
    };

    res.status(200).json({ ok: true });

    try {
      const response = await this.llm.processMessage(inbound);
      await this.messaging.send('whatsapp', remoteJid, response);
    } catch (err) {
      this.logger.error(`Processing error: ${err.message}`);
    }
  }

  @Post('telegram-inbound')
  async handleTelegram(@Req() req: Request, @Res() res: Response) {
    const update = req.body;
    const message = update?.message;
    if (!message?.text) {
      return res.status(200).json({ ok: true });
    }

    const chatId = String(message.chat.id);
    const isGroup = message.chat.type !== 'private';
    const text = message.text;
    const displayName = [message.from?.first_name, message.from?.last_name].filter(Boolean).join(' ') || 'Unknown';

    // Always log inbound
    await this.messaging.logInbound('telegram', chatId, `${displayName}: ${text}`, displayName);

    // Group: check if bot was mentioned
    if (isGroup) {
      const entities = message.entities || [];
      const botMentioned = entities.some(
        (e: any) => e.type === 'mention' && text.substring(e.offset, e.offset + e.length).includes(TELEGRAM_BOT_USERNAME),
      );
      const textMention = text.toLowerCase().includes('patrícia') || text.toLowerCase().includes('patricia');
      if (!botMentioned && !textMention) {
        return res.status(200).json({ ok: true, stored: true });
      }
    }

    this.logger.log(`Telegram ${isGroup ? 'group' : 'DM'}: ${chatId} → "${text.substring(0, 60)}"`);

    let groupContext = '';
    if (isGroup) {
      groupContext = await this.getGroupContext('telegram', chatId, GROUP_CONTEXT_LIMIT);
    }

    const inbound: InboundMessage = {
      channelType: 'telegram',
      remoteId: chatId,
      senderPhone: '',
      displayName,
      text,
      isGroup,
      groupContext,
      timestamp: new Date(message.date * 1000),
    };

    res.status(200).json({ ok: true });

    try {
      const response = await this.llm.processMessage(inbound);
      await this.messaging.send('telegram', chatId, response);
    } catch (err) {
      this.logger.error(`Telegram processing error: ${err.message}`);
    }
  }

  // Load recent group messages as context
  private async getGroupContext(channel: string, remoteId: string, limit: number): Promise<string> {
    try {
      const contact = await this.prisma.messagingContact.findFirst({
        where: { channelType: channel as any, remoteId },
      });
      if (!contact) return '';

      const messages = await this.prisma.messageLog.findMany({
        where: { contactId: contact.id, direction: 'inbound' },
        orderBy: { createdAt: 'desc' },
        take: limit,
        select: { content: true, createdAt: true },
      });

      if (messages.length === 0) return '';

      // Reverse to chronological order and format
      const lines = messages.reverse().map((m) => m.content).join('\n');
      return lines;
    } catch {
      return '';
    }
  }
}
