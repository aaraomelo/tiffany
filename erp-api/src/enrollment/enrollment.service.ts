import {
  ConflictException,
  Injectable,
  NotFoundException,
} from '@nestjs/common';
import { Prisma } from '@prisma/client';
import { requireTenantId } from '../common/tenant-context/tenant-context';
import { PrismaService } from '../prisma/prisma.service';
import {
  CreateEnrollmentDto,
  GenerateTuitionDto,
  ListEnrollmentDto,
  UpdateEnrollmentDto,
} from './dto/enrollment.dto';

@Injectable()
export class EnrollmentService {
  constructor(private readonly prisma: PrismaService) {}

  async create(dto: CreateEnrollmentDto) {
    const tenantId = requireTenantId();

    const [student, plan] = await Promise.all([
      this.prisma.student.findFirst({
        where: { id: dto.studentId, tenantId, deletedAt: null },
        select: { id: true },
      }),
      this.prisma.enrollmentPlan.findFirst({
        where: { id: dto.planId, tenantId },
        select: { id: true, price: true },
      }),
    ]);
    if (!student) throw new NotFoundException('Aluno não encontrado');
    if (!plan) throw new NotFoundException('Plano não encontrado');

    return this.prisma.enrollment.create({
      data: {
        tenantId,
        studentId: dto.studentId,
        planId: dto.planId,
        dueDay: dto.dueDay ?? 10,
        monthlyPrice: dto.monthlyPrice ?? plan.price,
        notes: dto.notes,
        ...(dto.startDate ? { startDate: new Date(dto.startDate) } : {}),
        ...(dto.endDate ? { endDate: new Date(dto.endDate) } : {}),
      },
      include: { plan: true, student: { select: { id: true, name: true } } },
    });
  }

  list(dto: ListEnrollmentDto) {
    const tenantId = requireTenantId();
    const where: Prisma.EnrollmentWhereInput = {
      tenantId,
      ...(dto.studentId ? { studentId: dto.studentId } : {}),
      ...(dto.status ? { status: dto.status } : {}),
    };
    return this.prisma.enrollment.findMany({
      where,
      orderBy: { createdAt: 'desc' },
      include: {
        plan: true,
        student: { select: { id: true, name: true } },
      },
    });
  }

  async findOne(id: string) {
    const tenantId = requireTenantId();
    const enrollment = await this.prisma.enrollment.findFirst({
      where: { id, tenantId },
      include: {
        plan: true,
        student: { select: { id: true, name: true } },
        tuitions: { orderBy: [{ referenceYear: 'desc' }, { referenceMonth: 'desc' }] },
      },
    });
    if (!enrollment) throw new NotFoundException('Matrícula não encontrada');
    return enrollment;
  }

  async update(id: string, dto: UpdateEnrollmentDto) {
    const tenantId = requireTenantId();
    const existing = await this.prisma.enrollment.findFirst({
      where: { id, tenantId },
      select: { id: true },
    });
    if (!existing) throw new NotFoundException('Matrícula não encontrada');

    const { startDate, endDate, studentId, planId, ...rest } = dto;
    return this.prisma.enrollment.update({
      where: { id },
      data: {
        ...rest,
        ...(startDate ? { startDate: new Date(startDate) } : {}),
        ...(endDate !== undefined
          ? { endDate: endDate ? new Date(endDate) : null }
          : {}),
      },
    });
  }

  /**
   * Gera em lote a mensalidade do mês de referência para todas as matrículas
   * ACTIVE do tenant que ainda não a possuem. Idempotente: matrículas que já
   * têm a mensalidade do mês são puladas.
   */
  async generateTuitionsBatch(dto: GenerateTuitionDto) {
    const tenantId = requireTenantId();

    const enrollments = await this.prisma.enrollment.findMany({
      where: { tenantId, status: 'ACTIVE' },
      select: { id: true, studentId: true, dueDay: true, monthlyPrice: true },
    });

    const existing = await this.prisma.tuition.findMany({
      where: {
        tenantId,
        referenceYear: dto.referenceYear,
        referenceMonth: dto.referenceMonth,
      },
      select: { enrollmentId: true },
    });
    const already = new Set(existing.map((e) => e.enrollmentId));
    const toCreate = enrollments.filter((e) => !already.has(e.id));

    if (toCreate.length > 0) {
      await this.prisma.tuition.createMany({
        data: toCreate.map((e) => ({
          tenantId,
          studentId: e.studentId,
          enrollmentId: e.id,
          referenceMonth: dto.referenceMonth,
          referenceYear: dto.referenceYear,
          dueDate: new Date(
            Date.UTC(dto.referenceYear, dto.referenceMonth - 1, e.dueDay),
          ),
          amount: e.monthlyPrice,
        })),
      });
    }

    return {
      created: toCreate.length,
      skipped: enrollments.length - toCreate.length,
      activeEnrollments: enrollments.length,
    };
  }

  /** Gera (ou retorna conflito se já existe) a mensalidade do mês de referência. */
  async generateTuition(id: string, dto: GenerateTuitionDto) {
    const enrollment = await this.findOne(id);

    const existing = await this.prisma.tuition.findUnique({
      where: {
        enrollmentId_referenceYear_referenceMonth: {
          enrollmentId: id,
          referenceYear: dto.referenceYear,
          referenceMonth: dto.referenceMonth,
        },
      },
    });
    if (existing) {
      throw new ConflictException('Mensalidade desse mês já foi gerada');
    }

    const dueDate = new Date(
      Date.UTC(dto.referenceYear, dto.referenceMonth - 1, enrollment.dueDay),
    );

    return this.prisma.tuition.create({
      data: {
        tenantId: enrollment.tenantId,
        studentId: enrollment.studentId,
        enrollmentId: enrollment.id,
        referenceMonth: dto.referenceMonth,
        referenceYear: dto.referenceYear,
        dueDate,
        amount: enrollment.monthlyPrice,
      },
    });
  }
}
