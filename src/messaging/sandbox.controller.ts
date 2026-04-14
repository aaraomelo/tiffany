import { Controller, Post, Body, Logger } from '@nestjs/common';
import { PrismaService } from '../prisma.service';
import * as crypto from 'crypto';

const TELEGRAM_TOKEN = process.env.TELEGRAM_BOT_TOKEN || '';

@Controller('api/sandbox')
export class SandboxController {
  private readonly logger = new Logger('Sandbox');

  constructor(private prisma: PrismaService) {}

  @Post('activate')
  async activate(@Body() body: { initData: string }) {
    if (!body.initData) return { ok: false, error: 'initData required' };

    // Parse and verify Telegram initData
    const params = new URLSearchParams(body.initData);
    const hash = params.get('hash');
    params.delete('hash');

    // Sort and create check string
    const sorted = [...params.entries()].sort(([a], [b]) => a.localeCompare(b));
    const checkString = sorted.map(([k, v]) => `${k}=${v}`).join('\n');

    // Verify HMAC
    const secretKey = crypto.createHmac('sha256', 'WebAppData').update(TELEGRAM_TOKEN).digest();
    const expectedHash = crypto.createHmac('sha256', secretKey).update(checkString).digest('hex');

    if (hash !== expectedHash) {
      this.logger.warn('Invalid initData hash');
      return { ok: false, error: 'Invalid signature' };
    }

    // Extract user
    const userData = JSON.parse(params.get('user') || '{}');
    const telegramId = String(userData.id);
    if (!telegramId) return { ok: false, error: 'No user in initData' };

    // Find session for this Telegram user
    const session = await this.prisma.conversationSession.findFirst({
      where: { channel: 'telegram', target: telegramId },
    });
    if (!session) return { ok: false, error: 'No session found' };

    // Derive encryption key from initData (unique per user+session+time)
    const encKey = crypto.createHash('sha256').update(body.initData + session.id).digest('hex');

    // Store key in RAM
    if (!(global as any).__sandboxKeys) (global as any).__sandboxKeys = new Map();
    (global as any).__sandboxKeys.set(session.id, encKey);

    // Activate sandbox
    const meta = (session.metadata as any) || {};
    await this.prisma.conversationSession.update({
      where: { id: session.id },
      data: { metadata: { ...meta, privacyMode: true, sandboxHistory: [] } },
    });

    this.logger.log(`Sandbox activated for Telegram user ${telegramId}`);
    return { ok: true };
  }

  @Post('sync')
  async sync(@Body() body: { chatId: string; history?: any[] }) {
    if (!body.chatId) return { ok: false, error: 'chatId required' };

    // Get server-side store (messages since last sync)
    if (!(global as any).__sandboxStore) (global as any).__sandboxStore = new Map();
    const serverHistory = (global as any).__sandboxStore.get(body.chatId) || [];

    // Merge: client sends their history, we return the latest
    // Client localStorage is the source of truth
    if (body.history?.length) {
      // Client has history — update server store with it
      (global as any).__sandboxStore.set(body.chatId, body.history.slice(-100));
    }

    // Return server history (may have new messages since last sync)
    return { ok: true, history: serverHistory };
  }

  @Post('deactivate')
  async deactivate(@Body() body: { chatId: string }) {
    if (!body.chatId) return { ok: false, error: 'chatId required' };

    // Clear server-side store
    if ((global as any).__sandboxStore) (global as any).__sandboxStore.delete(body.chatId);

    // Deactivate sandbox in session
    const session = await this.prisma.conversationSession.findFirst({
      where: { channel: 'telegram', target: body.chatId },
    });
    if (session) {
      const meta = (session.metadata as any) || {};
      await this.prisma.conversationSession.update({
        where: { id: session.id },
        data: { metadata: { ...meta, privacyMode: false, sandboxHistory: [] } },
      });
    }

    this.logger.log(`Sandbox deactivated for Telegram user ${body.chatId}`);
    return { ok: true };
  }
}
