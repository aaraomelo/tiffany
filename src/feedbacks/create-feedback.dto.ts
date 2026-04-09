import { IsString, IsEmail, IsInt, Min, Max, IsOptional } from 'class-validator';
import { ApiProperty, ApiPropertyOptional } from '@nestjs/swagger';

export class CreateFeedbackDto {
  @ApiProperty({ description: 'Nome do remetente', example: 'João Silva' })
  @IsString()
  nome: string;

  @ApiProperty({ description: 'Email do remetente', example: 'joao@email.com' })
  @IsEmail()
  email: string;

  @ApiProperty({ description: 'Nota de 1 a 5', minimum: 1, maximum: 5, example: 5 })
  @IsInt()
  @Min(1)
  @Max(5)
  nota: number;

  @ApiPropertyOptional({ description: 'Mensagem opcional', example: 'Ótimo trabalho!' })
  @IsOptional()
  @IsString()
  mensagem?: string;
}
