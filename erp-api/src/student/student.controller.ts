import {
  Body,
  Controller,
  Delete,
  Get,
  Param,
  ParseUUIDPipe,
  Patch,
  Post,
  Put,
  Query,
} from '@nestjs/common';
import { CheckPolicies } from '../access/check-policies.decorator';
import { RequiresModule } from '../common/decorators/requires-module.decorator';
import {
  CreateStudentDto,
  ListStudentDto,
  UpdateStudentDto,
  UpsertHealthRecordDto,
} from './dto/student.dto';
import { StudentService } from './student.service';

@RequiresModule('student')
@Controller('students')
export class StudentController {
  constructor(private readonly service: StudentService) {}

  @Post()
  @CheckPolicies({ action: 'create', subject: 'Student' })
  create(@Body() dto: CreateStudentDto) {
    return this.service.create(dto);
  }

  @Get()
  @CheckPolicies({ action: 'read', subject: 'Student' })
  list(@Query() dto: ListStudentDto) {
    return this.service.list(dto);
  }

  @Get(':id')
  @CheckPolicies({ action: 'read', subject: 'Student' })
  findOne(@Param('id', ParseUUIDPipe) id: string) {
    return this.service.findOne(id);
  }

  @Patch(':id')
  @CheckPolicies({ action: 'update', subject: 'Student' })
  update(
    @Param('id', ParseUUIDPipe) id: string,
    @Body() dto: UpdateStudentDto,
  ) {
    return this.service.update(id, dto);
  }

  @Delete(':id')
  @CheckPolicies({ action: 'delete', subject: 'Student' })
  remove(@Param('id', ParseUUIDPipe) id: string) {
    return this.service.remove(id);
  }

  @Get(':id/health-record')
  @CheckPolicies({ action: 'read', subject: 'Student' })
  getHealthRecord(@Param('id', ParseUUIDPipe) id: string) {
    return this.service.getHealthRecord(id);
  }

  @Put(':id/health-record')
  @CheckPolicies({ action: 'update', subject: 'Student' })
  upsertHealthRecord(
    @Param('id', ParseUUIDPipe) id: string,
    @Body() dto: UpsertHealthRecordDto,
  ) {
    return this.service.upsertHealthRecord(id, dto);
  }
}
