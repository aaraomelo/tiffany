import {
  Body,
  Controller,
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
  CreateEnrollmentDto,
  GenerateTuitionDto,
  ListEnrollmentDto,
  UpdateEnrollmentDto,
} from './dto/enrollment.dto';
import { EnrollmentService } from './enrollment.service';

@RequiresModule('enrollment')
@Controller('enrollments')
export class EnrollmentController {
  constructor(private readonly service: EnrollmentService) {}

  @Post()
  @CheckPolicies({ action: 'create', subject: 'Enrollment' })
  create(@Body() dto: CreateEnrollmentDto) {
    return this.service.create(dto);
  }

  @Get()
  @CheckPolicies({ action: 'read', subject: 'Enrollment' })
  list(@Query() dto: ListEnrollmentDto) {
    return this.service.list(dto);
  }

  @Post('generate-tuitions')
  @CheckPolicies({ action: 'create', subject: 'Tuition' })
  generateBatch(@Body() dto: GenerateTuitionDto) {
    return this.service.generateTuitionsBatch(dto);
  }

  @Get(':id')
  @CheckPolicies({ action: 'read', subject: 'Enrollment' })
  findOne(@Param('id', ParseUUIDPipe) id: string) {
    return this.service.findOne(id);
  }

  @Patch(':id')
  @CheckPolicies({ action: 'update', subject: 'Enrollment' })
  update(
    @Param('id', ParseUUIDPipe) id: string,
    @Body() dto: UpdateEnrollmentDto,
  ) {
    return this.service.update(id, dto);
  }

  @Post(':id/generate-tuition')
  @CheckPolicies({ action: 'create', subject: 'Tuition' })
  generateTuition(
    @Param('id', ParseUUIDPipe) id: string,
    @Body() dto: GenerateTuitionDto,
  ) {
    return this.service.generateTuition(id, dto);
  }
}
