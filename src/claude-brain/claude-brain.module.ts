import { Module } from '@nestjs/common';
import { MessagingModule } from '../messaging/messaging.module';
import { PrismaService } from '../prisma.service';
import { ClaudeBrainController } from './claude-brain.controller';
import { CodeChangeService } from './code-change.service';
import { OrganismStateService } from './organism-state.service';
import { ImagesController } from './images.controller';

@Module({
  imports: [MessagingModule],
  controllers: [ClaudeBrainController, ImagesController],
  providers: [PrismaService, CodeChangeService, OrganismStateService],
  exports: [CodeChangeService, OrganismStateService],
})
export class ClaudeBrainModule {}
