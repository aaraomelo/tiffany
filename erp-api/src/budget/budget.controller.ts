import {
  Body,
  Controller,
  Get,
  Param,
  ParseUUIDPipe,
  Post,
  Query,
} from '@nestjs/common';
import { BudgetStatus } from '@prisma/client';
import { RequiresModule } from '../common/decorators/requires-module.decorator';
import { BudgetService } from './budget.service';
import { ConvertBudgetDto, CreateBudgetDto } from './dto/budget.dto';

@RequiresModule('budget')
@Controller('budgets')
export class BudgetController {
  constructor(private readonly service: BudgetService) {}

  @Post()
  create(@Body() dto: CreateBudgetDto) {
    return this.service.create(dto);
  }

  @Get()
  list(@Query('status') status?: BudgetStatus) {
    return this.service.list(status);
  }

  @Get(':id')
  findOne(@Param('id', ParseUUIDPipe) id: string) {
    return this.service.findOne(id);
  }

  @Post(':id/status')
  setStatus(
    @Param('id', ParseUUIDPipe) id: string,
    @Body() body: { status: BudgetStatus },
  ) {
    return this.service.setStatus(id, body.status);
  }

  @Post(':id/convert')
  convert(
    @Param('id', ParseUUIDPipe) id: string,
    @Body() dto: ConvertBudgetDto,
  ) {
    return this.service.convert(id, dto);
  }
}
