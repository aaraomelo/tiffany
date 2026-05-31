import {
  Body,
  Controller,
  Get,
  Param,
  ParseUUIDPipe,
  Post,
} from '@nestjs/common';
import { CashOperationType } from '@prisma/client';
import { RequiresModule } from '../common/decorators/requires-module.decorator';
import { CashService } from './cash.service';
import {
  CashOperationDto,
  CloseSessionDto,
  CreateCashDto,
  OpenSessionDto,
  OperatorChangeDto,
  PhysicalCloseDto,
} from './dto/cash.dto';

@RequiresModule('cash')
@Controller('cash')
export class CashController {
  constructor(private readonly service: CashService) {}

  @Post()
  create(@Body() dto: CreateCashDto) {
    return this.service.create(dto);
  }

  @Get()
  list() {
    return this.service.list();
  }

  @Get(':cashId/session/current')
  current(@Param('cashId', ParseUUIDPipe) cashId: string) {
    return this.service.currentSession(cashId);
  }

  @Post(':cashId/session/open')
  open(
    @Param('cashId', ParseUUIDPipe) cashId: string,
    @Body() dto: OpenSessionDto,
  ) {
    return this.service.openSession(cashId, dto);
  }

  @Post(':cashId/session/:sessionId/close')
  close(
    @Param('cashId', ParseUUIDPipe) cashId: string,
    @Param('sessionId', ParseUUIDPipe) sessionId: string,
    @Body() dto: CloseSessionDto,
  ) {
    return this.service.closeSession(cashId, sessionId, dto);
  }

  @Get(':cashId/session/:sessionId')
  detail(
    @Param('cashId', ParseUUIDPipe) cashId: string,
    @Param('sessionId', ParseUUIDPipe) sessionId: string,
  ) {
    return this.service.sessionDetail(cashId, sessionId);
  }

  @Post(':cashId/withdrawal')
  withdrawal(
    @Param('cashId', ParseUUIDPipe) cashId: string,
    @Body() dto: CashOperationDto,
  ) {
    return this.service.addOperation(cashId, CashOperationType.WITHDRAWAL, dto);
  }

  @Post(':cashId/reinforcement')
  reinforcement(
    @Param('cashId', ParseUUIDPipe) cashId: string,
    @Body() dto: CashOperationDto,
  ) {
    return this.service.addOperation(cashId, CashOperationType.REINFORCEMENT, dto);
  }

  @Post(':cashId/operator-change')
  operatorChange(
    @Param('cashId', ParseUUIDPipe) cashId: string,
    @Body() dto: OperatorChangeDto,
  ) {
    return this.service.operatorChange(cashId, dto);
  }

  @Post(':cashId/physical-close')
  physicalClose(
    @Param('cashId', ParseUUIDPipe) cashId: string,
    @Body() dto: PhysicalCloseDto,
  ) {
    return this.service.physicalClose(cashId, dto);
  }
}
