import { Controller, Get, Post, Patch, Delete, Body, Param } from '@nestjs/common';
import { ApiTags, ApiOperation, ApiResponse, ApiSecurity, ApiProperty, ApiPropertyOptional } from '@nestjs/swagger';
import { IsString, IsEmail, IsInt, Min, Max, IsOptional } from 'class-validator';
import { FeedbacksService } from './feedbacks.service';

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

  @ApiPropertyOptional({ description: 'Mensagem opcional', example: 'Excelente serviço!' })
  @IsOptional()
  @IsString()
  mensagem?: string;
}

export class UpdateFeedbackDto {
  @ApiPropertyOptional({ description: 'Nome do remetente', example: 'João Silva' })
  @IsOptional()
  @IsString()
  nome?: string;

  @ApiPropertyOptional({ description: 'Email do remetente', example: 'joao@email.com' })
  @IsOptional()
  @IsEmail()
  email?: string;

  @ApiPropertyOptional({ description: 'Nota de 1 a 5', minimum: 1, maximum: 5, example: 4 })
  @IsOptional()
  @IsInt()
  @Min(1)
  @Max(5)
  nota?: number;

  @ApiPropertyOptional({ description: 'Mensagem opcional', example: 'Bom serviço!' })
  @IsOptional()
  @IsString()
  mensagem?: string;
}

@ApiTags('Feedbacks')
@ApiSecurity('api-key')
@Controller('api/feedbacks')
export class FeedbacksController {
  constructor(private readonly feedbacksService: FeedbacksService) {}

  @Post()
  @ApiOperation({ summary: 'Criar feedback' })
  @ApiResponse({ status: 201, description: 'Feedback criado com sucesso' })
  @ApiResponse({ status: 400, description: 'Dados inválidos' })
  create(@Body() dto: CreateFeedbackDto) {
    return this.feedbacksService.create(dto);
  }

  @Get()
  @ApiOperation({ summary: 'Listar todos os feedbacks' })
  @ApiResponse({ status: 200, description: 'Lista de feedbacks' })
  findAll() {
    return this.feedbacksService.findAll();
  }

  @Get(':id')
  @ApiOperation({ summary: 'Buscar feedback por ID' })
  @ApiResponse({ status: 200, description: 'Feedback encontrado' })
  @ApiResponse({ status: 404, description: 'Feedback não encontrado' })
  findOne(@Param('id') id: string) {
    return this.feedbacksService.findOne(id);
  }

  @Patch(':id')
  @ApiOperation({ summary: 'Atualizar feedback' })
  @ApiResponse({ status: 200, description: 'Feedback atualizado' })
  @ApiResponse({ status: 400, description: 'Dados inválidos' })
  @ApiResponse({ status: 404, description: 'Feedback não encontrado' })
  update(@Param('id') id: string, @Body() dto: UpdateFeedbackDto) {
    return this.feedbacksService.update(id, dto);
  }

  @Delete(':id')
  @ApiOperation({ summary: 'Remover feedback' })
  @ApiResponse({ status: 200, description: 'Feedback removido' })
  @ApiResponse({ status: 404, description: 'Feedback não encontrado' })
  remove(@Param('id') id: string) {
    return this.feedbacksService.remove(id);
  }
}
