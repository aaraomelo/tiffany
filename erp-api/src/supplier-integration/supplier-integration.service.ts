import {
  BadRequestException,
  Injectable,
  Logger,
  NotFoundException,
} from '@nestjs/common';
import { Prisma } from '@prisma/client';
import {
  getTenantContext,
  requireTenantId,
  tenantContextStorage,
} from '../common/tenant-context/tenant-context';
import { EmbeddingService } from '../embedding/embedding.service';
import { PrismaService } from '../prisma/prisma.service';
import { withTenantScope } from '../prisma/prisma-rls-native';
import { decryptSecret, encryptSecret, maskSecret } from './crypto.util';
import {
  CreateSupplierAccountDto,
  ImportProductsDto,
  ListSupplierProductsDto,
  PlaceSupplierOrderDto,
  UpdateSupplierAccountDto,
} from './dto/supplier.dto';
import {
  NuvemshopAdapter,
  NuvemshopProduct,
  SessionExpiredError,
} from './nuvemshop.adapter';

@Injectable()
export class SupplierIntegrationService {
  private readonly logger = new Logger(SupplierIntegrationService.name);
  /// IDs de contas com sync em andamento (background). Singleton em memória.
  private readonly syncing = new Set<string>();

  constructor(
    private readonly prisma: PrismaService,
    private readonly adapter: NuvemshopAdapter,
    private readonly embedding: EmbeddingService,
  ) {}

  // ---------------------------------------------------------------------------
  // Contas
  // ---------------------------------------------------------------------------

  async createAccount(dto: CreateSupplierAccountDto) {
    const tenantId = requireTenantId();
    if (dto.consent === false) {
      throw new BadRequestException(
        'É necessário confirmar que você tem autorização para acessar esta loja.',
      );
    }
    const account = await this.prisma.supplierAccount.create({
      data: {
        tenantId,
        baseUrl: dto.baseUrl.replace(/\/+$/, ''),
        label: dto.label ?? null,
        loginEmail: dto.loginEmail,
        passwordEnc: encryptSecret(dto.password),
        customerSupplierId: dto.customerSupplierId ?? null,
        defaultMarkupPct: dto.defaultMarkupPct ?? null,
      },
    });
    return this.toSafeAccount(account);
  }

  async listAccounts() {
    const tenantId = requireTenantId();
    const items = await this.prisma.supplierAccount.findMany({
      where: { tenantId },
      orderBy: { createdAt: 'desc' },
      include: { _count: { select: { products: true } } },
    });
    return items.map((a) => this.toSafeAccount(a));
  }

  async getAccount(id: string) {
    const account = await this.findAccount(id);
    return this.toSafeAccount(account);
  }

  async updateAccount(id: string, dto: UpdateSupplierAccountDto) {
    await this.findAccount(id);
    const data: Prisma.SupplierAccountUpdateInput = {};
    if (dto.label !== undefined) data.label = dto.label;
    if (dto.loginEmail !== undefined) data.loginEmail = dto.loginEmail;
    if (dto.password) data.passwordEnc = encryptSecret(dto.password);
    if (dto.defaultMarkupPct !== undefined)
      data.defaultMarkupPct = dto.defaultMarkupPct;
    if (dto.active !== undefined) data.active = dto.active;
    if (dto.customerSupplierId !== undefined) {
      data.customerSupplier = dto.customerSupplierId
        ? { connect: { id: dto.customerSupplierId } }
        : { disconnect: true };
    }
    // Trocar senha invalida a sessão cacheada.
    if (dto.password) {
      data.sessionCookieEnc = null;
      data.sessionExpiresAt = null;
    }
    const updated = await this.prisma.supplierAccount.update({
      where: { id },
      data,
    });
    return this.toSafeAccount(updated);
  }

  async removeAccount(id: string) {
    await this.findAccount(id);
    await this.prisma.supplierAccount.update({
      where: { id },
      data: { active: false },
    });
    return { ok: true };
  }

  async testLogin(id: string) {
    const account = await this.findAccount(id);
    const password = decryptSecret(account.passwordEnc);
    if (!password) {
      throw new BadRequestException('Credencial corrompida — recadastre a senha.');
    }
    try {
      const { cookie, expiresAt } = await this.adapter.login(
        account.baseUrl,
        account.loginEmail,
        password,
      );
      await this.prisma.supplierAccount.update({
        where: { id },
        data: {
          sessionCookieEnc: encryptSecret(cookie),
          sessionExpiresAt: expiresAt,
          lastError: null,
        },
      });
      return { ok: true, message: 'Login bem-sucedido', stub: this.adapter.isStub };
    } catch (err) {
      const message = sanitizeError((err as Error).message);
      await this.prisma.supplierAccount.update({
        where: { id },
        data: { lastError: message },
      });
      return { ok: false, message };
    }
  }

  // ---------------------------------------------------------------------------
  // Sincronização de catálogo (público — não exige login)
  // ---------------------------------------------------------------------------

  /// Dispara o sync em background (pode levar minutos via FlareSolverr) e
  /// retorna na hora. O sweep roda fora do request → re-cria o contexto do tenant.
  async startSync(id: string) {
    const account = await this.findAccount(id);
    if (this.syncing.has(id)) {
      return { ok: true, started: false, message: 'Sincronização já em andamento' };
    }
    const ctx = getTenantContext();
    const runCtx = {
      tenantId: account.tenantId,
      userId: ctx.userId ?? null,
      role: ctx.role ?? null,
    };
    this.syncing.add(id);
    void tenantContextStorage
      .run(runCtx, () => this.syncCatalog(id))
      .catch((e) =>
        this.logger.error(`sync background ${id} falhou: ${(e as Error).message}`),
      )
      .finally(() => this.syncing.delete(id));
    return { ok: true, started: true, message: 'Sincronização iniciada' };
  }

  async syncCatalog(id: string) {
    const account = await this.findAccount(id);
    const tenantId = account.tenantId;
    const startedAt = Date.now();
    let variantCount = 0;
    let productCount = 0;

    let failed = 0;
    try {
      const urls = await this.adapter.listProductUrls(account.baseUrl);
      this.logger.log(`sync ${account.baseUrl}: ${urls.length} produtos a buscar`);
      // Via FlareSolverr cada página passa por um browser real (lento e serial).
      // Grava INCREMENTALMENTE (cada produto assim que chega): progresso visível
      // e resiliente a falha no meio do sweep. Concorrência baixa + tolerância.
      await this.adapter.mapWithConcurrency(
        urls,
        2,
        async (url) => {
          let product;
          try {
            product = await this.adapter.fetchProduct(account.baseUrl, url);
          } catch (e) {
            failed++;
            this.logger.warn(`produto ${url} falhou: ${(e as Error).message}`);
            return;
          }
          if (product.variants.length === 0) return;
          productCount++;
          variantCount += await this.upsertSupplierProduct(tenantId, id, product);
        },
        300,
      );

      await this.prisma.supplierAccount.update({
        where: { id },
        data: { lastSyncAt: new Date(), lastError: null },
      });
      await this.logSync(
        tenantId,
        variantCount,
        true,
        failed > 0 ? `${failed} produto(s) falharam` : null,
      );

      // Embeddings (fail-soft, não bloqueia o resultado).
      void this.embedNewProducts(id);

      return {
        ok: true,
        products: productCount,
        variants: variantCount,
        failed,
        durationMs: Date.now() - startedAt,
        stub: this.adapter.isStub,
      };
    } catch (err) {
      const message = sanitizeError((err as Error).message);
      await this.prisma.supplierAccount.update({
        where: { id },
        data: { lastError: message },
      });
      await this.logSync(tenantId, variantCount, false, message);
      throw new BadRequestException(`Sincronização falhou: ${message}`);
    }
  }

  async listProducts(accountId: string, dto: ListSupplierProductsDto) {
    const tenantId = requireTenantId();
    await this.findAccount(accountId);
    const page = dto.page ?? 1;
    const pageSize = dto.pageSize ?? 50;
    const where: Prisma.SupplierProductWhereInput = {
      tenantId,
      supplierAccountId: accountId,
      ...(dto.onlyAvailable ? { available: true } : {}),
      ...(dto.q
        ? {
            OR: [
              { name: { contains: dto.q, mode: 'insensitive' } },
              { sku: { contains: dto.q, mode: 'insensitive' } },
              { gtin: { contains: dto.q } },
              { option0: { contains: dto.q, mode: 'insensitive' } },
            ],
          }
        : {}),
    };
    const [items, total] = await Promise.all([
      this.prisma.supplierProduct.findMany({
        where,
        orderBy: [{ name: 'asc' }, { option1: 'asc' }],
        skip: (page - 1) * pageSize,
        take: pageSize,
      }),
      this.prisma.supplierProduct.count({ where }),
    ]);
    return { items, total, page, pageSize };
  }

  // ---------------------------------------------------------------------------
  // Vínculo / importação como Product local
  // ---------------------------------------------------------------------------

  async linkToLocalProduct(supplierProductId: string, productId: string) {
    const tenantId = requireTenantId();
    const sp = await this.prisma.supplierProduct.findFirst({
      where: { id: supplierProductId, tenantId },
    });
    if (!sp) throw new NotFoundException('Produto de fornecedor não encontrado');
    const product = await this.prisma.product.findFirst({
      where: { id: productId, tenantId, deletedAt: null },
    });
    if (!product) throw new NotFoundException('Produto local não encontrado');
    return this.prisma.supplierProduct.update({
      where: { id: supplierProductId },
      data: { linkedProductId: productId },
    });
  }

  async importAsProducts(accountId: string, dto: ImportProductsDto) {
    const tenantId = requireTenantId();
    const account = await this.findAccount(accountId);
    const markupPct =
      dto.markupPct ?? Number(account.defaultMarkupPct ?? 0) ?? 0;

    const unitId = await this.resolveDefaultUnit(tenantId);

    const where: Prisma.SupplierProductWhereInput = {
      tenantId,
      supplierAccountId: accountId,
      ...(dto.supplierProductIds?.length
        ? { id: { in: dto.supplierProductIds } }
        : {}),
    };
    const supplierProducts = await this.prisma.supplierProduct.findMany({
      where,
    });

    // Agrupa por produto do fornecedor (externalProductId): 1 Product + N variações.
    const groups = new Map<string, typeof supplierProducts>();
    for (const sp of supplierProducts) {
      const arr = groups.get(sp.externalProductId);
      if (arr) arr.push(sp);
      else groups.set(sp.externalProductId, [sp]);
    }

    let productsCreated = 0;
    let variantsCreated = 0;
    let linked = 0;
    let skipped = 0;
    const errors: string[] = [];

    for (const items of groups.values()) {
      try {
        const costs = items.map((i) => Number(i.price));
        const baseCost = Math.min(...costs);
        const baseSale = round2(baseCost * (1 + markupPct / 100));
        const photoUrl = items.find((i) => i.imageUrl)?.imageUrl ?? null;
        const hasVariants =
          items.length > 1 ||
          items.some((i) => i.option0 || i.option1 || i.option2);

        // Pai: reusa o Product já vinculado (idempotente), senão cria.
        const linkedProductId =
          items.map((i) => i.linkedProductId).find(Boolean) ?? null;
        let product;
        if (linkedProductId) {
          product = await this.prisma.product.update({
            where: { id: linkedProductId },
            data: {
              costPrice: baseCost,
              salePrice: baseSale,
              hasVariants,
              ...(photoUrl ? { photoUrl } : {}),
            },
          });
          linked++;
        } else {
          product = await this.prisma.product.create({
            data: {
              tenantId,
              sku: `FORN-P-${items[0].externalProductId}`,
              name: items[0].name,
              unitId,
              costPrice: baseCost,
              salePrice: baseSale,
              hasVariants,
              photoUrl,
            },
          });
          productsCreated++;
        }

        // Variações (subprodutos): upsert por variação, idempotente via linkedVariantId.
        for (const sp of items) {
          const cost = Number(sp.price);
          const sale = round2(cost * (1 + markupPct / 100));
          if (sp.linkedVariantId) {
            await this.prisma.productVariant.update({
              where: { id: sp.linkedVariantId },
              data: {
                costPrice: cost,
                salePrice: sale,
                option0: sp.option0,
                option1: sp.option1,
                option2: sp.option2,
                gtin: sp.gtin,
                ...(sp.imageUrl ? { photoUrl: sp.imageUrl } : {}),
              },
            });
          } else {
            const variant = await this.prisma.productVariant.create({
              data: {
                tenantId,
                productId: product.id,
                sku: `FORN-${sp.externalVariantId}`,
                gtin: sp.gtin,
                option0: sp.option0,
                option1: sp.option1,
                option2: sp.option2,
                costPrice: cost,
                salePrice: sale,
                photoUrl: sp.imageUrl,
              },
            });
            await this.prisma.supplierProduct.update({
              where: { id: sp.id },
              data: { linkedProductId: product.id, linkedVariantId: variant.id },
            });
            variantsCreated++;
          }
        }
      } catch (err) {
        if (
          err instanceof Prisma.PrismaClientKnownRequestError &&
          err.code === 'P2002'
        ) {
          skipped++;
        } else {
          errors.push(sanitizeError((err as Error).message));
        }
      }
    }

    return {
      productsCreated,
      variantsCreated,
      linked,
      skipped,
      errors,
      markupPct,
    };
  }

  // ---------------------------------------------------------------------------
  // Pedidos de reposição (dry-run)
  // ---------------------------------------------------------------------------

  async listOrders(accountId: string) {
    const tenantId = requireTenantId();
    await this.findAccount(accountId);
    return this.prisma.supplierOrder.findMany({
      where: { tenantId, supplierAccountId: accountId },
      orderBy: { createdAt: 'desc' },
      include: { items: true },
    });
  }

  async placeReplenishmentOrder(accountId: string, dto: PlaceSupplierOrderDto) {
    const tenantId = requireTenantId();
    const account = await this.findAccount(accountId);
    const ctx = getTenantContext();

    // Resolve preço atual de cada variação a partir do cache.
    const variants = await this.prisma.supplierProduct.findMany({
      where: {
        tenantId,
        supplierAccountId: accountId,
        externalVariantId: { in: dto.items.map((i) => i.externalVariantId) },
      },
    });
    const priceByVariant = new Map(
      variants.map((v) => [v.externalVariantId, Number(v.price)]),
    );

    const items = dto.items.map((i) => ({
      externalVariantId: i.externalVariantId,
      qty: i.qty,
      unitPrice: priceByVariant.get(i.externalVariantId) ?? 0,
    }));

    let cookie = '';
    if (!this.adapter.isStub) {
      cookie = await this.ensureSession(account.id);
    }

    try {
      const preview = await this.adapter.placeOrder(
        account.baseUrl,
        cookie,
        items,
        { submit: false }, // Fase A/B1: sempre dry-run.
      );

      const order = await this.prisma.supplierOrder.create({
        data: {
          tenantId,
          supplierAccountId: accountId,
          status: 'DRY_RUN',
          dryRun: true,
          totalAmount: preview.total,
          requestPayload: dto as unknown as Prisma.InputJsonValue,
          responsePayload: preview as unknown as Prisma.InputJsonValue,
          placedByUserId: ctx.userId ?? null,
          items: {
            create: preview.items.map((it) => ({
              tenantId,
              externalVariantId: it.externalVariantId,
              qty: it.qty,
              unitPrice: it.unitPrice,
              lineTotal: it.lineTotal,
            })),
          },
        },
        include: { items: true },
      });
      return order;
    } catch (err) {
      if (err instanceof SessionExpiredError) {
        throw new BadRequestException(
          'Sessão expirou. Rode "Testar login" novamente.',
        );
      }
      throw new BadRequestException(sanitizeError((err as Error).message));
    }
  }

  // ---------------------------------------------------------------------------
  // Internos
  // ---------------------------------------------------------------------------

  private async findAccount(id: string) {
    const tenantId = requireTenantId();
    const account = await this.prisma.supplierAccount.findFirst({
      where: { id, tenantId },
    });
    if (!account) throw new NotFoundException('Fornecedor não encontrado');
    return account;
  }

  /// Garante uma sessão válida (re-loga se ausente/expirada). Retorna o cookie.
  private async ensureSession(id: string): Promise<string> {
    const account = await this.findAccount(id);
    const valid =
      account.sessionCookieEnc &&
      account.sessionExpiresAt &&
      account.sessionExpiresAt.getTime() > Date.now();
    if (valid) {
      const cookie = decryptSecret(account.sessionCookieEnc);
      if (cookie) return cookie;
    }
    const password = decryptSecret(account.passwordEnc);
    if (!password) {
      throw new BadRequestException('Credencial corrompida — recadastre a senha.');
    }
    const { cookie, expiresAt } = await this.adapter.login(
      account.baseUrl,
      account.loginEmail,
      password,
    );
    await this.prisma.supplierAccount.update({
      where: { id },
      data: {
        sessionCookieEnc: encryptSecret(cookie),
        sessionExpiresAt: expiresAt,
      },
    });
    return cookie;
  }

  /// Resolve (ou cria) uma unidade padrão "UN" pro tenant — Product.unitId é obrigatório.
  private async resolveDefaultUnit(tenantId: string): Promise<string> {
    const existing = await this.prisma.unit.findFirst({
      where: { tenantId },
      orderBy: { createdAt: 'asc' },
    });
    if (existing) return existing.id;
    const unit = await this.prisma.unit.create({
      data: { tenantId, code: 'UN', description: 'Unidade' },
    });
    return unit.id;
  }

  /// Upsert de todas as variações de um produto. Retorna quantas gravou.
  private async upsertSupplierProduct(
    tenantId: string,
    supplierAccountId: string,
    product: NuvemshopProduct,
  ): Promise<number> {
    for (const v of product.variants) {
      await this.prisma.supplierProduct.upsert({
        where: {
          tenantId_supplierAccountId_externalVariantId: {
            tenantId,
            supplierAccountId,
            externalVariantId: v.externalVariantId,
          },
        },
        create: {
          tenantId,
          supplierAccountId,
          externalProductId: v.externalProductId,
          externalVariantId: v.externalVariantId,
          productUrl: product.url,
          sku: v.sku,
          gtin: v.gtin,
          name: product.name,
          option0: v.option0,
          option1: v.option1,
          option2: v.option2,
          price: v.price,
          pixPrice: parsePix(v.pixPrice),
          stock: v.stock,
          available: v.available,
          imageUrl: v.imageUrl,
        },
        update: {
          name: product.name,
          productUrl: product.url,
          sku: v.sku,
          gtin: v.gtin,
          option0: v.option0,
          option1: v.option1,
          option2: v.option2,
          price: v.price,
          pixPrice: parsePix(v.pixPrice),
          stock: v.stock,
          available: v.available,
          imageUrl: v.imageUrl,
          syncedAt: new Date(),
        },
      });
    }
    return product.variants.length;
  }

  private async logSync(
    tenantId: string,
    count: number,
    success: boolean,
    message: string | null,
  ) {
    await this.prisma.catalogSyncLog.create({
      data: {
        tenantId,
        direction: 'DOWN',
        kind: 'supplier_offer',
        count,
        success,
        message,
      },
    });
  }

  /// Gera embeddings dos SupplierProduct sem vetor (fail-soft). Async.
  private async embedNewProducts(accountId: string): Promise<void> {
    try {
      const tenantId = requireTenantId();
      const pending = await this.prisma.$queryRawUnsafe<Array<{ id: string }>>(
        `SELECT id::text FROM "SupplierProduct"
         WHERE "tenantId" = $1::uuid
           AND "supplierAccountId" = $2::uuid
           AND embedding IS NULL
         LIMIT 500`,
        tenantId,
        accountId,
      );
      for (const row of pending) {
        const sp = await this.prisma.supplierProduct.findUnique({
          where: { id: row.id },
        });
        if (!sp) continue;
        const text = [sp.name, sp.option0, sp.option1, sp.sku, sp.gtin]
          .filter(Boolean)
          .join(' · ');
        const vec = await this.embedding.embedQuery(text);
        if (!vec) continue;
        const v = EmbeddingService.toVectorLiteral(vec);
        await this.prisma.$executeRawUnsafe(
          `UPDATE "SupplierProduct"
           SET embedding = $1::vector, "embedding_model" = $2, "embedded_at" = NOW()
           WHERE id = $3::uuid`,
          v,
          this.embedding.model,
          row.id,
        );
      }
    } catch (err) {
      this.logger.warn(`embedNewProducts falhou: ${(err as Error).message}`);
    }
  }

  /// Resposta segura: mascara senha, omite cookie de sessão.
  private toSafeAccount(account: {
    id: string;
    baseUrl: string;
    platform: string;
    label: string | null;
    loginEmail: string;
    passwordEnc: string;
    sessionCookieEnc: string | null;
    sessionExpiresAt: Date | null;
    customerSupplierId: string | null;
    defaultMarkupPct: Prisma.Decimal | null;
    active: boolean;
    lastSyncAt: Date | null;
    lastError: string | null;
    createdAt: Date;
    updatedAt: Date;
    _count?: { products: number };
  }) {
    return {
      id: account.id,
      platform: account.platform,
      baseUrl: account.baseUrl,
      label: account.label,
      loginEmail: account.loginEmail,
      password: maskSecret(account.passwordEnc),
      hasSession:
        !!account.sessionCookieEnc &&
        !!account.sessionExpiresAt &&
        account.sessionExpiresAt.getTime() > Date.now(),
      customerSupplierId: account.customerSupplierId,
      defaultMarkupPct: account.defaultMarkupPct,
      active: account.active,
      syncing: this.syncing.has(account.id),
      lastSyncAt: account.lastSyncAt,
      lastError: account.lastError,
      productCount: account._count?.products,
      createdAt: account.createdAt,
      updatedAt: account.updatedAt,
    };
  }
}

// =============================================================================
// Helpers livres de estado
// =============================================================================

/// "R$34,99" → 34.99 (Decimal); null se não parsear.
function parsePix(pix: string | null): number | null {
  if (!pix) return null;
  const n = Number(pix.replace(/[^\d,.-]/g, '').replace(/\./g, '').replace(',', '.'));
  return Number.isFinite(n) ? n : null;
}

function round2(n: number): number {
  return Math.round(n * 100) / 100;
}

/// Remove qualquer trecho que possa conter senha/cookie antes de persistir/exibir.
function sanitizeError(message: string): string {
  return message
    .replace(/password=[^&\s]*/gi, 'password=***')
    .replace(/store_login_session=[^;\s]*/gi, 'store_login_session=***')
    .slice(0, 500);
}
