import {
  ArrayNotEmpty,
  IsArray,
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

  /// Segmento(s) escolhido(s) — slugs de ModulePack. O 1º é o principal
  /// (define o tema da landing); os demais são somados (merge).
  @IsArray()
  @ArrayNotEmpty()
  @IsString({ each: true })
  packSlugs!: string[];

  @IsEmail()
  adminEmail!: string;

  @IsString()
  @IsNotEmpty()
  adminName!: string;

  @IsString()
  @MinLength(8)
  adminPassword!: string;
}
