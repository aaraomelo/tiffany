import { Module } from '@nestjs/common';
import { MessagingModule } from '../messaging/messaging.module';
import { PrismaService } from '../prisma.service';
import { ClaudeBrainController } from './claude-brain.controller';
import { CodeChangeService } from './code-change.service';
import { OrganismStateService } from './organism-state.service';
import { ImagesController } from './images.controller';
import { AudioController } from './audio.controller';
import { MediaQueueService } from './media-queue.service';
import { CouncilService } from './council.service';

@Module({
  imports: [MessagingModule],
  controllers: [ClaudeBrainController, ImagesController, AudioController],
  providers: [PrismaService, CodeChangeService, OrganismStateService, MediaQueueService, CouncilService],
  exports: [CodeChangeService, OrganismStateService, MediaQueueService, CouncilService],
})
export class ClaudeBrainModule {}
