import { Body, Controller, Get, Patch, Post } from '@nestjs/common';
import { CheckPolicies } from '../access/check-policies.decorator';
import { Public } from '../common/decorators/public.decorator';
import { BootstrapTenantDto } from './dto/bootstrap-tenant.dto';
import { UpdateCompanyDto } from './dto/update-company.dto';
import { TenantService } from './tenant.service';

@Controller('tenants')
export class TenantController {
  constructor(private readonly tenant: TenantService) {}

  @Public()
  @Post('bootstrap')
  bootstrap(@Body() dto: BootstrapTenantDto) {
    return this.tenant.bootstrap(dto);
  }

  /// Dados de cabeçalho da empresa (lido por qualquer usuário autenticado;
  /// usado no PDF da Ordem de Serviço e no formulário de configurações).
  @Get('company')
  getCompany() {
    return this.tenant.getCompany();
  }

  @Patch('company')
  @CheckPolicies({ action: 'update', subject: 'Theme' })
  updateCompany(@Body() dto: UpdateCompanyDto) {
    return this.tenant.updateCompany(dto);
  }
}
