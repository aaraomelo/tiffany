import { Module } from '@nestjs/common';
import { PaymentModule } from '../payment/payment.module';
import { CheckoutController } from './checkout.controller';
import { CheckoutService } from './checkout.service';
import { PagBankAdapter } from './pagbank.adapter';

@Module({
  imports: [PaymentModule],
  controllers: [CheckoutController],
  providers: [CheckoutService, PagBankAdapter],
  exports: [CheckoutService],
})
export class CheckoutModule {}
