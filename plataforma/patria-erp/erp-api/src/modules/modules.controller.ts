import { Controller, Get } from '@nestjs/common';
import { ModulesService } from './modules.service';

@Controller()
export class ModulesCatalogController {
  constructor(private readonly service: ModulesService) {}

  @Get('modules')
  listModules() {
    return this.service.listModules();
  }

  @Get('module-packs')
  listPacks() {
    return this.service.listPacks();
  }
}
