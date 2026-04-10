import { ApiProperty } from '@nestjs/swagger';
import { IsEmail, IsString, MinLength } from 'class-validator';

export class LoginDto {
  @ApiProperty({ description: 'Email do usuário', example: 'admin@patria.dev' })
  @IsEmail()
  email: string;

  @ApiProperty({ description: 'Senha do usuário', example: 'senha123' })
  @IsString()
  @MinLength(6)
  password: string;
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
