import { Injectable } from '@nestjs/common';
import { PrismaService } from '../prisma.service';
import { CreateFeedbackDto } from './create-feedback.dto';

@Injectable()
export class FeedbacksService {
  constructor(private prisma: PrismaService) {}

  create(dto: CreateFeedbackDto) {
    return this.prisma.feedback.create({ data: dto });
  }

  findAll() {
    return this.prisma.feedback.findMany({ orderBy: { createdAt: 'desc' } });
  }

  findOne(id: number) {
    return this.prisma.feedback.findUnique({ where: { id } });
  }

  remove(id: number) {
    return this.prisma.feedback.delete({ where: { id } });
  }
}
