import { Module } from '@nestjs/common';
import { BillingService } from './billing.service';
import { StripeService } from './stripe.service';
import { PrismaService } from '../prisma.service';

@Module({
  providers: [BillingService, StripeService, PrismaService],
  exports: [BillingService],
})
export class BillingModule {}
