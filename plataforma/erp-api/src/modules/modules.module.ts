import { Global, Module } from '@nestjs/common';
import { ModulesCatalogController } from './modules.controller';
import { ModulesService } from './modules.service';
import { TenantModulesController } from './tenant-modules.controller';

@Global()
@Module({
  controllers: [ModulesCatalogController, TenantModulesController],
  providers: [ModulesService],
  exports: [ModulesService],
})
export class ModulesModule {}
