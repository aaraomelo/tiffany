import { Body, Controller, Post } from '@nestjs/common';
import { Public } from '../common/decorators/public.decorator';
import { BootstrapTenantDto } from './dto/bootstrap-tenant.dto';
import { TenantService } from './tenant.service';

@Controller('tenants')
export class TenantController {
  constructor(private readonly tenant: TenantService) {}

  @Public()
  @Post('bootstrap')
  bootstrap(@Body() dto: BootstrapTenantDto) {
    return this.tenant.bootstrap(dto);
  }
}
