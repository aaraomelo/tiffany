import { Type } from 'class-transformer';
import {
  IsArray,
  IsOptional,
  IsString,
  ValidateNested,
} from 'class-validator';

export class LandingServiceDto {
  @IsString()
  title!: string;

  @IsOptional()
  @IsString()
  description?: string;
}

export class LandingContactDto {
  @IsOptional() @IsString() phone?: string;
  @IsOptional() @IsString() whatsapp?: string;
  @IsOptional() @IsString() email?: string;
  @IsOptional() @IsString() address?: string;
}

export class LandingSocialDto {
  @IsOptional() @IsString() instagram?: string;
  @IsOptional() @IsString() facebook?: string;
  @IsOptional() @IsString() website?: string;
}

export class UpdateLandingDto {
  @IsOptional() @IsString() headline?: string;
  @IsOptional() @IsString() subheadline?: string;
  @IsOptional() @IsString() about?: string;
  @IsOptional() @IsString() logoUrl?: string;
  @IsOptional() @IsString() heroImageUrl?: string;
  @IsOptional() @IsString() ctaText?: string;
  @IsOptional() @IsString() ctaUrl?: string;

  @IsOptional()
  @IsArray()
  @ValidateNested({ each: true })
  @Type(() => LandingServiceDto)
  services?: LandingServiceDto[];

  @IsOptional()
  @ValidateNested()
  @Type(() => LandingContactDto)
  contact?: LandingContactDto;

  @IsOptional()
  @ValidateNested()
  @Type(() => LandingSocialDto)
  social?: LandingSocialDto;
}
