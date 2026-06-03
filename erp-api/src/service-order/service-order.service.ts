import {
  BadRequestException,
  Injectable,
  NotFoundException,
} from '@nestjs/common';
import {
  Prisma,
  ServiceOrderStatus,
  StockMovementType,
} from '@prisma/client';
import { requireTenantId } from '../common/tenant-context/tenant-context';
import { PrismaService } from '../prisma/prisma.service';
import {
  ChangeStatusDto,
  CreateLaborInputDto,
  CreatePartInputDto,
  CreateServiceOrderDto,
  ListServiceOrderDto,
  UpdateServiceOrderDto,
} from './dto/service-order.dto';

const ALLOWED_TRANSITIONS: Record<ServiceOrderStatus, ServiceOrderStatus[]> = {
  OPEN: ['IN_PROGRESS', 'CANCELLED', 'WAITING_PARTS', 'WAITING_CUSTOMER'] as ServiceOrderStatus[],
  IN_PROGRESS: ['WAITING_PARTS', 'WAITING_CUSTOMER', 'FINISHED', 'CANCELLED'] as ServiceOrderStatus[],
  WAITING_PARTS: ['IN_PROGRESS', 'CANCELLED'] as ServiceOrderStatus[],
  WAITING_CUSTOMER: ['IN_PROGRESS', 'FINISHED', 'CANCELLED'] as ServiceOrderStatus[],
  FINISHED: ['DELIVERED'] as ServiceOrderStatus[],
  DELIVERED: [] as ServiceOrderStatus[],
  CANCELLED: [] as ServiceOrderStatus[],
};

@Injectable()
export class ServiceOrderService {
  constructor(private readonly prisma: PrismaService) {}

  async create(dto: CreateServiceOrderDto) {
    const tenantId = requireTenantId();

    return this.prisma.$transaction(async (tx) => {
      const customer = await tx.customerSupplier.findFirst({
        where: { id: dto.customerId, tenantId, deletedAt: null },
      });
      if (!customer) throw new NotFoundException('Cliente não encontrado');

      if (dto.vehicleId) {
        const vh = await tx.vehicle.findFirst({
          where: { id: dto.vehicleId, tenantId, customerId: dto.customerId },
        });
        if (!vh) throw new NotFoundException('Veículo não pertence ao cliente');
      }

      const number = await this.nextNumber(tx, tenantId);
      const parts = await this.priceParts(tx, tenantId, dto.parts ?? []);
      const labors = this.priceLabors(dto.labors ?? []);

      const partsTotal = parts.reduce((acc, p) => acc.add(p.total), new Prisma.Decimal(0));
      const laborTotal = labors.reduce((acc, l) => acc.add(l.total), new Prisma.Decimal(0));
      const discount = new Prisma.Decimal(dto.discount ?? 0);
      const total = partsTotal.add(laborTotal).sub(discount);
      if (total.lt(0)) throw new BadRequestException('Desconto maior que valor da OS');

      return tx.serviceOrder.create({
        data: {
          tenantId,
          number,
          customerId: dto.customerId,
          vehicleId: dto.vehicleId,
          mechanicId: dto.mechanicId,
          warrantyTermId: dto.warrantyTermId,
          status: ServiceOrderStatus.OPEN,
          description: dto.description,
          diagnosis: dto.diagnosis,
          partsTotal,
          laborTotal,
          discount,
          total,
          parts: { create: parts },
          labors: { create: labors },
        },
        include: { parts: true, labors: true },
      });
    });
  }

  async list(dto: ListServiceOrderDto) {
    const tenantId = requireTenantId();
    const page = dto.page ?? 1;
    const pageSize = dto.pageSize ?? 20;

    const where: Prisma.ServiceOrderWhereInput = {
      tenantId,
      ...(dto.status ? { status: dto.status } : {}),
      ...(dto.customerId ? { customerId: dto.customerId } : {}),
      ...(dto.vehicleId ? { vehicleId: dto.vehicleId } : {}),
    };

    const [items, total] = await Promise.all([
      this.prisma.serviceOrder.findMany({
        where,
        orderBy: { createdAt: 'desc' },
        skip: (page - 1) * pageSize,
        take: pageSize,
        include: {
          customer: { select: { id: true, name: true } },
          vehicle: { select: { id: true, plate: true, brand: true, model: true } },
        },
      }),
      this.prisma.serviceOrder.count({ where }),
    ]);

    return { items, total, page, pageSize };
  }

  async findOne(id: string) {
    const tenantId = requireTenantId();
    const so = await this.prisma.serviceOrder.findFirst({
      where: { id, tenantId },
      include: {
        customer: true,
        vehicle: true,
        mechanic: { select: { id: true, name: true } },
        warrantyTerm: true,
        parts: {
          include: {
            product: {
              select: { sku: true, name: true, unit: { select: { code: true } } },
            },
          },
        },
        labors: true,
      },
    });
    if (!so) throw new NotFoundException('OS não encontrada');
    return so;
  }

  async update(id: string, dto: UpdateServiceOrderDto) {
    const so = await this.findOne(id);
    if (this.isTerminal(so.status)) {
      throw new BadRequestException(`OS em estado ${so.status} não pode ser editada`);
    }
    return this.prisma.$transaction(async (tx) => {
      await tx.serviceOrder.update({ where: { id }, data: dto });
      return this.recomputeTotals(tx, id);
    });
  }

  async addPart(id: string, dto: CreatePartInputDto) {
    const so = await this.findOne(id);
    if (this.isTerminal(so.status) || so.status === ServiceOrderStatus.FINISHED) {
      throw new BadRequestException(`OS em estado ${so.status} não aceita itens`);
    }
    return this.prisma.$transaction(async (tx) => {
      const [priced] = await this.priceParts(tx, so.tenantId, [dto]);
      await tx.serviceOrderPart.create({
        data: { serviceOrderId: id, ...priced },
      });
      return this.recomputeTotals(tx, id);
    });
  }

  async removePart(id: string, partId: string) {
    const so = await this.findOne(id);
    if (this.isTerminal(so.status) || so.status === ServiceOrderStatus.FINISHED) {
      throw new BadRequestException(`OS em estado ${so.status} não aceita edição`);
    }
    return this.prisma.$transaction(async (tx) => {
      const part = await tx.serviceOrderPart.findFirst({
        where: { id: partId, serviceOrderId: id },
      });
      if (!part) throw new NotFoundException('Peça não encontrada na OS');
      await tx.serviceOrderPart.delete({ where: { id: partId } });
      return this.recomputeTotals(tx, id);
    });
  }

  async addLabor(id: string, dto: CreateLaborInputDto) {
    const so = await this.findOne(id);
    if (this.isTerminal(so.status) || so.status === ServiceOrderStatus.FINISHED) {
      throw new BadRequestException(`OS em estado ${so.status} não aceita itens`);
    }
    return this.prisma.$transaction(async (tx) => {
      const [priced] = this.priceLabors([dto]);
      await tx.serviceOrderLabor.create({
        data: { serviceOrderId: id, ...priced },
      });
      return this.recomputeTotals(tx, id);
    });
  }

  async removeLabor(id: string, laborId: string) {
    const so = await this.findOne(id);
    if (this.isTerminal(so.status) || so.status === ServiceOrderStatus.FINISHED) {
      throw new BadRequestException(`OS em estado ${so.status} não aceita edição`);
    }
    return this.prisma.$transaction(async (tx) => {
      const lb = await tx.serviceOrderLabor.findFirst({
        where: { id: laborId, serviceOrderId: id },
      });
      if (!lb) throw new NotFoundException('Mão de obra não encontrada na OS');
      await tx.serviceOrderLabor.delete({ where: { id: laborId } });
      return this.recomputeTotals(tx, id);
    });
  }

  async changeStatus(id: string, dto: ChangeStatusDto) {
    const tenantId = requireTenantId();
    return this.prisma.$transaction(async (tx) => {
      const so = await tx.serviceOrder.findFirst({
        where: { id, tenantId },
        include: { parts: { include: { product: true } } },
      });
      if (!so) throw new NotFoundException('OS não encontrada');

      const allowed = ALLOWED_TRANSITIONS[so.status] ?? [];
      if (!allowed.includes(dto.status)) {
        throw new BadRequestException(
          `Transição inválida: ${so.status} → ${dto.status}`,
        );
      }

      // Baixa de estoque ao finalizar
      if (dto.status === ServiceOrderStatus.FINISHED) {
        const defWh = await tx.warehouse.findFirst({
          where: { tenantId, isDefault: true, active: true },
        });
        if (!defWh) throw new BadRequestException('Sem depósito padrão configurado');

        for (const part of so.parts) {
          if (!part.product.trackStock) continue;
          const stock = await tx.stock.findUnique({
            where: {
              productId_warehouseId: {
                productId: part.productId,
                warehouseId: defWh.id,
              },
            },
          });
          const current = stock?.quantity ?? new Prisma.Decimal(0);
          const newQty = current.sub(part.quantity);
          if (newQty.lt(0)) {
            throw new BadRequestException(
              `Estoque insuficiente para ${part.product.sku}: saldo=${current}, OS=${part.quantity}`,
            );
          }
          await tx.stock.upsert({
            where: {
              productId_warehouseId: {
                productId: part.productId,
                warehouseId: defWh.id,
              },
            },
            create: {
              tenantId,
              productId: part.productId,
              warehouseId: defWh.id,
              quantity: newQty,
              lastMovementAt: new Date(),
            },
            update: { quantity: newQty, lastMovementAt: new Date() },
          });
          await tx.stockMovement.create({
            data: {
              tenantId,
              productId: part.productId,
              warehouseId: defWh.id,
              type: StockMovementType.SERVICE_OUT,
              quantity: part.quantity.neg(),
              refType: 'ServiceOrder',
              refId: so.id,
            },
          });
        }
      }

      const updates: Prisma.ServiceOrderUpdateInput = { status: dto.status };
      if (dto.status === ServiceOrderStatus.FINISHED) updates.finishedAt = new Date();
      if (dto.status === ServiceOrderStatus.DELIVERED) updates.deliveredAt = new Date();
      if (dto.status === ServiceOrderStatus.CANCELLED) {
        updates.cancelledAt = new Date();
        updates.cancelReason = dto.reason;
      }

      return tx.serviceOrder.update({ where: { id }, data: updates });
    });
  }

  // ---------- helpers ----------

  private isTerminal(s: ServiceOrderStatus): boolean {
    const terminal: ServiceOrderStatus[] = [
      ServiceOrderStatus.CANCELLED,
      ServiceOrderStatus.DELIVERED,
    ];
    return terminal.includes(s);
  }

  private async priceParts(
    tx: Prisma.TransactionClient,
    tenantId: string,
    items: CreatePartInputDto[],
  ) {
    if (items.length === 0) return [];
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
      if (total.lt(0)) throw new BadRequestException('Desconto maior que valor da peça');
      return {
        productId: product.id,
        quantity,
        unitPrice,
        discount,
        total,
      };
    });
  }

  private priceLabors(items: CreateLaborInputDto[]) {
    return items.map((i) => {
      const quantity = new Prisma.Decimal(i.quantity ?? 1);
      const unitPrice = new Prisma.Decimal(i.unitPrice);
      const discount = new Prisma.Decimal(i.discount ?? 0);
      const total = unitPrice.mul(quantity).sub(discount);
      if (total.lt(0)) throw new BadRequestException('Desconto maior que valor da mão de obra');
      return {
        description: i.description,
        quantity,
        unitPrice,
        discount,
        total,
        performedBy: i.performedBy,
      };
    });
  }

  private async recomputeTotals(
    tx: Prisma.TransactionClient,
    id: string,
  ) {
    const so = await tx.serviceOrder.findUnique({
      where: { id },
      include: { parts: true, labors: true },
    });
    if (!so) throw new NotFoundException('OS não encontrada');
    const partsTotal = so.parts.reduce((acc, p) => acc.add(p.total), new Prisma.Decimal(0));
    const laborTotal = so.labors.reduce((acc, l) => acc.add(l.total), new Prisma.Decimal(0));
    const total = partsTotal.add(laborTotal).sub(so.discount);
    return tx.serviceOrder.update({
      where: { id },
      data: { partsTotal, laborTotal, total },
      include: { parts: true, labors: true },
    });
  }

  private async nextNumber(
    tx: Prisma.TransactionClient,
    tenantId: string,
  ): Promise<number> {
    const last = await tx.serviceOrder.findFirst({
      where: { tenantId },
      orderBy: { number: 'desc' },
      select: { number: true },
    });
    return (last?.number ?? 0) + 1;
  }
}
