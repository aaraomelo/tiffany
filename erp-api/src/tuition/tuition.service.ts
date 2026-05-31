import {
  BadRequestException,
  Injectable,
  NotFoundException,
} from '@nestjs/common';
import { Prisma } from '@prisma/client';
import { requireTenantId } from '../common/tenant-context/tenant-context';
import { PrismaService } from '../prisma/prisma.service';
import { ListTuitionDto, PayTuitionDto } from './dto/tuition.dto';

@Injectable()
export class TuitionService {
  constructor(private readonly prisma: PrismaService) {}

  async list(dto: ListTuitionDto) {
    const tenantId = requireTenantId();
    const page = dto.page ?? 1;
    const pageSize = dto.pageSize ?? 50;

    const where: Prisma.TuitionWhereInput = {
      tenantId,
      ...(dto.studentId ? { studentId: dto.studentId } : {}),
      ...(dto.enrollmentId ? { enrollmentId: dto.enrollmentId } : {}),
      ...(dto.status ? { status: dto.status } : {}),
      ...(dto.referenceMonth ? { referenceMonth: dto.referenceMonth } : {}),
      ...(dto.referenceYear ? { referenceYear: dto.referenceYear } : {}),
    };

    const [items, total] = await Promise.all([
      this.prisma.tuition.findMany({
        where,
        orderBy: [{ dueDate: 'desc' }],
        include: { student: { select: { id: true, name: true } } },
        skip: (page - 1) * pageSize,
        take: pageSize,
      }),
      this.prisma.tuition.count({ where }),
    ]);

    return { items, total, page, pageSize };
  }

  async findOne(id: string) {
    const tenantId = requireTenantId();
    const tuition = await this.prisma.tuition.findFirst({
      where: { id, tenantId },
      include: {
        student: { select: { id: true, name: true } },
        enrollment: { include: { plan: true } },
      },
    });
    if (!tuition) throw new NotFoundException('Mensalidade não encontrada');
    return tuition;
  }

  async pay(id: string, dto: PayTuitionDto) {
    const tuition = await this.findOne(id);
    if (tuition.status === 'PAID') {
      throw new BadRequestException('Mensalidade já está paga');
    }
    if (tuition.status === 'CANCELLED') {
      throw new BadRequestException('Mensalidade cancelada não pode ser paga');
    }

    return this.prisma.tuition.update({
      where: { id },
      data: {
        status: 'PAID',
        paidAt: dto.paidAt ? new Date(dto.paidAt) : new Date(),
        paidAmount: dto.paidAmount ?? tuition.amount,
        ...(dto.paymentMethod ? { paymentMethod: dto.paymentMethod } : {}),
        ...(dto.notes !== undefined ? { notes: dto.notes } : {}),
      },
    });
  }

  async cancel(id: string) {
    const tuition = await this.findOne(id);
    if (tuition.status === 'PAID') {
      throw new BadRequestException('Mensalidade paga não pode ser cancelada');
    }
    return this.prisma.tuition.update({
      where: { id },
      data: { status: 'CANCELLED' },
    });
  }
}
