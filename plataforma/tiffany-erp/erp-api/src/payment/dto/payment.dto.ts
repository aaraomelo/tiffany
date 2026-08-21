import { PaymentMethod } from '@prisma/client';
import { Type } from 'class-transformer';
import {
  IsEnum,
  IsInt,
  IsNumber,
  IsOptional,
  IsUUID,
  Min,
} from 'class-validator';

export class CreatePaymentDto {
  @IsUUID()
  orderId!: string;

  @IsEnum(PaymentMethod)
  method!: PaymentMethod;

  @Type(() => Number)
  @IsNumber({ maxDecimalPlaces: 2 })
  @Min(0.01)
  amount!: number;

  @IsOptional()
  @Type(() => Number)
  @IsInt()
  @Min(1)
  installments?: number;
}
