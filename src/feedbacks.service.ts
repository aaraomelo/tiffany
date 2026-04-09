import { Injectable } from '@nestjs/common';
import { PrismaService } from './prisma.service';

@Injectable()
export class FeedbacksService {
  constructor(private prisma: PrismaService) {}

  async create(data: { nome: string; email: string; mensagem: string; nota: number }) {
    return this.prisma.feedback.create({ data });
  }

  async findAll() {
    return this.prisma.feedback.findMany({ orderBy: { createdAt: 'desc' } });
  }

  async findOne(id: string) {
    return this.prisma.feedback.findUnique({ where: { id } });
  }

  async update(id: string, data: { nome?: string; email?: string; mensagem?: string; nota?: number }) {
    return this.prisma.feedback.update({ where: { id }, data });
  }

  async remove(id: string) {
    return this.prisma.feedback.delete({ where: { id } });
  }
}
