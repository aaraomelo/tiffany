import { Injectable, Logger } from '@nestjs/common';
import { PrismaService } from '../prisma.service';

const GEMINI_API_KEY = process.env.GEMINI_API_KEY;

@Injectable()
export class MemoryService {
  private readonly logger = new Logger('Memory');

  constructor(private prisma: PrismaService) {}

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

  async getContext(query: string): Promise<string> {
    // 1. Always load core memories
    const core = await this.prisma.patriciaMemory.findMany({
      where: { priority: 'core' },
      orderBy: { category: 'asc' },
    });

    // 2. Search long_term + short_term by semantic similarity
    const relevant = await this.searchByEmbedding(query, ['long_term', 'short_term'], 5);

    // 3. Cleanup expired short_term (>30 days)
    const thirtyDaysAgo = new Date(Date.now() - 30 * 24 * 60 * 60 * 1000);
    await this.prisma.patriciaMemory.deleteMany({
      where: { priority: 'short_term', updatedAt: { lt: thirtyDaysAgo } },
    }).catch(() => {});

    // Format
    const parts: string[] = [];

    if (core.length > 0) {
      parts.push('## Conhecimento base\n' + core.map((m) => `**${m.title}:** ${m.content}`).join('\n'));
    }

    if (relevant.length > 0) {
      parts.push('## Memórias relevantes\n' + relevant.map((m) => `**[${m.category}] ${m.title}:** ${m.content}`).join('\n'));
    }

    return parts.join('\n\n');
  }

  async searchByEmbedding(
    query: string,
    priorities: string[] = ['long_term', 'short_term'],
    limit = 5,
  ): Promise<Array<{ title: string; content: string; category: string; priority: string }>> {
    try {
      const embedding = await this.generateEmbedding(query);
      const vectorStr = `[${embedding.join(',')}]`;

      const results: any[] = await this.prisma.$queryRawUnsafe(
        `SELECT title, content, category, priority,
                1 - (embedding <=> $1::vector) as similarity
         FROM patricia_memories
         WHERE embedding IS NOT NULL
           AND priority = ANY($3::text[])
         ORDER BY embedding <=> $1::vector
         LIMIT $2`,
        vectorStr,
        limit,
        priorities,
      );

      // Filter by minimum similarity (0.3)
      return results
        .filter((r) => r.similarity > 0.3)
        .map((r) => ({
          title: r.title,
          content: r.content,
          category: r.category,
          priority: r.priority,
        }));
    } catch (err) {
      this.logger.error(`Embedding search error: ${err.message}`);
      // Fallback to tsvector if embedding fails
      return this.searchByText(query, priorities, limit);
    }
  }

  // Fallback: tsvector search (when embedding API is unavailable)
  private async searchByText(
    query: string,
    priorities: string[],
    limit: number,
  ): Promise<Array<{ title: string; content: string; category: string; priority: string }>> {
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
        `SELECT title, content, category, priority
         FROM patricia_memories
         WHERE search_text @@ to_tsquery('portuguese', $1)
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

  async save(category: string, title: string, content: string, priority = 'short_term'): Promise<string> {
    const safePriority = priority === 'core' ? 'long_term' : priority;
    const text = `${title} ${content}`;

    // Generate embedding
    let embedding: number[] | null = null;
    try {
      embedding = await this.generateEmbedding(text);
    } catch (err) {
      this.logger.warn(`Embedding generation failed: ${err.message}`);
    }

    // Search for similar existing memory by embedding
    if (embedding) {
      try {
        const vectorStr = `[${embedding.join(',')}]`;
        const similar: any[] = await this.prisma.$queryRawUnsafe(
          `SELECT id, title, 1 - (embedding <=> $1::vector) as similarity
           FROM patricia_memories
           WHERE embedding IS NOT NULL
             AND category = $2
             AND priority != 'core'
           ORDER BY embedding <=> $1::vector
           LIMIT 1`,
          vectorStr,
          category,
        );

        if (similar.length > 0 && similar[0].similarity > 0.7) {
          // Update existing — content is similar enough
          const vectorUpdate = `[${embedding.join(',')}]`;
          await this.prisma.$executeRawUnsafe(
            `UPDATE patricia_memories SET title = $1, content = $2, priority = $3, embedding = $4::vector, updated_at = NOW() WHERE id = $5`,
            title, content, safePriority, vectorUpdate, similar[0].id,
          );
          this.logger.log(`Memory updated (${(similar[0].similarity * 100).toFixed(0)}% similar): [${safePriority}/${category}] ${title}`);
          return similar[0].id;
        }
      } catch {}
    }

    // Create new memory with embedding
    const mem = await this.prisma.patriciaMemory.create({
      data: { category, title, content, priority: safePriority },
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

  async forget(titleOrId: string): Promise<boolean> {
    // Try by ID
    try {
      await this.prisma.patriciaMemory.delete({ where: { id: titleOrId } });
      return true;
    } catch {}

    // Try by title (partial match, not core)
    const mem = await this.prisma.patriciaMemory.findFirst({
      where: {
        title: { contains: titleOrId, mode: 'insensitive' },
        priority: { not: 'core' },
      },
    });
    if (mem) {
      await this.prisma.patriciaMemory.delete({ where: { id: mem.id } });
      return true;
    }
    return false;
  }

  // Generate embeddings for existing memories that don't have one
  async backfillEmbeddings(): Promise<number> {
    const memories = await this.prisma.$queryRawUnsafe(
      `SELECT id, title, content FROM patricia_memories WHERE embedding IS NULL`,
    ) as any[];

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
}
