import { Injectable, Logger } from '@nestjs/common';
import { ConfigService } from '@nestjs/config';
import { randomBytes, randomUUID } from 'node:crypto';

interface PixOrderArgs {
  referenceId: string;
  amountCents: number;
  description: string;
  expiresAt: Date;
}

export interface PixOrderResult {
  pagbankOrderId: string;
  chargeId: string;
  qrText: string;
  qrPngBase64: string;
  expiresAt: Date;
}

/// Adapter PagBank.
/// - Modo real: usa PAGBANK_TOKEN do .env, chama POST api.pagseguro.com/orders.
/// - Modo stub: sem token, devolve PIX falso (qrText sintético, base64 placeholder).
///   Útil em ambiente local — fluxo de PDV roda end-to-end sem dependência externa.
@Injectable()
export class PagBankAdapter {
  private readonly logger = new Logger(PagBankAdapter.name);
  private readonly token: string | null;
  private readonly baseUrl: string;
  private readonly notificationUrl: string | null;

  constructor(config: ConfigService) {
    this.token = config.get<string>('PAGBANK_TOKEN') ?? null;
    this.baseUrl =
      config.get<string>('PAGBANK_BASE_URL') ?? 'https://api.pagseguro.com';
    this.notificationUrl =
      config.get<string>('PAGBANK_NOTIFICATION_URL') ?? null;
    if (!this.token) {
      this.logger.warn(
        'PAGBANK_TOKEN ausente — checkout opera em modo STUB (PIX simulado)',
      );
    }
  }

  get isStub() {
    return this.token === null;
  }

  async createPixOrder(args: PixOrderArgs): Promise<PixOrderResult> {
    if (this.isStub) {
      const id = `ORDE_STUB_${randomBytes(8).toString('hex').toUpperCase()}`;
      const charge = `CHAR_STUB_${randomBytes(8).toString('hex').toUpperCase()}`;
      const qrText = `00020126850014BR.GOV.BCB.PIX${randomUUID()}5204000053039865802BR62${args.description.slice(0, 25)}6304STUB`;
      return {
        pagbankOrderId: id,
        chargeId: charge,
        qrText,
        qrPngBase64: '',
        expiresAt: args.expiresAt,
      };
    }

    const body = {
      reference_id: args.referenceId,
      customer: { name: 'Cliente', email: 'cliente@example.com', tax_id: '00000000000' },
      qr_codes: [
        {
          amount: { value: args.amountCents },
          expiration_date: args.expiresAt.toISOString(),
        },
      ],
      notification_urls: this.notificationUrl ? [this.notificationUrl] : undefined,
    };

    const res = await fetch(`${this.baseUrl}/orders`, {
      method: 'POST',
      headers: {
        Authorization: `Bearer ${this.token}`,
        'Content-Type': 'application/json',
        Accept: 'application/json',
      },
      body: JSON.stringify(body),
    });

    if (!res.ok) {
      const text = await res.text();
      throw new Error(`PagBank order ${res.status}: ${text}`);
    }
    const data = (await res.json()) as {
      id: string;
      qr_codes: Array<{
        id: string;
        text: string;
        links: Array<{ rel: string; href: string; media: string }>;
      }>;
    };

    const qr = data.qr_codes[0];
    const pngLink = qr.links.find((l) => l.rel === 'QRCODE.PNG');
    let qrPngBase64 = '';
    if (pngLink) {
      try {
        const png = await fetch(pngLink.href);
        const buf = Buffer.from(await png.arrayBuffer());
        qrPngBase64 = buf.toString('base64');
      } catch (e) {
        this.logger.warn(`Falha ao baixar QR PNG: ${(e as Error).message}`);
      }
    }

    return {
      pagbankOrderId: data.id,
      chargeId: qr.id,
      qrText: qr.text,
      qrPngBase64,
      expiresAt: args.expiresAt,
    };
  }
}
