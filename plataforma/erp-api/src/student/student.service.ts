import {
  BadRequestException,
  Injectable,
  NotFoundException,
} from '@nestjs/common';
import { Prisma } from '@prisma/client';
import { requireTenantId } from '../common/tenant-context/tenant-context';
import { PrismaService } from '../prisma/prisma.service';
import {
  CreateStudentDto,
  ListStudentDto,
  SetStudentPhotoDto,
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
              { fatherName: { contains: dto.q, mode: 'insensitive' } },
              { motherName: { contains: dto.q, mode: 'insensitive' } },
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

  // ----- Foto (1:1 sub-recurso, binário em StudentPhoto) -----

  private static readonly MAX_PHOTO_BYTES = 2 * 1024 * 1024; // 2MB (já vem redimensionada)
  private static readonly ALLOWED_MIME = ['image/jpeg', 'image/png', 'image/webp'];

  async getPhoto(studentId: string) {
    await this.ensureExists(studentId);
    return this.prisma.studentPhoto.findUnique({ where: { studentId } });
  }

  async setPhoto(studentId: string, dto: SetStudentPhotoDto) {
    const tenantId = await this.ensureExists(studentId);

    // Aceita data URL ("data:image/jpeg;base64,...") ou base64 puro + mimeType.
    let mimeType = dto.mimeType ?? 'image/jpeg';
    let b64 = dto.dataBase64;
    const match = /^data:([^;]+);base64,(.*)$/s.exec(dto.dataBase64);
    if (match) {
      mimeType = match[1];
      b64 = match[2];
    }

    if (!StudentService.ALLOWED_MIME.includes(mimeType)) {
      throw new BadRequestException('Formato de imagem não suportado (use JPEG, PNG ou WebP)');
    }

    const data = Buffer.from(b64, 'base64');
    if (data.length === 0) {
      throw new BadRequestException('Imagem inválida');
    }
    if (data.length > StudentService.MAX_PHOTO_BYTES) {
      throw new BadRequestException('Imagem muito grande (máximo 2MB)');
    }

    const now = new Date();
    await this.prisma.$transaction([
      this.prisma.studentPhoto.upsert({
        where: { studentId },
        create: { tenantId, studentId, data, mimeType },
        update: { data, mimeType },
      }),
      this.prisma.student.update({
        where: { id: studentId },
        data: { photoUpdatedAt: now },
      }),
    ]);
    return { ok: true, photoUpdatedAt: now };
  }

  async deletePhoto(studentId: string) {
    await this.ensureExists(studentId);
    await this.prisma.$transaction([
      this.prisma.studentPhoto.deleteMany({ where: { studentId } }),
      this.prisma.student.update({
        where: { id: studentId },
        data: { photoUpdatedAt: null },
      }),
    ]);
    return { ok: true };
  }
}
