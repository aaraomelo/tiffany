/**
 * MultiversoService — interface da Patrícia com a frota distribuída.
 *
 * Capacidades:
 *  - control(target, factor, duration): publica comando no /ws/ingest do
 *    hiper-chat. server.py broadcasta pra /ws/live; volunteer-hs ouve e
 *    aplica. Grava log em multiverso_control_log pra auditoria.
 *  - listVoluntarios / searchVoluntarios: catálogo persistente com embedding
 *    pra busca semântica ("paubrasil", "general Haskell em laboratório").
 *  - statusNow: leitura ao vivo do coord (HTTP /api/multiverso/leaderboard).
 *
 * O canal WS /ws/ingest exige X-API-Key. Lemos de WS_KEY (env) ou
 * /root/hiper-chat/.ws_key (default em prod).
 */
import { Injectable, Logger, BadRequestException } from '@nestjs/common';
import { PrismaService } from '../prisma.service';
import { readFileSync } from 'fs';
// eslint-disable-next-line @typescript-eslint/no-var-requires
const WebSocket = require('ws');

const HIPER_BASE_URL = process.env.HIPER_BASE_URL || 'https://hiper.patriatechnology.com';
const WS_INGEST_URL = process.env.WS_INGEST_URL || 'wss://hiper.patriatechnology.com/ws/ingest';
const COORD_URL = process.env.COORD_URL || 'https://hiper.patriatechnology.com/multiverso';
const EMBEDDER_URL = process.env.THEORY_EMBEDDER_URL || 'http://127.0.0.1:9301';
const WS_KEY_FILE = process.env.WS_KEY_FILE || '/root/hiper-chat/.ws_key';

@Injectable()
export class MultiversoService {
  private readonly logger = new Logger('MultiversoService');
  private readonly wsKey: string;

  constructor(private readonly prisma: PrismaService) {
    this.wsKey = this.loadWsKey();
  }

  // -------- WS_KEY loader --------
  private loadWsKey(): string {
    if (process.env.WS_KEY) return process.env.WS_KEY.trim();
    try {
      return readFileSync(WS_KEY_FILE, 'utf8').trim();
    } catch (e) {
      this.logger.warn(`WS_KEY ausente (env e ${WS_KEY_FILE}); control vai falhar até existir`);
      return '';
    }
  }

  // ============================================================
  // CONTROL — manda comando via /ws/ingest
  // ============================================================
  async sendControl(opts: {
    target: string;
    factor: number;
    durationSec: number;
    reason?: string;
  }): Promise<{ ok: boolean; logId?: string; ackOk?: boolean; error?: string }> {
    const { target, factor, durationSec, reason } = opts;
    if (factor <= 0 || factor > 1) {
      throw new BadRequestException('factor deve estar em (0, 1]');
    }
    if (durationSec <= 0 || durationSec > 7 * 24 * 3600) {
      throw new BadRequestException('duration_sec entre 1s e 7 dias');
    }
    if (!this.wsKey) {
      throw new BadRequestException('WS_KEY não configurada — não consigo publicar');
    }

    const payload = JSON.stringify({
      type: 'control',
      target,
      factor,
      duration: durationSec,
    });

    let ackOk: boolean | null = null;
    let errorMsg: string | undefined;

    try {
      ackOk = await this.publishViaWs(payload);
    } catch (e: any) {
      errorMsg = String(e?.message || e);
      this.logger.error(`sendControl falhou: ${errorMsg}`);
    }

    const expiresAt = new Date(Date.now() + durationSec * 1000);
    const logEntry = await this.prisma.multiversoControlLog.create({
      data: {
        target,
        cmd: 'override',
        factor,
        durationSec,
        expiresAt,
        reason: reason || null,
        ackOk,
      },
    });

    return {
      ok: ackOk === true,
      logId: logEntry.id.toString(),
      ackOk: ackOk ?? undefined,
      error: errorMsg,
    };
  }

  /** Conecta WS, espera ACK, manda payload, fecha. ~1-2s por chamada. */
  private publishViaWs(payload: string, timeoutMs = 8000): Promise<boolean> {
    return new Promise((resolve, reject) => {
      const ws = new WebSocket(WS_INGEST_URL, {
        headers: { 'X-API-Key': this.wsKey },
      });
      const timer = setTimeout(() => {
        try { ws.close(); } catch {}
        reject(new Error(`timeout ${timeoutMs}ms aguardando ack`));
      }, timeoutMs);
      let acked = false;

      ws.on('open', () => { /* esperando ACK antes de mandar */ });
      ws.on('message', (raw) => {
        try {
          const msg = JSON.parse(raw.toString());
          if (msg.type === 'ack' && !acked) {
            acked = true;
            ws.send(payload);
            // fecha logo após enviar — server broadcasta
            setTimeout(() => {
              clearTimeout(timer);
              try { ws.close(); } catch {}
              resolve(true);
            }, 200);
          } else if (msg.type === 'error') {
            clearTimeout(timer);
            try { ws.close(); } catch {}
            reject(new Error(`server error: ${msg.msg || 'unknown'}`));
          }
        } catch {
          /* ignora mensagens não-JSON */
        }
      });
      ws.on('error', (err) => {
        clearTimeout(timer);
        reject(err);
      });
      ws.on('close', () => {
        if (!acked) {
          clearTimeout(timer);
          reject(new Error('conexão fechou antes do ack'));
        }
      });
    });
  }

  // ============================================================
  // VOLUNTÁRIOS — catálogo persistente com embedding
  // ============================================================

  async upsertVoluntario(opts: {
    vid: string;
    displayName: string;
    host?: string;
    role?: 'voluntary' | 'general' | 'coord';
    runtime?: 'python' | 'haskell-hs';
    isLab?: boolean;
    description: string;
  }) {
    const { vid, displayName, host, role, runtime, isLab, description } = opts;
    const embedding = await this.embedText(description);

    const existing = await this.prisma.multiversoVoluntario.findUnique({ where: { vid } });
    const baseData = {
      vid, displayName, host: host ?? null, role: role || 'voluntary',
      runtime: runtime || 'python', isLab: !!isLab, description,
    };

    if (existing) {
      await this.prisma.multiversoVoluntario.update({
        where: { vid },
        data: baseData,
      });
    } else {
      await this.prisma.multiversoVoluntario.create({ data: baseData });
    }
    if (embedding) {
      // pgvector via raw SQL — Prisma não suporta nativamente
      await this.prisma.$executeRawUnsafe(
        `UPDATE multiverso_voluntarios SET embedding = $1::vector, embedded_at = NOW() WHERE vid = $2`,
        `[${embedding.join(',')}]`,
        vid,
      );
    }
    return { ok: true, vid, embedded: !!embedding };
  }

  async listVoluntarios() {
    const rows = await this.prisma.$queryRawUnsafe<any[]>(
      `SELECT id, vid, display_name AS "displayName", host, role, runtime,
              is_lab AS "isLab", description, last_seen_at AS "lastSeenAt",
              last_metrics AS "lastMetrics", embedded_at AS "embeddedAt"
       FROM multiverso_voluntarios
       ORDER BY display_name`,
    );
    return rows;
  }

  async searchVoluntarios(query: string, limit = 5) {
    const emb = await this.embedText(query);
    if (!emb) {
      throw new BadRequestException('embedder offline — busca semântica indisponível');
    }
    const rows = await this.prisma.$queryRawUnsafe<any[]>(
      `SELECT vid, display_name AS "displayName", host, role, runtime,
              is_lab AS "isLab", description,
              1 - (embedding <=> $1::vector) AS similarity
       FROM multiverso_voluntarios
       WHERE embedding IS NOT NULL
       ORDER BY embedding <=> $1::vector
       LIMIT $2`,
      `[${emb.join(',')}]`,
      limit,
    );
    return rows;
  }

  // ============================================================
  // STATUS — leitura ao vivo do coord
  // ============================================================

  async statusNow(vid?: string) {
    const res = await fetch(`${COORD_URL}/api/multiverso/leaderboard`, {
      signal: AbortSignal.timeout(8000),
    });
    if (!res.ok) {
      throw new Error(`coord HTTP ${res.status}`);
    }
    const data: any = await res.json();
    const board: any[] = data.leaderboard || [];
    if (vid) {
      const row = board.find((r) => r.voluntary_id === vid);
      if (!row) {
        throw new BadRequestException(`vid '${vid}' não encontrado no leaderboard`);
      }
      return row;
    }
    return {
      n_voluntarios: board.length,
      online: board.filter((r) => r.online).length,
      voluntarios: board,
    };
  }

  // ============================================================
  // EMBEDDER helper
  // ============================================================
  private async embedText(text: string): Promise<number[] | null> {
    try {
      const res = await fetch(`${EMBEDDER_URL}/embed-query`, {
        method: 'POST',
        headers: { 'Content-Type': 'application/json' },
        body: JSON.stringify({ text }),
        signal: AbortSignal.timeout(8000),
      });
      if (!res.ok) return null;
      const data: any = await res.json();
      return data.vector || null;
    } catch (e) {
      this.logger.warn(`embedText falhou: ${String(e)}`);
      return null;
    }
  }
}
