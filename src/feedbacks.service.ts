import { Injectable, NotFoundException } from '@nestjs/common';
import { PrismaService } from './prisma.service';

@Injectable()
export class FeedbacksService {
  constructor(private prisma: PrismaService) {}

  async create(data: { nome: string; email: string; nota: number; mensagem?: string }) {
    return this.prisma.feedback.create({ data });
  }

  async findAll() {
    return this.prisma.feedback.findMany({ orderBy: { createdAt: 'desc' } });
  }

  async findOne(id: string) {
    const feedback = await this.prisma.feedback.findUnique({ where: { id } });
    if (!feedback) throw new NotFoundException('Feedback não encontrado');
    return feedback;
  }

  async update(id: string, data: { nome?: string; email?: string; nota?: number; mensagem?: string }) {
    await this.findOne(id);
    return this.prisma.feedback.update({ where: { id }, data });
  }

  async remove(id: string) {
    await this.findOne(id);
    return this.prisma.feedback.delete({ where: { id } });
  }
}
