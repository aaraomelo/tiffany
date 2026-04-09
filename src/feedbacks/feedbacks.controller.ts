import { Controller, Get, Post, Delete, Param, Body, ParseIntPipe } from '@nestjs/common';
import { ApiTags, ApiOperation, ApiResponse, ApiSecurity } from '@nestjs/swagger';
import { FeedbacksService } from './feedbacks.service';
import { CreateFeedbackDto } from './create-feedback.dto';

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
  findOne(@Param('id', ParseIntPipe) id: number) {
    return this.feedbacksService.findOne(id);
  }

  @Delete(':id')
  @ApiOperation({ summary: 'Remover feedback por ID' })
  @ApiResponse({ status: 200, description: 'Feedback removido' })
  @ApiResponse({ status: 404, description: 'Feedback não encontrado' })
  remove(@Param('id', ParseIntPipe) id: number) {
    return this.feedbacksService.remove(id);
  }
}
