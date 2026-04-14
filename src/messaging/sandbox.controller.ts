import { Controller, Post, Get, Body, Query, Req, Res, Logger } from '@nestjs/common';
import { Request, Response } from 'express';
import { PrismaService } from '../prisma.service';
import * as crypto from 'crypto';

const TELEGRAM_TOKEN = process.env.TELEGRAM_BOT_TOKEN || '';

// Track active SSE connections per chat
if (!(global as any).__sandboxConnections) (global as any).__sandboxConnections = new Map<string, Response>();

@Controller('api/sandbox')
export class SandboxController {
  private readonly logger = new Logger('Sandbox');

  constructor(private prisma: PrismaService) {}

  static isMiniAppAlive(chatId: string): boolean {
    return (global as any).__sandboxConnections?.has(chatId) || false;
  }

  static closeMiniApp(chatId: string): void {
    const res = (global as any).__sandboxConnections?.get(chatId);
    if (res) {
      try { res.write('data: close\n\n'); } catch {}
    }
  }

  @Post('activate')
  async activate(@Body() body: { initData: string }) {
    if (!body.initData) return { ok: false, error: 'initData required' };

    const params = new URLSearchParams(body.initData);
    const hash = params.get('hash');
    params.delete('hash');

    const sorted = [...params.entries()].sort(([a], [b]) => a.localeCompare(b));
    const checkString = sorted.map(([k, v]) => `${k}=${v}`).join('\n');

    const secretKey = crypto.createHmac('sha256', 'WebAppData').update(TELEGRAM_TOKEN).digest();
    const expectedHash = crypto.createHmac('sha256', secretKey).update(checkString).digest('hex');

    if (hash !== expectedHash) {
      this.logger.warn('Invalid initData hash');
      return { ok: false, error: 'Invalid signature' };
    }

    const userData = JSON.parse(params.get('user') || '{}');
    const telegramId = String(userData.id);
    if (!telegramId) return { ok: false, error: 'No user in initData' };

    const session = await this.prisma.conversationSession.findFirst({
      where: { channel: 'telegram', target: telegramId },
    });
    if (!session) return { ok: false, error: 'No session found' };

    const meta = (session.metadata as any) || {};
    await this.prisma.conversationSession.update({
      where: { id: session.id },
      data: { metadata: { ...meta, privacyMode: true, sandboxHistory: [] } },
    });

    // Notify user in chat
    try {
      if (TELEGRAM_TOKEN) {
        await fetch(`https://api.telegram.org/bot${TELEGRAM_TOKEN}/sendMessage`, {
          method: 'POST',
          headers: { 'Content-Type': 'application/json' },
          body: JSON.stringify({ chat_id: telegramId, text: '🔒 Modo privado ativado. Nada será salvo no servidor. Feche o app de privacidade pra encerrar.' }),
        });
      }
    } catch {}

    this.logger.log(`Sandbox activated for Telegram user ${telegramId}`);
    return { ok: true };
  }

  // SSE stream — Mini App connects, server detects disconnect
  @Get('stream')
  stream(@Query('chatId') chatId: string, @Req() req: Request, @Res() res: Response) {
    if (!chatId) { res.status(400).json({ error: 'chatId required' }); return; }

    res.setHeader('Content-Type', 'text/event-stream');
    res.setHeader('Cache-Control', 'no-cache, no-transform');
    res.setHeader('Connection', 'keep-alive');
    res.setHeader('X-Accel-Buffering', 'no');
    res.flushHeaders();

    // Register connection
    (global as any).__sandboxConnections.set(chatId, res);
    this.logger.log(`SSE connected: ${chatId}`);

    // Send keepalive every 15s
    const keepalive = setInterval(() => {
      try { res.write(':\n\n'); } catch { clearInterval(keepalive); }
    }, 15_000);

    // On disconnect — deactivate sandbox
    req.on('close', async () => {
      clearInterval(keepalive);
      (global as any).__sandboxConnections.delete(chatId);
      this.logger.log(`SSE disconnected: ${chatId} — deactivating sandbox`);

      // Deactivate
      try {
        const session = await this.prisma.conversationSession.findFirst({
          where: { channel: 'telegram', target: chatId },
        });
        if (session) {
          const meta = (session.metadata as any) || {};
          await this.prisma.conversationSession.update({
            where: { id: session.id },
            data: { metadata: { ...meta, privacyMode: false, sandboxHistory: [] } },
          });
        }

        // Clear RAM store
        if ((global as any).__sandboxStore) (global as any).__sandboxStore.delete(chatId);
        if ((global as any).__sandboxMemories) (global as any).__sandboxMemories.delete(chatId);

        // Notify user
        if (TELEGRAM_TOKEN) {
          await fetch(`https://api.telegram.org/bot${TELEGRAM_TOKEN}/sendMessage`, {
            method: 'POST',
            headers: { 'Content-Type': 'application/json' },
            body: JSON.stringify({ chat_id: chatId, text: '🔓 Modo privado encerrado. Histórico da sessão apagado.' }),
          });
        }
      } catch (err) {
        this.logger.error(`Deactivation on disconnect failed: ${err.message}`);
      }
    });

    // Initial event
    res.write('data: connected\n\n');
  }

  @Post('sync')
  async sync(@Body() body: { chatId: string; history?: any[] }) {
    if (!body.chatId) return { ok: false, error: 'chatId required' };

    if (!(global as any).__sandboxStore) (global as any).__sandboxStore = new Map();
    const serverHistory = (global as any).__sandboxStore.get(body.chatId) || [];

    if (body.history?.length) {
      (global as any).__sandboxStore.set(body.chatId, body.history.slice(-100));
    }

    return { ok: true, history: serverHistory };
  }

  @Post('deactivate')
  async deactivate(@Body() body: { chatId: string }) {
    if (!body.chatId) return { ok: false, error: 'chatId required' };

    if ((global as any).__sandboxStore) (global as any).__sandboxStore.delete(body.chatId);
    if ((global as any).__sandboxMemories) (global as any).__sandboxMemories.delete(body.chatId);
    if ((global as any).__sandboxConnections) (global as any).__sandboxConnections.delete(body.chatId);

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

    try {
      if (TELEGRAM_TOKEN) {
        await fetch(`https://api.telegram.org/bot${TELEGRAM_TOKEN}/sendMessage`, {
          method: 'POST',
          headers: { 'Content-Type': 'application/json' },
          body: JSON.stringify({ chat_id: body.chatId, text: '🔓 Modo privado encerrado. Histórico da sessão apagado.' }),
        });
      }
    } catch {}

    this.logger.log(`Sandbox deactivated for Telegram user ${body.chatId}`);
    return { ok: true };
  }
}
