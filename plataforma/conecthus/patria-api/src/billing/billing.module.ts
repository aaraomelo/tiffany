import { Module } from '@nestjs/common';
import { BillingController } from '../billing.controller';
import { PagbankService } from './pagbank.service';
import { BillingEventsHub } from './billing-events.hub';
import { PrismaService } from '../prisma.service';
import { EmailService } from '../email.service';

@Module({
  controllers: [BillingController],
  providers: [PagbankService, BillingEventsHub, PrismaService, EmailService],
  exports: [BillingEventsHub],
})
export class BillingModule {}
