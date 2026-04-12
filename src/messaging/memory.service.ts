import { Injectable, Logger } from '@nestjs/common';
import { PrismaService } from '../prisma.service';

@Injectable()
export class MemoryService {
  private readonly logger = new Logger('Memory');

  constructor(private prisma: PrismaService) {}

  async getContext(query: string): Promise<string> {
    // 1. Always load core memories
    const core = await this.prisma.patriciaMemory.findMany({
      where: { priority: 'core' },
      orderBy: { category: 'asc' },
    });

    // 2. Search long_term + short_term by relevance
    const relevant = await this.search(query, ['long_term', 'short_term'], 5);

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

  async search(
    query: string,
    priorities: string[] = ['long_term', 'short_term'],
    limit = 5,
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
        `SELECT title, content, category, priority,
                ts_rank(search_text, to_tsquery('portuguese', $1)) as rank
         FROM patricia_memories
         WHERE search_text @@ to_tsquery('portuguese', $1)
           AND priority = ANY($3::text[])
         ORDER BY rank DESC
         LIMIT $2`,
        tsquery,
        limit,
        priorities,
      );
      return results.map((r) => ({
        title: r.title,
        content: r.content,
        category: r.category,
        priority: r.priority,
      }));
    } catch (err) {
      this.logger.error(`Search error: ${err.message}`);
      return [];
    }
  }

  // Item 1: Upsert — find similar by category+title, update if exists
  async save(category: string, title: string, content: string, priority = 'short_term'): Promise<string> {
    const safePriority = priority === 'core' ? 'long_term' : priority;

    // Search for existing memory with same category and similar title
    const existing = await this.prisma.patriciaMemory.findFirst({
      where: { category, title: { contains: title.substring(0, 20), mode: 'insensitive' } },
    });

    if (existing) {
      // Update existing instead of creating duplicate
      await this.prisma.patriciaMemory.update({
        where: { id: existing.id },
        data: { content, priority: safePriority, title },
      });
      this.logger.log(`Memory updated: [${safePriority}/${category}] ${title} (was: ${existing.title})`);
      return existing.id;
    }

    // Also check by tsvector similarity — catch "Foco atual" vs "Foco da empresa"
    const words = title.toLowerCase().replace(/[^\w\sáàãâéêíóôõúç]/g, '').split(/\s+/).filter(w => w.length > 2);
    if (words.length > 0) {
      const tsquery = words.join(' & ');
      try {
        const similar: any[] = await this.prisma.$queryRawUnsafe(
          `SELECT id, title, ts_rank(search_text, to_tsquery('portuguese', $1)) as rank
           FROM patricia_memories
           WHERE search_text @@ to_tsquery('portuguese', $1)
             AND category = $2
             AND priority != 'core'
           ORDER BY rank DESC
           LIMIT 1`,
          tsquery,
          category,
        );
        if (similar.length > 0 && similar[0].rank > 0.3) {
          await this.prisma.patriciaMemory.update({
            where: { id: similar[0].id },
            data: { content, priority: safePriority, title },
          });
          this.logger.log(`Memory replaced: [${safePriority}/${category}] ${title} (was: ${similar[0].title})`);
          return similar[0].id;
        }
      } catch {}
    }

    const mem = await this.prisma.patriciaMemory.create({
      data: { category, title, content, priority: safePriority },
    });
    this.logger.log(`Memory created: [${safePriority}/${category}] ${title}`);
    return mem.id;
  }

  // Item 3: Update and forget
  async update(id: string, content: string): Promise<void> {
    await this.prisma.patriciaMemory.update({
      where: { id },
      data: { content },
    });
  }

  async forget(titleOrId: string): Promise<boolean> {
    // Try by ID first
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
}
