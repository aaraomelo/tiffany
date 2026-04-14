import { Controller, Post, Req, Res, Logger } from '@nestjs/common';
import { Request, Response } from 'express';
import { MessagingService } from './messaging.service';
import { PatriciaLlmService } from './patricia-llm.service';
import { PrismaService } from '../prisma.service';
import { InboundMessage } from './dto/inbound-message.dto';

const BOT_JID = process.env.WHATSAPP_BOT_JID || '559584227029@s.whatsapp.net';
const BOT_LID = process.env.WHATSAPP_BOT_LID || '205398674039020@lid';
const TELEGRAM_BOT_USERNAME = process.env.TELEGRAM_BOT_USERNAME || 'PatriaTechnologyBot';
const GROUP_CONTEXT_LIMIT = parseInt(process.env.GROUP_CONTEXT_LIMIT || '20');
const DEBOUNCE_MS = parseInt(process.env.DEBOUNCE_MS || '3000');

@Controller('api/webhooks')
export class MessagingWebhookController {
  private readonly logger = new Logger('MessagingWebhook');
  private debounceTimers = new Map<string, NodeJS.Timeout>();
  private debounceMessages = new Map<string, { texts: string[]; inbound: any }>();

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
      ? (key.participant || '').replace(/@.*/, '')
      : remoteJid.replace(/@.*/, '');
    const displayName = data.pushName || senderPhone;

    // Always log inbound (group or DM)
    await this.messaging.logInbound('whatsapp', remoteJid, `${displayName}: ${text}`, displayName, key.id);

    // Group: check if bot was mentioned
    if (isGroup) {
      const mentioned = data.message?.extendedTextMessage?.contextInfo?.mentionedJid || [];
      const botMentioned = mentioned.some((jid: string) =>
        jid === BOT_JID || jid === BOT_LID || jid.includes('559584227029') || jid.includes('205398674039020'),
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

    // Debounce: accumulate messages from same sender within DEBOUNCE_MS
    this.debounceProcess('whatsapp', remoteJid, text, inbound);
  }

  private debounceProcess(channel: string, remoteId: string, text: string, inbound: InboundMessage) {
    const key = `${channel}:${remoteId}`;
    const existing = this.debounceMessages.get(key);
    if (existing) {
      existing.texts.push(text);
      existing.inbound = inbound;
    } else {
      this.debounceMessages.set(key, { texts: [text], inbound });
    }

    const existingTimer = this.debounceTimers.get(key);
    if (existingTimer) clearTimeout(existingTimer);

    this.debounceTimers.set(key, setTimeout(async () => {
      this.debounceTimers.delete(key);
      const data = this.debounceMessages.get(key);
      this.debounceMessages.delete(key);
      if (!data) return;

      const combinedText = data.texts.join('\n');
      const combinedInbound = { ...data.inbound, text: combinedText };
      this.logger.log(`Debounced ${data.texts.length} msg(s) from ${remoteId}`);

      // Show "typing..." indicator
      this.sendTyping(channel, remoteId).catch(() => {});

      try {
        const response = await this.llm.processMessage(combinedInbound);
        if (response) await this.messaging.send(channel, remoteId, response);
      } catch (err) {
        this.logger.error(`Processing error: ${err.message}`);
      }
    }, DEBOUNCE_MS));
  }

  private async sendTyping(channel: string, remoteId: string) {
    if (channel === 'whatsapp') {
      const WA_URL = process.env.WA_BRIDGE_URL || 'http://127.0.0.1:8089';
      const WA_KEY = process.env.WA_BRIDGE_KEY || 'wa_bridge_2026';
      await fetch(`${WA_URL}/typing`, {
        method: 'POST',
        headers: { 'Content-Type': 'application/json', 'x-api-key': WA_KEY },
        body: JSON.stringify({ to: remoteId }),
        signal: AbortSignal.timeout(5_000),
      }).catch(() => {});
    } else if (channel === 'telegram') {
      const TG_TOKEN = process.env.TELEGRAM_BOT_TOKEN;
      if (TG_TOKEN) {
        await fetch(`https://api.telegram.org/bot${TG_TOKEN}/sendChatAction`, {
          method: 'POST',
          headers: { 'Content-Type': 'application/json' },
          body: JSON.stringify({ chat_id: remoteId, action: 'typing' }),
          signal: AbortSignal.timeout(5_000),
        }).catch(() => {});
      }
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

    this.debounceProcess('telegram', chatId, text, inbound);
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
