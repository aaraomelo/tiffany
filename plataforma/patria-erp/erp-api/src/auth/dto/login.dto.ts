import { IsEmail, IsNotEmpty, IsOptional, IsString } from 'class-validator';

export class LoginDto {
  @IsEmail()
  email!: string;

  @IsString()
  @IsNotEmpty()
  password!: string;

  /// Opcional: alias do tenant. Permite que o mesmo email exista em tenants diferentes.
  /// Se omitido e o email pertencer a só um tenant, o backend resolve sozinho.
  @IsOptional()
  @IsString()
  tenantAlias?: string;
}
