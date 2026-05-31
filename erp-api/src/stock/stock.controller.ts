import { Body, Controller, Get, Post, Query } from '@nestjs/common';
import { CreateStockMovementDto, ListStockDto } from './dto/stock.dto';
import { StockService } from './stock.service';

@Controller()
export class StockController {
  constructor(private readonly service: StockService) {}

  @Get('stock')
  listStock(@Query() dto: ListStockDto) {
    return this.service.listStock(dto);
  }

  @Get('stock-movements')
  listMovements(
    @Query('productId') productId?: string,
    @Query('warehouseId') warehouseId?: string,
  ) {
    return this.service.listMovements(productId, warehouseId);
  }

  @Post('stock-movements')
  createMovement(@Body() dto: CreateStockMovementDto) {
    return this.service.createMovement(dto);
  }
}
