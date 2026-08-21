import { Module } from '@nestjs/common';
import { TuitionController } from './tuition.controller';
import { TuitionScheduler } from './tuition.scheduler';
import { TuitionService } from './tuition.service';

@Module({
  controllers: [TuitionController],
  providers: [TuitionService, TuitionScheduler],
  exports: [TuitionService],
})
export class TuitionModule {}
