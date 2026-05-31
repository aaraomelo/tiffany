import {
  Body,
  Controller,
  Delete,
  Get,
  Param,
  ParseUUIDPipe,
  Patch,
  Post,
  Query,
} from '@nestjs/common';
import { CheckPolicies } from '../access/check-policies.decorator';
import { CustomerSupplierService } from './customer-supplier.service';
import { CreateCustomerSupplierDto } from './dto/create-customer-supplier.dto';
import { ListCustomerSupplierDto } from './dto/list-customer-supplier.dto';
import { UpdateCustomerSupplierDto } from './dto/update-customer-supplier.dto';

@Controller('customer-suppliers')
export class CustomerSupplierController {
  constructor(private readonly service: CustomerSupplierService) {}

  @Post()
  @CheckPolicies({ action: 'create', subject: 'Customer' })
  create(@Body() dto: CreateCustomerSupplierDto) {
    return this.service.create(dto);
  }

  @Get()
  @CheckPolicies({ action: 'read', subject: 'Customer' })
  list(@Query() dto: ListCustomerSupplierDto) {
    return this.service.list(dto);
  }

  @Get(':id')
  @CheckPolicies({ action: 'read', subject: 'Customer' })
  findOne(@Param('id', ParseUUIDPipe) id: string) {
    return this.service.findOne(id);
  }

  @Patch(':id')
  @CheckPolicies({ action: 'update', subject: 'Customer' })
  update(
    @Param('id', ParseUUIDPipe) id: string,
    @Body() dto: UpdateCustomerSupplierDto,
  ) {
    return this.service.update(id, dto);
  }

  @Delete(':id')
  @CheckPolicies({ action: 'delete', subject: 'Customer' })
  remove(@Param('id', ParseUUIDPipe) id: string) {
    return this.service.remove(id);
  }
}
