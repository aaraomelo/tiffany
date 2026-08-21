import { Injectable, NotFoundException } from '@nestjs/common';
import { requireTenantId } from '../common/tenant-context/tenant-context';
import { PrismaService } from '../prisma/prisma.service';
import { CreateVehicleDto, UpdateVehicleDto } from './dto/vehicle.dto';

@Injectable()
export class VehicleService {
  constructor(private readonly prisma: PrismaService) {}

  create(dto: CreateVehicleDto) {
    return this.prisma.vehicle.create({
      data: { tenantId: requireTenantId(), ...dto },
    });
  }

  list(customerId?: string) {
    return this.prisma.vehicle.findMany({
      where: {
        tenantId: requireTenantId(),
        ...(customerId ? { customerId } : {}),
      },
      orderBy: { createdAt: 'desc' },
      include: { customer: { select: { id: true, name: true } } },
    });
  }

  async findOne(id: string) {
    const v = await this.prisma.vehicle.findFirst({
      where: { id, tenantId: requireTenantId() },
      include: { customer: { select: { id: true, name: true } } },
    });
    if (!v) throw new NotFoundException('Veículo não encontrado');
    return v;
  }

  async update(id: string, dto: UpdateVehicleDto) {
    await this.findOne(id);
    return this.prisma.vehicle.update({ where: { id }, data: dto });
  }

  async remove(id: string) {
    await this.findOne(id);
    await this.prisma.vehicle.delete({ where: { id } });
    return { ok: true };
  }
}
