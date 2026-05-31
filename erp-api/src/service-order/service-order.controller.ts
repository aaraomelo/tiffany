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
import {
  ChangeStatusDto,
  CreateLaborInputDto,
  CreatePartInputDto,
  CreateServiceOrderDto,
  ListServiceOrderDto,
  UpdateServiceOrderDto,
} from './dto/service-order.dto';
import { RequiresModule } from '../common/decorators/requires-module.decorator';
import { ServiceOrderService } from './service-order.service';

@RequiresModule('service-order')
@Controller('service-orders')
export class ServiceOrderController {
  constructor(private readonly service: ServiceOrderService) {}

  @Post()
  @CheckPolicies({ action: 'create', subject: 'ServiceOrder' })
  create(@Body() dto: CreateServiceOrderDto) {
    return this.service.create(dto);
  }

  @Get()
  @CheckPolicies({ action: 'read', subject: 'ServiceOrder' })
  list(@Query() dto: ListServiceOrderDto) {
    return this.service.list(dto);
  }

  @Get(':id')
  @CheckPolicies({ action: 'read', subject: 'ServiceOrder' })
  findOne(@Param('id', ParseUUIDPipe) id: string) {
    return this.service.findOne(id);
  }

  @Patch(':id')
  @CheckPolicies({ action: 'update', subject: 'ServiceOrder' })
  update(
    @Param('id', ParseUUIDPipe) id: string,
    @Body() dto: UpdateServiceOrderDto,
  ) {
    return this.service.update(id, dto);
  }

  @Post(':id/parts')
  @CheckPolicies({ action: 'update', subject: 'ServiceOrder' })
  addPart(
    @Param('id', ParseUUIDPipe) id: string,
    @Body() dto: CreatePartInputDto,
  ) {
    return this.service.addPart(id, dto);
  }

  @Delete(':id/parts/:partId')
  @CheckPolicies({ action: 'delete', subject: 'ServiceOrder' })
  removePart(
    @Param('id', ParseUUIDPipe) id: string,
    @Param('partId', ParseUUIDPipe) partId: string,
  ) {
    return this.service.removePart(id, partId);
  }

  @Post(':id/labors')
  @CheckPolicies({ action: 'update', subject: 'ServiceOrder' })
  addLabor(
    @Param('id', ParseUUIDPipe) id: string,
    @Body() dto: CreateLaborInputDto,
  ) {
    return this.service.addLabor(id, dto);
  }

  @Delete(':id/labors/:laborId')
  @CheckPolicies({ action: 'delete', subject: 'ServiceOrder' })
  removeLabor(
    @Param('id', ParseUUIDPipe) id: string,
    @Param('laborId', ParseUUIDPipe) laborId: string,
  ) {
    return this.service.removeLabor(id, laborId);
  }

  @Post(':id/status')
  @CheckPolicies({ action: 'update', subject: 'ServiceOrder' })
  changeStatus(
    @Param('id', ParseUUIDPipe) id: string,
    @Body() dto: ChangeStatusDto,
  ) {
    return this.service.changeStatus(id, dto);
  }
}
