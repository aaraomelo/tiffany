import {
  Body,
  Controller,
  Get,
  Param,
  Patch,
  Post,
} from '@nestjs/common';
import { requireTenantId } from '../common/tenant-context/tenant-context';
import { ApplyPackDto } from './dto/apply-pack.dto';
import { ToggleModuleDto } from './dto/toggle-module.dto';
import { ModulesService } from './modules.service';

@Controller('tenant/modules')
export class TenantModulesController {
  constructor(private readonly service: ModulesService) {}

  @Get()
  list() {
    return this.service.listTenantModules(requireTenantId());
  }

  @Post('apply-pack')
  applyPack(@Body() dto: ApplyPackDto) {
    return this.service.applyPack(requireTenantId(), dto.packSlug);
  }

  @Patch(':slug')
  toggle(@Param('slug') slug: string, @Body() dto: ToggleModuleDto) {
    return this.service.toggleModule(requireTenantId(), slug, dto.enabled);
  }
}
