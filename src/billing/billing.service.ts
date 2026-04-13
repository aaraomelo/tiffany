import { Injectable, NotFoundException, BadRequestException } from '@nestjs/common';
import { PrismaService } from '../prisma.service';
import { StripeService } from './stripe.service';
import { BillingStatus, TenantPlan } from '@prisma/client';

const PLAN_LIMITS: Record<TenantPlan, { tasks: number; projects: number; repos: number }> = {
  starter:    { tasks: 10,       projects: 2,        repos: 1 },
  pro:        { tasks: 50,       projects: 10,       repos: 3 },
  enterprise: { tasks: Infinity, projects: Infinity, repos: Infinity },
};

// Mapa de priceId → plan lido de variáveis de ambiente
function resolvePlan(priceId: string): TenantPlan | null {
  if (priceId === process.env.STRIPE_PRICE_STARTER)    return 'starter';
  if (priceId === process.env.STRIPE_PRICE_PRO)        return 'pro';
  if (priceId === process.env.STRIPE_PRICE_ENTERPRISE) return 'enterprise';
  return null;
}

function stripeBillingStatus(stripeStatus: string): BillingStatus {
  const map: Record<string, BillingStatus> = {
    trialing:   'trialing',
    active:     'active',
    past_due:   'past_due',
    canceled:   'canceled',
    unpaid:     'unpaid',
    incomplete: 'incomplete',
    incomplete_expired: 'incomplete',
  };
  return map[stripeStatus] ?? 'incomplete';
}

@Injectable()
export class BillingService {
  constructor(
    private readonly prisma: PrismaService,
    private readonly stripe: StripeService,
  ) {}

  async createCustomerForTenant(tenantId: string) {
    const tenant = await this.prisma.tenant.findUnique({
      where: { id: tenantId },
      include: { users: { take: 1 } },
    });
    if (!tenant) throw new NotFoundException(`Tenant ${tenantId} não encontrado`);

    if (tenant.stripeCustomerId) return tenant;

    const adminUser = tenant.users[0];
    if (!adminUser) throw new BadRequestException('Tenant sem usuários — não é possível criar customer no Stripe');

    const customer = await this.stripe.createCustomer(adminUser.email, tenant.name, {
      tenantId: tenant.id,
      alias: tenant.alias,
    });

    return this.prisma.tenant.update({
      where: { id: tenantId },
      data: { stripeCustomerId: customer.id },
    });
  }

  async createSubscription(tenantId: string, priceId: string) {
    const tenant = await this.createCustomerForTenant(tenantId);

    const stripeSub = await this.stripe.createSubscription(tenant.stripeCustomerId!, priceId);

    const status = stripeBillingStatus(stripeSub.status);
    const plan   = resolvePlan(priceId);

    const sub = await this.prisma.subscription.upsert({
      where: { stripeSubscriptionId: stripeSub.id },
      create: {
        tenantId,
        stripeSubscriptionId: stripeSub.id,
        stripePriceId:        priceId,
        status,
        currentPeriodStart: new Date((stripeSub as any).current_period_start * 1000),
        currentPeriodEnd:   new Date((stripeSub as any).current_period_end   * 1000),
        cancelAtPeriodEnd:  stripeSub.cancel_at_period_end,
      },
      update: {
        status,
        stripePriceId:      priceId,
        currentPeriodStart: new Date((stripeSub as any).current_period_start * 1000),
        currentPeriodEnd:   new Date((stripeSub as any).current_period_end   * 1000),
        cancelAtPeriodEnd:  stripeSub.cancel_at_period_end,
        canceledAt:         null,
      },
    });

    await this.prisma.tenant.update({
      where: { id: tenantId },
      data: {
        stripeSubscriptionId: stripeSub.id,
        stripePriceId:        priceId,
        billingStatus:        status,
        currentPeriodEnd:     new Date((stripeSub as any).current_period_end * 1000),
        status:               'active',
        ...(plan ? { plan } : {}),
      },
    });

    return sub;
  }

  async cancelSubscription(tenantId: string, atPeriodEnd = true) {
    const tenant = await this.prisma.tenant.findUnique({ where: { id: tenantId } });
    if (!tenant)                       throw new NotFoundException(`Tenant ${tenantId} não encontrado`);
    if (!tenant.stripeSubscriptionId)  throw new BadRequestException('Tenant não possui assinatura ativa');

    const stripeSub = await this.stripe.cancelSubscription(tenant.stripeSubscriptionId, atPeriodEnd);
    const status    = stripeBillingStatus(stripeSub.status);

    const sub = await this.prisma.subscription.update({
      where: { stripeSubscriptionId: tenant.stripeSubscriptionId },
      data: {
        cancelAtPeriodEnd: stripeSub.cancel_at_period_end,
        status,
        canceledAt: atPeriodEnd ? null : new Date(),
      },
    });

    await this.prisma.tenant.update({
      where: { id: tenantId },
      data: { billingStatus: status },
    });

    return sub;
  }

  async syncSubscription(tenantId: string) {
    const tenant = await this.prisma.tenant.findUnique({ where: { id: tenantId } });
    if (!tenant)                      throw new NotFoundException(`Tenant ${tenantId} não encontrado`);
    if (!tenant.stripeSubscriptionId) throw new BadRequestException('Tenant não possui stripeSubscriptionId');

    const stripeSub = await this.stripe.retrieveSubscription(tenant.stripeSubscriptionId);
    const status    = stripeBillingStatus(stripeSub.status);

    const sub = await this.prisma.subscription.update({
      where: { stripeSubscriptionId: tenant.stripeSubscriptionId },
      data: {
        status,
        currentPeriodStart: new Date((stripeSub as any).current_period_start * 1000),
        currentPeriodEnd:   new Date((stripeSub as any).current_period_end   * 1000),
        cancelAtPeriodEnd:  stripeSub.cancel_at_period_end,
        canceledAt:         (stripeSub as any).canceled_at
          ? new Date((stripeSub as any).canceled_at * 1000)
          : null,
      },
    });

    await this.prisma.tenant.update({
      where: { id: tenantId },
      data: {
        billingStatus:    status,
        currentPeriodEnd: new Date((stripeSub as any).current_period_end * 1000),
      },
    });

    return sub;
  }

  async getUsageSummary(tenantId: string) {
    const tenant = await this.prisma.tenant.findUnique({ where: { id: tenantId } });
    if (!tenant) throw new NotFoundException(`Tenant ${tenantId} não encontrado`);

    const period  = new Date().toISOString().slice(0, 7); // "YYYY-MM"
    const limits  = PLAN_LIMITS[tenant.plan];

    const usage = await this.prisma.usageCounter.findUnique({
      where: { tenantId_period: { tenantId, period } },
    });

    return {
      period,
      plan: tenant.plan,
      usage: {
        tasks:    usage?.tasksUsed    ?? 0,
        projects: usage?.projectsUsed ?? 0,
        repos:    usage?.reposUsed    ?? 0,
      },
      limits: {
        tasks:    limits.tasks    === Infinity ? null : limits.tasks,
        projects: limits.projects === Infinity ? null : limits.projects,
        repos:    limits.repos    === Infinity ? null : limits.repos,
      },
    };
  }
}
