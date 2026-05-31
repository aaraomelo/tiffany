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
  create(@Body() dto: CreateStudentDto) {
    return this.service.create(dto);
  }

  @Get()
  list(@Query() dto: ListStudentDto) {
    return this.service.list(dto);
  }

  @Get(':id')
  findOne(@Param('id', ParseUUIDPipe) id: string) {
    return this.service.findOne(id);
  }

  @Patch(':id')
  update(
    @Param('id', ParseUUIDPipe) id: string,
    @Body() dto: UpdateStudentDto,
  ) {
    return this.service.update(id, dto);
  }

  @Delete(':id')
  remove(@Param('id', ParseUUIDPipe) id: string) {
    return this.service.remove(id);
  }

  @Get(':id/health-record')
  getHealthRecord(@Param('id', ParseUUIDPipe) id: string) {
    return this.service.getHealthRecord(id);
  }

  @Put(':id/health-record')
  upsertHealthRecord(
    @Param('id', ParseUUIDPipe) id: string,
    @Body() dto: UpsertHealthRecordDto,
  ) {
    return this.service.upsertHealthRecord(id, dto);
  }
}
