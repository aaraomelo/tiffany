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
  create(@Body() dto: CreateEnrollmentDto) {
    return this.service.create(dto);
  }

  @Get()
  list(@Query() dto: ListEnrollmentDto) {
    return this.service.list(dto);
  }

  @Get(':id')
  findOne(@Param('id', ParseUUIDPipe) id: string) {
    return this.service.findOne(id);
  }

  @Patch(':id')
  update(
    @Param('id', ParseUUIDPipe) id: string,
    @Body() dto: UpdateEnrollmentDto,
  ) {
    return this.service.update(id, dto);
  }

  @Post(':id/generate-tuition')
  generateTuition(
    @Param('id', ParseUUIDPipe) id: string,
    @Body() dto: GenerateTuitionDto,
  ) {
    return this.service.generateTuition(id, dto);
  }
}
