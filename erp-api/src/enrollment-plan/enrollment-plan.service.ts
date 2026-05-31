import { Injectable, NotFoundException } from '@nestjs/common';
import { Prisma } from '@prisma/client';
import { requireTenantId } from '../common/tenant-context/tenant-context';
import { PrismaService } from '../prisma/prisma.service';
import {
  CreateEnrollmentPlanDto,
  ListEnrollmentPlanDto,
  UpdateEnrollmentPlanDto,
} from './dto/enrollment-plan.dto';

@Injectable()
export class EnrollmentPlanService {
  constructor(private readonly prisma: PrismaService) {}

  create(dto: CreateEnrollmentPlanDto) {
    const tenantId = requireTenantId();
    return this.prisma.enrollmentPlan.create({ data: { tenantId, ...dto } });
  }

  list(dto: ListEnrollmentPlanDto) {
    const tenantId = requireTenantId();
    const where: Prisma.EnrollmentPlanWhereInput = {
      tenantId,
      ...(dto.active !== undefined ? { active: dto.active } : {}),
      ...(dto.q ? { name: { contains: dto.q, mode: 'insensitive' } } : {}),
    };
    return this.prisma.enrollmentPlan.findMany({
      where,
      orderBy: { name: 'asc' },
    });
  }

  async findOne(id: string) {
    const tenantId = requireTenantId();
    const plan = await this.prisma.enrollmentPlan.findFirst({
      where: { id, tenantId },
    });
    if (!plan) throw new NotFoundException('Plano não encontrado');
    return plan;
  }

  async update(id: string, dto: UpdateEnrollmentPlanDto) {
    await this.findOne(id);
    return this.prisma.enrollmentPlan.update({ where: { id }, data: dto });
  }

  async remove(id: string) {
    await this.findOne(id);
    // sem deletedAt no schema — desativa para preservar histórico de matrículas
    await this.prisma.enrollmentPlan.update({
      where: { id },
      data: { active: false },
    });
    return { ok: true };
  }
}
