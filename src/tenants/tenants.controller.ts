import { Controller, Get, Post, Patch, Param, Body } from '@nestjs/common';
import { ApiTags, ApiOperation, ApiResponse, ApiSecurity, ApiParam } from '@nestjs/swagger';
import { JwtService } from '@nestjs/jwt';
import { TenantsService } from './tenants.service';
import { CreateTenantDto, UpdateTenantDto, RegisterTenantDto, RegisterTenantResponseDto } from './tenants.dto';

@ApiTags('Tenants')
@Controller('api/tenants')
export class TenantsController {
  constructor(
    private readonly tenantsService: TenantsService,
    private readonly jwtService: JwtService,
  ) {}

  @Post('register')
  @ApiOperation({ summary: 'Registrar novo tenant e obter JWT' })
  @ApiResponse({ status: 201, type: RegisterTenantResponseDto })
  @ApiResponse({ status: 400, description: 'Dados inválidos' })
  @ApiResponse({ status: 409, description: 'Alias já está em uso' })
  async register(@Body() dto: RegisterTenantDto): Promise<RegisterTenantResponseDto> {
    const { tenant, user } = await this.tenantsService.register(dto);
    const token = this.jwtService.sign({
      sub: user.id,
      email: user.email,
      name: user.name,
      tenantId: user.tenantId,
      role: user.role,
      type: 'tenant',
    });
    return { token, user, tenant };
  }

  @Get()
  @ApiSecurity('api-key')
  @ApiOperation({ summary: 'Listar todos os tenants' })
  @ApiResponse({ status: 200, description: 'Lista de tenants retornada com sucesso' })
  findAll() {
    return this.tenantsService.findAll();
  }

  @Post()
  @ApiSecurity('api-key')
  @ApiOperation({ summary: 'Criar tenant com usuário admin' })
  @ApiResponse({ status: 201, description: 'Tenant criado com sucesso' })
  @ApiResponse({ status: 400, description: 'Dados inválidos' })
  @ApiResponse({ status: 409, description: 'Alias já está em uso' })
  create(@Body() dto: CreateTenantDto) {
    return this.tenantsService.create(dto);
  }

  @Get('check-alias/:alias')
  @ApiOperation({ summary: 'Verificar disponibilidade de alias' })
  @ApiParam({ name: 'alias', description: 'Alias a verificar' })
  @ApiResponse({ status: 200, description: 'Resultado da verificação de disponibilidade' })
  checkAlias(@Param('alias') alias: string) {
    return this.tenantsService.checkAlias(alias);
  }

  @Get(':id')
  @ApiSecurity('api-key')
  @ApiOperation({ summary: 'Buscar tenant por ID (inclui usuários)' })
  @ApiResponse({ status: 200, description: 'Tenant encontrado' })
  @ApiResponse({ status: 404, description: 'Tenant não encontrado' })
  findOne(@Param('id') id: string) {
    return this.tenantsService.findOne(id);
  }

  @Patch(':id')
  @ApiSecurity('api-key')
  @ApiOperation({ summary: 'Atualizar campos do tenant' })
  @ApiResponse({ status: 200, description: 'Tenant atualizado com sucesso' })
  @ApiResponse({ status: 400, description: 'Dados inválidos' })
  @ApiResponse({ status: 404, description: 'Tenant não encontrado' })
  update(@Param('id') id: string, @Body() dto: UpdateTenantDto) {
    return this.tenantsService.update(id, dto);
  }
}
