import { Injectable, Logger, NotFoundException } from '@nestjs/common';
import { Prisma } from '@prisma/client';
import {
  getTenantContext,
  requireTenantId,
} from '../common/tenant-context/tenant-context';
import { EmbeddingService } from '../embedding/embedding.service';
import { PrismaService } from '../prisma/prisma.service';
import { withTenantScope } from '../prisma/prisma-rls-native';
import { CreateProductDto } from './dto/create-product.dto';
import { ListProductDto } from './dto/list-product.dto';
import { UpdateProductDto } from './dto/update-product.dto';

@Injectable()
export class ProductService {
  private readonly logger = new Logger(ProductService.name);

  constructor(
    private readonly prisma: PrismaService,
    private readonly embedding: EmbeddingService,
  ) {}

  async create(dto: CreateProductDto) {
    const product = await this.prisma.product.create({
      data: {
        tenantId: requireTenantId(),
        ...dto,
        costPrice: dto.costPrice ?? 0,
        salePrice: dto.salePrice ?? 0,
      },
    });
    void this.embedProduct(product.id);
    return product;
  }

  async list(dto: ListProductDto) {
    const tenantId = requireTenantId();
    const page = dto.page ?? 1;
    const pageSize = dto.pageSize ?? 20;

    const where: Prisma.ProductWhereInput = {
      tenantId,
      deletedAt: null,
      ...(typeof dto.active === 'boolean' ? { active: dto.active } : {}),
      ...(dto.brandId ? { brandId: dto.brandId } : {}),
      ...(dto.divisionId ? { divisionId: dto.divisionId } : {}),
      ...(dto.sectionId ? { sectionId: dto.sectionId } : {}),
      ...(dto.groupId ? { groupId: dto.groupId } : {}),
      ...(dto.subgroupId ? { subgroupId: dto.subgroupId } : {}),
      ...(dto.q
        ? {
            OR: [
              { name: { contains: dto.q, mode: 'insensitive' } },
              { sku: { contains: dto.q, mode: 'insensitive' } },
              { gtin: { contains: dto.q } },
            ],
          }
        : {}),
    };

    const [items, total] = await Promise.all([
      this.prisma.product.findMany({
        where,
        orderBy: { name: 'asc' },
        skip: (page - 1) * pageSize,
        take: pageSize,
        include: { brand: true, unit: true },
      }),
      this.prisma.product.count({ where }),
    ]);

    return { items, total, page, pageSize };
  }

  /// Busca semântica via BGE-M3 + pgvector. Fallback gracioso pra busca
  /// textual quando o daemon estiver fora.
  async semanticSearch(q: string, limit = 20) {
    const tenantId = requireTenantId();
    const vec = await this.embedding.embedQuery(q);
    if (!vec) {
      // fallback: busca textual
      return this.list({ q, page: 1, pageSize: limit });
    }
    const v = EmbeddingService.toVectorLiteral(vec);
    const rows = await withTenantScope(this.prisma, getTenantContext, (db) =>
      db.$queryRawUnsafe<Array<{ id: string; similarity: number }>>(
        `SELECT id::text, 1 - (embedding <=> $1::vector) AS similarity
       FROM "Product"
       WHERE "tenantId" = $2::uuid
         AND "deletedAt" IS NULL
         AND embedding IS NOT NULL
       ORDER BY embedding <=> $1::vector
       LIMIT $3`,
        v,
        tenantId,
        limit,
      ),
    );
    if (rows.length === 0) {
      return { items: [], total: 0, page: 1, pageSize: limit };
    }
    const products = await this.prisma.product.findMany({
      where: { id: { in: rows.map((r) => r.id) } },
      include: { brand: true, unit: true },
    });
    const byId = new Map(products.map((p) => [p.id, p]));
    const ordered = rows
      .map((r) => {
        const p = byId.get(r.id);
        return p ? { ...p, similarity: Number(r.similarity) } : null;
      })
      .filter(Boolean);
    return { items: ordered, total: ordered.length, page: 1, pageSize: limit };
  }

  async findOne(id: string) {
    const tenantId = requireTenantId();
    const entity = await this.prisma.product.findFirst({
      where: { id, tenantId, deletedAt: null },
      include: { brand: true, unit: true },
    });
    if (!entity) throw new NotFoundException('Produto não encontrado');
    return entity;
  }

  async update(id: string, dto: UpdateProductDto) {
    await this.findOne(id);
    const updated = await this.prisma.product.update({
      where: { id },
      data: dto,
    });
    void this.embedProduct(id);
    return updated;
  }

  async remove(id: string) {
    await this.findOne(id);
    await this.prisma.product.update({
      where: { id },
      data: { deletedAt: new Date() },
    });
    return { ok: true };
  }

  /// Gera embedding pro produto e persiste. Async, não bloqueia request.
  /// Texto = SKU + nome + descrição + marca + categoria (concatenado).
  private async embedProduct(productId: string): Promise<void> {
    try {
      const product = await this.prisma.product.findUnique({
        where: { id: productId },
        include: {
          brand: { select: { name: true } },
          division: { select: { name: true } },
          section: { select: { name: true } },
          group: { select: { name: true } },
          subgroup: { select: { name: true } },
        },
      });
      if (!product) return;

      const parts = [
        product.sku,
        product.name,
        product.description ?? '',
        product.brand?.name ?? '',
        product.division?.name ?? '',
        product.section?.name ?? '',
        product.group?.name ?? '',
        product.subgroup?.name ?? '',
        product.gtin ?? '',
      ].filter(Boolean);
      const text = parts.join(' · ');

      const vec = await this.embedding.embedQuery(text);
      if (!vec) return;
      const v = EmbeddingService.toVectorLiteral(vec);
      await this.prisma.$executeRawUnsafe(
        `UPDATE "Product"
         SET embedding = $1::vector,
             "embedding_model" = $2,
             "embedded_at" = NOW()
         WHERE id = $3::uuid`,
        v,
        this.embedding.model,
        productId,
      );
    } catch (err) {
      this.logger.warn(
        `embedProduct ${productId} falhou: ${(err as Error).message}`,
      );
    }
  }

  /// Backfill em batch — gera embedding para todos os produtos sem vetor.
  async backfillEmbeddings(): Promise<{ embedded: number }> {
    const tenantId = requireTenantId();
    const pending = await this.prisma.$queryRawUnsafe<Array<{ id: string }>>(
      `SELECT id::text FROM "Product"
       WHERE "tenantId" = $1::uuid
         AND "deletedAt" IS NULL
         AND embedding IS NULL
       LIMIT 500`,
      tenantId,
    );
    let count = 0;
    for (const p of pending) {
      await this.embedProduct(p.id);
      count++;
    }
    return { embedded: count };
  }
}
