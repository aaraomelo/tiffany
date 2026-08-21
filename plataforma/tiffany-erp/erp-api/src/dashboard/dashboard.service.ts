import { Injectable, Logger } from '@nestjs/common';
import { PrismaService } from '../prisma/prisma.service';
import {
  getTenantContext,
  requireTenantId,
} from '../common/tenant-context/tenant-context';
import { withTenantScope } from '../prisma/prisma-rls-native';

export type MetricIntent = 'default' | 'success' | 'warning' | 'error';

export interface DashboardMetric {
  /** sufixo de chave i18n (dash.metric.<key>) */
  key: string;
  /** slug do módulo de origem */
  module: string;
  value: number;
  format: 'count' | 'currency';
  route: string | null;
  intent: MetricIntent;
}

const INTENT_ORDER: Record<MetricIntent, number> = {
  error: 0,
  warning: 1,
  success: 2,
  default: 3,
};

@Injectable()
export class DashboardService {
  private readonly logger = new Logger(DashboardService.name);

  constructor(private readonly prisma: PrismaService) {}

  async metrics(): Promise<{ metrics: DashboardMetric[] }> {
    const tenantId = requireTenantId();

    const enabled = await this.prisma.tenantModule.findMany({
      where: { tenantId, enabled: true },
      select: { moduleSlug: true },
    });
    const on = new Set(enabled.map((m) => m.moduleSlug));

    const now = new Date();
    const dayStart = new Date(now.getFullYear(), now.getMonth(), now.getDate());

    const out: DashboardMetric[] = [];
    const push = (m: DashboardMetric) => out.push(m);

    // executa cada bloco isoladamente: falha de um módulo não derruba o resto
    const safe = async (slug: string, fn: () => Promise<void>) => {
      if (!on.has(slug)) return;
      try {
        await fn();
      } catch (e) {
        this.logger.warn(`métrica '${slug}' falhou: ${(e as Error).message}`);
      }
    };

    await Promise.all([
      safe('customer-supplier', async () => {
        const value = await this.prisma.customerSupplier.count({
          where: { tenantId, active: true, deletedAt: null },
        });
        push({ key: 'customers', module: 'customer-supplier', value, format: 'count', route: '/customers', intent: 'default' });
      }),

      safe('product', async () => {
        const value = await this.prisma.product.count({
          where: { tenantId, active: true, deletedAt: null },
        });
        push({ key: 'products', module: 'product', value, format: 'count', route: '/products', intent: 'default' });
      }),

      safe('stock', async () => {
        // comparação entre colunas → raw (Prisma where não compara dois campos)
        const rows = await withTenantScope(this.prisma, getTenantContext, (db) =>
          db.$queryRaw<{ count: bigint }[]>`
          SELECT COUNT(*)::bigint AS count FROM "Stock"
          WHERE "tenantId" = ${tenantId}::uuid
            AND "minQty" IS NOT NULL
            AND quantity <= "minQty"`,
        );
        const value = Number(rows[0]?.count ?? 0);
        push({ key: 'stock_low', module: 'stock', value, format: 'count', route: '/stock', intent: value > 0 ? 'warning' : 'default' });
      }),

      safe('pos', async () => {
        const agg = await this.prisma.order.aggregate({
          where: {
            tenantId,
            createdAt: { gte: dayStart },
            status: { in: ['PAID', 'COMPLETED', 'PARTIALLY_PAID', 'FULFILLING'] },
          },
          _sum: { total: true },
        });
        push({ key: 'sales_today', module: 'pos', value: Number(agg._sum.total ?? 0), format: 'currency', route: '/pos', intent: 'success' });
      }),

      safe('order', async () => {
        const value = await this.prisma.order.count({
          where: { tenantId, status: { in: ['DRAFT', 'AWAITING_PAYMENT', 'FULFILLING', 'PARTIALLY_PAID'] } },
        });
        push({ key: 'orders_open', module: 'order', value, format: 'count', route: '/orders', intent: 'default' });
      }),

      safe('service-order', async () => {
        const value = await this.prisma.serviceOrder.count({
          where: { tenantId, status: { in: ['OPEN', 'IN_PROGRESS', 'WAITING_PARTS', 'WAITING_CUSTOMER'] } },
        });
        push({ key: 'service_orders_open', module: 'service-order', value, format: 'count', route: '/service-orders', intent: 'default' });
      }),

      safe('budget', async () => {
        const value = await this.prisma.budget.count({
          where: { tenantId, status: 'SENT' },
        });
        push({ key: 'budgets_pending', module: 'budget', value, format: 'count', route: '/budgets', intent: 'default' });
      }),

      safe('cash', async () => {
        const value = await this.prisma.cashSession.count({
          where: { tenantId, status: { in: ['OPEN', 'PARTIALLY_CLOSED'] } },
        });
        push({ key: 'cash_open', module: 'cash', value, format: 'count', route: '/cash', intent: 'default' });
      }),

      safe('wallet', async () => {
        const agg = await this.prisma.merchantWallet.aggregate({
          where: { tenantId },
          _sum: { balance: true },
        });
        push({ key: 'wallet_balance', module: 'wallet', value: Number(agg._sum.balance ?? 0), format: 'currency', route: '/wallet', intent: 'default' });
      }),

      safe('student', async () => {
        const value = await this.prisma.student.count({
          where: { tenantId, deletedAt: null, status: 'ACTIVE' },
        });
        push({ key: 'students', module: 'student', value, format: 'count', route: '/students', intent: 'default' });
      }),

      safe('enrollment', async () => {
        const value = await this.prisma.enrollment.count({
          where: { tenantId, status: 'ACTIVE' },
        });
        push({ key: 'enrollments_active', module: 'enrollment', value, format: 'count', route: '/enrollments', intent: 'default' });
      }),

      safe('tuition', async () => {
        const [overdue, agg] = await Promise.all([
          this.prisma.tuition.count({ where: { tenantId, status: 'OVERDUE' } }),
          this.prisma.tuition.aggregate({ where: { tenantId, status: 'OVERDUE' }, _sum: { amount: true } }),
        ]);
        push({ key: 'tuitions_overdue', module: 'tuition', value: overdue, format: 'count', route: '/tuitions', intent: overdue > 0 ? 'error' : 'success' });
        if (overdue > 0) {
          push({ key: 'tuitions_overdue_amount', module: 'tuition', value: Number(agg._sum.amount ?? 0), format: 'currency', route: '/tuitions', intent: 'error' });
        }
      }),

      safe('health-record', async () => {
        const value = await this.prisma.student.count({
          where: {
            tenantId,
            deletedAt: null,
            status: 'ACTIVE',
            OR: [{ healthRecord: null }, { healthRecord: { medicalClearance: false } }],
          },
        });
        push({ key: 'health_no_clearance', module: 'health-record', value, format: 'count', route: '/health', intent: value > 0 ? 'warning' : 'success' });
      }),
    ]);

    out.sort((a, b) => INTENT_ORDER[a.intent] - INTENT_ORDER[b.intent]);
    return { metrics: out };
  }
}
