import { Module } from '@nestjs/common';
import { FeedbacksController } from './feedbacks.controller';
import { FeedbacksService } from './feedbacks.service';
import { PrismaService } from '../prisma.service';

@Module({
  controllers: [FeedbacksController],
  providers: [FeedbacksService, PrismaService],
})
export class FeedbacksModule {}
