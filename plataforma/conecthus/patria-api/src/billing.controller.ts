import { Body, Controller, Get, Headers, HttpException, HttpStatus, Logger, MessageEvent, Post, Query, Req, Sse, UnauthorizedException } from '@nestjs/common';
import type { Request } from 'express';
import type { Observable } from 'rxjs';
import { PrismaService } from './prisma.service';
import { EmailService } from './email.service';
import { PagbankService } from './billing/pagbank.service';
import { BillingEventsHub } from './billing/billing-events.hub';
import {
  SANDBOX,
  PAGBANK_BASE_URL,
  PAGBANK_TOKEN,
  PRO_AMOUNT_CENTS,
  SUPPORT_EMAIL,
  SUBSCRIPTION_PERIOD_MS,
  UPGRADE_REQUEST_COOLDOWN_MS,
  SANDBOX_PLACEHOLDER_CPF,
} from './billing/billing.config';
import {
  upgradeToProEmail,
  upgradeRequestInternalEmail,
  upgradeRequestConfirmationEmail,
} from './billing/billing-emails';

interface UpgradeRequestBody {
  plan?: string;
  cnpj?: string;
  companyName?: string;
  contactName?: string;
  phone?: string;
  whatsappConsent?: boolean;
  useCase?: string;
  estimatedVolume?: string;
}

interface CheckoutBody {
  plan?: string;
  method?: 'PIX' | 'CARD';
  tax_id?: string;
  name?: string;
}

interface CheckoutCardBody {
  plan?: string;
  tax_id?: string;
  name?: string;
  encrypted: string;
  holder_name?: string;
  installments?: number;
}

@Controller()
export class BillingController {
  private readonly logger = new Logger(BillingController.name);

  constructor(
    private readonly prisma: PrismaService,
    private readonly emailService: EmailService,
    private readonly pagbank: PagbankService,
    private readonly hub: BillingEventsHub,
  ) {}

  // ─────────────────────────────────────────────────────────────
  // Health & catalog
  // ─────────────────────────────────────────────────────────────

  @Get('api/billing/health')
  async health() {
    return {
      provider: 'pagbank',
      sandbox: SANDBOX,
      base_url: PAGBANK_BASE_URL,
      configured: !!PAGBANK_TOKEN,
    };
  }

  @Get('api/billing/plans')
  async listPlans() {
    return {
      plans: [
        { code: 'starter',    label: 'Starter',    price_brl: 0,    monthly_calls: 1000,  max_n: 500,  max_rollouts: 32 },
        { code: 'pro',        label: 'Pro',        price_brl: 299,  monthly_calls: 50000, max_n: 2000, max_rollouts: 64 },
        { code: 'enterprise', label: 'Enterprise', price_brl: null, monthly_calls: null,  max_n: 5000, max_rollouts: 128, contact_only: true },
      ],
    };
  }

  // ─────────────────────────────────────────────────────────────
  // Lead capture (form de upgrade — Pro/Enterprise high-touch)
  // ─────────────────────────────────────────────────────────────

  @Get('api/billing/upgrade-request/latest')
  async latestUpgradeRequest(@Req() req: Request) {
    const tenant = await this.requireTenant(req, { id: true, companyName: true });
    const owner = await this.findOwner(tenant.id);
    if (!owner?.email) return { previous: null };

    const c = await this.prisma.contact.findFirst({
      where: { email: owner.email, message: { contains: '"type": "upgrade_request"' } },
      orderBy: { createdAt: 'desc' },
    });
    if (!c) return { previous: null };

    let parsed: any = {};
    try { parsed = JSON.parse(c.message); } catch { /* ignore */ }
    return {
      previous: {
        contact_id: c.id,
        submitted_at: c.createdAt.toISOString(),
        plan: parsed.plan_requested || 'pro',
        cnpj: parsed.cnpj || '',
        companyName: parsed.companyName || tenant.companyName,
        contactName: parsed.contactName || c.name,
        phone: c.phone || '',
        whatsappConsent: c.whatsappConsent,
        useCase: parsed.useCase || '',
        estimatedVolume: parsed.estimatedVolume || '',
      },
    };
  }

  // Upgrade instantâneo Starter → Pro (livre durante MVP — não cobra)
  @Post('api/billing/upgrade-to-pro')
  async upgradeToPro(@Req() req: Request) {
    const tenant = await this.requireTenant(req);
    if (tenant.plan === 'pro' || tenant.plan === 'enterprise') {
      throw new HttpException(`Você já está no plano ${tenant.plan}.`, HttpStatus.BAD_REQUEST);
    }

    const renewsAt = new Date(Date.now() + SUBSCRIPTION_PERIOD_MS);
    await this.prisma.tenant.update({
      where: { id: tenant.id },
      data: { plan: 'pro', subscriptionStatus: 'active_free', planRenewsAt: renewsAt },
    });
    await this.prisma.billingEvent.create({
      data: {
        provider: 'manual',
        eventType: 'upgrade.starter_to_pro_free',
        tenantId: tenant.id,
        payload: { from: 'starter', to: 'pro', renews_at: renewsAt.toISOString() },
        processed: true,
      },
    });

    const owner = await this.findOwner(tenant.id);
    if (owner?.email) {
      this.sendEmailAsync(
        owner.email,
        'Bem-vindo ao Pro · Patria Technology',
        upgradeToProEmail({
          userName: owner.name,
          tenantCompanyName: tenant.companyName,
          renewsAt,
        }),
        'upgrade-to-pro confirm',
      );
    }

    return {
      ok: true,
      plan: 'pro',
      plan_renews_at: renewsAt.toISOString(),
      message: 'Plano Pro ativado. Email de confirmação enviado.',
    };
  }

  @Post('api/billing/upgrade-request')
  async upgradeRequest(@Body() body: UpgradeRequestBody, @Req() req: Request) {
    const tenant = await this.requireTenant(req);
    const owner = await this.findOwner(tenant.id);
    const planRequested = body.plan || 'pro';

    // Cooldown: evita spam de email se o user submete várias vezes
    if (owner?.email) {
      await this.assertNotCooldown(owner.email);
    }

    const messagePayload = {
      type: 'upgrade_request',
      plan_requested: planRequested,
      tenant: { alias: tenant.alias, companyName: tenant.companyName, id: tenant.id },
      cnpj: body.cnpj || null,
      companyName: body.companyName || tenant.companyName,
      contactName: body.contactName || owner?.name || null,
      phone: body.phone || null,
      useCase: body.useCase || null,
      estimatedVolume: body.estimatedVolume || null,
      submitted_at: new Date().toISOString(),
    };

    const contact = await this.prisma.contact.create({
      data: {
        name: body.contactName || owner?.name || tenant.companyName,
        email: owner?.email || tenant.billingEmail || 'no-reply@patriatechnology.com',
        message: JSON.stringify(messagePayload, null, 2),
        phone: body.phone || null,
        whatsappConsent: body.whatsappConsent ?? false,
      },
    });

    // Notifica suporte
    this.sendEmailAsync(
      SUPPORT_EMAIL,
      `[Patria] Upgrade ${planRequested.toUpperCase()} — ${tenant.companyName}`,
      upgradeRequestInternalEmail({
        planRequested,
        tenantCompanyName: tenant.companyName,
        tenantAlias: tenant.alias,
        contactName: body.contactName,
        ownerName: owner?.name,
        ownerEmail: owner?.email,
        phone: body.phone,
        whatsappConsent: body.whatsappConsent ?? false,
        cnpj: body.cnpj,
        companyName: body.companyName,
        useCase: body.useCase,
        estimatedVolume: body.estimatedVolume,
        contactId: contact.id,
      }),
      'upgrade-request internal notify',
    );

    // Confirmação pro cliente
    if (owner?.email) {
      this.sendEmailAsync(
        owner.email,
        'Recebemos seu pedido de upgrade · Patria',
        upgradeRequestConfirmationEmail({
          contactName: body.contactName,
          ownerName: owner.name,
          planRequested,
          companyName: body.companyName || tenant.companyName,
          contactId: contact.id,
          cnpj: body.cnpj,
        }),
        'upgrade-request confirm',
      );
    }

    this.logger.log(`Upgrade request: tenant=${tenant.alias} plan=${planRequested} contact_id=${contact.id}`);
    return {
      ok: true,
      contact_id: contact.id,
      contact_email: SUPPORT_EMAIL,
      sla_hours: 24,
      message: `Recebemos seu pedido! Vamos entrar em contato em até 24h úteis pelo email cadastrado (${owner?.email || 'no-reply'}). Se preferir adiantar, escreva direto pra ${SUPPORT_EMAIL}.`,
    };
  }

  // ─────────────────────────────────────────────────────────────
  // Checkout self-service: PIX (link) + Cartão (tokenizado PCI-safe)
  // ─────────────────────────────────────────────────────────────

  @Post('api/billing/checkout')
  async checkout(@Body() body: CheckoutBody, @Req() req: Request) {
    const tenant = await this.requireTenant(req);
    this.requireProPlan(body.plan);
    const owner = await this.findOwner(tenant.id);
    const taxId = this.requireTaxId(body.tax_id);

    const referenceId = this.newReferenceId(tenant.alias);
    const data = await this.pagbank.createOrder({
      referenceId,
      customer: this.buildCustomer(body.name, tenant, owner?.email, taxId),
      pix: { expirationDate: PagbankService.pixExpirationDate() },
    });

    await this.recordOrderCreated(tenant.id, owner?.email, data, 'order.created');

    return {
      order_id: data.id,
      reference_id: referenceId,
      amount_cents: PRO_AMOUNT_CENTS,
      amount_brl: PRO_AMOUNT_CENTS / 100,
      sandbox: SANDBOX,
      pix: PagbankService.extractPix(data),
      checkout_url: PagbankService.extractCheckoutUrl(data),
      raw: SANDBOX ? data : undefined,
    };
  }

  // SSE — frontend escuta confirmação de pagamento em real-time
  @Sse('api/billing/events')
  async events(@Query('api_key') apiKey: string): Promise<Observable<MessageEvent>> {
    if (!apiKey) throw new UnauthorizedException('api_key obrigatório');
    const tenant = await this.prisma.tenant.findUnique({
      where: { apiKey },
      select: { id: true },
    });
    if (!tenant) throw new UnauthorizedException('apiKey inválido');
    this.logger.log(`SSE subscribe: tenant=${tenant.id.slice(0, 8)}`);
    return this.hub.stream(tenant.id);
  }

  // ─────────────────────────────────────────────────────────────
  // Cartão tokenizado client-side (PCI safe) — PAN/CVV nunca chegam aqui
  // ─────────────────────────────────────────────────────────────

  @Get('api/billing/public-key')
  async publicKey() {
    return this.pagbank.getPublicKey();
  }

  @Post('api/billing/checkout-card')
  async checkoutCard(@Body() body: CheckoutCardBody, @Req() req: Request) {
    const tenant = await this.requireTenant(req);
    this.requireProPlan(body.plan);
    if (!body.encrypted) {
      throw new HttpException('Campo encrypted é obrigatório.', HttpStatus.BAD_REQUEST);
    }
    const owner = await this.findOwner(tenant.id);
    const taxId = this.requireTaxId(body.tax_id);

    const referenceId = this.newReferenceId(tenant.alias);
    const installments = body.installments && body.installments > 0
      ? Math.min(body.installments, 12)
      : 1;

    const data = await this.pagbank.createOrder({
      referenceId,
      customer: this.buildCustomer(body.name, tenant, owner?.email, taxId),
      cardCharge: {
        encrypted: body.encrypted,
        holderName: body.holder_name || body.name || 'CARD HOLDER',
        installments,
      },
    });

    const charge = data.charges?.[0];
    const isPaid = charge?.status === 'PAID';

    await this.prisma.billingEvent.create({
      data: {
        provider: 'pagbank',
        eventType: isPaid ? 'order.paid.card' : 'order.created.card',
        tenantId: tenant.id,
        payload: data,
        processed: isPaid,
      },
    });

    if (isPaid) {
      await this.activateProSubscription(tenant.id, owner?.email, data.id);
    } else {
      await this.prisma.tenant.update({
        where: { id: tenant.id },
        data: {
          billingProvider: 'pagbank',
          billingEmail: owner?.email || null,
          subscriptionCode: data.id || null,
          subscriptionStatus: charge?.status === 'DECLINED' ? 'declined' : 'pending_payment',
        },
      });
    }

    return {
      order_id: data.id,
      reference_id: referenceId,
      amount_cents: PRO_AMOUNT_CENTS,
      amount_brl: PRO_AMOUNT_CENTS / 100,
      sandbox: SANDBOX,
      charge_id: charge?.id || null,
      status: charge?.status || 'unknown',
      payment_response: charge?.payment_response || null,
      card: charge?.payment_method?.card
        ? {
            brand: charge.payment_method.card.brand,
            first_digits: charge.payment_method.card.first_digits,
            last_digits: charge.payment_method.card.last_digits,
          }
        : null,
    };
  }

  // ─────────────────────────────────────────────────────────────
  // Webhook PagBank — confirmação assíncrona PAID/DECLINED
  // ─────────────────────────────────────────────────────────────

  @Post('api/webhooks/pagbank')
  async webhook(@Body() body: any, @Headers() _headers: Record<string, string>) {
    this.logger.log(`PagBank webhook: ${JSON.stringify(body).slice(0, 500)}`);

    const charges = body?.charges || [];
    const tenantId = await this.tenantIdFromReference(body?.reference_id);

    const event = await this.prisma.billingEvent.create({
      data: {
        provider: 'pagbank',
        eventType: charges[0]?.status || body?.status || 'unknown',
        tenantId,
        payload: body,
        processed: false,
      },
    });

    const paid = charges.find((c: any) => c.status === 'PAID');
    if (paid && tenantId) {
      const renewsAt = await this.activateProSubscription(tenantId, null, body?.id);
      await this.prisma.billingEvent.update({
        where: { id: event.id },
        data: { processed: true },
      });
      this.logger.log(`Tenant ${tenantId} ativado em Pro até ${renewsAt.toISOString()}`);
      this.hub.emit(tenantId, {
        type: 'upgrade_completed',
        plan: 'pro',
        plan_renews_at: renewsAt.toISOString(),
        order_id: body?.id || null,
      });
    } else if (charges.find((c: any) => c.status === 'DECLINED' || c.status === 'CANCELED')) {
      this.logger.log(`Cobrança recusada/cancelada para tenant ${tenantId}`);
      if (tenantId) {
        this.hub.emit(tenantId, {
          type: 'payment_failed',
          status: charges[0]?.status || 'unknown',
        });
      }
    }

    return { ok: true, event_id: event.id };
  }

  // ═════════════════════════════════════════════════════════════
  // Helpers privados — única fonte das regras de validação/persistência
  // ═════════════════════════════════════════════════════════════

  private async requireTenant(req: Request, select?: any) {
    const apiKey = (req.headers['x-api-key'] as string) || '';
    const tenant = select
      ? await this.prisma.tenant.findUnique({ where: { apiKey }, select })
      : await this.prisma.tenant.findUnique({ where: { apiKey } });
    if (!tenant) throw new HttpException('Tenant inválido.', HttpStatus.UNAUTHORIZED);
    return tenant as any;
  }

  private findOwner(tenantId: string) {
    return this.prisma.user.findFirst({
      where: { tenantId },
      select: { email: true, name: true },
    });
  }

  private requireProPlan(plan?: string) {
    const p = plan || 'pro';
    if (p !== 'pro') {
      throw new HttpException(
        'Apenas plano Pro disponível por checkout self-service.',
        HttpStatus.BAD_REQUEST,
      );
    }
  }

  private requireTaxId(taxId?: string): string {
    const cleaned = (taxId || '').replace(/\D/g, '') || (SANDBOX ? SANDBOX_PLACEHOLDER_CPF : '');
    if (!cleaned) {
      throw new HttpException('CPF ou CNPJ obrigatório no checkout.', HttpStatus.BAD_REQUEST);
    }
    return cleaned;
  }

  private buildCustomer(name: string | undefined, tenant: any, ownerEmail: string | null | undefined, taxId: string) {
    return {
      name: (name || tenant.companyName).slice(0, 30) || 'Cliente',
      email: ownerEmail || tenant.billingEmail || 'no-reply@patriatechnology.com',
      tax_id: taxId,
    };
  }

  private newReferenceId(tenantAlias: string): string {
    return `pat-${tenantAlias}-${Date.now()}`;
  }

  private async recordOrderCreated(
    tenantId: string,
    ownerEmail: string | null | undefined,
    data: any,
    eventType: string,
  ) {
    await this.prisma.billingEvent.create({
      data: {
        provider: 'pagbank',
        eventType,
        tenantId,
        payload: data,
        processed: false,
      },
    });
    await this.prisma.tenant.update({
      where: { id: tenantId },
      data: {
        billingProvider: 'pagbank',
        billingEmail: ownerEmail || null,
        subscriptionCode: data.id || null,
        subscriptionStatus: 'pending_payment',
      },
    });
  }

  private async activateProSubscription(
    tenantId: string,
    ownerEmail: string | null | undefined,
    subscriptionCode?: string | null,
  ): Promise<Date> {
    const renewsAt = new Date(Date.now() + SUBSCRIPTION_PERIOD_MS);
    await this.prisma.tenant.update({
      where: { id: tenantId },
      data: {
        plan: 'pro',
        subscriptionStatus: 'active',
        billingProvider: 'pagbank',
        billingEmail: ownerEmail || undefined,
        subscriptionCode: subscriptionCode || undefined,
        planRenewsAt: renewsAt,
      },
    });
    return renewsAt;
  }

  private async tenantIdFromReference(referenceId?: string): Promise<string | null> {
    if (!referenceId?.startsWith('pat-')) return null;
    // formato: pat-<alias>-<ts> · alias pode ter hífens
    const m = referenceId.match(/^pat-(.+)-\d+$/);
    const alias = m?.[1];
    if (!alias) return null;
    const t = await this.prisma.tenant.findUnique({
      where: { alias },
      select: { id: true },
    });
    return t?.id || null;
  }

  private async assertNotCooldown(email: string) {
    const lastReq = await this.prisma.contact.findFirst({
      where: { email, message: { contains: '"type": "upgrade_request"' } },
      orderBy: { createdAt: 'desc' },
    });
    if (!lastReq) return;
    const elapsed = Date.now() - lastReq.createdAt.getTime();
    if (elapsed >= UPGRADE_REQUEST_COOLDOWN_MS) return;
    const waitMs = UPGRADE_REQUEST_COOLDOWN_MS - elapsed;
    const waitMin = Math.ceil(waitMs / 60000);
    throw new HttpException(
      {
        message: `Pedido recente já recebido. Aguarde ${waitMin} min antes de atualizar — assim a gente não te inunda de email.`,
        retry_after_seconds: Math.ceil(waitMs / 1000),
        last_submitted_at: lastReq.createdAt.toISOString(),
      },
      HttpStatus.TOO_MANY_REQUESTS,
    );
  }

  private sendEmailAsync(to: string, subject: string, html: string, label: string) {
    this.emailService
      .send(to, subject, html)
      .catch((e) => this.logger.error(`${label} email failed: ${e.message}`));
  }
}
