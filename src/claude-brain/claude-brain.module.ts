import { Module } from '@nestjs/common';
import { MessagingModule } from '../messaging/messaging.module';
import { PrismaService } from '../prisma.service';
import { ClaudeBrainController } from './claude-brain.controller';
import { CodeChangeService } from './code-change.service';

@Module({
  imports: [MessagingModule],
  controllers: [ClaudeBrainController],
  providers: [PrismaService, CodeChangeService],
  exports: [CodeChangeService],
})
export class ClaudeBrainModule {}
