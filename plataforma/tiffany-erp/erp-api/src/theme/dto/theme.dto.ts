import {
  IsHexColor,
  IsInt,
  IsOptional,
  IsString,
  Max,
  Min,
} from 'class-validator';

/// Configuração visual do tenant. Todos os campos opcionais — fallback para defaults.
/// O front aplica essa config como CSS variables.
export class ThemeConfigDto {
  @IsOptional()
  @IsString()
  presetId?: string; // referencia uma paleta pré-definida no front

  // ---- cores ----
  @IsOptional() @IsHexColor() primary?: string;
  @IsOptional() @IsHexColor() primaryHover?: string;
  @IsOptional() @IsHexColor() primaryLight?: string;
  @IsOptional() @IsHexColor() secondary?: string;
  @IsOptional() @IsHexColor() accent?: string;
  @IsOptional() @IsHexColor() bg?: string;
  @IsOptional() @IsHexColor() surface?: string;
  @IsOptional() @IsHexColor() surfaceAlt?: string;
  @IsOptional() @IsHexColor() border?: string;
  @IsOptional() @IsHexColor() text?: string;
  @IsOptional() @IsHexColor() textMuted?: string;
  @IsOptional() @IsHexColor() success?: string;
  @IsOptional() @IsHexColor() warn?: string;
  @IsOptional() @IsHexColor() danger?: string;
  @IsOptional() @IsHexColor() textOnPrimary?: string;

  // ---- geometria ----
  /// pixels para border-radius pequeno (botões, inputs)
  @IsOptional() @IsInt() @Min(0) @Max(40) radius?: number;
  /// pixels para border-radius grande (cards, modais)
  @IsOptional() @IsInt() @Min(0) @Max(40) radiusLg?: number;
  /// pixels para espessura padrão de borda
  @IsOptional() @IsInt() @Min(0) @Max(6) borderWidth?: number;
  /// unidade base de spacing em rem * 100 (ex: 50 = 0.5rem)
  @IsOptional() @IsInt() @Min(20) @Max(200) spacingBase?: number;
}
