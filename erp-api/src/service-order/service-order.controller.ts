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
  create(@Body() dto: CreateServiceOrderDto) {
    return this.service.create(dto);
  }

  @Get()
  list(@Query() dto: ListServiceOrderDto) {
    return this.service.list(dto);
  }

  @Get(':id')
  findOne(@Param('id', ParseUUIDPipe) id: string) {
    return this.service.findOne(id);
  }

  @Patch(':id')
  update(
    @Param('id', ParseUUIDPipe) id: string,
    @Body() dto: UpdateServiceOrderDto,
  ) {
    return this.service.update(id, dto);
  }

  @Post(':id/parts')
  addPart(
    @Param('id', ParseUUIDPipe) id: string,
    @Body() dto: CreatePartInputDto,
  ) {
    return this.service.addPart(id, dto);
  }

  @Delete(':id/parts/:partId')
  removePart(
    @Param('id', ParseUUIDPipe) id: string,
    @Param('partId', ParseUUIDPipe) partId: string,
  ) {
    return this.service.removePart(id, partId);
  }

  @Post(':id/labors')
  addLabor(
    @Param('id', ParseUUIDPipe) id: string,
    @Body() dto: CreateLaborInputDto,
  ) {
    return this.service.addLabor(id, dto);
  }

  @Delete(':id/labors/:laborId')
  removeLabor(
    @Param('id', ParseUUIDPipe) id: string,
    @Param('laborId', ParseUUIDPipe) laborId: string,
  ) {
    return this.service.removeLabor(id, laborId);
  }

  @Post(':id/status')
  changeStatus(
    @Param('id', ParseUUIDPipe) id: string,
    @Body() dto: ChangeStatusDto,
  ) {
    return this.service.changeStatus(id, dto);
  }
}
