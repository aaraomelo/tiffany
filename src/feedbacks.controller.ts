import { Controller, Get, Post, Patch, Delete, Body, Param } from '@nestjs/common';
import { FeedbacksService } from './feedbacks.service';

@Controller('api/feedbacks')
export class FeedbacksController {
  constructor(private readonly feedbacksService: FeedbacksService) {}

  @Post()
  create(@Body() body: { nome: string; email: string; mensagem: string; nota: number }) {
    return this.feedbacksService.create(body);
  }

  @Get()
  findAll() {
    return this.feedbacksService.findAll();
  }

  @Get(':id')
  findOne(@Param('id') id: string) {
    return this.feedbacksService.findOne(id);
  }

  @Patch(':id')
  update(@Param('id') id: string, @Body() body: { nome?: string; email?: string; mensagem?: string; nota?: number }) {
    return this.feedbacksService.update(id, body);
  }

  @Delete(':id')
  remove(@Param('id') id: string) {
    return this.feedbacksService.remove(id);
  }
}
