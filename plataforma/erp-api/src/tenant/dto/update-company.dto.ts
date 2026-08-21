import { IsOptional, IsString } from 'class-validator';

/** Dados de cabeçalho de documento da empresa (Ordem de Serviço etc.). */
export class UpdateCompanyDto {
  @IsOptional() @IsString() companyName?: string;
  @IsOptional() @IsString() document?: string;
  @IsOptional() @IsString() responsible?: string;
  @IsOptional() @IsString() email?: string;
  @IsOptional() @IsString() phone?: string;
  @IsOptional() @IsString() phone2?: string;
  @IsOptional() @IsString() instagram?: string;
  @IsOptional() @IsString() logoUrl?: string;
  @IsOptional() @IsString() paymentMethods?: string;
  @IsOptional() @IsString() paymentTerms?: string;

  // Endereço (model Address compartilhado)
  @IsOptional() @IsString() street?: string;
  @IsOptional() @IsString() number?: string;
  @IsOptional() @IsString() complement?: string;
  @IsOptional() @IsString() neighborhood?: string;
  @IsOptional() @IsString() zipCode?: string;
  @IsOptional() @IsString() cityName?: string;
  @IsOptional() @IsString() state?: string;
}
