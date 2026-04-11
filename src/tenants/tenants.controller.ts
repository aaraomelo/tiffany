import { Controller, Delete, Get, HttpCode, Post, Patch, Param, Body, UseGuards } from '@nestjs/common';
import { ApiTags, ApiOperation, ApiResponse, ApiSecurity, ApiParam } from '@nestjs/swagger';
import { TenantsService } from './tenants.service';
import { CreateTenantDto, UpdateTenantDto } from './tenants.dto';
import { AuthService, GithubRepo } from '../auth/auth.service';
import { JwtAuthGuard } from '../auth/jwt-auth.guard';
import { CurrentUser, AuthUser } from '../auth/current-user.decorator';

@ApiTags('Tenants')
@Controller('api/tenants')
export class TenantsController {
  constructor(
    private readonly tenantsService: TenantsService,
    private readonly authService: AuthService,
  ) {}

  @Get('me/github/repos')
  @UseGuards(JwtAuthGuard)
  @ApiSecurity('bearer')
  @ApiOperation({ summary: 'Listar repositórios GitHub do tenant autenticado' })
  @ApiResponse({ status: 200, description: 'Lista de repositórios GitHub' })
  @ApiResponse({ status: 401, description: 'GitHub não conectado ou token inválido' })
  listGithubRepos(@CurrentUser() user: AuthUser): Promise<GithubRepo[]> {
    return this.authService.listGithubRepos(user.tenantId);
  }

  @Delete('me/github')
  @UseGuards(JwtAuthGuard)
  @HttpCode(200)
  @ApiSecurity('bearer')
  @ApiOperation({ summary: 'Desconectar GitHub do tenant autenticado' })
  @ApiResponse({ status: 200, description: 'GitHub desconectado com sucesso' })
  @ApiResponse({ status: 401, description: 'Token inválido ou tenant não encontrado' })
  async disconnectGithub(@CurrentUser() user: AuthUser): Promise<{ message: string }> {
    await this.authService.disconnectGithub(user.tenantId);
    return { message: 'GitHub desconectado com sucesso' };
  }

  @Get()
  @ApiSecurity('api-key')
  @ApiOperation({ summary: 'Listar todos os tenants' })
  @ApiResponse({ status: 200, description: 'Lista de tenants retornada com sucesso' })
  findAll() {
    return this.tenantsService.findAll();
  }

  @Post()
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
  @ApiOperation({ summary: 'Buscar tenant por ID (inclui usuários)' })
  @ApiResponse({ status: 200, description: 'Tenant encontrado' })
  @ApiResponse({ status: 404, description: 'Tenant não encontrado' })
  findOne(@Param('id') id: string) {
    return this.tenantsService.findOne(id);
  }

  @Patch(':id')
  @ApiOperation({ summary: 'Atualizar campos do tenant' })
  @ApiResponse({ status: 200, description: 'Tenant atualizado com sucesso' })
  @ApiResponse({ status: 400, description: 'Dados inválidos' })
  @ApiResponse({ status: 404, description: 'Tenant não encontrado' })
  update(@Param('id') id: string, @Body() dto: UpdateTenantDto) {
    return this.tenantsService.update(id, dto);
  }
}
