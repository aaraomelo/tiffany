import {
  IsEmail,
  IsNotEmpty,
  IsOptional,
  IsString,
  Matches,
  MinLength,
} from 'class-validator';

export class BootstrapTenantDto {
  /// Subdomínio/alias do tenant — só letras, números e hífen.
  @Matches(/^[a-z0-9](?:[a-z0-9-]{0,30}[a-z0-9])?$/, {
    message: 'alias deve ter 1-32 chars, [a-z0-9-], começar/terminar com alfanumérico',
  })
  alias!: string;

  @IsString()
  @IsNotEmpty()
  name!: string;

  @IsOptional()
  @IsString()
  companyName?: string;

  @IsOptional()
  @IsString()
  document?: string;

  @IsEmail()
  adminEmail!: string;

  @IsString()
  @IsNotEmpty()
  adminName!: string;

  @IsString()
  @MinLength(8)
  adminPassword!: string;
}
