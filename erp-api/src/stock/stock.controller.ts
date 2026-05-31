import { Body, Controller, Get, Post, Query } from '@nestjs/common';
import { CheckPolicies } from '../access/check-policies.decorator';
import { CreateStockMovementDto, ListStockDto } from './dto/stock.dto';
import { StockService } from './stock.service';

@Controller()
export class StockController {
  constructor(private readonly service: StockService) {}

  @Get('stock')
  @CheckPolicies({ action: 'read', subject: 'Stock' })
  listStock(@Query() dto: ListStockDto) {
    return this.service.listStock(dto);
  }

  @Get('stock-movements')
  @CheckPolicies({ action: 'read', subject: 'Stock' })
  listMovements(
    @Query('productId') productId?: string,
    @Query('warehouseId') warehouseId?: string,
  ) {
    return this.service.listMovements(productId, warehouseId);
  }

  @Post('stock-movements')
  @CheckPolicies({ action: 'update', subject: 'Stock' })
  createMovement(@Body() dto: CreateStockMovementDto) {
    return this.service.createMovement(dto);
  }
}
