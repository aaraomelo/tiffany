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
import { RequiresModule } from '../common/decorators/requires-module.decorator';
import {
  CreateSupplierAccountDto,
  ImportProductsDto,
  LinkProductDto,
  ListSupplierProductsDto,
  PlaceSupplierOrderDto,
  UpdateSupplierAccountDto,
} from './dto/supplier.dto';
import { SupplierIntegrationService } from './supplier-integration.service';

@RequiresModule('supplier-integration')
@Controller('suppliers')
export class SupplierIntegrationController {
  constructor(private readonly service: SupplierIntegrationService) {}

  @Post('accounts')
  @CheckPolicies({ action: 'create', subject: 'Supplier' })
  createAccount(@Body() dto: CreateSupplierAccountDto) {
    return this.service.createAccount(dto);
  }

  @Get('accounts')
  @CheckPolicies({ action: 'read', subject: 'Supplier' })
  listAccounts() {
    return this.service.listAccounts();
  }

  @Get('accounts/:id')
  @CheckPolicies({ action: 'read', subject: 'Supplier' })
  getAccount(@Param('id', ParseUUIDPipe) id: string) {
    return this.service.getAccount(id);
  }

  @Patch('accounts/:id')
  @CheckPolicies({ action: 'update', subject: 'Supplier' })
  updateAccount(
    @Param('id', ParseUUIDPipe) id: string,
    @Body() dto: UpdateSupplierAccountDto,
  ) {
    return this.service.updateAccount(id, dto);
  }

  @Delete('accounts/:id')
  @CheckPolicies({ action: 'delete', subject: 'Supplier' })
  removeAccount(@Param('id', ParseUUIDPipe) id: string) {
    return this.service.removeAccount(id);
  }

  @Post('accounts/:id/test-login')
  @CheckPolicies({ action: 'update', subject: 'Supplier' })
  testLogin(@Param('id', ParseUUIDPipe) id: string) {
    return this.service.testLogin(id);
  }

  @Post('accounts/:id/sync')
  @CheckPolicies({ action: 'update', subject: 'Supplier' })
  sync(@Param('id', ParseUUIDPipe) id: string) {
    return this.service.syncCatalog(id);
  }

  @Get('accounts/:id/products')
  @CheckPolicies({ action: 'read', subject: 'Supplier' })
  listProducts(
    @Param('id', ParseUUIDPipe) id: string,
    @Query() dto: ListSupplierProductsDto,
  ) {
    return this.service.listProducts(id, dto);
  }

  @Post('accounts/:id/import')
  @CheckPolicies({ action: 'create', subject: 'Product' })
  import(
    @Param('id', ParseUUIDPipe) id: string,
    @Body() dto: ImportProductsDto,
  ) {
    return this.service.importAsProducts(id, dto);
  }

  @Get('accounts/:id/orders')
  @CheckPolicies({ action: 'read', subject: 'Supplier' })
  listOrders(@Param('id', ParseUUIDPipe) id: string) {
    return this.service.listOrders(id);
  }

  @Post('accounts/:id/orders')
  @CheckPolicies({ action: 'create', subject: 'Supplier' })
  placeOrder(
    @Param('id', ParseUUIDPipe) id: string,
    @Body() dto: PlaceSupplierOrderDto,
  ) {
    return this.service.placeReplenishmentOrder(id, dto);
  }

  @Post('products/:id/link')
  @CheckPolicies({ action: 'update', subject: 'Supplier' })
  link(
    @Param('id', ParseUUIDPipe) id: string,
    @Body() dto: LinkProductDto,
  ) {
    return this.service.linkToLocalProduct(id, dto.productId);
  }
}
