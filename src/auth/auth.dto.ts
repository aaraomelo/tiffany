import { ApiProperty, ApiPropertyOptional } from '@nestjs/swagger';
import { IsEmail, IsOptional, IsString, MinLength } from 'class-validator';

export class LoginDto {
  @ApiProperty({ description: 'Email do usuário', example: 'admin@patria.dev' })
  @IsEmail()
  email: string;

  @ApiProperty({ description: 'Senha do usuário', example: 'senha123' })
  @IsString()
  @MinLength(6)
  password: string;
}

export class RegisterDto {
  @ApiProperty({ description: 'Email do usuário', example: 'novo@patria.dev' })
  @IsEmail()
  email: string;

  @ApiProperty({ description: 'Senha (mínimo 8 caracteres)', example: 'senha123' })
  @IsString()
  @MinLength(8)
  password: string;

  @ApiPropertyOptional({ description: 'Nome do usuário', example: 'João Silva' })
  @IsOptional()
  @IsString()
  name?: string;

  @ApiPropertyOptional({ description: 'Sobrenome do usuário', example: 'Silva' })
  @IsOptional()
  @IsString()
  lastName?: string;
}

export class LoginResponseDto {
  @ApiProperty({ description: 'JWT de acesso' })
  token: string;

  @ApiProperty({ description: 'Dados do usuário autenticado' })
  user: {
    id: string;
    email: string;
    name: string;
  };

  @ApiPropertyOptional({ description: 'Tenant do usuário (se houver)' })
  tenant?: {
    id: string;
    alias: string;
    companyName: string;
    apiKey: string;
  };
}

export class SignupDto {
  @ApiProperty({ description: 'Nome completo', example: 'Maria Souza' })
  @IsString()
  @MinLength(2)
  name: string;

  @ApiProperty({ description: 'Email', example: 'maria@empresa.com' })
  @IsEmail()
  email: string;

  @ApiProperty({ description: 'Senha (mínimo 8)', example: 'senha1234' })
  @IsString()
  @MinLength(8)
  password: string;

  @ApiProperty({ description: 'Nome da empresa', example: 'Acme S.A.' })
  @IsString()
  @MinLength(2)
  companyName: string;
}

export class TenantLoginDto {
  @ApiProperty({ description: 'Email do usuário do tenant', example: 'admin@empresa.com' })
  @IsEmail()
  email: string;

  @ApiProperty({ description: 'Senha', example: 'senha123' })
  @IsString()
  @MinLength(6)
  password: string;
}

export class TenantLoginResponseDto {
  @ApiProperty({ description: 'JWT de acesso com tenantId e role' })
  token: string;

  @ApiProperty({ description: 'Dados do usuário autenticado' })
  user: {
    id: string;
    email: string;
    name: string;
    tenantId: string;
    role: string;
  };
}
