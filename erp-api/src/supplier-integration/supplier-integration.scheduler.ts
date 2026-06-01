import { Injectable, Logger } from '@nestjs/common';
import { Cron, CronExpression } from '@nestjs/schedule';
import {
  TenantContext,
  tenantContextStorage,
} from '../common/tenant-context/tenant-context';
import { PrismaService } from '../prisma/prisma.service';
import { SupplierIntegrationService } from './supplier-integration.service';

/// Re-sincroniza o catálogo (preço/estoque) de todas as contas ativas, 1x/dia.
/// Roda fora de contexto de request → envolve cada conta no ALS do seu tenant
/// (o service usa requireTenantId internamente). NUNCA agenda pedidos.
@Injectable()
export class SupplierIntegrationScheduler {
  private readonly logger = new Logger(SupplierIntegrationScheduler.name);

  constructor(
    private readonly prisma: PrismaService,
    private readonly service: SupplierIntegrationService,
  ) {}

  @Cron(CronExpression.EVERY_DAY_AT_3AM, { name: 'supplier-catalog-sync' })
  async resyncAll() {
    const accounts = await this.prisma.supplierAccount.findMany({
      where: { active: true },
      select: { id: true, tenantId: true, baseUrl: true },
    });
    if (accounts.length === 0) return;

    this.logger.log(`Re-sync de catálogo: ${accounts.length} conta(s)`);
    for (const account of accounts) {
      const ctx: TenantContext = {
        tenantId: account.tenantId,
        userId: null,
        role: null,
      };
      try {
        const result = await tenantContextStorage.run(ctx, () =>
          this.service.syncCatalog(account.id),
        );
        this.logger.log(
          `· ${account.baseUrl}: ${result.variants} variações`,
        );
      } catch (err) {
        this.logger.warn(
          `· ${account.baseUrl} falhou: ${(err as Error).message}`,
        );
      }
    }
  }
}
