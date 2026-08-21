import { Injectable, NotFoundException } from '@nestjs/common';
import { PrismaService } from '../prisma.service';

const GEMINI_API_KEY = process.env.GEMINI_API_KEY;

const contactSelect = {
  channelType: true,
  remoteId: true,
  displayName: true,
  phone: true,
};

@Injectable()
export class PeopleService {
  constructor(private readonly prisma: PrismaService) {}

  async search(q: string, tenantId?: string) {
    const tenantFilter = tenantId ? { tenantId } : {};

    // 1. Exact/partial match (contains)
    let results = await this.prisma.person.findMany({
      where: {
        ...tenantFilter,
        OR: [
          { name: { contains: q, mode: 'insensitive' } },
          { email: { contains: q, mode: 'insensitive' } },
          { phone: { contains: q, mode: 'insensitive' } },
        ],
      },
      include: { contacts: { select: contactSelect }, profile: true },
      orderBy: { name: 'asc' },
    });

    if (results.length > 0) return results;

    // 2. Fuzzy match (trigram similarity) — handles typos and nicknames
    try {
      const fuzzy: any[] = await this.prisma.$queryRawUnsafe(
        `SELECT id, name, similarity(name, $1) as sim
         FROM people
         WHERE similarity(name, $1) > 0.2
         ORDER BY sim DESC
         LIMIT 5`,
        q,
      );
      if (fuzzy.length > 0) {
        const ids = fuzzy.map((f) => f.id);
        results = await this.prisma.person.findMany({
          where: { id: { in: ids }, ...tenantFilter },
          include: { contacts: { select: contactSelect }, profile: true },
        });
        if (results.length > 0) return results;
      }
    } catch {}

    // 3. Semantic: search people by description embedding
    if (GEMINI_API_KEY) {
      try {
        const embedding = await this.generateEmbedding(q);
        const vectorStr = `[${embedding.join(',')}]`;

        // 3a. Search people.embedding (name + description)
        const peopleMatch: any[] = await this.prisma.$queryRawUnsafe(
          `SELECT id, name, 1 - (embedding <=> $1::vector) as similarity
           FROM people
           WHERE embedding IS NOT NULL
           ORDER BY embedding <=> $1::vector
           LIMIT 3`,
          vectorStr,
        );
        if (peopleMatch.length > 0 && peopleMatch[0].similarity > 0.3) {
          const ids = peopleMatch.map((m) => m.id);
          results = await this.prisma.person.findMany({
            where: { id: { in: ids }, ...tenantFilter },
            include: { contacts: { select: contactSelect }, profile: true },
          });
          if (results.length > 0) return results;
        }

        // 3b. Search memories linked to people
        const memoryMatch: any[] = await this.prisma.$queryRawUnsafe(
          `SELECT DISTINCT pm.person_id, p.name,
                  1 - (pm.embedding <=> $1::vector) as similarity
           FROM patricia_memories pm
           JOIN people p ON p.id = pm.person_id
           WHERE pm.embedding IS NOT NULL
             AND pm.person_id IS NOT NULL
             AND pm.state = 'active'
           ORDER BY pm.embedding <=> $1::vector
           LIMIT 3`,
          vectorStr,
        );
        if (memoryMatch.length > 0 && memoryMatch[0].similarity > 0.3) {
          const ids = memoryMatch.map((m) => m.person_id);
          results = await this.prisma.person.findMany({
            where: { id: { in: ids }, ...tenantFilter },
            include: { contacts: { select: contactSelect }, profile: true },
          });
          if (results.length > 0) return results;
        }
      } catch {}
    }

    return [];
  }

  async findById(id: string, tenantId: string) {
    const person = await this.prisma.person.findFirst({
      where: { id, tenantId },
      include: { contacts: { select: contactSelect }, profile: true },
    });
    if (!person) throw new NotFoundException(`Person ${id} not found`);
    return person;
  }

  private async generateEmbedding(text: string): Promise<number[]> {
    const res = await fetch(
      `https://generativelanguage.googleapis.com/v1beta/models/gemini-embedding-001:embedContent?key=${GEMINI_API_KEY}`,
      {
        method: 'POST',
        headers: { 'Content-Type': 'application/json' },
        body: JSON.stringify({ content: { parts: [{ text }] }, outputDimensionality: 768 }),
        signal: AbortSignal.timeout(10_000),
      },
    );
    if (!res.ok) throw new Error(`Gemini ${res.status}`);
    const data = await res.json();
    return data.embedding.values;
  }
}
