import { Injectable, NotFoundException } from '@nestjs/common';
import {
  PaymentMethod,
  Prisma,
  WalletTransactionType,
} from '@prisma/client';
import { requireTenantId } from '../common/tenant-context/tenant-context';
import { PrismaService } from '../prisma/prisma.service';

/// Taxas padrão repassadas ao lojista (mesmas do PagBank público).
/// Sem markup — Patria não lucra na taxa.
function feeRate(method: PaymentMethod): number {
  switch (method) {
    case PaymentMethod.PIX:
      return 0.0099;
    case PaymentMethod.DEBIT_CARD:
      return 0.0199;
    case PaymentMethod.CREDIT_CARD:
      return 0.0319;
    default:
      return 0;
  }
}

@Injectable()
export class WalletService {
  constructor(private readonly prisma: PrismaService) {}

  async getOrCreate(tx?: Prisma.TransactionClient) {
    const client = tx ?? this.prisma;
    const tenantId = requireTenantId();
    return client.merchantWallet.upsert({
      where: { tenantId },
      create: { tenantId },
      update: {},
    });
  }

  computeFee(method: PaymentMethod, amount: Prisma.Decimal): Prisma.Decimal {
    return amount.mul(feeRate(method)).toDecimalPlaces(4);
  }

  /// Credita venda + debita taxa em transação atômica.
  /// Chamado pelo PaymentService quando um pagamento confirma.
  async creditSale(
    tx: Prisma.TransactionClient,
    args: {
      paymentId: string;
      method: PaymentMethod;
      amount: Prisma.Decimal;
      shouldDebitFee: boolean;
    },
  ) {
    const tenantId = requireTenantId();
    const wallet = await tx.merchantWallet.upsert({
      where: { tenantId },
      create: { tenantId },
      update: {},
    });

    let balance = wallet.balance.add(args.amount);
    let totalReceived = wallet.totalReceived.add(args.amount);

    await tx.walletTransaction.create({
      data: {
        walletId: wallet.id,
        type: WalletTransactionType.SALE_CREDIT,
        amount: args.amount,
        balanceAfter: balance,
        refType: 'Payment',
        refId: args.paymentId,
        paymentId: args.paymentId,
      },
    });

    let feeAmount = new Prisma.Decimal(0);
    if (args.shouldDebitFee) {
      feeAmount = this.computeFee(args.method, args.amount);
      if (feeAmount.gt(0)) {
        balance = balance.sub(feeAmount);
        await tx.walletTransaction.create({
          data: {
            walletId: wallet.id,
            type: WalletTransactionType.FEE_DEBIT,
            amount: feeAmount.neg(),
            balanceAfter: balance,
            refType: 'Payment',
            refId: args.paymentId,
            paymentId: args.paymentId,
          },
        });
      }
    }

    await tx.merchantWallet.update({
      where: { id: wallet.id },
      data: { balance, totalReceived },
    });

    return { feeAmount, netAmount: args.amount.sub(feeAmount) };
  }

  async listTransactions(limit = 50) {
    const tenantId = requireTenantId();
    const wallet = await this.prisma.merchantWallet.findUnique({
      where: { tenantId },
    });
    if (!wallet) throw new NotFoundException('Wallet ainda não existe');
    const items = await this.prisma.walletTransaction.findMany({
      where: { walletId: wallet.id },
      orderBy: { createdAt: 'desc' },
      take: limit,
    });
    return { wallet, items };
  }
}
