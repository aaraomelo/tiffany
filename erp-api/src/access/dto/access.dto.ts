import { PartialType } from '@nestjs/mapped-types';
import { Type } from 'class-transformer';
import {
  IsArray,
  IsBoolean,
  IsNotEmpty,
  IsOptional,
  IsString,
  IsUUID,
  ValidateNested,
} from 'class-validator';

export class RuleDto {
  // action e subject aceitam string ou string[]; validação fina é no service
  @IsNotEmpty()
  action!: unknown;

  @IsNotEmpty()
  subject!: unknown;

  @IsOptional()
  fields?: unknown;

  @IsOptional()
  conditions?: Record<string, unknown> | null;

  @IsOptional()
  @IsBoolean()
  inverted?: boolean;

  @IsOptional()
  @IsString()
  reason?: string;
}

export class CreateRoleDto {
  @IsString()
  @IsNotEmpty()
  name!: string;

  @IsOptional()
  @IsString()
  description?: string;

  @IsArray()
  @ValidateNested({ each: true })
  @Type(() => RuleDto)
  rules: RuleDto[] = [];
}

export class UpdateRoleDto extends PartialType(CreateRoleDto) {}

export class AssignRolesDto {
  @IsArray()
  @IsUUID('all', { each: true })
  roleIds!: string[];
}
