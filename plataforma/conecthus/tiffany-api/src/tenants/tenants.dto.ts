import {
  IsString,
  IsEmail,
  IsEnum,
  IsOptional,
  IsArray,
  IsDateString,
  IsNotEmpty,
  MinLength,
  Matches,
} from 'class-validator';
import { ApiProperty, ApiPropertyOptional } from '@nestjs/swagger';

export enum TenantPlan {
  starter = 'starter',
  pro = 'pro',
  enterprise = 'enterprise',
}

export enum TenantStatus {
  trial = 'trial',
  active = 'active',
  suspended = 'suspended',
}

export class CreateTenantDto {
  @ApiProperty({ description: 'Identificador único (slug) do tenant — subdomínio, imutável', example: 'minha-empresa' })
  @IsString()
  @Matches(/^[a-z0-9-]+$/, { message: 'alias deve conter apenas letras minúsculas, números e hífens' })
  alias: string;

  @ApiProperty({ description: 'Nome do responsável', example: 'João Silva' })
  @IsString()
  name: string;

  @ApiProperty({ description: 'Nome da empresa', example: 'Minha Empresa Ltda' })
  @IsString()
  companyName: string;

  @ApiProperty({ description: 'Email do usuário administrador', example: 'joao@empresa.com' })
  @IsEmail()
  adminEmail: string;

  @ApiProperty({ description: 'Nome do usuário administrador', example: 'João Silva' })
  @IsString()
  adminName: string;

  @ApiProperty({ description: 'Senha do usuário administrador (mínimo 8 caracteres)', example: 'senha1234' })
  @IsString()
  @MinLength(8)
  adminPassword: string;

  @ApiPropertyOptional({ enum: TenantPlan, description: 'Plano do tenant', default: TenantPlan.starter })
  @IsOptional()
  @IsEnum(TenantPlan)
  plan?: TenantPlan;

  @ApiPropertyOptional({ description: 'Organização no GitHub', example: 'minha-org' })
  @IsOptional()
  @IsString()
  githubOrg?: string;

  @ApiPropertyOptional({ description: 'Lista de repositórios GitHub', example: ['repo1', 'repo2'] })
  @IsOptional()
  @IsArray()
  @IsString({ each: true })
  githubRepos?: string[];
}

export class RegisterTenantDto {
  @ApiProperty({ description: 'Nome do responsável', example: 'João Silva' })
  @IsString()
  @IsNotEmpty()
  nome: string;

  @ApiProperty({ description: 'Nome da empresa', example: 'Minha Empresa Ltda' })
  @IsString()
  @IsNotEmpty()
  empresa: string;

  @ApiProperty({ description: 'Email do usuário admin', example: 'joao@empresa.com' })
  @IsEmail()
  email: string;

  @ApiProperty({ description: 'Senha do admin (mínimo 8 caracteres)', example: 'senha1234' })
  @IsString()
  @MinLength(8)
  senha: string;

  @ApiProperty({ description: 'Subdomínio único e imutável', example: 'minha-empresa' })
  @IsString()
  @Matches(/^[a-z0-9-]+$/, { message: 'alias deve conter apenas letras minúsculas, números e hífens' })
  alias: string;
}

export class RegisterTenantUserDto {
  @ApiProperty({ example: 'uuid' })
  id: string;

  @ApiProperty({ example: 'joao@empresa.com' })
  email: string;

  @ApiProperty({ example: 'João Silva' })
  name: string;

  @ApiProperty({ example: 'uuid-do-tenant' })
  tenantId: string;

  @ApiProperty({ example: 'admin' })
  role: string;
}

export class RegisterTenantTenantDto {
  @ApiProperty({ example: 'uuid' })
  id: string;

  @ApiProperty({ example: 'minha-empresa' })
  alias: string;

  @ApiProperty({ example: 'João Silva' })
  name: string;

  @ApiProperty({ example: 'Minha Empresa Ltda' })
  companyName: string;

  @ApiProperty({ example: 'starter' })
  plan: string;

  @ApiProperty({ example: 'trial' })
  status: string;

  @ApiProperty({ example: '2026-04-25T00:00:00.000Z' })
  trialEndsAt: Date;
}

export class RegisterTenantResponseDto {
  @ApiProperty({ description: 'JWT de autenticação' })
  token: string;

  @ApiProperty({ type: RegisterTenantUserDto })
  user: RegisterTenantUserDto;

  @ApiProperty({ type: RegisterTenantTenantDto })
  tenant: RegisterTenantTenantDto;
}

export class UpdateTenantDto {
  @ApiPropertyOptional({ description: 'Nome do responsável', example: 'Maria Silva' })
  @IsOptional()
  @IsString()
  name?: string;

  @ApiPropertyOptional({ description: 'Nome da empresa', example: 'Nova Empresa SA' })
  @IsOptional()
  @IsString()
  companyName?: string;

  @ApiPropertyOptional({ enum: TenantPlan, description: 'Plano do tenant' })
  @IsOptional()
  @IsEnum(TenantPlan)
  plan?: TenantPlan;

  @ApiPropertyOptional({ enum: TenantStatus, description: 'Status do tenant' })
  @IsOptional()
  @IsEnum(TenantStatus)
  status?: TenantStatus;

  @ApiPropertyOptional({ description: 'Organização no GitHub', example: 'nova-org' })
  @IsOptional()
  @IsString()
  githubOrg?: string;

  @ApiPropertyOptional({ description: 'Lista de repositórios GitHub', example: ['repo1', 'repo2'] })
  @IsOptional()
  @IsArray()
  @IsString({ each: true })
  githubRepos?: string[];

  @ApiPropertyOptional({ description: 'Data de fim do trial (ISO 8601)', example: '2026-05-10T00:00:00Z' })
  @IsOptional()
  @IsDateString()
  trialEndsAt?: string;
}
