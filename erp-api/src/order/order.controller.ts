import {
  Body,
  Controller,
  Get,
  Param,
  ParseUUIDPipe,
  Post,
  Query,
} from '@nestjs/common';
import { CheckPolicies } from '../access/check-policies.decorator';
import { RequiresModule } from '../common/decorators/requires-module.decorator';
import { CreateOrderDto, ListOrderDto } from './dto/order.dto';
import { OrderService } from './order.service';

@RequiresModule('order')
@Controller('orders')
export class OrderController {
  constructor(private readonly service: OrderService) {}

  @Post()
  @CheckPolicies({ action: 'create', subject: 'Order' })
  create(@Body() dto: CreateOrderDto) {
    return this.service.create(dto);
  }

  @Get()
  @CheckPolicies({ action: 'read', subject: 'Order' })
  list(@Query() dto: ListOrderDto) {
    return this.service.list(dto);
  }

  @Get(':id')
  @CheckPolicies({ action: 'read', subject: 'Order' })
  findOne(@Param('id', ParseUUIDPipe) id: string) {
    return this.service.findOne(id);
  }

  @Post(':id/fulfill')
  @CheckPolicies({ action: 'update', subject: 'Order' })
  fulfill(@Param('id', ParseUUIDPipe) id: string) {
    return this.service.fulfill(id);
  }

  @Post(':id/cancel')
  @CheckPolicies({ action: 'update', subject: 'Order' })
  cancel(
    @Param('id', ParseUUIDPipe) id: string,
    @Body() body: { reason?: string } = {},
  ) {
    return this.service.cancel(id, body.reason);
  }
}
