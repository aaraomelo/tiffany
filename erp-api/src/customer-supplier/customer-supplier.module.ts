import { Module } from '@nestjs/common';
import { CustomerSupplierController } from './customer-supplier.controller';
import { CustomerSupplierService } from './customer-supplier.service';

@Module({
  controllers: [CustomerSupplierController],
  providers: [CustomerSupplierService],
  exports: [CustomerSupplierService],
})
export class CustomerSupplierModule {}
