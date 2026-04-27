import { Module } from '@nestjs/common';
import { MessagingModule } from '../messaging/messaging.module';
import { ClaudeBrainController } from './claude-brain.controller';

@Module({
  imports: [MessagingModule],
  controllers: [ClaudeBrainController],
})
export class ClaudeBrainModule {}
