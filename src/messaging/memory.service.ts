import { Injectable, Logger } from '@nestjs/common';
import { PrismaService } from '../prisma.service';

@Injectable()
export class MemoryService {
  private readonly logger = new Logger('Memory');

  constructor(private prisma: PrismaService) {}

  async search(query: string, limit = 5): Promise<Array<{ title: string; content: string; category: string }>> {
    // Normalize query for PostgreSQL tsquery
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
        `SELECT title, content, category,
                ts_rank(search_text, to_tsquery('portuguese', $1)) as rank
         FROM patricia_memories
         WHERE search_text @@ to_tsquery('portuguese', $1)
         ORDER BY rank DESC
         LIMIT $2`,
        tsquery,
        limit,
      );
      return results.map((r) => ({ title: r.title, content: r.content, category: r.category }));
    } catch (err) {
      this.logger.error(`Search error: ${err.message}`);
      return [];
    }
  }

  async save(category: string, title: string, content: string): Promise<string> {
    const mem = await this.prisma.patriciaMemory.create({
      data: { category, title, content },
    });
    return mem.id;
  }

  async listByCategory(category: string) {
    return this.prisma.patriciaMemory.findMany({
      where: { category },
      orderBy: { updatedAt: 'desc' },
    });
  }
}
