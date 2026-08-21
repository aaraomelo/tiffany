import {
  Injectable,
  NotFoundException,
} from '@nestjs/common';
import { Prisma } from '@prisma/client';
import { requireTenantId } from '../common/tenant-context/tenant-context';
import { PrismaService } from '../prisma/prisma.service';
import { CreateCustomerSupplierDto } from './dto/create-customer-supplier.dto';
import { ListCustomerSupplierDto } from './dto/list-customer-supplier.dto';
import { UpdateCustomerSupplierDto } from './dto/update-customer-supplier.dto';

@Injectable()
export class CustomerSupplierService {
  constructor(private readonly prisma: PrismaService) {}

  create(dto: CreateCustomerSupplierDto) {
    const tenantId = requireTenantId();
    return this.prisma.customerSupplier.create({
      data: { tenantId, ...dto },
    });
  }

  async list(dto: ListCustomerSupplierDto) {
    const tenantId = requireTenantId();
    const page = dto.page ?? 1;
    const pageSize = dto.pageSize ?? 20;

    const where: Prisma.CustomerSupplierWhereInput = {
      tenantId,
      deletedAt: null,
      ...(dto.role ? { role: dto.role } : {}),
      ...(dto.q
        ? {
            OR: [
              { name: { contains: dto.q, mode: 'insensitive' } },
              { tradeName: { contains: dto.q, mode: 'insensitive' } },
              { document: { contains: dto.q } },
              { email: { contains: dto.q, mode: 'insensitive' } },
            ],
          }
        : {}),
    };

    const [items, total] = await Promise.all([
      this.prisma.customerSupplier.findMany({
        where,
        orderBy: { name: 'asc' },
        skip: (page - 1) * pageSize,
        take: pageSize,
      }),
      this.prisma.customerSupplier.count({ where }),
    ]);

    return { items, total, page, pageSize };
  }

  async findOne(id: string) {
    const tenantId = requireTenantId();
    const entity = await this.prisma.customerSupplier.findFirst({
      where: { id, tenantId, deletedAt: null },
    });
    if (!entity) throw new NotFoundException('Cadastro não encontrado');
    return entity;
  }

  async update(id: string, dto: UpdateCustomerSupplierDto) {
    await this.findOne(id);
    return this.prisma.customerSupplier.update({
      where: { id },
      data: dto,
    });
  }

  async remove(id: string) {
    await this.findOne(id);
    await this.prisma.customerSupplier.update({
      where: { id },
      data: { deletedAt: new Date() },
    });
    return { ok: true };
  }
}
