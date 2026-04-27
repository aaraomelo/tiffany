import { Injectable, Logger } from '@nestjs/common';
import { PrismaService } from '../prisma.service';

const GEMINI_API_KEY = process.env.GEMINI_API_KEY;

@Injectable()
export class OrganismEventsService {
  private readonly logger = new Logger('OrganismEvents');

  constructor(private prisma: PrismaService) {}

  async generateEmbedding(text: string): Promise<number[] | null> {
    if (!GEMINI_API_KEY) return null;
    try {
      const res = await fetch(
        `https://generativelanguage.googleapis.com/v1beta/models/gemini-embedding-001:embedContent?key=${GEMINI_API_KEY}`,
        {
          method: 'POST',
          headers: { 'Content-Type': 'application/json' },
          body: JSON.stringify({
            content: { parts: [{ text }] },
            outputDimensionality: 768,
          }),
          signal: AbortSignal.timeout(8_000),
        },
      );
      if (!res.ok) return null;
      const data = await res.json();
      return data.embedding?.values || null;
    } catch (e: any) {
      this.logger.warn(`Embedding failed: ${e.message}`);
      return null;
    }
  }

  async record(input: {
    source: string;
    kind: string;
    severity?: number;
    data?: any;
    ts?: string;
  }): Promise<{ id: string }> {
    const severity = Math.max(1, Math.min(9, input.severity ?? 3));
    const data = input.data ?? {};
    const tsExpr = input.ts ? '$1::timestamptz' : 'now()';
    const sql = `
      INSERT INTO organism_events (ts, source, kind, severity, data)
      VALUES (${tsExpr}, $${input.ts ? '2' : '1'}, $${input.ts ? '3' : '2'}, $${input.ts ? '4' : '3'}, $${input.ts ? '5' : '4'}::jsonb)
      RETURNING id
    `;
    const args = input.ts
      ? [input.ts, input.source.slice(0, 50), input.kind.slice(0, 50), severity, JSON.stringify(data)]
      : [input.source.slice(0, 50), input.kind.slice(0, 50), severity, JSON.stringify(data)];
    const rows: any[] = await this.prisma.$queryRawUnsafe(sql, ...args);
    const id = String(rows[0].id);

    // Embedding async (não bloqueia)
    const text = `${input.source} ${input.kind} ${JSON.stringify(data).slice(0, 1000)}`;
    this.generateEmbedding(text).then(async (emb) => {
      if (!emb) return;
      const vec = `[${emb.join(',')}]`;
      try {
        await this.prisma.$executeRawUnsafe(
          `UPDATE organism_events SET embedding = $1::vector WHERE id = $2::bigint`,
          vec, id,
        );
      } catch {}
    }).catch(() => {});

    return { id };
  }

  async loadUnconsumed(limit = 50): Promise<any[]> {
    const rows: any[] = await this.prisma.$queryRawUnsafe(
      `SELECT id::text, ts, source, kind, severity, data
       FROM organism_events
       WHERE consumed_at IS NULL
       ORDER BY ts DESC LIMIT $1`,
      limit,
    );
    return rows;
  }

  async markConsumed(ids: string[], by: string = 'claude_watch'): Promise<number> {
    if (ids.length === 0) return 0;
    const idList = ids.map((i) => BigInt(i));
    const result: any = await this.prisma.$executeRawUnsafe(
      `UPDATE organism_events
       SET consumed_at = NOW(), consumed_by = $1
       WHERE id = ANY($2::bigint[]) AND consumed_at IS NULL`,
      by, idList as any,
    );
    return Number(result) || 0;
  }

  async stats(): Promise<any> {
    const rows: any[] = await this.prisma.$queryRawUnsafe(`
      SELECT kind, COUNT(*)::int as count,
             COUNT(*) FILTER (WHERE consumed_at IS NULL)::int as unconsumed,
             MAX(ts) as last_ts
      FROM organism_events
      WHERE ts > NOW() - INTERVAL '24 hours'
      GROUP BY kind
      ORDER BY count DESC
    `);
    return rows;
  }
}
