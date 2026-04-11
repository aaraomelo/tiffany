import { Injectable, Logger } from '@nestjs/common';
import { ChannelSender } from './interfaces/channel-sender.interface';

const TELEGRAM_TOKEN = process.env.TELEGRAM_BOT_TOKEN || '';
const TELEGRAM_API = 'https://api.telegram.org';

@Injectable()
export class TelegramService implements ChannelSender {
  private readonly logger = new Logger('Telegram');

  async send(chatId: string, text: string): Promise<{ messageId: string }> {
    if (!TELEGRAM_TOKEN) throw new Error('TELEGRAM_BOT_TOKEN not configured');
    try {
      const res = await fetch(`${TELEGRAM_API}/bot${TELEGRAM_TOKEN}/sendMessage`, {
        method: 'POST',
        headers: { 'Content-Type': 'application/json' },
        body: JSON.stringify({ chat_id: chatId, text, parse_mode: 'Markdown' }),
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

  async sendMedia(chatId: string, url: string, caption: string): Promise<{ messageId: string }> {
    if (!TELEGRAM_TOKEN) throw new Error('TELEGRAM_BOT_TOKEN not configured');
    const res = await fetch(`${TELEGRAM_API}/bot${TELEGRAM_TOKEN}/sendPhoto`, {
      method: 'POST',
      headers: { 'Content-Type': 'application/json' },
      body: JSON.stringify({ chat_id: chatId, photo: url, caption }),
      signal: AbortSignal.timeout(15_000),
    });
    const data = await res.json();
    return { messageId: String(data?.result?.message_id || 'unknown') };
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
