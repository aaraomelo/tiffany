import { EnrollmentStatus } from '@prisma/client';
import { PartialType } from '@nestjs/mapped-types';
import { Type } from 'class-transformer';
import {
  IsDateString,
  IsEnum,
  IsInt,
  IsNumber,
  IsOptional,
  IsString,
  IsUUID,
  Max,
  Min,
} from 'class-validator';

export class CreateEnrollmentDto {
  @IsUUID()
  studentId!: string;

  @IsUUID()
  planId!: string;

  @IsOptional()
  @IsDateString()
  startDate?: string;

  @IsOptional()
  @IsDateString()
  endDate?: string;

  @IsOptional()
  @Type(() => Number)
  @IsInt()
  @Min(1)
  @Max(28)
  dueDay?: number;

  // se omitido, snapshot do preço do plano
  @IsOptional()
  @Type(() => Number)
  @IsNumber({ maxDecimalPlaces: 4 })
  @Min(0)
  monthlyPrice?: number;

  @IsOptional()
  @IsString()
  notes?: string;
}

export class UpdateEnrollmentDto extends PartialType(CreateEnrollmentDto) {
  @IsOptional()
  @IsEnum(EnrollmentStatus)
  status?: EnrollmentStatus;
}

export class ListEnrollmentDto {
  @IsOptional()
  @IsUUID()
  studentId?: string;

  @IsOptional()
  @IsEnum(EnrollmentStatus)
  status?: EnrollmentStatus;
}

export class GenerateTuitionDto {
  @Type(() => Number)
  @IsInt()
  @Min(1)
  @Max(12)
  referenceMonth!: number;

  @Type(() => Number)
  @IsInt()
  @Min(2000)
  @Max(2100)
  referenceYear!: number;
}
