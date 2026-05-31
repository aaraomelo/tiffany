import { Injectable, NotFoundException } from '@nestjs/common';
import { Prisma } from '@prisma/client';
import { requireTenantId } from '../common/tenant-context/tenant-context';
import { PrismaService } from '../prisma/prisma.service';
import {
  CreateStudentDto,
  ListStudentDto,
  UpdateStudentDto,
  UpsertHealthRecordDto,
} from './dto/student.dto';

@Injectable()
export class StudentService {
  constructor(private readonly prisma: PrismaService) {}

  create(dto: CreateStudentDto) {
    const tenantId = requireTenantId();
    const { birthDate, ...rest } = dto;
    return this.prisma.student.create({
      data: {
        tenantId,
        ...rest,
        ...(birthDate ? { birthDate: new Date(birthDate) } : {}),
      },
    });
  }

  async list(dto: ListStudentDto) {
    const tenantId = requireTenantId();
    const page = dto.page ?? 1;
    const pageSize = dto.pageSize ?? 20;

    const where: Prisma.StudentWhereInput = {
      tenantId,
      deletedAt: null,
      ...(dto.status ? { status: dto.status } : {}),
      ...(dto.q
        ? {
            OR: [
              { name: { contains: dto.q, mode: 'insensitive' } },
              { document: { contains: dto.q } },
              { email: { contains: dto.q, mode: 'insensitive' } },
              { phone: { contains: dto.q } },
              { guardianName: { contains: dto.q, mode: 'insensitive' } },
            ],
          }
        : {}),
    };

    const [items, total] = await Promise.all([
      this.prisma.student.findMany({
        where,
        orderBy: { name: 'asc' },
        skip: (page - 1) * pageSize,
        take: pageSize,
      }),
      this.prisma.student.count({ where }),
    ]);

    return { items, total, page, pageSize };
  }

  async findOne(id: string) {
    const tenantId = requireTenantId();
    const student = await this.prisma.student.findFirst({
      where: { id, tenantId, deletedAt: null },
      include: {
        healthRecord: true,
        enrollments: {
          orderBy: { createdAt: 'desc' },
          include: { plan: true },
        },
      },
    });
    if (!student) throw new NotFoundException('Aluno não encontrado');
    return student;
  }

  private async ensureExists(id: string) {
    const tenantId = requireTenantId();
    const student = await this.prisma.student.findFirst({
      where: { id, tenantId, deletedAt: null },
      select: { id: true },
    });
    if (!student) throw new NotFoundException('Aluno não encontrado');
    return tenantId;
  }

  async update(id: string, dto: UpdateStudentDto) {
    await this.ensureExists(id);
    const { birthDate, ...rest } = dto;
    return this.prisma.student.update({
      where: { id },
      data: {
        ...rest,
        ...(birthDate !== undefined
          ? { birthDate: birthDate ? new Date(birthDate) : null }
          : {}),
      },
    });
  }

  async remove(id: string) {
    await this.ensureExists(id);
    await this.prisma.student.update({
      where: { id },
      data: { deletedAt: new Date() },
    });
    return { ok: true };
  }

  // ----- HealthRecord (1:1 sub-recurso) -----

  async getHealthRecord(studentId: string) {
    await this.ensureExists(studentId);
    return this.prisma.healthRecord.findUnique({ where: { studentId } });
  }

  async upsertHealthRecord(studentId: string, dto: UpsertHealthRecordDto) {
    const tenantId = await this.ensureExists(studentId);
    const { medicalClearanceDate, ...rest } = dto;
    const data = {
      ...rest,
      ...(medicalClearanceDate !== undefined
        ? {
            medicalClearanceDate: medicalClearanceDate
              ? new Date(medicalClearanceDate)
              : null,
          }
        : {}),
    };
    return this.prisma.healthRecord.upsert({
      where: { studentId },
      create: { tenantId, studentId, ...data },
      update: data,
    });
  }
}
