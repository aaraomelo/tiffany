import { Module } from '@nestjs/common';
import { MessagingModule } from '../messaging/messaging.module';
import { PrismaService } from '../prisma.service';
import { ClaudeBrainController } from './claude-brain.controller';

@Module({
  imports: [MessagingModule],
  controllers: [ClaudeBrainController],
  providers: [PrismaService],
})
export class ClaudeBrainModule {}
