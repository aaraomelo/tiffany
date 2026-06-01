import {
  IsEmail,
  IsNotEmpty,
  IsString,
  Matches,
  MinLength,
} from 'class-validator';

export class SignupDto {
  /// Subdomínio/alias — só letras minúsculas, números e hífen.
  @Matches(/^[a-z0-9](?:[a-z0-9-]{0,30}[a-z0-9])?$/, {
    message: 'alias deve ter 1-32 chars, [a-z0-9-], começar/terminar com alfanumérico',
  })
  alias!: string;

  /// Nome do negócio (vira o nome do tenant e título da landing).
  @IsString()
  @IsNotEmpty()
  name!: string;

  /// Pack/segmento escolhido (slug do ModulePack).
  @IsString()
  @IsNotEmpty()
  packSlug!: string;

  @IsEmail()
  adminEmail!: string;

  @IsString()
  @IsNotEmpty()
  adminName!: string;

  @IsString()
  @MinLength(8)
  adminPassword!: string;
}
