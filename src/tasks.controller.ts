import { Controller, Get, Post, Patch, Body, Param, Query } from '@nestjs/common';
import { TasksService } from './tasks.service';

@Controller('api/tasks')
export class TasksController {
  constructor(private readonly tasksService: TasksService) {}

  @Post()
  create(@Body() body: { command: string; description: string; createdBy?: string; channel?: string; target?: string }) {
    return this.tasksService.create(
      body.command,
      body.description,
      body.createdBy || 'patricia',
      body.channel,
      body.target,
    );
  }

  @Get()
  findAll(@Query('status') status?: string) {
    return this.tasksService.findAll(status);
  }

  @Get(':id')
  findOne(@Param('id') id: string) {
    return this.tasksService.findOne(id);
  }

  @Patch(':id')
  update(@Param('id') id: string, @Body() body: { status?: any; result?: string; plan?: string; question?: string; feedback?: string; channel?: string; target?: string }) {
    return this.tasksService.update(id, body);
  }
}
