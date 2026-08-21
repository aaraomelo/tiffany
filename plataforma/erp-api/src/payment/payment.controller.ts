import {
  Body,
  Controller,
  Get,
  Param,
  ParseUUIDPipe,
  Post,
} from '@nestjs/common';
import { RequiresModule } from '../common/decorators/requires-module.decorator';
import { CreatePaymentDto } from './dto/payment.dto';
import { PaymentService } from './payment.service';

@RequiresModule('payment')
@Controller('payments')
export class PaymentController {
  constructor(private readonly service: PaymentService) {}

  @Post()
  create(@Body() dto: CreatePaymentDto) {
    return this.service.create(dto);
  }

  @Get('by-order/:orderId')
  byOrder(@Param('orderId', ParseUUIDPipe) orderId: string) {
    return this.service.listByOrder(orderId);
  }
}
