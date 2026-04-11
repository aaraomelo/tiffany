import { Controller, Post, Req, Res } from '@nestjs/common';
import { Request, Response } from 'express';
import { MessagingService } from './messaging.service';
import { PatriciaLlmService } from './patricia-llm.service';
import { InboundMessage } from './dto/inbound-message.dto';
import { Logger } from '@nestjs/common';

const BOT_JID = process.env.WHATSAPP_BOT_JID || '559584227029@s.whatsapp.net';
const TELEGRAM_BOT_USERNAME = process.env.TELEGRAM_BOT_USERNAME || 'PatriaTechnologyBot';

@Controller('api/webhooks')
export class MessagingWebhookController {
  private readonly logger = new Logger('MessagingWebhook');

  constructor(
    private messaging: MessagingService,
    private llm: PatriciaLlmService,
  ) {}

  @Post('evolution')
  async handleEvolution(@Req() req: Request, @Res() res: Response) {
    const payload = req.body;

    // Only handle inbound text messages
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

    // Group: only process if bot was mentioned
    if (isGroup) {
      const mentioned = data.message?.extendedTextMessage?.contextInfo?.mentionedJid || [];
      const botMentioned = mentioned.some((jid: string) =>
        jid === BOT_JID || jid.includes('559584227029'),
      );
      const textMention = text.toLowerCase().includes('patrícia') || text.toLowerCase().includes('patricia');
      if (!botMentioned && !textMention) {
        return res.status(200).json({ ok: true, skipped: true, reason: 'not_mentioned' });
      }
    }

    this.logger.log(`Inbound ${isGroup ? 'group' : 'DM'}: ${remoteJid} → "${text.substring(0, 60)}"`);

    const inbound: InboundMessage = {
      channelType: 'whatsapp',
      remoteId: remoteJid,
      senderPhone: `+${senderPhone}`,
      displayName: data.pushName || senderPhone,
      text,
      isGroup,
      timestamp: new Date(data.messageTimestamp * 1000),
    };

    // Log inbound
    await this.messaging.logInbound('whatsapp', remoteJid, text, data.pushName, key.id);

    // Process async — respond 200 immediately
    res.status(200).json({ ok: true });

    // Process message and respond
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

    // Group: only process if bot was mentioned
    if (isGroup) {
      const entities = message.entities || [];
      const botMentioned = entities.some(
        (e: any) => e.type === 'mention' && text.substring(e.offset, e.offset + e.length).includes(TELEGRAM_BOT_USERNAME),
      );
      const textMention = text.toLowerCase().includes('patrícia') || text.toLowerCase().includes('patricia');
      if (!botMentioned && !textMention) {
        return res.status(200).json({ ok: true });
      }
    }

    this.logger.log(`Telegram ${isGroup ? 'group' : 'DM'}: ${chatId} → "${text.substring(0, 60)}"`);

    const inbound: InboundMessage = {
      channelType: 'telegram',
      remoteId: chatId,
      senderPhone: '',
      displayName: [message.from?.first_name, message.from?.last_name].filter(Boolean).join(' ') || 'Unknown',
      text,
      isGroup,
      timestamp: new Date(message.date * 1000),
    };

    await this.messaging.logInbound('telegram', chatId, text, inbound.displayName);

    res.status(200).json({ ok: true });

    try {
      const response = await this.llm.processMessage(inbound);
      await this.messaging.send('telegram', chatId, response);
    } catch (err) {
      this.logger.error(`Telegram processing error: ${err.message}`);
    }
  }
}
