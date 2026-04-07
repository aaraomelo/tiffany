import { Controller, Get, Post, Patch, Body, Param, Query } from '@nestjs/common';
import { TasksService } from './tasks.service';
import { TaskStateMachine } from './task-state-machine';

@Controller('api/tasks')
export class TasksController {
  constructor(
    private readonly tasksService: TasksService,
    private readonly stateMachine: TaskStateMachine,
  ) {}

  @Post()
  create(@Body() body: { command: string; description: string; createdBy?: string; channel?: string; target?: string; project?: string }) {
    return this.tasksService.create(
      body.command,
      body.description,
      body.createdBy || 'patricia',
      body.channel,
      body.target,
      body.project,
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

  @Get(':id/history')
  getHistory(@Param('id') id: string) {
    return this.tasksService.getHistory(id);
  }

  // Action endpoints
  @Post(':id/approve')
  approve(@Param('id') id: string, @Body() body: { actor?: string }) {
    return this.stateMachine.transition(id, 'approved', body.actor || 'director');
  }

  @Post(':id/reject')
  reject(@Param('id') id: string, @Body() body: { actor?: string; feedback?: string }) {
    const toStatus = body.feedback ? 'replanning' : 'rejected';
    return this.stateMachine.transition(id, toStatus as any, body.actor || 'director', { feedback: body.feedback });
  }

  @Post(':id/cancel')
  cancel(@Param('id') id: string, @Body() body: { actor?: string }) {
    return this.stateMachine.transition(id, 'cancelled', body.actor || 'director');
  }

  @Post(':id/provide-info')
  provideInfo(@Param('id') id: string, @Body() body: { feedback: string; actor?: string }) {
    return this.stateMachine.transition(id, 'planning', body.actor || 'director', { feedback: body.feedback });
  }

  @Post(':id/retry')
  retry(@Param('id') id: string, @Body() body: { actor?: string }) {
    return this.stateMachine.transition(id, 'pending', body.actor || 'director');
  }

  // Worker PATCH endpoint (backward compatible, with state machine validation)
  @Patch(':id')
  update(@Param('id') id: string, @Body() body: any) {
    return this.tasksService.update(id, body);
  }
}
