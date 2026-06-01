import { Type } from 'class-transformer';
import {
  IsArray,
  IsBoolean,
  IsEmail,
  IsInt,
  IsNotEmpty,
  IsNumber,
  IsOptional,
  IsString,
  IsUrl,
  IsUUID,
  Min,
  ValidateNested,
} from 'class-validator';

export class CreateSupplierAccountDto {
  @IsUrl({ require_tld: true })
  baseUrl!: string;

  @IsEmail()
  loginEmail!: string;

  @IsString()
  @IsNotEmpty()
  password!: string;

  @IsOptional()
  @IsString()
  label?: string;

  @IsOptional()
  @IsUUID()
  customerSupplierId?: string;

  @IsOptional()
  @IsNumber()
  @Min(0)
  defaultMarkupPct?: number;

  /// Confirmação explícita de que o cliente tem autorização pra acessar a loja.
  @IsOptional()
  @IsBoolean()
  consent?: boolean;
}

export class UpdateSupplierAccountDto {
  @IsOptional()
  @IsString()
  label?: string;

  @IsOptional()
  @IsEmail()
  loginEmail?: string;

  @IsOptional()
  @IsString()
  @IsNotEmpty()
  password?: string;

  @IsOptional()
  @IsUUID()
  customerSupplierId?: string;

  @IsOptional()
  @IsNumber()
  @Min(0)
  defaultMarkupPct?: number;

  @IsOptional()
  @IsBoolean()
  active?: boolean;
}

export class ListSupplierProductsDto {
  @IsOptional()
  @Type(() => Number)
  @IsInt()
  @Min(1)
  page?: number;

  @IsOptional()
  @Type(() => Number)
  @IsInt()
  @Min(1)
  pageSize?: number;

  @IsOptional()
  @IsString()
  q?: string;

  @IsOptional()
  @IsBoolean()
  @Type(() => Boolean)
  onlyAvailable?: boolean;
}

export class LinkProductDto {
  @IsUUID()
  productId!: string;
}

export class ImportProductsDto {
  /// IDs de SupplierProduct a importar. Vazio = importa todos do fornecedor.
  @IsOptional()
  @IsArray()
  @IsUUID('all', { each: true })
  supplierProductIds?: string[];

  @IsOptional()
  @IsNumber()
  @Min(0)
  markupPct?: number;
}

export class PlaceSupplierOrderItemDto {
  @IsString()
  @IsNotEmpty()
  externalVariantId!: string;

  @IsInt()
  @Min(1)
  qty!: number;
}

export class PlaceSupplierOrderDto {
  @IsArray()
  @ValidateNested({ each: true })
  @Type(() => PlaceSupplierOrderItemDto)
  items!: PlaceSupplierOrderItemDto[];

  /// Submissão real do pedido. Bloqueada na Fase A/B1 (sempre dry-run).
  @IsOptional()
  @IsBoolean()
  submit?: boolean;
}
