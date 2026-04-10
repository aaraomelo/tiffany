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

  @ApiProperty({ description: 'Senha (mínimo 6 caracteres)', example: 'senha123' })
  @IsString()
  @MinLength(6)
  password: string;

  @ApiProperty({ description: 'Confirmação de senha', example: 'senha123' })
  @IsString()
  @MinLength(6)
  confirmPassword: string;

  @ApiPropertyOptional({ description: 'Nome do usuário', example: 'João Silva' })
  @IsOptional()
  @IsString()
  name?: string;
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
}
