import {
  Body,
  Controller,
  Delete,
  Get,
  Param,
  ParseUUIDPipe,
  Patch,
  Post,
  Query,
} from '@nestjs/common';
import { CheckPolicies } from '../access/check-policies.decorator';
import { RequiresModule } from '../common/decorators/requires-module.decorator';
import {
  CreateEnrollmentPlanDto,
  ListEnrollmentPlanDto,
  UpdateEnrollmentPlanDto,
} from './dto/enrollment-plan.dto';
import { EnrollmentPlanService } from './enrollment-plan.service';

@RequiresModule('enrollment-plan')
@Controller('enrollment-plans')
export class EnrollmentPlanController {
  constructor(private readonly service: EnrollmentPlanService) {}

  @Post()
  @CheckPolicies({ action: 'create', subject: 'EnrollmentPlan' })
  create(@Body() dto: CreateEnrollmentPlanDto) {
    return this.service.create(dto);
  }

  @Get()
  @CheckPolicies({ action: 'read', subject: 'EnrollmentPlan' })
  list(@Query() dto: ListEnrollmentPlanDto) {
    return this.service.list(dto);
  }

  @Get(':id')
  @CheckPolicies({ action: 'read', subject: 'EnrollmentPlan' })
  findOne(@Param('id', ParseUUIDPipe) id: string) {
    return this.service.findOne(id);
  }

  @Patch(':id')
  @CheckPolicies({ action: 'update', subject: 'EnrollmentPlan' })
  update(
    @Param('id', ParseUUIDPipe) id: string,
    @Body() dto: UpdateEnrollmentPlanDto,
  ) {
    return this.service.update(id, dto);
  }

  @Delete(':id')
  @CheckPolicies({ action: 'delete', subject: 'EnrollmentPlan' })
  remove(@Param('id', ParseUUIDPipe) id: string) {
    return this.service.remove(id);
  }
}
