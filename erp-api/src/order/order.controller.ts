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
import { CreateOrderDto, ListOrderDto } from './dto/order.dto';
import { OrderService } from './order.service';

@RequiresModule('order')
@Controller('orders')
export class OrderController {
  constructor(private readonly service: OrderService) {}

  @Post()
  create(@Body() dto: CreateOrderDto) {
    return this.service.create(dto);
  }

  @Get()
  list(@Query() dto: ListOrderDto) {
    return this.service.list(dto);
  }

  @Get(':id')
  findOne(@Param('id', ParseUUIDPipe) id: string) {
    return this.service.findOne(id);
  }

  @Post(':id/fulfill')
  fulfill(@Param('id', ParseUUIDPipe) id: string) {
    return this.service.fulfill(id);
  }

  @Post(':id/cancel')
  cancel(
    @Param('id', ParseUUIDPipe) id: string,
    @Body() body: { reason?: string } = {},
  ) {
    return this.service.cancel(id, body.reason);
  }
}
