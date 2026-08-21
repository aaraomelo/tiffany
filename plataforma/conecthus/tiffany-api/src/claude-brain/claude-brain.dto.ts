import { IsString, IsOptional, IsIn, IsArray, IsInt, Min, Max } from 'class-validator';

export class SaveMemoryDto {
  @IsString() title!: string;
  @IsString() content!: string;
  @IsString()
  @IsIn(['decision', 'preference', 'project', 'technical', 'person', 'self', 'product'])
  category!: string;

  @IsOptional()
  @IsString()
  @IsIn(['core', 'long_term', 'short_term'])
  priority?: string;

  @IsOptional()
  @IsString()
  @IsIn(['sealed', 'private', 'group', 'global'])
  visibility?: string;

  @IsOptional() @IsString() personId?: string;
  @IsOptional() @IsString() sourceModel?: string;
}

export class SearchMemoryDto {
  @IsString() q!: string;
  @IsOptional() @IsInt() @Min(1) @Max(50) limit?: number;
  @IsOptional() @IsArray() priorities?: string[];
  @IsOptional() @IsString() personId?: string;
  @IsOptional() @IsString() accessLevel?: string;
}

export class BulkSaveDto {
  @IsArray() items!: SaveMemoryDto[];
}
