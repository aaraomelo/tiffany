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

    // DM redact: privacidade. Group mantém texto (multi-pessoa, contexto compartilhado)
    if (isGroup) {
      this.logger.log(`Inbound group: ${remoteJid} → "${text.substring(0, 60)}"`);
    } else {
      this.logger.log(`Inbound DM: ${remoteJid} (${text.length} chars)`);
    }

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
    if (!message) return res.status(200).json({ ok: true });

    const hasText = !!message.text;
    const hasVoice = !!message.voice;
    const hasAudio = !!message.audio;
    if (!hasText && !hasVoice && !hasAudio) {
      return res.status(200).json({ ok: true });
    }

    const chatId = String(message.chat.id);
    const isGroup = message.chat.type !== 'private';
    let text = message.text || '';
    const displayName = [message.from?.first_name, message.from?.last_name].filter(Boolean).join(' ') || 'Unknown';

    // Áudio: baixa e transcreve antes de continuar (devolve 200 imediato pro Telegram)
    if (!hasText && (hasVoice || hasAudio)) {
      res.status(200).json({ ok: true });
      this.processVoiceMessage({ message, chatId, isGroup, displayName }).catch((e) =>
        this.logger.error(`voice processing failed: ${e.message}`),
      );
      return;
    }

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

    if (isGroup) {
      this.logger.log(`Telegram group: ${chatId} → "${text.substring(0, 60)}"`);
    } else {
      this.logger.log(`Telegram DM: ${chatId} (${text.length} chars)`);
    }

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
  private async processVoiceMessage(opts: {
    message: any; chatId: string; isGroup: boolean; displayName: string;
  }): Promise<void> {
    const { message, chatId, isGroup, displayName } = opts;
    const fileId = message.voice?.file_id || message.audio?.file_id;
    const duration = message.voice?.duration || message.audio?.duration;
    const TG_TOKEN = process.env.TELEGRAM_BOT_TOKEN || '';
    if (!TG_TOKEN) {
      this.logger.error('TELEGRAM_BOT_TOKEN missing');
      return;
    }
    // 1. resolve file_path via getFile
    const getFileRes = await fetch(`https://api.telegram.org/bot${TG_TOKEN}/getFile?file_id=${fileId}`);
    const getFileData = await getFileRes.json();
    const filePath = getFileData?.result?.file_path;
    if (!filePath) throw new Error('no file_path from Telegram getFile');
    const downloadUrl = `https://api.telegram.org/file/bot${TG_TOKEN}/${filePath}`;

    // 2. envia pra bridge transcrever
    const BRIDGE_URL = process.env.BRIDGE_URL || 'http://host.docker.internal:9090';
    const BRIDGE_SECRET = process.env.BRIDGE_SECRET || 'wk_infer_patria_2026';
    const tRes = await fetch(`${BRIDGE_URL}/audio/transcribe`, {
      method: 'POST',
      headers: { 'Content-Type': 'application/json', 'X-Bridge-Key': BRIDGE_SECRET },
      body: JSON.stringify({
        url: downloadUrl,
        filename: `tg_${fileId}.ogg`,
        mime: 'audio/ogg',
      }),
      signal: AbortSignal.timeout(60_000),
    });
    if (!tRes.ok) {
      const t = await tRes.text().catch(() => '');
      throw new Error(`bridge transcribe failed: ${tRes.status} ${t.slice(0, 200)}`);
    }
    const tData = await tRes.json();
    const transcribed = (tData.text || '').trim();
    if (!transcribed) {
      this.logger.warn(`empty transcription for ${fileId}`);
      return;
    }

    // Marca origem do áudio pro contexto: Patrícia sabe que veio falado
    const text = `[áudio ${duration || '?'}s, transcrito] ${transcribed}`;

    await this.messaging.logInbound('telegram', chatId, `${displayName}: ${text}`, displayName);

    if (isGroup) {
      // Grupos: precisaria mention check; áudios quase sempre são DM, mas por segurança ignora se grupo
      this.logger.log(`Telegram group voice ignored (no mention check yet): ${chatId}`);
      return;
    }

    this.logger.log(`Telegram voice DM: ${chatId} (audio ${duration || '?'}s, ${transcribed.length} chars)`);

    const inbound: InboundMessage = {
      channelType: 'telegram',
      remoteId: chatId,
      senderPhone: '',
      displayName,
      text,
      isGroup,
      groupContext: '',
      timestamp: new Date(message.date * 1000),
    };
    this.debounceProcess('telegram', chatId, text, inbound);
  }

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
