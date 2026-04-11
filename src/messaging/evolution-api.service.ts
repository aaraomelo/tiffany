import { Injectable, Logger } from '@nestjs/common';
import { ChannelSender } from './interfaces/channel-sender.interface';

const EVOLUTION_URL = process.env.EVOLUTION_API_URL || 'http://localhost:8088';
const EVOLUTION_KEY = process.env.EVOLUTION_API_KEY || 'patria-evolution-2026';
const EVOLUTION_INSTANCE = process.env.EVOLUTION_INSTANCE || 'patria';

@Injectable()
export class EvolutionApiService implements ChannelSender {
  private readonly logger = new Logger('EvolutionApi');

  private toRemoteJid(target: string): string {
    if (target.endsWith('@s.whatsapp.net') || target.endsWith('@g.us')) return target;
    const digits = target.replace(/\D/g, '');
    return `${digits}@s.whatsapp.net`;
  }

  async send(target: string, text: string): Promise<{ messageId: string }> {
    const remoteJid = this.toRemoteJid(target);
    try {
      const res = await fetch(`${EVOLUTION_URL}/message/sendText/${EVOLUTION_INSTANCE}`, {
        method: 'POST',
        headers: {
          'Content-Type': 'application/json',
          apikey: EVOLUTION_KEY,
        },
        body: JSON.stringify({ number: remoteJid, text }),
        signal: AbortSignal.timeout(30_000),
      });

      if (!res.ok) {
        const body = await res.text();
        this.logger.error(`Send failed (${res.status}): ${body}`);
        throw new Error(`Evolution API ${res.status}: ${body}`);
      }

      const data = await res.json();
      const messageId = data?.key?.id || data?.messageId || 'unknown';
      this.logger.log(`Sent to ${remoteJid} (${messageId})`);
      return { messageId };
    } catch (err) {
      this.logger.error(`Send error to ${remoteJid}: ${err.message}`);
      throw err;
    }
  }

  async sendMedia(target: string, url: string, caption: string): Promise<{ messageId: string }> {
    const remoteJid = this.toRemoteJid(target);
    try {
      const res = await fetch(`${EVOLUTION_URL}/message/sendMedia/${EVOLUTION_INSTANCE}`, {
        method: 'POST',
        headers: {
          'Content-Type': 'application/json',
          apikey: EVOLUTION_KEY,
        },
        body: JSON.stringify({
          number: remoteJid,
          mediatype: 'image',
          media: url,
          caption,
        }),
        signal: AbortSignal.timeout(30_000),
      });

      if (!res.ok) throw new Error(`Evolution API ${res.status}`);
      const data = await res.json();
      return { messageId: data?.key?.id || 'unknown' };
    } catch (err) {
      this.logger.error(`SendMedia error: ${err.message}`);
      throw err;
    }
  }

  async healthCheck(): Promise<boolean> {
    try {
      const res = await fetch(
        `${EVOLUTION_URL}/instance/connectionState/${EVOLUTION_INSTANCE}`,
        {
          headers: { apikey: EVOLUTION_KEY },
          signal: AbortSignal.timeout(5_000),
        },
      );
      if (!res.ok) return false;
      const data = await res.json();
      return data?.instance?.state === 'open';
    } catch {
      return false;
    }
  }
}
