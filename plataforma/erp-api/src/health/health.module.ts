import { Module } from '@nestjs/common';
import { StudentModule } from '../student/student.module';
import { HealthController } from './health.controller';
import { HealthService } from './health.service';

@Module({
  imports: [StudentModule],
  controllers: [HealthController],
  providers: [HealthService],
})
export class HealthModule {}
