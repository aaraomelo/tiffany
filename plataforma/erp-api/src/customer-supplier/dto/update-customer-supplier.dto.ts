import { PartialType } from '@nestjs/mapped-types';
import { CreateCustomerSupplierDto } from './create-customer-supplier.dto';

export class UpdateCustomerSupplierDto extends PartialType(
  CreateCustomerSupplierDto,
) {}
