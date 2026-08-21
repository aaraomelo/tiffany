import {
  BadRequestException,
  Injectable,
  NotFoundException,
} from '@nestjs/common';
import {
  OrderChannel,
  OrderStatus,
  Prisma,
  StockMovementType,
} from '@prisma/client';
import {
  getTenantContext,
  requireTenantId,
} from '../common/tenant-context/tenant-context';
import { PrismaService } from '../prisma/prisma.service';
import { CreateOrderDto, ListOrderDto, OrderItemInputDto } from './dto/order.dto';

interface PricedItem {
  productId: string;
  quantity: Prisma.Decimal;
  unitPrice: Prisma.Decimal;
  discount: Prisma.Decimal;
  total: Prisma.Decimal;
  notes?: string;
  trackStock: boolean;
}

@Injectable()
export class OrderService {
  constructor(private readonly prisma: PrismaService) {}

  async create(dto: CreateOrderDto) {
    const tenantId = requireTenantId();
    const { userId } = getTenantContext();

    if (dto.items.length === 0) {
      throw new BadRequestException('Pedido sem itens');
    }

    return this.prisma.$transaction(async (tx) => {
      const warehouseId = await this.resolveWarehouse(tx, tenantId, dto.warehouseId);
      const priced = await this.priceItems(tx, tenantId, dto.items);

      const subtotal = priced.reduce(
        (acc, i) => acc.add(i.total),
        new Prisma.Decimal(0),
      );
      const discount = new Prisma.Decimal(dto.discount ?? 0);
      const freight = new Prisma.Decimal(dto.freight ?? 0);
      const total = subtotal.sub(discount).add(freight);
      if (total.lt(0)) throw new BadRequestException('Total negativo');

      const number = await this.nextNumber(tx, tenantId);

      const order = await tx.order.create({
        data: {
          tenantId,
          number,
          customerId: dto.customerId,
          sellerId: userId ?? undefined,
          cashSessionId: dto.cashSessionId,
          warehouseId,
          channel: dto.channel ?? OrderChannel.POS,
          status: OrderStatus.AWAITING_PAYMENT,
          subtotal,
          discount,
          freight,
          total,
          notes: dto.notes,
          items: {
            create: priced.map((i) => ({
              productId: i.productId,
              quantity: i.quantity,
              unitPrice: i.unitPrice,
              discount: i.discount,
              total: i.total,
              notes: i.notes,
            })),
          },
        },
        include: { items: true },
      });

      return order;
    });
  }

  /// Aplica baixa de estoque e marca o pedido como COMPLETED.
  /// Chamado quando todos os pagamentos forem confirmados.
  async fulfill(orderId: string) {
    const tenantId = requireTenantId();
    return this.prisma.$transaction(async (tx) => {
      const order = await tx.order.findFirst({
        where: { id: orderId, tenantId },
        include: { items: { include: { product: true } } },
      });
      if (!order) throw new NotFoundException('Pedido não encontrado');
      if (order.status === OrderStatus.COMPLETED) return order;
      const allowedForFulfill: OrderStatus[] = [
        OrderStatus.PAID,
        OrderStatus.AWAITING_PAYMENT,
        OrderStatus.FULFILLING,
      ];
      if (!allowedForFulfill.includes(order.status)) {
        throw new BadRequestException(`Estado inválido para fulfill: ${order.status}`);
      }
      if (!order.warehouseId) {
        throw new BadRequestException('Pedido sem depósito definido');
      }

      for (const item of order.items) {
        if (!item.product.trackStock) continue;

        const stock = await tx.stock.findUnique({
          where: {
            productId_warehouseId: {
              productId: item.productId,
              warehouseId: order.warehouseId,
            },
          },
        });
        const current = stock?.quantity ?? new Prisma.Decimal(0);
        const newQty = current.sub(item.quantity);
        if (newQty.lt(0)) {
          throw new BadRequestException(
            `Estoque insuficiente para ${item.product.sku}: saldo=${current}, pedido=${item.quantity}`,
          );
        }

        await tx.stock.upsert({
          where: {
            productId_warehouseId: {
              productId: item.productId,
              warehouseId: order.warehouseId,
            },
          },
          create: {
            tenantId,
            productId: item.productId,
            warehouseId: order.warehouseId,
            quantity: newQty,
            lastMovementAt: new Date(),
          },
          update: { quantity: newQty, lastMovementAt: new Date() },
        });

        await tx.stockMovement.create({
          data: {
            tenantId,
            productId: item.productId,
            warehouseId: order.warehouseId,
            type: StockMovementType.SALE_OUT,
            quantity: item.quantity.neg(),
            refType: 'Order',
            refId: order.id,
          },
        });
      }

      return tx.order.update({
        where: { id: order.id },
        data: { status: OrderStatus.COMPLETED, closedAt: new Date() },
      });
    });
  }

  async list(dto: ListOrderDto) {
    const tenantId = requireTenantId();
    const page = dto.page ?? 1;
    const pageSize = dto.pageSize ?? 20;

    const where: Prisma.OrderWhereInput = {
      tenantId,
      ...(dto.status ? { status: dto.status as OrderStatus } : {}),
      ...(dto.customerId ? { customerId: dto.customerId } : {}),
    };

    const [items, total] = await Promise.all([
      this.prisma.order.findMany({
        where,
        orderBy: { createdAt: 'desc' },
        skip: (page - 1) * pageSize,
        take: pageSize,
        include: {
          customer: { select: { id: true, name: true } },
          items: { select: { id: true } },
        },
      }),
      this.prisma.order.count({ where }),
    ]);

    return { items, total, page, pageSize };
  }

  async findOne(id: string) {
    const tenantId = requireTenantId();
    const order = await this.prisma.order.findFirst({
      where: { id, tenantId },
      include: {
        customer: true,
        items: { include: { product: { select: { sku: true, name: true } } } },
        payments: true,
      },
    });
    if (!order) throw new NotFoundException('Pedido não encontrado');
    return order;
  }

  async cancel(id: string, reason?: string) {
    const order = await this.findOne(id);
    const terminalForCancel: OrderStatus[] = [OrderStatus.COMPLETED, OrderStatus.CANCELLED];
    if (terminalForCancel.includes(order.status)) {
      throw new BadRequestException(`Não é possível cancelar pedido ${order.status}`);
    }
    return this.prisma.order.update({
      where: { id },
      data: {
        status: OrderStatus.CANCELLED,
        cancelledAt: new Date(),
        cancelReason: reason,
      },
    });
  }

  // ---------- helpers ----------

  private async resolveWarehouse(
    tx: Prisma.TransactionClient,
    tenantId: string,
    given: string | undefined,
  ): Promise<string> {
    if (given) {
      const wh = await tx.warehouse.findFirst({
        where: { id: given, tenantId },
      });
      if (!wh) throw new NotFoundException('Depósito não encontrado');
      return wh.id;
    }
    const def = await tx.warehouse.findFirst({
      where: { tenantId, isDefault: true, active: true },
    });
    if (!def) throw new BadRequestException('Sem depósito padrão configurado');
    return def.id;
  }

  private async priceItems(
    tx: Prisma.TransactionClient,
    tenantId: string,
    items: OrderItemInputDto[],
  ): Promise<PricedItem[]> {
    const ids = [...new Set(items.map((i) => i.productId))];
    const products = await tx.product.findMany({
      where: { id: { in: ids }, tenantId, deletedAt: null, active: true },
    });
    const byId = new Map(products.map((p) => [p.id, p]));

    return items.map((i) => {
      const product = byId.get(i.productId);
      if (!product) {
        throw new NotFoundException(`Produto ${i.productId} não encontrado`);
      }
      const unitPrice = new Prisma.Decimal(i.unitPrice ?? product.salePrice);
      const quantity = new Prisma.Decimal(i.quantity);
      const discount = new Prisma.Decimal(i.discount ?? 0);
      const total = unitPrice.mul(quantity).sub(discount);
      if (total.lt(0)) {
        throw new BadRequestException('Desconto maior que valor do item');
      }
      return {
        productId: product.id,
        quantity,
        unitPrice,
        discount,
        total,
        notes: i.notes,
        trackStock: product.trackStock,
      };
    });
  }

  private async nextNumber(
    tx: Prisma.TransactionClient,
    tenantId: string,
  ): Promise<number> {
    const last = await tx.order.findFirst({
      where: { tenantId },
      orderBy: { number: 'desc' },
      select: { number: true },
    });
    return (last?.number ?? 0) + 1;
  }
}
