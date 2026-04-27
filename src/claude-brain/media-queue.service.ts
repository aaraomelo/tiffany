import { Injectable, Logger, OnModuleInit } from '@nestjs/common';
import { PrismaService } from '../prisma.service';
import { TelegramService } from '../messaging/telegram.service';

const BRIDGE_URL = process.env.BRIDGE_URL || 'http://host.docker.internal:9090';
const BRIDGE_SECRET = process.env.BRIDGE_SECRET || 'wk_infer_patria_2026';
const SELF_API_URL = process.env.SELF_API_URL || 'http://127.0.0.1:8080';

@Injectable()
export class MediaQueueService implements OnModuleInit {
  private readonly logger = new Logger('MediaQueue');
  private processing = false;
  private telegram = new TelegramService();

  constructor(private prisma: PrismaService) {}

  onModuleInit() {
    // Recovery: jobs em "processing" no boot foram interrompidos por restart
    this.prisma.mediaJob.updateMany({
      where: { status: 'processing' },
      data: { status: 'pending' },
    }).then((r) => r.count > 0 && this.logger.log(`recovered ${r.count} stuck jobs`)).catch(() => {});
    // Periodic check (a cada 30s) caso enqueue caia / restart
    setInterval(() => this.tickIfIdle().catch(() => {}), 30_000);
  }

  /** Cria job + dispara processamento async. Retorna o id imediatamente. */
  async enqueue(opts: {
    kind: 'image' | 'audio';
    params: any;
    channel: string;
    target: string;
  }): Promise<{ id: string; queue_position: number }> {
    const job = await this.prisma.mediaJob.create({
      data: {
        kind: opts.kind,
        status: 'pending',
        params: opts.params,
        channel: opts.channel,
        target: opts.target,
      },
    });
    const pos = await this.prisma.mediaJob.count({
      where: { status: { in: ['pending', 'processing'] }, createdAt: { lte: job.createdAt } },
    });
    setImmediate(() => this.tickIfIdle().catch((e) => this.logger.error(`tick: ${e.message}`)));
    return { id: job.id, queue_position: pos };
  }

  private async tickIfIdle() {
    if (this.processing) return;
    this.processing = true;
    try {
      while (true) {
        const job = await this.prisma.mediaJob.findFirst({
          where: { status: 'pending' },
          orderBy: { createdAt: 'asc' },
        });
        if (!job) break;
        await this.processJob(job);
      }
    } finally {
      this.processing = false;
    }
  }

  private async processJob(job: any) {
    this.logger.log(`processing ${job.id} (${job.kind} → ${job.channel}/${job.target})`);
    await this.prisma.mediaJob.update({
      where: { id: job.id },
      data: { status: 'processing', startedAt: new Date(), attempts: { increment: 1 } },
    });

    // Mantém typing ativo no canal durante processamento
    let stopTyping: (() => void) | null = null;
    if (job.channel === 'telegram') {
      try {
        stopTyping = this.telegram.startTyping(job.target);
      } catch {}
    }

    try {
      if (job.kind === 'image') {
        await this.processImage(job);
      } else {
        throw new Error(`unsupported kind: ${job.kind}`);
      }
    } catch (e: any) {
      const errMsg = e.message?.slice(0, 500) || String(e);
      this.logger.error(`job ${job.id} failed: ${errMsg}`);
      await this.prisma.mediaJob.update({
        where: { id: job.id },
        data: { status: 'failed', error: errMsg, completedAt: new Date() },
      });
      // Notifica falha pelo canal
      try {
        await this.sendToChannel(job.channel, job.target, `❌ Geração falhou: ${errMsg.slice(0, 200)}`);
      } catch {}
    } finally {
      if (stopTyping) stopTyping();
    }
  }

  private async processImage(job: any) {
    const params = job.params as any;
    const res = await fetch(`${BRIDGE_URL}/image/generate`, {
      method: 'POST',
      headers: { 'Content-Type': 'application/json', 'X-Bridge-Key': BRIDGE_SECRET },
      body: JSON.stringify({
        prompt: params.prompt,
        provider: params.provider || 'openai',
        size: params.size || '1024x1024',
        quality: params.quality || 'standard',
        base_image_url: params.base_image_url || undefined,
      }),
      signal: AbortSignal.timeout(120_000),
    });
    if (!res.ok) {
      const t = await res.text().catch(() => '');
      throw new Error(`bridge ${res.status}: ${t.slice(0, 200)}`);
    }
    const data = await res.json();
    const url: string = data.url;
    if (!url) throw new Error('no url in response');

    // Envia mídia + caption pro canal (caption custom > prompt como fallback)
    const caption = (params.caption || params.prompt || '').slice(0, 200);
    if (job.channel === 'telegram') {
      await this.telegram.sendMedia(job.target, url, caption);
    } else {
      // whatsapp ou outro: usa endpoint /api/messaging/send (passa pelo MessagingService)
      await fetch(`${SELF_API_URL}/api/messaging/send`, {
        method: 'POST',
        headers: { 'Content-Type': 'application/json', 'X-API-Key': process.env.API_KEY_WORKER || '' },
        body: JSON.stringify({ channel: job.channel, target: job.target,
          message: `🖼️ Pronto:\n${url}\n\n${caption}` }),
        signal: AbortSignal.timeout(15_000),
      });
    }

    await this.prisma.mediaJob.update({
      where: { id: job.id },
      data: { status: 'done', resultUrl: url, completedAt: new Date() },
    });
  }

  private async sendToChannel(channel: string, target: string, message: string) {
    if (channel === 'telegram') {
      await this.telegram.send(target, message).catch(() => {});
    } else {
      await fetch(`${SELF_API_URL}/api/messaging/send`, {
        method: 'POST',
        headers: { 'Content-Type': 'application/json', 'X-API-Key': process.env.API_KEY_WORKER || '' },
        body: JSON.stringify({ channel, target, message }),
        signal: AbortSignal.timeout(10_000),
      }).catch(() => {});
    }
  }
}
