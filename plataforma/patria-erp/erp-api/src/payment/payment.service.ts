import {
  BadRequestException,
  Injectable,
  NotFoundException,
} from '@nestjs/common';
import {
  OrderStatus,
  PaymentMethod,
  PaymentStatus,
  Prisma,
} from '@prisma/client';
import { requireTenantId } from '../common/tenant-context/tenant-context';
import { OrderService } from '../order/order.service';
import { PrismaService } from '../prisma/prisma.service';
import { WalletService } from '../wallet/wallet.service';
import { CreatePaymentDto } from './dto/payment.dto';

/// Métodos para os quais o saldo cai imediatamente no wallet ao registrar:
/// CASH e CREDIT_NOTE são fluxos internos do lojista (caixa físico / crediário).
/// PIX, CREDIT_CARD, DEBIT_CARD só creditam quando o PSP confirmar via webhook.
const IMMEDIATE_CONFIRM: PaymentMethod[] = [
  PaymentMethod.CASH,
  PaymentMethod.CREDIT_NOTE,
  PaymentMethod.VOUCHER,
];

const WALLET_METHODS: PaymentMethod[] = [
  PaymentMethod.PIX,
  PaymentMethod.CREDIT_CARD,
  PaymentMethod.DEBIT_CARD,
];

@Injectable()
export class PaymentService {
  constructor(
    private readonly prisma: PrismaService,
    private readonly wallet: WalletService,
    private readonly orders: OrderService,
  ) {}

  async create(dto: CreatePaymentDto) {
    const tenantId = requireTenantId();

    return this.prisma.$transaction(async (tx) => {
      const order = await tx.order.findFirst({
        where: { id: dto.orderId, tenantId },
        include: { payments: true },
      });
      if (!order) throw new NotFoundException('Pedido não encontrado');
      const orderTerminal: OrderStatus[] = [OrderStatus.CANCELLED, OrderStatus.REFUNDED];
      if (orderTerminal.includes(order.status)) {
        throw new BadRequestException(`Pedido ${order.status} não aceita pagamento`);
      }

      const paidSoFar = order.payments
        .filter((p) => p.status === PaymentStatus.PAID)
        .reduce((acc, p) => acc.add(p.amount), new Prisma.Decimal(0));
      const remaining = order.total.sub(paidSoFar);
      const amount = new Prisma.Decimal(dto.amount);
      if (amount.gt(remaining)) {
        throw new BadRequestException(
          `Valor excede restante: pago=${paidSoFar} total=${order.total}`,
        );
      }

      const immediate = IMMEDIATE_CONFIRM.includes(dto.method);
      const status = immediate ? PaymentStatus.PAID : PaymentStatus.PENDING;

      // Estima fee/net se for método com taxa
      let feeAmount = new Prisma.Decimal(0);
      let netAmount = amount;
      if (WALLET_METHODS.includes(dto.method)) {
        feeAmount = this.wallet.computeFee(dto.method, amount);
        netAmount = amount.sub(feeAmount);
      }

      const payment = await tx.payment.create({
        data: {
          tenantId,
          orderId: order.id,
          method: dto.method,
          status,
          amount,
          feeAmount,
          netAmount,
          installments: dto.installments ?? 1,
          paidAt: immediate ? new Date() : null,
        },
      });

      if (immediate) {
        if (WALLET_METHODS.includes(dto.method)) {
          await this.wallet.creditSale(tx, {
            paymentId: payment.id,
            method: dto.method,
            amount,
            shouldDebitFee: true,
          });
        }
        await this.advanceOrderIfPaid(tx, order.id);
      }

      return payment;
    });
  }

  /// Confirma pagamento externo (PIX/cartão via webhook ou stub).
  async confirmPayment(paymentId: string, externalData?: Record<string, string>) {
    return this.prisma.$transaction(async (tx) => {
      const payment = await tx.payment.findUnique({ where: { id: paymentId } });
      if (!payment) throw new NotFoundException('Payment não encontrado');
      if (payment.status === PaymentStatus.PAID) return payment;
      const paymentTerminal: PaymentStatus[] = [PaymentStatus.CANCELLED, PaymentStatus.FAILED];
      if (paymentTerminal.includes(payment.status)) {
        throw new BadRequestException(`Pagamento ${payment.status} não confirma`);
      }

      const updated = await tx.payment.update({
        where: { id: paymentId },
        data: {
          status: PaymentStatus.PAID,
          paidAt: new Date(),
          ...(externalData ?? {}),
        },
      });

      if (WALLET_METHODS.includes(payment.method)) {
        await this.wallet.creditSale(tx, {
          paymentId: payment.id,
          method: payment.method,
          amount: payment.amount,
          shouldDebitFee: true,
        });
      }

      await this.advanceOrderIfPaid(tx, payment.orderId);
      return updated;
    });
  }

  private async advanceOrderIfPaid(
    tx: Prisma.TransactionClient,
    orderId: string,
  ) {
    const order = await tx.order.findUnique({
      where: { id: orderId },
      include: { payments: true },
    });
    if (!order) return;

    const paidTotal = order.payments
      .filter((p) => p.status === PaymentStatus.PAID)
      .reduce((acc, p) => acc.add(p.amount), new Prisma.Decimal(0));

    if (paidTotal.gte(order.total) && order.status !== OrderStatus.COMPLETED) {
      await tx.order.update({
        where: { id: order.id },
        data: { status: OrderStatus.PAID },
      });
    } else if (paidTotal.gt(0) && order.status === OrderStatus.AWAITING_PAYMENT) {
      await tx.order.update({
        where: { id: order.id },
        data: { status: OrderStatus.PARTIALLY_PAID },
      });
    }
  }

  async listByOrder(orderId: string) {
    const tenantId = requireTenantId();
    return this.prisma.payment.findMany({
      where: { orderId, tenantId },
      orderBy: { createdAt: 'asc' },
    });
  }
}
