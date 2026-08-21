import {
  Body,
  Controller,
  Get,
  Param,
  ParseUUIDPipe,
  Put,
  Query,
} from '@nestjs/common';
import { CheckPolicies } from '../access/check-policies.decorator';
import { RequiresModule } from '../common/decorators/requires-module.decorator';
import { UpsertHealthRecordDto } from '../student/dto/student.dto';
import { HealthService } from './health.service';

// Módulo Saúde: ficha de saúde / anamnese das pessoas (alunos, pacientes,
// membros). Genérico — serve escola, clínica, academia, etc.
@RequiresModule('health-record')
@Controller('health')
export class HealthController {
  constructor(private readonly service: HealthService) {}

  @Get('people')
  @CheckPolicies({ action: 'read', subject: 'Student' })
  listPeople(@Query('q') q?: string) {
    return this.service.listPeople(q);
  }

  @Get('people/:id')
  @CheckPolicies({ action: 'read', subject: 'Student' })
  getRecord(@Param('id', ParseUUIDPipe) id: string) {
    return this.service.getRecord(id);
  }

  @Put('people/:id')
  @CheckPolicies({ action: 'update', subject: 'Student' })
  upsertRecord(
    @Param('id', ParseUUIDPipe) id: string,
    @Body() dto: UpsertHealthRecordDto,
  ) {
    return this.service.upsertRecord(id, dto);
  }
}
