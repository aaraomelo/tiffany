import {
  Body,
  Controller,
  Delete,
  Get,
  NotFoundException,
  Param,
  ParseUUIDPipe,
  Patch,
  Post,
  Put,
  Query,
  Res,
  StreamableFile,
} from '@nestjs/common';
import type { Response } from 'express';
import { CheckPolicies } from '../access/check-policies.decorator';
import { RequiresModule } from '../common/decorators/requires-module.decorator';
import {
  CreateStudentDto,
  ListStudentDto,
  SetStudentPhotoDto,
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

  // ----- Foto do aluno -----

  @Get(':id/photo')
  @CheckPolicies({ action: 'read', subject: 'Student' })
  async getPhoto(
    @Param('id', ParseUUIDPipe) id: string,
    @Res({ passthrough: true }) res: Response,
  ): Promise<StreamableFile> {
    const photo = await this.service.getPhoto(id);
    if (!photo) throw new NotFoundException('Foto não encontrada');
    res.set({
      'Content-Type': photo.mimeType,
      'Cache-Control': 'private, max-age=60',
    });
    return new StreamableFile(Buffer.from(photo.data));
  }

  @Put(':id/photo')
  @CheckPolicies({ action: 'update', subject: 'Student' })
  setPhoto(
    @Param('id', ParseUUIDPipe) id: string,
    @Body() dto: SetStudentPhotoDto,
  ) {
    return this.service.setPhoto(id, dto);
  }

  @Delete(':id/photo')
  @CheckPolicies({ action: 'update', subject: 'Student' })
  deletePhoto(@Param('id', ParseUUIDPipe) id: string) {
    return this.service.deletePhoto(id);
  }
}
