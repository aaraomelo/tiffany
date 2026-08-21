import { Module } from '@nestjs/common';
import { EnrollmentPlanController } from './enrollment-plan.controller';
import { EnrollmentPlanService } from './enrollment-plan.service';

@Module({
  controllers: [EnrollmentPlanController],
  providers: [EnrollmentPlanService],
  exports: [EnrollmentPlanService],
})
export class EnrollmentPlanModule {}
