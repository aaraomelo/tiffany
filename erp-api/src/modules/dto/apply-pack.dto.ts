import { IsNotEmpty, IsString } from 'class-validator';

export class ApplyPackDto {
  @IsString()
  @IsNotEmpty()
  packSlug!: string;
}
