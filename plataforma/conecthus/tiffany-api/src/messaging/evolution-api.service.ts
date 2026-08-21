import { Injectable, Logger } from '@nestjs/common';
import { ChannelSender } from './interfaces/channel-sender.interface';

const WA_BRIDGE_URL = process.env.WA_BRIDGE_URL || 'http://127.0.0.1:8089';
const WA_BRIDGE_KEY = process.env.WA_BRIDGE_KEY || 'wa_bridge_2026';

@Injectable()
export class EvolutionApiService implements ChannelSender {
  private readonly logger = new Logger('WhatsApp');

  private toRemoteJid(target: string): string {
    // Pass through any WhatsApp JID format (@s.whatsapp.net, @g.us, @lid)
    if (target.includes('@')) return target;
    const digits = target.replace(/\D/g, '');
    return `${digits}@s.whatsapp.net`;
  }

  async send(target: string, text: string): Promise<{ messageId: string }> {
    const remoteJid = this.toRemoteJid(target);
    try {
      const res = await fetch(`${WA_BRIDGE_URL}/send`, {
        method: 'POST',
        headers: {
          'Content-Type': 'application/json',
          'x-api-key': WA_BRIDGE_KEY,
        },
        body: JSON.stringify({ to: remoteJid, text }),
        signal: AbortSignal.timeout(30_000),
      });

      if (!res.ok) {
        const body = await res.text();
        this.logger.error(`Send failed (${res.status}): ${body}`);
        throw new Error(`WA Bridge ${res.status}: ${body}`);
      }

      this.logger.log(`Sent to ${remoteJid}`);
      return { messageId: 'sent' };
    } catch (err) {
      this.logger.error(`Send error to ${remoteJid}: ${err.message}`);
      throw err;
    }
  }

  async sendMedia(target: string, url: string, caption: string): Promise<{ messageId: string }> {
    // TODO: implement media sending via Baileys bridge
    return this.send(target, `${caption}\n${url}`);
  }

  async healthCheck(): Promise<boolean> {
    try {
      const res = await fetch(`${WA_BRIDGE_URL}/health`, {
        signal: AbortSignal.timeout(5_000),
      });
      if (!res.ok) return false;
      const data = await res.json();
      return data.status === 'connected';
    } catch {
      return false;
    }
  }
}
