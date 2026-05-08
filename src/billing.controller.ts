import { Body, Controller, Get, Headers, HttpException, HttpStatus, Logger, MessageEvent, Post, Query, Req, Sse, UnauthorizedException } from '@nestjs/common';
import type { Request } from 'express';
import { Observable, Subject } from 'rxjs';
import { PrismaService } from './prisma.service';
import { EmailService } from './email.service';

const SUPPORT_EMAIL = process.env.SMTP_USER || 'contato@patriatechnology.com';

// Hub de eventos por tenant (in-memory). Patria-api é instância única, fine.
class BillingEventHub {
  private static subjects = new Map<string, Subject<MessageEvent>>();
  static stream(tenantId: string): Observable<MessageEvent> {
    let s = this.subjects.get(tenantId);
    if (!s) {
      s = new Subject<MessageEvent>();
      this.subjects.set(tenantId, s);
    }
    return s.asObservable();
  }
  static emit(tenantId: string, payload: any) {
    const s = this.subjects.get(tenantId);
    if (s) s.next({ data: JSON.stringify(payload), type: payload.type });
  }
  static cleanup(tenantId: string) {
    this.subjects.get(tenantId)?.complete();
    this.subjects.delete(tenantId);
  }
}

const SANDBOX = process.env.PAGBANK_SANDBOX === 'true';
const TOKEN = SANDBOX
  ? process.env.PAGBANK_TOKEN_SANDBOX || ''
  : process.env.PAGBANK_TOKEN_LIVE || '';
const BASE_URL = SANDBOX
  ? process.env.PAGBANK_BASE_URL_SANDBOX || 'https://sandbox.api.pagseguro.com'
  : process.env.PAGBANK_BASE_URL_LIVE || 'https://api.pagseguro.com';
const PRO_AMOUNT = parseInt(process.env.PAGBANK_PRO_AMOUNT_CENTS || '29900', 10);
const PUBLIC_BASE = process.env.PUBLIC_BASE_URL || 'https://nco.patriatechnology.com';

@Controller()
export class BillingController {
  private readonly logger = new Logger(BillingController.name);
  constructor(
    private readonly prisma: PrismaService,
    private readonly emailService: EmailService,
  ) {}

  @Get('api/billing/health')
  async health() {
    return {
      provider: 'pagbank',
      sandbox: SANDBOX,
      base_url: BASE_URL,
      configured: !!TOKEN,
    };
  }

  @Get('api/billing/plans')
  async listPlans() {
    return {
      plans: [
        { code: 'starter',    label: 'Starter (grátis)', price_brl: 0,   monthly_calls: 1000,  max_n: 500,  max_rollouts: 32 },
        { code: 'pro',        label: 'Pro',              price_brl: 299, monthly_calls: 50000, max_n: 2000, max_rollouts: 64 },
        { code: 'enterprise', label: 'Enterprise',       price_brl: null, monthly_calls: null,  max_n: 5000, max_rollouts: 128, contact_only: true },
      ],
    };
  }

  // Cliente clica "Fazer upgrade" no dashboard → cria order PIX no PagBank → devolve link
  // Última request de upgrade do tenant — pra pré-preencher form
  @Get('api/billing/upgrade-request/latest')
  async latestUpgradeRequest(@Req() req: Request) {
    const apiKey = (req.headers['x-api-key'] as string) || '';
    const tenant = await this.prisma.tenant.findUnique({ where: { apiKey }, select: { id: true, companyName: true } });
    if (!tenant) {
      throw new HttpException('Tenant inválido.', HttpStatus.UNAUTHORIZED);
    }
    const owner = await this.prisma.user.findFirst({
      where: { tenantId: tenant.id },
      select: { email: true },
    });
    if (!owner?.email) return { previous: null };

    // Busca contatos com mensagem de upgrade pra esse user
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

  // Lead capture pra upgrade Pro/Enterprise — grava em Contact e notifica contato@
  // Upgrade instantâneo Starter → Pro (livre durante o MVP)
  @Post('api/billing/upgrade-to-pro')
  async upgradeToPro(@Req() req: Request) {
    const apiKey = (req.headers['x-api-key'] as string) || '';
    const tenant = await this.prisma.tenant.findUnique({ where: { apiKey } });
    if (!tenant) {
      throw new HttpException('Tenant inválido.', HttpStatus.UNAUTHORIZED);
    }
    if (tenant.plan === 'pro' || tenant.plan === 'enterprise') {
      throw new HttpException(`Você já está no plano ${tenant.plan}.`, HttpStatus.BAD_REQUEST);
    }

    const renewsAt = new Date(Date.now() + 30 * 24 * 3600 * 1000);
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

    // Confirma com email pro user
    const owner = await this.prisma.user.findFirst({
      where: { tenantId: tenant.id },
      select: { email: true, name: true },
    });
    if (owner?.email) {
      const html = `<!doctype html>
<html><body style="font-family:-apple-system,Segoe UI,Roboto,sans-serif;max-width:560px;margin:0 auto;padding:32px 24px;color:#1a1a1a;background:#f6f7fa;">
  <div style="background:white;border-radius:12px;padding:32px;border:1px solid #e6e8f0;">
    <div style="display:flex;align-items:center;gap:10px;margin-bottom:24px;">
      <span style="display:inline-flex;align-items:center;justify-content:center;width:32px;height:32px;background:linear-gradient(135deg,#cc1144,#ff5577);color:white;font-weight:700;border-radius:6px;font-size:14px;">P</span>
      <strong style="font-size:18px;">Patria Technology</strong>
    </div>
    <h2 style="font-size:20px;margin:0 0 12px;">Você está no plano Pro 🚀</h2>
    <p style="color:#5b6378;line-height:1.55;font-size:15px;">
      Olá ${owner.name || ''}, seu tenant <strong>${tenant.companyName}</strong> agora está no <strong>Pro</strong>.
    </p>
    <ul style="color:#5b6378;font-size:14px;line-height:1.7;">
      <li>2.000 nós/s sustentado · burst 10.000</li>
      <li>n ≤ 2.000 vértices por chamada</li>
      <li>rollouts ≤ 64</li>
    </ul>
    <p style="color:#5b6378;font-size:14px;">
      Renovação automática em <strong>${renewsAt.toLocaleDateString('pt-BR')}</strong>. Plano grátis durante o MVP — quando ativarmos cobrança, avisamos com antecedência.
    </p>
  </div>
</body></html>`;
      this.emailService
        .send(owner.email, 'Bem-vindo ao Pro · Patria Technology', html)
        .catch((e) => this.logger.error(`upgrade confirm fail: ${e.message}`));
    }

    return {
      ok: true,
      plan: 'pro',
      plan_renews_at: renewsAt.toISOString(),
      message: 'Plano Pro ativado. Email de confirmação enviado.',
    };
  }

  @Post('api/billing/upgrade-request')
  async upgradeRequest(
    @Body() body: {
      plan?: string;
      cnpj?: string;
      companyName?: string;
      contactName?: string;
      phone?: string;
      whatsappConsent?: boolean;
      useCase?: string;
      estimatedVolume?: string;
    },
    @Req() req: Request,
  ) {
    const apiKey = (req.headers['x-api-key'] as string) || '';
    const tenant = await this.prisma.tenant.findUnique({ where: { apiKey } });
    if (!tenant) {
      throw new HttpException('Tenant inválido.', HttpStatus.UNAUTHORIZED);
    }
    const owner = await this.prisma.user.findFirst({
      where: { tenantId: tenant.id },
      select: { email: true, name: true },
    });
    const planRequested = body.plan || 'pro';

    // Cooldown: 1h entre submits do mesmo user (evita spam de email)
    const COOLDOWN_MS = 3600 * 1000;
    if (owner?.email) {
      const lastReq = await this.prisma.contact.findFirst({
        where: { email: owner.email, message: { contains: '"type": "upgrade_request"' } },
        orderBy: { createdAt: 'desc' },
      });
      if (lastReq) {
        const elapsed = Date.now() - lastReq.createdAt.getTime();
        if (elapsed < COOLDOWN_MS) {
          const waitMs = COOLDOWN_MS - elapsed;
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
      }
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

    // Notifica contato@ assíncrono (não bloqueia)
    const subject = `[Patria] Upgrade ${planRequested.toUpperCase()} — ${tenant.companyName}`;
    const html = `<!doctype html><html><body style="font-family:sans-serif;max-width:560px;padding:24px;color:#1a1a1a;">
  <h2 style="margin:0 0 12px;">Novo pedido de upgrade · ${planRequested.toUpperCase()}</h2>
  <p><strong>Tenant:</strong> ${tenant.companyName} (<code>${tenant.alias}</code>)</p>
  <p><strong>Contato responsável:</strong> ${body.contactName || owner?.name || '—'}<br>
     <strong>Email:</strong> ${owner?.email || '—'}<br>
     <strong>Telefone:</strong> ${body.phone || '—'}${body.whatsappConsent ? ' (WhatsApp OK)' : ''}</p>
  <p><strong>CNPJ:</strong> <code>${body.cnpj || '—'}</code><br>
     <strong>Razão social:</strong> ${body.companyName || tenant.companyName}</p>
  ${body.useCase ? `<p><strong>Caso de uso:</strong><br>${body.useCase.replace(/\n/g, '<br>')}</p>` : ''}
  ${body.estimatedVolume ? `<p><strong>Volume estimado:</strong> ${body.estimatedVolume}</p>` : ''}
  <hr><p style="color:#888;font-size:12px;">Contact ID: ${contact.id} · ${new Date().toLocaleString('pt-BR')}</p>
</body></html>`;
    this.emailService
      .send(SUPPORT_EMAIL, subject, html)
      .catch((e) => this.logger.error(`Notify internal fail: ${e.message}`));

    // Confirmação automática pro cliente
    if (owner?.email) {
      const confirmHtml = `<!doctype html>
<html><body style="font-family:-apple-system,Segoe UI,Roboto,sans-serif;max-width:560px;margin:0 auto;padding:32px 24px;color:#1a1a1a;background:#f6f7fa;">
  <div style="background:white;border-radius:12px;padding:32px;border:1px solid #e6e8f0;">
    <div style="display:flex;align-items:center;gap:10px;margin-bottom:24px;">
      <span style="display:inline-flex;align-items:center;justify-content:center;width:32px;height:32px;background:linear-gradient(135deg,#cc1144,#ff5577);color:white;font-weight:700;border-radius:6px;font-size:14px;">P</span>
      <strong style="font-size:18px;">Patria Technology</strong>
    </div>
    <h2 style="font-size:20px;margin:0 0 12px;">Recebemos seu pedido de upgrade</h2>
    <p style="color:#5b6378;line-height:1.55;font-size:15px;">
      Olá ${body.contactName || owner.name || ''}, recebemos sua solicitação de upgrade pra
      <strong>${planRequested.toUpperCase()}</strong> da <strong>${body.companyName || tenant.companyName}</strong>.
    </p>
    <p style="color:#5b6378;line-height:1.55;font-size:15px;">
      Vamos analisar seus dados e entrar em contato em <strong>até 24 horas úteis</strong>
      por este mesmo email pra finalizar a contratação. Se precisar adiantar algo, responde direto pra
      <a href="mailto:${SUPPORT_EMAIL}" style="color:#cc1144;">${SUPPORT_EMAIL}</a>.
    </p>
    <div style="background:#f4f5fa;border-radius:6px;padding:14px 16px;margin:20px 0;font-family:ui-monospace,monospace;font-size:13px;color:#5b6378;">
      ID do pedido: ${contact.id}<br>
      Plano: ${planRequested}<br>
      Empresa: ${body.companyName || tenant.companyName}${body.cnpj ? `<br>CNPJ: ${body.cnpj}` : ''}
    </div>
    <p style="color:#8a91a8;font-size:12px;margin-top:24px;border-top:1px solid #e6e8f0;padding-top:16px;">
      Você não vai receber mais nenhum email automático sobre este pedido — apenas o retorno humano em até 24h.
    </p>
  </div>
</body></html>`;
      this.emailService
        .send(owner.email, `Recebemos seu pedido de upgrade · Patria`, confirmHtml)
        .catch((e) => this.logger.error(`Confirm email fail: ${e.message}`));
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

  // [legacy/dormente] checkout PIX direto via PagBank — descomentar quando o produto for self-service de novo
  @Post('api/billing/checkout')
  async checkout(
    @Body() body: { plan: string; method?: 'PIX' | 'CARD'; tax_id?: string; name?: string },
    @Req() req: Request,
  ) {
    if (!TOKEN) {
      throw new HttpException('Billing não configurado.', HttpStatus.SERVICE_UNAVAILABLE);
    }
    const apiKey = (req.headers['x-api-key'] as string) || '';
    const tenant = await this.prisma.tenant.findUnique({ where: { apiKey } });
    if (!tenant) {
      throw new HttpException('Tenant inválido.', HttpStatus.UNAUTHORIZED);
    }
    if (body.plan !== 'pro') {
      throw new HttpException('Apenas plano Pro disponível por checkout self-service.', HttpStatus.BAD_REQUEST);
    }

    // Busca user (owner do tenant) pra pegar email
    const owner = await this.prisma.user.findFirst({
      where: { tenantId: tenant.id },
      select: { email: true, name: true },
    });

    // tax_id: usa o passado pelo cliente; se ausente, placeholder válido pra sandbox
    const taxId = (body.tax_id || '').replace(/\D/g, '') || (SANDBOX ? '12345678909' : '');
    if (!taxId) {
      throw new HttpException('CPF ou CNPJ obrigatório no checkout.', HttpStatus.BAD_REQUEST);
    }

    const referenceId = `pat-${tenant.alias}-${Date.now()}`;
    const orderBody: any = {
      reference_id: referenceId,
      customer: {
        name: (body.name || tenant.companyName).slice(0, 30) || 'Cliente',
        email: owner?.email || tenant.billingEmail || 'no-reply@patriatechnology.com',
        tax_id: taxId,
      },
      items: [
        {
          reference_id: 'patria-nco-pro-monthly',
          name: 'Patria NCO API · Plano Pro · 1 mês',
          quantity: 1,
          unit_amount: PRO_AMOUNT,
        },
      ],
      qr_codes: [
        {
          amount: { value: PRO_AMOUNT },
          expiration_date: new Date(Date.now() + 30 * 60 * 1000).toISOString(), // 30min
        },
      ],
      notification_urls: [`${PUBLIC_BASE}/api/webhooks/pagbank`],
    };

    try {
      const r = await fetch(`${BASE_URL}/orders`, {
        method: 'POST',
        headers: {
          'Content-Type': 'application/json',
          Authorization: `Bearer ${TOKEN}`,
        },
        body: JSON.stringify(orderBody),
      });
      const data = await r.json();
      if (!r.ok) {
        this.logger.error(`PagBank checkout HTTP ${r.status}: ${JSON.stringify(data)}`);
        throw new HttpException(
          `Falha ao criar cobrança: ${(data as any).error_messages?.[0]?.description || (data as any).message || 'erro desconhecido'}`,
          r.status,
        );
      }

      // Persiste evento e ref no tenant
      await this.prisma.billingEvent.create({
        data: {
          provider: 'pagbank',
          eventType: 'order.created',
          tenantId: tenant.id,
          payload: data as any,
          processed: false,
        },
      });
      await this.prisma.tenant.update({
        where: { id: tenant.id },
        data: {
          billingProvider: 'pagbank',
          billingEmail: owner?.email || null,
          subscriptionCode: (data as any).id || null,
          subscriptionStatus: 'pending_payment',
        },
      });

      // Extrai os links úteis pro frontend
      const qr = (data as any).qr_codes?.[0];
      const checkoutLink = (data as any).links?.find((l: any) => l.rel === 'PAY')?.href;
      const pixCopy = qr?.text;
      const pixImage = qr?.links?.find((l: any) => l.media === 'image/png')?.href;

      return {
        order_id: (data as any).id,
        reference_id: referenceId,
        amount_cents: PRO_AMOUNT,
        amount_brl: PRO_AMOUNT / 100,
        sandbox: SANDBOX,
        pix: {
          copy_paste: pixCopy,
          qr_image_url: pixImage,
          expires_at: qr?.expiration_date,
        },
        checkout_url: checkoutLink || null,
        raw: SANDBOX ? data : undefined,
      };
    } catch (e: any) {
      if (e instanceof HttpException) throw e;
      this.logger.error(`PagBank network error: ${e.message}`);
      throw new HttpException('Falha na comunicação com PagBank.', HttpStatus.BAD_GATEWAY);
    }
  }

  // Webhook do PagBank — público (sem auth)
  // SSE: cliente escuta notificações de billing (pagamento confirmado, plano atualizado)
  @Sse('api/billing/events')
  async events(@Query('api_key') apiKey: string): Promise<Observable<MessageEvent>> {
    if (!apiKey) throw new UnauthorizedException('api_key obrigatório');
    const tenant = await this.prisma.tenant.findUnique({
      where: { apiKey },
      select: { id: true },
    });
    if (!tenant) throw new UnauthorizedException('apiKey inválido');
    this.logger.log(`SSE subscribe: tenant=${tenant.id.slice(0, 8)}`);
    return BillingEventHub.stream(tenant.id);
  }

  @Post('api/webhooks/pagbank')
  async webhook(@Body() body: any, @Headers() headers: Record<string, string>) {
    this.logger.log(`PagBank webhook: ${JSON.stringify(body).slice(0, 500)}`);

    // Persiste evento bruto antes de processar (auditoria)
    let tenantId: string | null = null;
    const charges = body?.charges || [];
    const referenceId = body?.reference_id;
    if (referenceId?.startsWith('pat-')) {
      // formato: pat-<alias>-<ts> · alias pode ter hífens
      const m = referenceId.match(/^pat-(.+)-\d+$/);
      const alias = m?.[1];
      if (alias) {
        const t = await this.prisma.tenant.findUnique({ where: { alias }, select: { id: true } });
        tenantId = t?.id || null;
      }
    }

    const event = await this.prisma.billingEvent.create({
      data: {
        provider: 'pagbank',
        eventType: charges[0]?.status || body?.status || 'unknown',
        tenantId,
        payload: body,
        processed: false,
      },
    });

    // Se algum charge tá PAID, ativa o tenant
    const paid = charges.find((c: any) => c.status === 'PAID');
    if (paid && tenantId) {
      const renewsAt = new Date(Date.now() + 30 * 24 * 3600 * 1000); // +30 dias
      await this.prisma.tenant.update({
        where: { id: tenantId },
        data: {
          plan: 'pro',
          subscriptionStatus: 'active',
          planRenewsAt: renewsAt,
        },
      });
      await this.prisma.billingEvent.update({
        where: { id: event.id },
        data: { processed: true },
      });
      this.logger.log(`Tenant ${tenantId} ativado em Pro até ${renewsAt.toISOString()}`);
      // Emite SSE pra notificar dashboard
      BillingEventHub.emit(tenantId, {
        type: 'upgrade_completed',
        plan: 'pro',
        plan_renews_at: renewsAt.toISOString(),
        order_id: body?.id || null,
      });
    } else if (charges.find((c: any) => c.status === 'DECLINED' || c.status === 'CANCELED')) {
      this.logger.log(`Cobrança recusada/cancelada para tenant ${tenantId}`);
      if (tenantId) {
        BillingEventHub.emit(tenantId, {
          type: 'payment_failed',
          status: charges[0]?.status || 'unknown',
        });
      }
    }

    return { ok: true, event_id: event.id };
  }
}
