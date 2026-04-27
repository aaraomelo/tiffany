import { Module } from '@nestjs/common';
import { MessagingModule } from '../messaging/messaging.module';
import { PrismaService } from '../prisma.service';
import { ClaudeBrainController } from './claude-brain.controller';
import { CodeChangeService } from './code-change.service';
import { OrganismStateService } from './organism-state.service';

@Module({
  imports: [MessagingModule],
  controllers: [ClaudeBrainController],
  providers: [PrismaService, CodeChangeService, OrganismStateService],
  exports: [CodeChangeService, OrganismStateService],
})
export class ClaudeBrainModule {}
