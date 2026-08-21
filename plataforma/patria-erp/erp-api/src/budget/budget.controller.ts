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
import { CheckPolicies } from '../access/check-policies.decorator';
import { RequiresModule } from '../common/decorators/requires-module.decorator';
import { BudgetService } from './budget.service';
import { ConvertBudgetDto, CreateBudgetDto } from './dto/budget.dto';

@RequiresModule('budget')
@Controller('budgets')
export class BudgetController {
  constructor(private readonly service: BudgetService) {}

  @Post()
  @CheckPolicies({ action: 'create', subject: 'Budget' })
  create(@Body() dto: CreateBudgetDto) {
    return this.service.create(dto);
  }

  @Get()
  @CheckPolicies({ action: 'read', subject: 'Budget' })
  list(@Query('status') status?: BudgetStatus) {
    return this.service.list(status);
  }

  @Get(':id')
  @CheckPolicies({ action: 'read', subject: 'Budget' })
  findOne(@Param('id', ParseUUIDPipe) id: string) {
    return this.service.findOne(id);
  }

  @Post(':id/status')
  @CheckPolicies({ action: 'update', subject: 'Budget' })
  setStatus(
    @Param('id', ParseUUIDPipe) id: string,
    @Body() body: { status: BudgetStatus },
  ) {
    return this.service.setStatus(id, body.status);
  }

  @Post(':id/convert')
  @CheckPolicies({ action: 'update', subject: 'Budget' })
  convert(
    @Param('id', ParseUUIDPipe) id: string,
    @Body() dto: ConvertBudgetDto,
  ) {
    return this.service.convert(id, dto);
  }
}
