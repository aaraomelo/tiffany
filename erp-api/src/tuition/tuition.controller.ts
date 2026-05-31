import {
  Body,
  Controller,
  Get,
  Param,
  ParseUUIDPipe,
  Post,
  Query,
} from '@nestjs/common';
import { RequiresModule } from '../common/decorators/requires-module.decorator';
import { ListTuitionDto, PayTuitionDto } from './dto/tuition.dto';
import { TuitionService } from './tuition.service';

@RequiresModule('tuition')
@Controller('tuitions')
export class TuitionController {
  constructor(private readonly service: TuitionService) {}

  @Get()
  list(@Query() dto: ListTuitionDto) {
    return this.service.list(dto);
  }

  @Get(':id')
  findOne(@Param('id', ParseUUIDPipe) id: string) {
    return this.service.findOne(id);
  }

  @Post(':id/pay')
  pay(@Param('id', ParseUUIDPipe) id: string, @Body() dto: PayTuitionDto) {
    return this.service.pay(id, dto);
  }

  @Post(':id/cancel')
  cancel(@Param('id', ParseUUIDPipe) id: string) {
    return this.service.cancel(id);
  }
}
