import { HttpException, HttpStatus, Injectable, Logger } from '@nestjs/common';
import { PAGBANK_BASE_URL, PAGBANK_TOKEN, PUBLIC_BASE, PRO_AMOUNT_CENTS, PRO_ITEM, PIX_EXPIRATION_MS } from './billing.config';

export interface Customer {
  name: string;
  email: string;
  tax_id: string;
}

export interface CardCharge {
  encrypted: string;
  holderName: string;
  installments: number;
}

export interface OrderRequest {
  referenceId: string;
  customer: Customer;
  /** se presente, gera order PIX. Se ausente e cardCharge presente, gera order cartão. */
  pix?: { expirationDate: Date };
  cardCharge?: CardCharge;
}

@Injectable()
export class PagbankService {
  private readonly logger = new Logger(PagbankService.name);

  get configured(): boolean {
    return !!PAGBANK_TOKEN;
  }

  /** GET RSA public-key (proxy pro PagBank /public-keys) — usado pra tokenização client-side. */
  async getPublicKey(): Promise<{ public_key: string; created_at: number }> {
    this.assertConfigured();
    const data = await this.call('/public-keys', { type: 'card' });
    return { public_key: data.public_key, created_at: data.created_at };
  }

  /** Cria uma order no PagBank. Single endpoint pra PIX e cartão — diferença é o body. */
  async createOrder(req: OrderRequest): Promise<any> {
    this.assertConfigured();

    const body: any = {
      reference_id: req.referenceId,
      customer: req.customer,
      items: [
        { ...PRO_ITEM, quantity: 1, unit_amount: PRO_AMOUNT_CENTS },
      ],
      notification_urls: [`${PUBLIC_BASE}/api/webhooks/pagbank`],
    };

    if (req.pix) {
      body.qr_codes = [
        {
          amount: { value: PRO_AMOUNT_CENTS },
          expiration_date: req.pix.expirationDate.toISOString(),
        },
      ];
    }

    if (req.cardCharge) {
      body.charges = [
        {
          reference_id: req.referenceId,
          description: 'Patria NCO Pro',
          amount: { value: PRO_AMOUNT_CENTS, currency: 'BRL' },
          payment_method: {
            type: 'CREDIT_CARD',
            installments: req.cardCharge.installments,
            capture: true,
            card: {
              encrypted: req.cardCharge.encrypted,
              holder: { name: req.cardCharge.holderName.slice(0, 30) },
            },
          },
        },
      ];
    }

    return this.call('/orders', body);
  }

  /** Helper de extração: dados PIX do response /orders. */
  static extractPix(orderResponse: any) {
    const qr = orderResponse?.qr_codes?.[0];
    return {
      copy_paste: qr?.text ?? null,
      qr_image_url: qr?.links?.find((l: any) => l.media === 'image/png')?.href ?? null,
      expires_at: qr?.expiration_date ?? null,
    };
  }

  /** Helper de extração: link de checkout cartão (hosted) do response /orders. */
  static extractCheckoutUrl(orderResponse: any): string | null {
    return orderResponse?.links?.find((l: any) => l.rel === 'PAY')?.href ?? null;
  }

  /** Helper: prazo de expiração PIX (default 30min). */
  static pixExpirationDate(): Date {
    return new Date(Date.now() + PIX_EXPIRATION_MS);
  }

  // ───────────────────────────────────────────────────────────
  // Internals
  // ───────────────────────────────────────────────────────────

  private assertConfigured() {
    if (!this.configured) {
      throw new HttpException('Billing não configurado.', HttpStatus.SERVICE_UNAVAILABLE);
    }
  }

  private async call(path: string, body: any): Promise<any> {
    try {
      const r = await fetch(`${PAGBANK_BASE_URL}${path}`, {
        method: 'POST',
        headers: {
          'Content-Type': 'application/json',
          Authorization: `Bearer ${PAGBANK_TOKEN}`,
        },
        body: JSON.stringify(body),
      });
      const data: any = await r.json();
      if (!r.ok) {
        const msg = data?.error_messages?.[0]?.description || data?.message || 'erro desconhecido';
        this.logger.error(`PagBank ${path} HTTP ${r.status}: ${JSON.stringify(data).slice(0, 300)}`);
        throw new HttpException(`PagBank ${path}: ${msg}`, r.status);
      }
      return data;
    } catch (e: any) {
      if (e instanceof HttpException) throw e;
      this.logger.error(`PagBank ${path} network error: ${e.message}`);
      throw new HttpException(`Falha na comunicação com PagBank (${path}).`, HttpStatus.BAD_GATEWAY);
    }
  }
}
