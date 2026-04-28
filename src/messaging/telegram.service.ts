import { Injectable, Logger } from '@nestjs/common';
import { ChannelSender } from './interfaces/channel-sender.interface';

const TELEGRAM_TOKEN = process.env.TELEGRAM_BOT_TOKEN || '';
const TELEGRAM_API = 'https://api.telegram.org';

@Injectable()
export class TelegramService implements ChannelSender {
  private readonly logger = new Logger('Telegram');

  async send(chatId: string, text: string): Promise<{ messageId: string }> {
    if (!TELEGRAM_TOKEN) throw new Error('TELEGRAM_BOT_TOKEN not configured');

    // Telegram limit is 4096 chars — split long messages
    const MAX_LEN = 4000;
    if (text.length > MAX_LEN) {
      const parts: string[] = [];
      let remaining = text;
      while (remaining.length > 0) {
        if (remaining.length <= MAX_LEN) {
          parts.push(remaining);
          break;
        }
        // Split at last newline before limit
        let splitAt = remaining.lastIndexOf('\n', MAX_LEN);
        if (splitAt < MAX_LEN / 2) splitAt = MAX_LEN;
        parts.push(remaining.substring(0, splitAt));
        remaining = remaining.substring(splitAt).trimStart();
      }
      let lastId = '';
      for (const part of parts) {
        const result = await this.sendSingle(chatId, part);
        lastId = result.messageId;
      }
      return { messageId: lastId };
    }

    return this.sendSingle(chatId, text);
  }

  private async sendSingle(chatId: string, text: string): Promise<{ messageId: string }> {
    try {
      const res = await fetch(`${TELEGRAM_API}/bot${TELEGRAM_TOKEN}/sendMessage`, {
        method: 'POST',
        headers: { 'Content-Type': 'application/json' },
        body: JSON.stringify({ chat_id: chatId, text }),
        signal: AbortSignal.timeout(15_000),
      });

      if (!res.ok) {
        const body = await res.text();
        this.logger.error(`Send failed (${res.status}): ${body}`);
        throw new Error(`Telegram API ${res.status}: ${body}`);
      }

      const data = await res.json();
      const messageId = String(data?.result?.message_id || 'unknown');
      this.logger.log(`Sent to ${chatId} (${messageId})`);
      return { messageId };
    } catch (err) {
      this.logger.error(`Send error to ${chatId}: ${err.message}`);
      throw err;
    }
  }

  /** Envia áudio como voice note. Tenta URL primeiro; fallback multipart. */
  async sendVoice(chatId: string, voiceUrl: string, caption?: string): Promise<{ messageId: string }> {
    if (!TELEGRAM_TOKEN) throw new Error('TELEGRAM_BOT_TOKEN not configured');
    // Tenta URL
    const tryUrl = await fetch(`${TELEGRAM_API}/bot${TELEGRAM_TOKEN}/sendVoice`, {
      method: 'POST',
      headers: { 'Content-Type': 'application/json' },
      body: JSON.stringify({ chat_id: chatId, voice: voiceUrl, caption: caption || undefined }),
      signal: AbortSignal.timeout(30_000),
    }).catch((e) => ({ ok: false, status: 0, json: async () => ({ error: e.message }) } as any));
    const urlData = await (tryUrl as any).json().catch(() => ({}));
    if ((tryUrl as any).ok && urlData?.ok) {
      this.logger.log(`sendVoice ok via URL → ${chatId} (${urlData.result?.message_id})`);
      return { messageId: String(urlData.result?.message_id || 'unknown') };
    }
    this.logger.warn(`sendVoice URL falhou (${(tryUrl as any).status}): ${urlData?.description || urlData?.error || ''} — multipart fallback`);

    // Fallback: download bytes + upload multipart
    const fileRes = await fetch(voiceUrl, { signal: AbortSignal.timeout(20_000) });
    if (!fileRes.ok) throw new Error(`download voice ${fileRes.status}`);
    const buf = Buffer.from(await fileRes.arrayBuffer());
    const ext = (voiceUrl.match(/\.([a-z]{3,4})(\?|$)/i) || ['', 'ogg'])[1].toLowerCase();
    const mime = ext === 'mp3' ? 'audio/mpeg' : 'audio/ogg';
    const form = new FormData();
    form.append('chat_id', chatId);
    if (caption) form.append('caption', caption);
    form.append('voice', new Blob([buf], { type: mime }), `voice.${ext}`);
    const upRes = await fetch(`${TELEGRAM_API}/bot${TELEGRAM_TOKEN}/sendVoice`, {
      method: 'POST', body: form, signal: AbortSignal.timeout(60_000),
    });
    if (!upRes.ok) {
      const t = await upRes.text();
      throw new Error(`sendVoice multipart ${upRes.status}: ${t.slice(0, 200)}`);
    }
    const upData = await upRes.json();
    this.logger.log(`sendVoice ok via multipart → ${chatId} (${upData.result?.message_id})`);
    return { messageId: String(upData?.result?.message_id || 'unknown') };
  }

  async sendMedia(chatId: string, url: string, caption: string): Promise<{ messageId: string }> {
    if (!TELEGRAM_TOKEN) throw new Error('TELEGRAM_BOT_TOKEN not configured');
    // Tenta primeiro via URL (rápido, Telegram baixa). Se Telegram rejeitar
    // ('wrong type of the web page content' acontece com algumas URLs), faz
    // upload via multipart com os bytes baixados.
    const tryUrl = await fetch(`${TELEGRAM_API}/bot${TELEGRAM_TOKEN}/sendPhoto`, {
      method: 'POST',
      headers: { 'Content-Type': 'application/json' },
      body: JSON.stringify({ chat_id: chatId, photo: url, caption }),
      signal: AbortSignal.timeout(20_000),
    });
    const urlData = await tryUrl.json().catch(() => ({}));
    if (tryUrl.ok && urlData?.ok) {
      this.logger.log(`sendMedia ok via URL → ${chatId} (${urlData.result?.message_id})`);
      return { messageId: String(urlData.result?.message_id || 'unknown') };
    }
    this.logger.warn(`sendMedia URL falhou (${tryUrl.status}): ${urlData?.description || ''} — tentando multipart upload`);

    // Fallback: baixa bytes e faz upload
    const fileRes = await fetch(url, { signal: AbortSignal.timeout(20_000) });
    if (!fileRes.ok) throw new Error(`download ${fileRes.status}`);
    const buf = Buffer.from(await fileRes.arrayBuffer());
    const ext = (url.match(/\.([a-z]{3,4})(\?|$)/i) || ['', 'png'])[1].toLowerCase();
    const mime = ext === 'jpg' || ext === 'jpeg' ? 'image/jpeg' : ext === 'webp' ? 'image/webp' : 'image/png';
    const form = new FormData();
    form.append('chat_id', chatId);
    if (caption) form.append('caption', caption);
    form.append('photo', new Blob([buf], { type: mime }), `photo.${ext}`);
    const upRes = await fetch(`${TELEGRAM_API}/bot${TELEGRAM_TOKEN}/sendPhoto`, {
      method: 'POST',
      body: form,
      signal: AbortSignal.timeout(60_000),
    });
    if (!upRes.ok) {
      const t = await upRes.text();
      throw new Error(`Telegram sendPhoto multipart ${upRes.status}: ${t.slice(0, 200)}`);
    }
    const upData = await upRes.json();
    this.logger.log(`sendMedia ok via multipart → ${chatId} (${upData.result?.message_id})`);
    return { messageId: String(upData?.result?.message_id || 'unknown') };
  }

  async sendWithWebApp(chatId: string, text: string, buttonText: string, webAppUrl: string): Promise<{ messageId: string }> {
    if (!TELEGRAM_TOKEN) throw new Error('TELEGRAM_BOT_TOKEN not configured');
    try {
      const res = await fetch(`${TELEGRAM_API}/bot${TELEGRAM_TOKEN}/sendMessage`, {
        method: 'POST',
        headers: { 'Content-Type': 'application/json' },
        body: JSON.stringify({
          chat_id: chatId,
          text,
          reply_markup: {
            inline_keyboard: [[{ text: buttonText, web_app: { url: webAppUrl } }]],
          },
        }),
        signal: AbortSignal.timeout(15_000),
      });
      const data = await res.json();
      return { messageId: String(data?.result?.message_id || 'unknown') };
    } catch (err) {
      this.logger.error(`SendWithWebApp error: ${err.message}`);
      throw err;
    }
  }

  /** Envia 'typing' indicator. Telegram mostra ~5s. Pra processamentos longos
   *  usar startTyping() que repete em background até stop().
   */
  async sendChatAction(chatId: string, action: 'typing' | 'upload_photo' = 'typing'): Promise<void> {
    if (!TELEGRAM_TOKEN) return;
    try {
      await fetch(`${TELEGRAM_API}/bot${TELEGRAM_TOKEN}/sendChatAction`, {
        method: 'POST',
        headers: { 'Content-Type': 'application/json' },
        body: JSON.stringify({ chat_id: chatId, action }),
        signal: AbortSignal.timeout(5_000),
      });
    } catch (err: any) {
      // não bloqueia fluxo principal
      this.logger.debug(`sendChatAction failed: ${err.message}`);
    }
  }

  /** Repete 'typing' a cada 4s até stop. Útil quando LLM demora >5s.
   *  Retorna função stop().
   */
  startTyping(chatId: string): () => void {
    let active = true;
    const tick = async () => {
      if (!active) return;
      await this.sendChatAction(chatId, 'typing');
      if (active) setTimeout(tick, 4000);
    };
    tick(); // dispara imediatamente
    return () => { active = false; };
  }

  async healthCheck(): Promise<boolean> {
    if (!TELEGRAM_TOKEN) return false;
    try {
      const res = await fetch(`${TELEGRAM_API}/bot${TELEGRAM_TOKEN}/getMe`, {
        signal: AbortSignal.timeout(5_000),
      });
      return res.ok;
    } catch {
      return false;
    }
  }
}
