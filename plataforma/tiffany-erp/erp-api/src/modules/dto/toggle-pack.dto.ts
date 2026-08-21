import { IsBoolean, IsNotEmpty, IsString } from 'class-validator';

export class TogglePackDto {
  @IsString()
  @IsNotEmpty()
  packSlug!: string;

  /** true → ativa (aditivo/merge); false → desativa os módulos do segmento. */
  @IsBoolean()
  enabled!: boolean;
}
