import { Injectable, Logger } from '@nestjs/common';
import {
  getTenantContext,
  requireTenantId,
} from '../common/tenant-context/tenant-context';
import { EmbeddingService } from '../embedding/embedding.service';
import { PrismaService } from '../prisma/prisma.service';

interface SaveInput {
  title: string;
  content: string;
  category: string;
  priority?: 'core' | 'long_term' | 'short_term';
  visibility?: 'tenant_global' | 'private';
}

const PROMOTE_THRESHOLD = 5;
const EXPIRE_DAYS = 30;

@Injectable()
export class AssistantMemoryService {
  private readonly logger = new Logger(AssistantMemoryService.name);

  constructor(
    private readonly prisma: PrismaService,
    private readonly embedding: EmbeddingService,
  ) {}

  async save(input: SaveInput, sourceModel?: string) {
    const tenantId = requireTenantId();
    const { userId } = getTenantContext();

    const priority = input.priority ?? 'short_term';
    const safePriority = priority === 'core' ? 'long_term' : priority;
    const visibility = input.visibility ?? 'private';
    const effectiveUserId = visibility === 'private' ? userId : null;

    const vec = await this.embedding.embedQuery(
      `${input.title} ${input.content}`,
    );

    // Dedup por similaridade > 0.7
    if (vec) {
      const v = EmbeddingService.toVectorLiteral(vec);
      const similar = await this.prisma.$queryRawUnsafe<
        Array<{ id: string; similarity: number }>
      >(
        `SELECT id::text, 1 - (embedding <=> $1::vector) AS similarity
         FROM assistant_memories
         WHERE "tenantId" = $2::uuid
           AND embedding IS NOT NULL
           AND category = $3
           AND state = 'active'
         ORDER BY embedding <=> $1::vector LIMIT 1`,
        v, tenantId, input.category,
      );
      if (similar.length > 0 && Number(similar[0].similarity) > 0.7) {
        await this.prisma.$executeRawUnsafe(
          `UPDATE assistant_memories
           SET title=$1, content=$2, priority=$3, embedding=$4::vector,
               "embedding_model"=$5, "embedded_at"=NOW(), "updated_at"=NOW(),
               state='active'
           WHERE id=$6::uuid`,
          input.title, input.content, safePriority, v,
          this.embedding.model, similar[0].id,
        );
        return { id: similar[0].id, deduped: true };
      }
    }

    const mem = await this.prisma.assistantMemory.create({
      data: {
        tenantId,
        userId: effectiveUserId,
        category: input.category,
        priority: safePriority,
        visibility,
        title: input.title,
        content: input.content,
        sourceModel: sourceModel,
      },
    });
    if (vec) {
      const v = EmbeddingService.toVectorLiteral(vec);
      await this.prisma.$executeRawUnsafe(
        `UPDATE assistant_memories
         SET embedding=$1::vector, "embedding_model"=$2, "embedded_at"=NOW()
         WHERE id=$3::uuid`,
        v, this.embedding.model, mem.id,
      );
    }
    return { id: mem.id, deduped: false };
  }

  async forget(idOrTitle: string) {
    const tenantId = requireTenantId();
    const { userId } = getTenantContext();
    // Try by id first
    const mem = await this.prisma.assistantMemory.findFirst({
      where: { tenantId, id: idOrTitle, priority: { not: 'core' } },
    });
    if (mem) {
      // private só dono apaga
      if (mem.visibility === 'private' && mem.userId !== userId) return false;
      await this.prisma.assistantMemory.update({
        where: { id: mem.id },
        data: { state: 'archived' },
      });
      return true;
    }
    // By title (partial, case-insensitive)
    const byTitle = await this.prisma.assistantMemory.findFirst({
      where: {
        tenantId,
        title: { contains: idOrTitle, mode: 'insensitive' },
        state: 'active',
        priority: { not: 'core' },
        OR: [
          { visibility: 'tenant_global' },
          { visibility: 'private', userId: userId ?? undefined },
        ],
      },
    });
    if (!byTitle) return false;
    await this.prisma.assistantMemory.update({
      where: { id: byTitle.id },
      data: { state: 'archived' },
    });
    return true;
  }

  async list(limit = 100) {
    const tenantId = requireTenantId();
    const { userId } = getTenantContext();
    return this.prisma.assistantMemory.findMany({
      where: {
        tenantId,
        state: 'active',
        OR: [
          { visibility: 'tenant_global' },
          { visibility: 'private', userId: userId ?? undefined },
        ],
      },
      orderBy: [{ priority: 'asc' }, { updatedAt: 'desc' }],
      take: limit,
    });
  }

  /// Constrói o contexto de memória pra injetar no prompt.
  /// Inclui: todas as `core` + top-K relevantes (long_term + short_term)
  /// por similaridade ao texto da consulta. Filtra por visibility do usuário.
  async buildContext(query: string, memoryAccess = 'own', topK = 8): Promise<string> {
    const tenantId = requireTenantId();
    const { userId } = getTenantContext();

    // core sempre
    const core = await this.prisma.assistantMemory.findMany({
      where: {
        tenantId,
        priority: 'core',
        state: 'active',
        OR:
          memoryAccess === 'all'
            ? undefined
            : [
                { visibility: 'tenant_global' },
                { visibility: 'private', userId: userId ?? undefined },
              ],
      },
      orderBy: { category: 'asc' },
    });

    // relevantes
    const vec = await this.embedding.embedQuery(query);
    let relevant: Array<{ category: string; title: string; content: string; similarity: number }> = [];
    if (vec) {
      const v = EmbeddingService.toVectorLiteral(vec);
      const access = memoryAccess === 'all'
        ? `1=1`
        : `(visibility = 'tenant_global' OR (visibility = 'private' AND "userId" = $3::uuid))`;
      const sql = `SELECT id::text, category, title, content,
                          1 - (embedding <=> $1::vector) AS similarity
                   FROM assistant_memories
                   WHERE "tenantId" = $2::uuid
                     AND embedding IS NOT NULL
                     AND state = 'active'
                     AND priority IN ('long_term', 'short_term')
                     AND ${access}
                   ORDER BY embedding <=> $1::vector LIMIT $4`;
      const args = memoryAccess === 'all'
        ? [v, tenantId, topK]
        : [v, tenantId, userId ?? '00000000-0000-0000-0000-000000000000', topK];
      relevant = await this.prisma.$queryRawUnsafe(sql, ...args);
      relevant = relevant.filter((r) => Number(r.similarity) > 0.3);

      // Bump access count for retrieved
      if (relevant.length > 0) {
        const ids = (relevant as Array<{ id?: string } & typeof relevant[number]>).map((r) => r.id).filter(Boolean) as string[];
        if (ids.length > 0) {
          this.prisma.$executeRawUnsafe(
            `UPDATE assistant_memories SET access_count = access_count + 1, "last_accessed_at" = NOW() WHERE id = ANY($1::uuid[])`,
            ids,
          ).catch(() => {});
        }
      }
    }

    this.runMaintenance().catch(() => {});

    const parts: string[] = [];
    if (core.length > 0) {
      parts.push(
        '## Conhecimento base\n' +
          core.map((m) => `- **${m.title}**: ${m.content}`).join('\n'),
      );
    }
    if (relevant.length > 0) {
      parts.push(
        '## Memórias relevantes\n' +
          relevant.map((m) => `- [${m.category}] **${m.title}**: ${m.content}`).join('\n'),
      );
    }
    return parts.join('\n\n');
  }

  /// Job interno: promove short_term → long_term quando acessada ≥5x;
  /// arquiva short_term ≥30d com baixo acesso.
  private async runMaintenance() {
    try {
      await this.prisma.$executeRawUnsafe(
        `UPDATE assistant_memories
         SET priority='long_term', "promoted_at"=NOW()
         WHERE priority='short_term' AND state='active'
           AND access_count >= $1`,
        PROMOTE_THRESHOLD,
      );
      await this.prisma.$executeRawUnsafe(
        `UPDATE assistant_memories
         SET state='archived'
         WHERE priority='short_term' AND state='active'
           AND "updated_at" < NOW() - INTERVAL '${EXPIRE_DAYS} days'
           AND access_count < $1`,
        PROMOTE_THRESHOLD,
      );
    } catch (err) {
      this.logger.warn(`maintenance: ${(err as Error).message}`);
    }
  }
}
