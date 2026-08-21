import { IsEnum, IsNotEmpty, IsOptional, IsString } from 'class-validator';

export type ApplyPackMode = 'replace' | 'merge';

export class ApplyPackDto {
  @IsString()
  @IsNotEmpty()
  packSlug!: string;

  /**
   * 'replace' (padrão): ativa os módulos do segmento e desativa os de fora.
   * 'merge': aditivo — ativa os módulos do segmento sem desativar nada que já
   * esteja ligado (segmentos cumulativos, ex.: escola + comércio).
   */
  @IsOptional()
  @IsEnum(['replace', 'merge'])
  mode?: ApplyPackMode;
}
