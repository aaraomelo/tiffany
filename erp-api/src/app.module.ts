import { Module } from '@nestjs/common';
import { ConfigModule } from '@nestjs/config';
import { APP_GUARD, APP_INTERCEPTOR } from '@nestjs/core';
import { ScheduleModule } from '@nestjs/schedule';
import { AccessModule } from './access/access.module';
import { PoliciesGuard } from './access/policies.guard';
import { AppController } from './app.controller';
import { AppService } from './app.service';
import { AssistantModule } from './assistant/assistant.module';
import { AuthModule } from './auth/auth.module';
import { JwtAuthGuard } from './auth/jwt-auth.guard';
import { BudgetModule } from './budget/budget.module';
import { CashModule } from './cash/cash.module';
import { CatalogModule } from './catalog/catalog.module';
import { CheckoutModule } from './checkout/checkout.module';
import { ModuleEnabledGuard } from './common/guards/module-enabled.guard';
import { TenantContextInterceptor } from './common/tenant-context/tenant-context.interceptor';
import { CustomerSupplierModule } from './customer-supplier/customer-supplier.module';
import { EmbeddingModule } from './embedding/embedding.module';
import { EnrollmentModule } from './enrollment/enrollment.module';
import { EnrollmentPlanModule } from './enrollment-plan/enrollment-plan.module';
import { ModulesModule } from './modules/modules.module';
import { OrderModule } from './order/order.module';
import { PaymentModule } from './payment/payment.module';
import { PrismaModule } from './prisma/prisma.module';
import { ProductModule } from './product/product.module';
import { ServiceOrderModule } from './service-order/service-order.module';
import { StockModule } from './stock/stock.module';
import { StudentModule } from './student/student.module';
import { TuitionModule } from './tuition/tuition.module';
import { TenantModule } from './tenant/tenant.module';
import { ThemeModule } from './theme/theme.module';
import { VehicleModule } from './vehicle/vehicle.module';
import { WalletModule } from './wallet/wallet.module';
import { WarehouseModule } from './warehouse/warehouse.module';
import { WarrantyTermModule } from './warranty-term/warranty-term.module';

@Module({
  imports: [
    ConfigModule.forRoot({ isGlobal: true }),
    ScheduleModule.forRoot(),
    PrismaModule,
    EmbeddingModule,
    AuthModule,
    TenantModule,
    ModulesModule,
    CustomerSupplierModule,
    CatalogModule,
    WarehouseModule,
    ProductModule,
    StockModule,
    CashModule,
    OrderModule,
    WalletModule,
    PaymentModule,
    CheckoutModule,
    VehicleModule,
    WarrantyTermModule,
    ServiceOrderModule,
    BudgetModule,
    StudentModule,
    EnrollmentPlanModule,
    EnrollmentModule,
    TuitionModule,
    ThemeModule,
    AssistantModule,
    AccessModule,
  ],
  controllers: [AppController],
  providers: [
    AppService,
    { provide: APP_GUARD, useClass: JwtAuthGuard },
    { provide: APP_INTERCEPTOR, useClass: TenantContextInterceptor },
    { provide: APP_GUARD, useClass: ModuleEnabledGuard },
    { provide: APP_GUARD, useClass: PoliciesGuard },
  ],
})
export class AppModule {}
