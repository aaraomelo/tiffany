import {
  BadRequestException,
  Injectable,
  NotFoundException,
} from '@nestjs/common';
import { Prisma, StockMovementType } from '@prisma/client';
import { getTenantContext, requireTenantId } from '../common/tenant-context/tenant-context';
import { PrismaService } from '../prisma/prisma.service';
import { CreateStockMovementDto, ListStockDto } from './dto/stock.dto';

const INBOUND_TYPES: StockMovementType[] = [
  StockMovementType.PURCHASE_IN,
  StockMovementType.ADJUST_IN,
  StockMovementType.TRANSFER_IN,
  StockMovementType.RETURN_IN,
  StockMovementType.PRODUCTION_IN,
];

const OUTBOUND_TYPES: StockMovementType[] = [
  StockMovementType.SALE_OUT,
  StockMovementType.ADJUST_OUT,
  StockMovementType.TRANSFER_OUT,
  StockMovementType.SERVICE_OUT,
  StockMovementType.RETURN_OUT,
  StockMovementType.LOSS,
];

function direction(type: StockMovementType): 1 | -1 {
  if (INBOUND_TYPES.includes(type)) return 1;
  if (OUTBOUND_TYPES.includes(type)) return -1;
  throw new BadRequestException(`Tipo de movimento não suportado: ${type}`);
}

@Injectable()
export class StockService {
  constructor(private readonly prisma: PrismaService) {}

  async listStock(dto: ListStockDto) {
    const tenantId = requireTenantId();
    const page = dto.page ?? 1;
    const pageSize = dto.pageSize ?? 50;

    const where: Prisma.StockWhereInput = {
      tenantId,
      ...(dto.productId ? { productId: dto.productId } : {}),
      ...(dto.warehouseId ? { warehouseId: dto.warehouseId } : {}),
    };

    const [items, total] = await Promise.all([
      this.prisma.stock.findMany({
        where,
        orderBy: [{ warehouseId: 'asc' }, { quantity: 'desc' }],
        skip: (page - 1) * pageSize,
        take: pageSize,
        include: {
          product: {
            select: {
              id: true,
              sku: true,
              name: true,
              unit: { select: { code: true } },
            },
          },
          warehouse: { select: { id: true, code: true, name: true } },
        },
      }),
      this.prisma.stock.count({ where }),
    ]);

    return { items, total, page, pageSize };
  }

  async listMovements(productId?: string, warehouseId?: string) {
    const tenantId = requireTenantId();
    return this.prisma.stockMovement.findMany({
      where: {
        tenantId,
        ...(productId ? { productId } : {}),
        ...(warehouseId ? { warehouseId } : {}),
      },
      orderBy: { createdAt: 'desc' },
      take: 100,
      include: {
        product: { select: { sku: true, name: true } },
        warehouse: { select: { code: true } },
      },
    });
  }

  /// Cria movimento + atualiza saldo Stock em transação.
  /// PURCHASE_IN recalcula custo médio ponderado.
  async createMovement(dto: CreateStockMovementDto) {
    const tenantId = requireTenantId();
    const { userId } = getTenantContext();
    const dir = direction(dto.type);
    const signedQty = new Prisma.Decimal(dto.quantity).mul(dir);

    return this.prisma.$transaction(async (tx) => {
      const product = await tx.product.findFirst({
        where: { id: dto.productId, tenantId, deletedAt: null },
      });
      if (!product) throw new NotFoundException('Produto não encontrado');

      const warehouse = await tx.warehouse.findFirst({
        where: { id: dto.warehouseId, tenantId },
      });
      if (!warehouse) throw new NotFoundException('Depósito não encontrado');

      const current = await tx.stock.findUnique({
        where: {
          productId_warehouseId: {
            productId: dto.productId,
            warehouseId: dto.warehouseId,
          },
        },
      });

      const currentQty = current?.quantity ?? new Prisma.Decimal(0);
      const newQty = currentQty.add(signedQty);

      if (newQty.lt(0)) {
        throw new BadRequestException(
          `Estoque insuficiente: atual=${currentQty.toString()}, movimento=${signedQty.toString()}`,
        );
      }

      // Custo médio ponderado em PURCHASE_IN, mantém em outros casos
      let newAvgCost = current?.avgCost ?? new Prisma.Decimal(0);
      if (dto.type === StockMovementType.PURCHASE_IN && dto.unitCost) {
        const inputCost = new Prisma.Decimal(dto.unitCost);
        const inputQty = new Prisma.Decimal(dto.quantity);
        if (currentQty.lte(0)) {
          newAvgCost = inputCost;
        } else {
          newAvgCost = currentQty
            .mul(newAvgCost)
            .add(inputQty.mul(inputCost))
            .div(currentQty.add(inputQty));
        }
      }

      const stock = await tx.stock.upsert({
        where: {
          productId_warehouseId: {
            productId: dto.productId,
            warehouseId: dto.warehouseId,
          },
        },
        create: {
          tenantId,
          productId: dto.productId,
          warehouseId: dto.warehouseId,
          quantity: newQty,
          avgCost: newAvgCost,
          lastMovementAt: new Date(),
        },
        update: {
          quantity: newQty,
          avgCost: newAvgCost,
          lastMovementAt: new Date(),
        },
      });

      const movement = await tx.stockMovement.create({
        data: {
          tenantId,
          productId: dto.productId,
          warehouseId: dto.warehouseId,
          type: dto.type,
          quantity: signedQty,
          unitCost: dto.unitCost,
          refType: dto.refType,
          refId: dto.refId,
          notes: dto.notes,
          createdBy: userId ?? undefined,
        },
      });

      return { movement, stock };
    });
  }
}
