import { Injectable, Logger } from '@nestjs/common';
import { Cron, CronExpression } from '@nestjs/schedule';
import { PrismaService } from '../prisma.service';

const PRIVACY_TIMEOUT_MS = parseInt(process.env.PRIVACY_TIMEOUT_MS || '1800000'); // 30min default
const TELEGRAM_TOKEN = process.env.TELEGRAM_BOT_TOKEN || '';

@Injectable()
export class PrivacyTimeoutService {
  private readonly logger = new Logger('PrivacyTimeout');

  constructor(private prisma: PrismaService) {}

  @Cron(CronExpression.EVERY_5_MINUTES)
  async sweep() {
    const cutoff = new Date(Date.now() - PRIVACY_TIMEOUT_MS);
    // Sessões com privacy ativo e sem atividade há mais de PRIVACY_TIMEOUT_MS
    const sessions = await this.prisma.conversationSession.findMany({
      where: {
        lastActionAt: { lt: cutoff },
      },
    });
    const expired = sessions.filter((s) => (s.metadata as any)?.privacyMode === true);
    if (expired.length === 0) return;

    this.logger.log(`sweep: ${expired.length} session(s) com privacy expirado (>${PRIVACY_TIMEOUT_MS / 60000}min)`);

    for (const s of expired) {
      try {
        await this.expireSession(s);
      } catch (e: any) {
        this.logger.error(`expire ${s.channel}/${s.target}: ${e.message}`);
      }
    }
  }

  private async expireSession(session: any) {
    const meta = (session.metadata as any) || {};
    // 1. Desativa privacy
    await this.prisma.conversationSession.update({
      where: { id: session.id },
      data: { metadata: { ...meta, privacyMode: false, sandboxHistory: [] } },
    });

    // 2. Purga mídias do canal/target (lazy import pra evitar circular dep)
    let purged: any = null;
    try {
      const { MediaQueueService } = require('./media-queue.service');
      const mq = new MediaQueueService(this.prisma);
      purged = await mq.purgePrivacyMedia(session.channel, session.target);
    } catch (e: any) {
      this.logger.warn(`purge falhou: ${e.message}`);
    }

    // 3. Limpa RAM stores
    if ((global as any).__sandboxStore) (global as any).__sandboxStore.delete(session.target);
    if ((global as any).__sandboxMemories) (global as any).__sandboxMemories.delete(session.target);

    // 4. Avisa o usuário pelo canal
    if (session.channel === 'telegram' && TELEGRAM_TOKEN) {
      try {
        await fetch(`https://api.telegram.org/bot${TELEGRAM_TOKEN}/sendMessage`, {
          method: 'POST',
          headers: { 'Content-Type': 'application/json' },
          body: JSON.stringify({
            chat_id: session.target,
            text: `🔓 Modo privado expirou por inatividade (${PRIVACY_TIMEOUT_MS / 60000}min). ` +
                  `${purged ? `Apagados: ${purged.purged} mídias.` : ''}`,
          }),
          signal: AbortSignal.timeout(8_000),
        });
      } catch {}
    }

    this.logger.log(`expired ${session.channel}/${session.target}: ${purged?.purged || 0} mídias purgadas`);
  }
}
