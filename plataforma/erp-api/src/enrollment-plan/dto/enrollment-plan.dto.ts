import { PlanBillingCycle } from '@prisma/client';
import { PartialType } from '@nestjs/mapped-types';
import { Type } from 'class-transformer';
import {
  IsBoolean,
  IsEnum,
  IsInt,
  IsNotEmpty,
  IsNumber,
  IsOptional,
  IsString,
  Max,
  Min,
} from 'class-validator';

export class CreateEnrollmentPlanDto {
  @IsString()
  @IsNotEmpty()
  name!: string;

  @IsOptional()
  @IsString()
  description?: string;

  @Type(() => Number)
  @IsNumber({ maxDecimalPlaces: 4 })
  @Min(0)
  price!: number;

  @IsOptional()
  @IsEnum(PlanBillingCycle)
  billingCycle?: PlanBillingCycle;

  @IsOptional()
  @Type(() => Number)
  @IsInt()
  @Min(1)
  @Max(14)
  weeklySessions?: number;

  @IsOptional()
  @Type(() => Number)
  @IsNumber({ maxDecimalPlaces: 4 })
  @Min(0)
  enrollmentFee?: number;

  @IsOptional()
  @IsBoolean()
  active?: boolean;
}

export class UpdateEnrollmentPlanDto extends PartialType(
  CreateEnrollmentPlanDto,
) {}

export class ListEnrollmentPlanDto {
  @IsOptional()
  @IsString()
  q?: string;

  @IsOptional()
  @Type(() => Boolean)
  @IsBoolean()
  active?: boolean;
}
