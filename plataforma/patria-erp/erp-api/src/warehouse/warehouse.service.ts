import { Injectable, NotFoundException } from '@nestjs/common';
import { requireTenantId } from '../common/tenant-context/tenant-context';
import { PrismaService } from '../prisma/prisma.service';
import { CreateWarehouseDto, UpdateWarehouseDto } from './dto/warehouse.dto';

@Injectable()
export class WarehouseService {
  constructor(private readonly prisma: PrismaService) {}

  create(dto: CreateWarehouseDto) {
    return this.prisma.warehouse.create({
      data: { tenantId: requireTenantId(), ...dto },
    });
  }

  list() {
    return this.prisma.warehouse.findMany({
      where: { tenantId: requireTenantId() },
      orderBy: { code: 'asc' },
    });
  }

  async findOne(id: string) {
    const entity = await this.prisma.warehouse.findFirst({
      where: { id, tenantId: requireTenantId() },
    });
    if (!entity) throw new NotFoundException('Depósito não encontrado');
    return entity;
  }

  async update(id: string, dto: UpdateWarehouseDto) {
    await this.findOne(id);
    return this.prisma.warehouse.update({ where: { id }, data: dto });
  }
}
