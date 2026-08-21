import { Injectable, NotFoundException } from '@nestjs/common';
import { Prisma } from '@prisma/client';
import { requireTenantId } from '../common/tenant-context/tenant-context';
import { PrismaService } from '../prisma/prisma.service';
import { UpsertHealthRecordDto } from '../student/dto/student.dto';
import { StudentService } from '../student/student.service';

@Injectable()
export class HealthService {
  constructor(
    private readonly prisma: PrismaService,
    private readonly students: StudentService,
  ) {}

  // Lista pessoas (alunos/pacientes) com resumo da ficha de saúde.
  async listPeople(q?: string) {
    const tenantId = requireTenantId();
    const where: Prisma.StudentWhereInput = {
      tenantId,
      deletedAt: null,
      ...(q
        ? {
            OR: [
              { name: { contains: q, mode: 'insensitive' } },
              { document: { contains: q } },
              { phone: { contains: q } },
            ],
          }
        : {}),
    };
    const people = await this.prisma.student.findMany({
      where,
      orderBy: { name: 'asc' },
      select: {
        id: true,
        name: true,
        phone: true,
        birthDate: true,
        healthRecord: {
          select: {
            id: true,
            bloodType: true,
            medicalClearance: true,
            allergies: true,
            chronicConditions: true,
          },
        },
      },
    });
    return people.map((p) => ({
      id: p.id,
      name: p.name,
      phone: p.phone,
      birthDate: p.birthDate,
      hasRecord: !!p.healthRecord,
      bloodType: p.healthRecord?.bloodType ?? null,
      medicalClearance: p.healthRecord?.medicalClearance ?? false,
      hasAlerts: !!(p.healthRecord?.allergies || p.healthRecord?.chronicConditions),
    }));
  }

  async getRecord(studentId: string) {
    const record = await this.students.getHealthRecord(studentId); // valida tenant/aluno
    const tenantId = requireTenantId();
    const student = await this.prisma.student.findFirst({
      where: { id: studentId, tenantId, deletedAt: null },
      select: { id: true, name: true, birthDate: true, phone: true },
    });
    if (!student) throw new NotFoundException('Pessoa não encontrada');
    return { student, record };
  }

  upsertRecord(studentId: string, dto: UpsertHealthRecordDto) {
    return this.students.upsertHealthRecord(studentId, dto);
  }
}
