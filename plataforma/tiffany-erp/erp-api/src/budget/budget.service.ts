import {
  BadRequestException,
  Injectable,
  NotFoundException,
} from '@nestjs/common';
import {
  BudgetStatus,
  BudgetTarget,
  OrderStatus,
  Prisma,
  ServiceOrderStatus,
} from '@prisma/client';
import { requireTenantId } from '../common/tenant-context/tenant-context';
import { PrismaService } from '../prisma/prisma.service';
import {
  BudgetItemInputDto,
  ConvertBudgetDto,
  CreateBudgetDto,
} from './dto/budget.dto';

@Injectable()
export class BudgetService {
  constructor(private readonly prisma: PrismaService) {}

  async create(dto: CreateBudgetDto) {
    const tenantId = requireTenantId();
    if (dto.items.length === 0) {
      throw new BadRequestException('Orçamento sem itens');
    }
    return this.prisma.$transaction(async (tx) => {
      const number = await this.nextNumber(tx, tenantId);
      const priced = this.priceItems(dto.items);
      const subtotal = priced.reduce((a, i) => a.add(i.total), new Prisma.Decimal(0));
      const discount = new Prisma.Decimal(dto.discount ?? 0);
      const total = subtotal.sub(discount);
      if (total.lt(0)) throw new BadRequestException('Desconto maior que subtotal');

      return tx.budget.create({
        data: {
          tenantId,
          number,
          customerId: dto.customerId,
          target: dto.target ?? BudgetTarget.ORDER,
          status: BudgetStatus.DRAFT,
          subtotal,
          discount,
          total,
          validUntil: dto.validUntil ? new Date(dto.validUntil) : undefined,
          notes: dto.notes,
          items: { create: priced },
        },
        include: { items: true },
      });
    });
  }

  list(status?: BudgetStatus) {
    return this.prisma.budget.findMany({
      where: {
        tenantId: requireTenantId(),
        ...(status ? { status } : {}),
      },
      orderBy: { createdAt: 'desc' },
      include: { customer: { select: { id: true, name: true } } },
    });
  }

  async findOne(id: string) {
    const b = await this.prisma.budget.findFirst({
      where: { id, tenantId: requireTenantId() },
      include: { customer: true, items: true, serviceOrder: true },
    });
    if (!b) throw new NotFoundException('Orçamento não encontrado');
    return b;
  }

  async setStatus(id: string, status: BudgetStatus) {
    const b = await this.findOne(id);
    const ALLOWED: Record<BudgetStatus, BudgetStatus[]> = {
      DRAFT: ['SENT', 'CANCELLED'] as BudgetStatus[],
      SENT: ['APPROVED', 'REJECTED', 'EXPIRED', 'CANCELLED'] as BudgetStatus[],
      APPROVED: ['CONVERTED', 'CANCELLED'] as BudgetStatus[],
      REJECTED: [] as BudgetStatus[],
      EXPIRED: [] as BudgetStatus[],
      CONVERTED: [] as BudgetStatus[],
      CANCELLED: [] as BudgetStatus[],
    };
    if (!ALLOWED[b.status].includes(status)) {
      throw new BadRequestException(`Transição ${b.status} → ${status} inválida`);
    }
    return this.prisma.budget.update({ where: { id }, data: { status } });
  }

  async convert(id: string, dto: ConvertBudgetDto) {
    const tenantId = requireTenantId();

    return this.prisma.$transaction(async (tx) => {
      const b = await tx.budget.findFirst({
        where: { id, tenantId },
        include: { items: true },
      });
      if (!b) throw new NotFoundException('Orçamento não encontrado');
      if (b.status !== BudgetStatus.APPROVED) {
        throw new BadRequestException(`Apenas orçamentos APPROVED podem ser convertidos (atual: ${b.status})`);
      }
      if (!b.customerId) {
        throw new BadRequestException('Orçamento sem cliente — defina antes de converter');
      }

      if (b.target === BudgetTarget.ORDER) {
        const productItems = b.items.filter((i) => !i.isLabor && i.productId);
        if (productItems.length === 0) {
          throw new BadRequestException('Orçamento sem itens de produto válidos para gerar Order');
        }
        const defWh = await tx.warehouse.findFirst({
          where: { tenantId, isDefault: true, active: true },
        });
        if (!defWh) throw new BadRequestException('Sem depósito padrão');

        const subtotal = productItems.reduce((a, i) => a.add(i.total), new Prisma.Decimal(0));
        const orderNumber = await this.nextOrderNumber(tx, tenantId);

        const order = await tx.order.create({
          data: {
            tenantId,
            number: orderNumber,
            customerId: b.customerId,
            warehouseId: defWh.id,
            status: OrderStatus.AWAITING_PAYMENT,
            subtotal,
            discount: b.discount,
            total: b.total,
            notes: `Convertido do orçamento ${b.number}`,
            items: {
              create: productItems.map((i) => ({
                productId: i.productId!,
                quantity: i.quantity,
                unitPrice: i.unitPrice,
                discount: i.discount,
                total: i.total,
              })),
            },
          },
        });

        await tx.budget.update({
          where: { id: b.id },
          data: { status: BudgetStatus.CONVERTED, convertedAt: new Date() },
        });

        return { type: 'ORDER' as const, id: order.id, number: order.number };
      }

      // SERVICE_ORDER
      const parts = b.items.filter((i) => !i.isLabor && i.productId);
      const labors = b.items.filter((i) => i.isLabor);
      const soNumber = await this.nextServiceOrderNumber(tx, tenantId);

      const so = await tx.serviceOrder.create({
        data: {
          tenantId,
          number: soNumber,
          customerId: b.customerId,
          vehicleId: dto.vehicleId,
          budgetId: b.id,
          status: ServiceOrderStatus.OPEN,
          description: dto.description ?? `Convertido do orçamento ${b.number}`,
          partsTotal: parts.reduce((a, i) => a.add(i.total), new Prisma.Decimal(0)),
          laborTotal: labors.reduce((a, i) => a.add(i.total), new Prisma.Decimal(0)),
          discount: b.discount,
          total: b.total,
          parts: {
            create: parts.map((i) => ({
              productId: i.productId!,
              quantity: i.quantity,
              unitPrice: i.unitPrice,
              discount: i.discount,
              total: i.total,
            })),
          },
          labors: {
            create: labors.map((i) => ({
              description: i.description,
              quantity: i.quantity,
              unitPrice: i.unitPrice,
              discount: i.discount,
              total: i.total,
            })),
          },
        },
      });

      await tx.budget.update({
        where: { id: b.id },
        data: { status: BudgetStatus.CONVERTED, convertedAt: new Date() },
      });

      return { type: 'SERVICE_ORDER' as const, id: so.id, number: so.number };
    });
  }

  // ---------- helpers ----------

  private priceItems(items: BudgetItemInputDto[]) {
    return items.map((i) => {
      const quantity = new Prisma.Decimal(i.quantity);
      const unitPrice = new Prisma.Decimal(i.unitPrice);
      const discount = new Prisma.Decimal(i.discount ?? 0);
      const total = unitPrice.mul(quantity).sub(discount);
      if (total.lt(0)) throw new BadRequestException('Desconto maior que valor do item');
      return {
        productId: i.productId,
        description: i.description,
        quantity,
        unitPrice,
        discount,
        total,
        isLabor: i.isLabor ?? false,
      };
    });
  }

  private async nextNumber(tx: Prisma.TransactionClient, tenantId: string) {
    const last = await tx.budget.findFirst({
      where: { tenantId },
      orderBy: { number: 'desc' },
      select: { number: true },
    });
    return (last?.number ?? 0) + 1;
  }

  private async nextOrderNumber(tx: Prisma.TransactionClient, tenantId: string) {
    const last = await tx.order.findFirst({
      where: { tenantId },
      orderBy: { number: 'desc' },
      select: { number: true },
    });
    return (last?.number ?? 0) + 1;
  }

  private async nextServiceOrderNumber(tx: Prisma.TransactionClient, tenantId: string) {
    const last = await tx.serviceOrder.findFirst({
      where: { tenantId },
      orderBy: { number: 'desc' },
      select: { number: true },
    });
    return (last?.number ?? 0) + 1;
  }
}
