import { Injectable, Logger } from '@nestjs/common';
import { PrismaService } from '../prisma.service';

const GEMINI_API_KEY = process.env.GEMINI_API_KEY;
const PROMOTE_THRESHOLD = 5; // accesses needed to promote short_term → long_term
const EXPIRE_DAYS = 30;

@Injectable()
export class MemoryService {
  private readonly logger = new Logger('Memory');

  constructor(private prisma: PrismaService) {}

  // --- Embedding ---

  private async generateEmbedding(text: string): Promise<number[]> {
    if (!GEMINI_API_KEY) throw new Error('GEMINI_API_KEY not set');
    const res = await fetch(
      `https://generativelanguage.googleapis.com/v1beta/models/gemini-embedding-001:embedContent?key=${GEMINI_API_KEY}`,
      {
        method: 'POST',
        headers: { 'Content-Type': 'application/json' },
        body: JSON.stringify({
          content: { parts: [{ text }] },
          outputDimensionality: 768,
        }),
        signal: AbortSignal.timeout(10_000),
      },
    );
    if (!res.ok) throw new Error(`Gemini embedding API ${res.status}`);
    const data = await res.json();
    return data.embedding.values;
  }

  // --- Main context builder ---

  // Legacy: no person filter
  async getContext(query: string): Promise<string> {
    return this.getContextForPerson(query, null, 'all');
  }

  // Main: with person + access level
  async getContextForPerson(query: string, personId: string | null, accessLevel: string): Promise<string> {
    // 1. Load core — directors see all, others only non-work categories
    const workCategories = ['decision', 'technical', 'project', 'product'];
    const coreFilter: any = { priority: 'core', state: 'active' };
    if (accessLevel !== 'all') {
      coreFilter.category = { notIn: workCategories };
    }
    const core = await this.prisma.patriciaMemory.findMany({
      where: coreFilter,
      orderBy: { category: 'asc' },
    });

    // 2. Search by semantic similarity (all non-core memories)
    const relevant = await this.searchByEmbedding(query, ['long_term', 'short_term'], 8, personId, accessLevel);

    // 3. Track access
    await this.trackAccess(relevant.map((r) => r.id));

    // 4. Maintenance (async)
    this.runMaintenance().catch(() => {});

    // 5. Theory chunks (only for directors — teoria é privada)
    let theory: any[] = [];
    if (accessLevel === 'all') {
      theory = await this.searchTheoryChunks(query, 8).catch(() => []);
    }

    // 6. Organism events (only directors)
    let organism: any[] = [];
    if (accessLevel === 'all') {
      organism = await this.searchOrganismEvents(query, 3).catch(() => []);
    }

    // Format
    const parts: string[] = [];
    if (core.length > 0) {
      parts.push('## Conhecimento base\n' + core.map((m) => `**${m.title}:** ${m.content}`).join('\n'));
    }
    if (relevant.length > 0) {
      parts.push('## Memórias relevantes\n' + relevant.map((m) => `**[${m.category}] ${m.title}:** ${m.content}`).join('\n'));
    }
    if (theory.length > 0) {
      parts.push(
        '## Teoria hipercomplexa relevante\n' +
        theory.map((t: any) => `**${t.file}${t.section ? ' — ' + t.section : ''}**\n${t.content}`).join('\n\n')
      );
    }
    if (organism.length > 0) {
      parts.push(
        '## Eventos do organismo (multiverso) relevantes\n' +
        organism.map((o: any) => `[${o.ts} ${o.source}/${o.kind} sev=${o.severity}] ${JSON.stringify(o.data || {})}`).join('\n')
      );
    }
    return parts.join('\n\n');
  }

  // --- Theory chunks search (via embedder daemon Python) ---
  private async searchTheoryChunks(query: string, limit: number): Promise<any[]> {
    const url = process.env.THEORY_EMBEDDER_URL || 'http://127.0.0.1:9301';
    let vec: number[];
    try {
      const res = await fetch(`${url}/embed-query`, {
        method: 'POST',
        headers: { 'Content-Type': 'application/json' },
        body: JSON.stringify({ text: query }),
        signal: AbortSignal.timeout(8_000),
      });
      const data = await res.json();
      if (!res.ok || !data.vector) return [];
      vec = data.vector;
    } catch {
      return [];
    }
    const v = `[${vec.join(',')}]`;

    // Detecta "paper X" / "paper~X" na query — prioriza chunks daquele arquivo
    const paperMatches = Array.from(query.matchAll(/\bpaper[\s~]+([A-Z][A-Z0-9]*)\b/gi));
    const targetLetters = [...new Set(paperMatches.map((m) => m[1].toUpperCase()))];

    // Busca global
    const globalRows: any[] = await this.prisma.$queryRawUnsafe(
      `SELECT id::text, file, section, content,
              1 - (embedding <=> $1::vector) AS similarity
       FROM theory_chunks
       WHERE embedding IS NOT NULL
       ORDER BY embedding <=> $1::vector LIMIT $2`,
      v, limit * 2,
    );

    // Busca filtrada por arquivo se "paper X" detectado (boost com +0.15 no score)
    let scopedRows: any[] = [];
    if (targetLetters.length > 0) {
      const filePatterns = targetLetters.map((l) => `papers/paper_${l}_%`);
      scopedRows = await this.prisma.$queryRawUnsafe(
        `SELECT id::text, file, section, content,
                (1 - (embedding <=> $1::vector)) + 0.15 AS similarity
         FROM theory_chunks
         WHERE embedding IS NOT NULL
           AND file LIKE ANY($2::text[])
         ORDER BY embedding <=> $1::vector LIMIT $3`,
        v, filePatterns, Math.ceil(limit * 0.6),
      );
    }

    // Merge + dedup por id, ordena por similarity DESC, corta no limit
    const byId = new Map<string, any>();
    for (const r of [...scopedRows, ...globalRows]) {
      const prev = byId.get(r.id);
      if (!prev || prev.similarity < r.similarity) byId.set(r.id, r);
    }
    return Array.from(byId.values())
      .filter((r) => r.similarity > 0.40)
      .sort((a, b) => b.similarity - a.similarity)
      .slice(0, limit);
  }

  // --- Organism events search (via Gemini embedding) ---
  private async searchOrganismEvents(query: string, limit: number): Promise<any[]> {
    let emb: number[];
    try {
      emb = await this.generateEmbedding(query);
    } catch {
      return [];
    }
    const v = `[${emb.join(',')}]`;
    const rows: any[] = await this.prisma.$queryRawUnsafe(
      `SELECT id::text, ts, source, kind, severity, data,
              1 - (embedding <=> $1::vector) AS similarity
       FROM organism_events
       WHERE embedding IS NOT NULL
         AND ts > NOW() - INTERVAL '14 days'
       ORDER BY embedding <=> $1::vector LIMIT $2`,
      v, limit,
    );
    return rows.filter((r) => r.similarity > 0.5);
  }

  // --- Search ---

  async searchByEmbedding(
    query: string,
    priorities: string[] = ['long_term', 'short_term'],
    limit = 5,
    personId: string | null = null,
    accessLevel = 'all',
  ): Promise<Array<{ id: string; title: string; content: string; category: string; priority: string }>> {
    try {
      const embedding = await this.generateEmbedding(query);
      const vectorStr = `[${embedding.join(',')}]`;

      // Build access filter with parameterized query (prevent SQL injection)
      let sql: string;
      let sqlParams: any[];

      // sealed = only visible to the person who created it (not even directors)
      // private = visible to owner + directors
      // group = visible to group members + directors
      // global = visible to everyone

      if (!personId) {
        sql = `SELECT id, title, content, category, priority,
                1 - (embedding <=> $1::vector) as similarity
         FROM patricia_memories
         WHERE embedding IS NOT NULL AND state = 'active' AND priority = ANY($3::text[])
           AND visibility = 'global'
         ORDER BY embedding <=> $1::vector LIMIT $2`;
        sqlParams = [vectorStr, limit, priorities];
      } else if (accessLevel === 'all') {
        sql = `SELECT id, title, content, category, priority,
                1 - (embedding <=> $1::vector) as similarity
         FROM patricia_memories
         WHERE embedding IS NOT NULL AND state = 'active' AND priority = ANY($3::text[])
           AND (visibility != 'sealed' OR person_id = $4)
         ORDER BY embedding <=> $1::vector LIMIT $2`;
        sqlParams = [vectorStr, limit, priorities, personId];
      } else {
        // own/group: global + own private/sealed, but exclude work categories for non-directors
        sql = `SELECT id, title, content, category, priority,
                1 - (embedding <=> $1::vector) as similarity
         FROM patricia_memories
         WHERE embedding IS NOT NULL AND state = 'active' AND priority = ANY($3::text[])
           AND (
             (visibility = 'global' AND category NOT IN ('decision', 'technical', 'project', 'product'))
             OR (visibility IN ('private', 'sealed') AND person_id = $4)
             ${accessLevel === 'group' ? "OR visibility = 'group'" : ''}
           )
         ORDER BY embedding <=> $1::vector LIMIT $2`;
        sqlParams = [vectorStr, limit, priorities, personId];
      }

      const results: any[] = await this.prisma.$queryRawUnsafe(sql, ...sqlParams);

      return results
        .filter((r) => r.similarity > 0.3)
        .map((r) => ({
          id: r.id,
          title: r.title,
          content: r.content,
          category: r.category,
          priority: r.priority,
        }));
    } catch (err) {
      this.logger.error(`Embedding search error: ${err.message}`);
      return this.searchByText(query, priorities, limit);
    }
  }

  private async searchByText(
    query: string,
    priorities: string[],
    limit: number,
  ): Promise<Array<{ id: string; title: string; content: string; category: string; priority: string }>> {
    const words = query
      .toLowerCase()
      .replace(/[^\w\sáàãâéêíóôõúç]/g, '')
      .split(/\s+/)
      .filter((w) => w.length > 2)
      .slice(0, 8);

    if (words.length === 0) return [];
    const tsquery = words.join(' | ');

    try {
      const results: any[] = await this.prisma.$queryRawUnsafe(
        `SELECT id, title, content, category, priority
         FROM patricia_memories
         WHERE search_text @@ to_tsquery('portuguese', $1)
           AND state = 'active'
           AND priority = ANY($3::text[])
         ORDER BY ts_rank(search_text, to_tsquery('portuguese', $1)) DESC
         LIMIT $2`,
        tsquery,
        limit,
        priorities,
      );
      return results;
    } catch {
      return [];
    }
  }

  // --- Access tracking ---

  private async trackAccess(ids: string[]): Promise<void> {
    if (ids.length === 0) return;
    try {
      await this.prisma.$executeRawUnsafe(
        `UPDATE patricia_memories
         SET access_count = access_count + 1,
             last_accessed_at = NOW()
         WHERE id = ANY($1::text[])`,
        ids,
      );
    } catch {}
  }

  // --- State machine: maintenance ---

  private async runMaintenance(): Promise<void> {
    const now = new Date();
    const expireBefore = new Date(now.getTime() - EXPIRE_DAYS * 24 * 60 * 60 * 1000);

    // 1. Promote: short_term with high access → long_term
    try {
      const promoted: any[] = await this.prisma.$queryRawUnsafe(
        `UPDATE patricia_memories
         SET priority = 'long_term', promoted_at = NOW()
         WHERE priority = 'short_term'
           AND state = 'active'
           AND access_count >= $1
         RETURNING id, title`,
        PROMOTE_THRESHOLD,
      );
      for (const p of promoted) {
        this.logger.log(`Memory promoted: ${p.title} (accessed ${PROMOTE_THRESHOLD}+ times)`);
      }
    } catch {}

    // 2. Expire: short_term older than 30 days with low access → archived
    try {
      const archived: any[] = await this.prisma.$queryRawUnsafe(
        `UPDATE patricia_memories
         SET state = 'archived'
         WHERE priority = 'short_term'
           AND state = 'active'
           AND updated_at < $1
           AND access_count < $2
         RETURNING id, title`,
        expireBefore,
        PROMOTE_THRESHOLD,
      );
      for (const a of archived) {
        this.logger.log(`Memory archived: ${a.title} (expired, low access)`);
      }
    } catch {}
  }

  // --- Save (with dedup by embedding similarity) ---

  async save(category: string, title: string, content: string, priority = 'short_term', personId?: string, visibility = 'global', sourceModel?: string): Promise<string> {
    const safePriority = priority === 'core' ? 'long_term' : priority;
    const text = `${title} ${content}`;

    let embedding: number[] | null = null;
    try {
      embedding = await this.generateEmbedding(text);
    } catch (err) {
      this.logger.warn(`Embedding generation failed: ${err.message}`);
    }

    // Dedup by embedding similarity
    if (embedding) {
      try {
        const vectorStr = `[${embedding.join(',')}]`;
        const similar: any[] = await this.prisma.$queryRawUnsafe(
          `SELECT id, title, 1 - (embedding <=> $1::vector) as similarity
           FROM patricia_memories
           WHERE embedding IS NOT NULL
             AND category = $2
             AND priority != 'core'
             AND state = 'active'
           ORDER BY embedding <=> $1::vector
           LIMIT 1`,
          vectorStr,
          category,
        );

        if (similar.length > 0 && similar[0].similarity > 0.7) {
          const vectorUpdate = `[${embedding.join(',')}]`;
          await this.prisma.$executeRawUnsafe(
            `UPDATE patricia_memories
             SET title = $1, content = $2, priority = $3, embedding = $4::vector,
                 state = 'active', updated_at = NOW()
             WHERE id = $5`,
            title, content, safePriority, vectorUpdate, similar[0].id,
          );
          this.logger.log(`Memory updated (${(similar[0].similarity * 100).toFixed(0)}% similar): [${safePriority}/${category}] ${title}`);
          return similar[0].id;
        }
      } catch {}
    }

    // Create new
    const mem = await this.prisma.patriciaMemory.create({
      data: { category, title, content, priority: safePriority, state: 'active', personId: personId || null, visibility, sourceModel: sourceModel || null },
    });

    if (embedding) {
      const vectorStr = `[${embedding.join(',')}]`;
      await this.prisma.$executeRawUnsafe(
        `UPDATE patricia_memories SET embedding = $1::vector WHERE id = $2`,
        vectorStr, mem.id,
      ).catch(() => {});
    }

    this.logger.log(`Memory created: [${safePriority}/${category}] ${title}`);
    return mem.id;
  }

  // --- Forget (archive, not delete) ---

  async forget(titleOrId: string, personId?: string | null, accessLevel = 'own'): Promise<boolean> {
    // Build ownership filter
    // directors (all): can forget any non-core memory
    // others (own): can only forget their own private memories
    const ownershipFilter: any = accessLevel === 'all'
      ? { priority: { not: 'core' } }
      : { priority: { not: 'core' }, personId: personId || 'NONE' };

    // Try by ID
    try {
      const mem = await this.prisma.patriciaMemory.findUnique({ where: { id: titleOrId } });
      if (mem && mem.priority !== 'core') {
        // Sealed: only the owner can forget, even directors can't
        if (mem.visibility === 'sealed' && mem.personId !== personId) {
          return false;
        }
        // Private/global: directors can forget, others only own
        if (mem.visibility !== 'sealed' && accessLevel !== 'all' && mem.personId !== personId) {
          return false;
        }
        await this.prisma.patriciaMemory.update({
          where: { id: titleOrId },
          data: { state: 'archived' },
        });
        return true;
      }
    } catch {}

    // Try by title (partial match, respecting ownership)
    const mem = await this.prisma.patriciaMemory.findFirst({
      where: {
        title: { contains: titleOrId, mode: 'insensitive' },
        state: 'active',
        ...ownershipFilter,
      },
    });
    if (mem) {
      await this.prisma.patriciaMemory.update({
        where: { id: mem.id },
        data: { state: 'archived' },
      });
      return true;
    }
    return false;
  }

  // --- Backfill embeddings ---

  async backfillEmbeddings(): Promise<number> {
    const memories: any[] = await this.prisma.$queryRawUnsafe(
      `SELECT id, title, content FROM patricia_memories WHERE embedding IS NULL AND state = 'active'`,
    );

    let count = 0;
    for (const mem of memories) {
      try {
        const embedding = await this.generateEmbedding(`${mem.title} ${mem.content}`);
        const vectorStr = `[${embedding.join(',')}]`;
        await this.prisma.$executeRawUnsafe(
          `UPDATE patricia_memories SET embedding = $1::vector WHERE id = $2`,
          vectorStr, mem.id,
        );
        count++;
      } catch (err) {
        this.logger.warn(`Backfill failed for ${mem.id}: ${err.message}`);
      }
    }
    return count;
  }

  // --- Stats ---

  async getStats(): Promise<any> {
    const stats: any[] = await this.prisma.$queryRawUnsafe(`
      SELECT priority, state, count(*)::int as count,
             avg(access_count)::int as avg_access,
             max(access_count)::int as max_access
      FROM patricia_memories
      GROUP BY priority, state
      ORDER BY priority, state
    `);
    return stats;
  }
}
