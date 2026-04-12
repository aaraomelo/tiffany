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

  async save(category: string, title: string, content: string, priority = 'short_term'): Promise<string> {
    // Patricia cannot save as core
    const safePriority = priority === 'core' ? 'long_term' : priority;

    const mem = await this.prisma.patriciaMemory.create({
      data: { category, title, content, priority: safePriority },
    });
    this.logger.log(`Memory saved: [${safePriority}/${category}] ${title}`);
    return mem.id;
  }
}
