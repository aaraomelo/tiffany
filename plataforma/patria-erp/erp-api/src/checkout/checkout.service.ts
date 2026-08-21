import {
  BadRequestException,
  Injectable,
  NotFoundException,
} from '@nestjs/common';
import { OrderStatus, PaymentMethod, PaymentStatus, Prisma } from '@prisma/client';
import { requireTenantId } from '../common/tenant-context/tenant-context';
import { PaymentService } from '../payment/payment.service';
import { PrismaService } from '../prisma/prisma.service';
import { WalletService } from '../wallet/wallet.service';
import { CreatePixCheckoutDto } from './dto/checkout.dto';
import { PagBankAdapter } from './pagbank.adapter';

const PIX_EXPIRATION_MS = 15 * 60 * 1000;

@Injectable()
export class CheckoutService {
  constructor(
    private readonly prisma: PrismaService,
    private readonly pagbank: PagBankAdapter,
    private readonly wallet: WalletService,
    private readonly payments: PaymentService,
  ) {}

  async createPix(dto: CreatePixCheckoutDto) {
    const tenantId = requireTenantId();
    const order = await this.prisma.order.findFirst({
      where: { id: dto.orderId, tenantId },
    });
    if (!order) throw new NotFoundException('Pedido não encontrado');
    const closed: OrderStatus[] = [
      OrderStatus.CANCELLED,
      OrderStatus.REFUNDED,
      OrderStatus.COMPLETED,
    ];
    if (closed.includes(order.status)) {
      throw new BadRequestException(`Pedido ${order.status} não aceita PIX`);
    }

    const amount = new Prisma.Decimal(dto.amount);
    const feeAmount = this.wallet.computeFee(PaymentMethod.PIX, amount);

    const referenceId = `erp-${tenantId.slice(0, 8)}-${Date.now()}`;
    const expiresAt = new Date(Date.now() + PIX_EXPIRATION_MS);

    const pix = await this.pagbank.createPixOrder({
      referenceId,
      amountCents: Math.round(amount.toNumber() * 100),
      description: dto.description ?? `Pedido ${order.number}`,
      expiresAt,
    });

    const payment = await this.prisma.payment.create({
      data: {
        tenantId,
        orderId: order.id,
        method: PaymentMethod.PIX,
        status: PaymentStatus.PENDING,
        amount,
        feeAmount,
        netAmount: amount.sub(feeAmount),
        installments: 1,
        pagbankOrderId: pix.pagbankOrderId,
        pagbankChargeId: pix.chargeId,
        pagbankReferenceId: referenceId,
      },
    });

    return {
      paymentId: payment.id,
      orderId: order.id,
      stub: this.pagbank.isStub,
      pagbankOrderId: pix.pagbankOrderId,
      chargeId: pix.chargeId,
      qrText: pix.qrText,
      qrPngBase64: pix.qrPngBase64,
      amount: amount.toString(),
      feeAmount: feeAmount.toString(),
      netAmount: amount.sub(feeAmount).toString(),
      expiresAt: pix.expiresAt.toISOString(),
    };
  }

  /// Simula confirmação manual (PIX stub) - apenas em ambiente sem token PagBank.
  async simulateConfirm(paymentId: string) {
    if (!this.pagbank.isStub) {
      throw new BadRequestException(
        'Simulação só é permitida no modo stub (sem PAGBANK_TOKEN)',
      );
    }
    return this.payments.confirmPayment(paymentId, {
      endToEndId: `STUB${Date.now()}`,
    });
  }
}
