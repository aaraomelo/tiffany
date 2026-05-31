import { Module } from '@nestjs/common';
import { BudgetModule } from '../budget/budget.module';
import { CashModule } from '../cash/cash.module';
import { CustomerSupplierModule } from '../customer-supplier/customer-supplier.module';
import { OrderModule } from '../order/order.module';
import { PaymentModule } from '../payment/payment.module';
import { ProductModule } from '../product/product.module';
import { ServiceOrderModule } from '../service-order/service-order.module';
import { AssistantConfigService } from './assistant-config.service';
import { AssistantController } from './assistant.controller';
import { AssistantLlmService } from './assistant-llm.service';
import { AssistantMemoryService } from './assistant-memory.service';
import { AssistantProfileService } from './assistant-profile.service';
import { AssistantToolRunnerService } from './assistant-tool-runner.service';

@Module({
  imports: [
    CustomerSupplierModule,
    ProductModule,
    OrderModule,
    PaymentModule,
    ServiceOrderModule,
    BudgetModule,
    CashModule,
  ],
  controllers: [AssistantController],
  providers: [
    AssistantConfigService,
    AssistantProfileService,
    AssistantMemoryService,
    AssistantToolRunnerService,
    AssistantLlmService,
  ],
})
export class AssistantModule {}
