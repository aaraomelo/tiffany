import { Module } from '@nestjs/common';
import { NuvemshopAdapter } from './nuvemshop.adapter';
import { SupplierIntegrationController } from './supplier-integration.controller';
import { SupplierIntegrationScheduler } from './supplier-integration.scheduler';
import { SupplierIntegrationService } from './supplier-integration.service';

@Module({
  controllers: [SupplierIntegrationController],
  providers: [
    SupplierIntegrationService,
    NuvemshopAdapter,
    SupplierIntegrationScheduler,
  ],
  exports: [SupplierIntegrationService],
})
export class SupplierIntegrationModule {}
