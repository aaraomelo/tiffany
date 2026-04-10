import { Controller, Get, Post, Patch, Body, Param, Query, Req, Res, BadRequestException } from '@nestjs/common';
import { Request, Response } from 'express';
import { TasksService } from './tasks.service';
import { TaskStateMachine } from './task-state-machine';
import { TaskEventsService } from './task-events.service';
import { ClaudeService } from './claude.service';
import { PrismaService } from './prisma.service';

@Controller('api/tasks')
export class TasksController {
  constructor(
    private readonly tasksService: TasksService,
    private readonly stateMachine: TaskStateMachine,
    private readonly taskEvents: TaskEventsService,
    private readonly claude: ClaudeService,
    private readonly prisma: PrismaService,
  ) {}

  @Post()
  create(@Body() body: { command: string; description: string; createdBy?: string; channel?: string; target?: string; repo?: string }) {
    return this.tasksService.create(
      body.command,
      body.description,
      body.createdBy || 'patricia',
      body.channel,
      body.target,
      body.repo,
    );
  }

  @Get()
  findAll(@Query('status') status?: string) {
    return this.tasksService.findAll(status);
  }

  @Get('search')
  async searchTasks(@Query('q') query: string) {
    if (!query) throw new BadRequestException('Query parameter "q" is required');
    return this.claude.searchTasks(query);
  }

  // SSE endpoint - must be before :id routes
  @Get('events/stream')
  stream(@Req() req: Request, @Res() res: Response) {
    res.setHeader('Content-Type', 'text/event-stream');
    res.setHeader('Cache-Control', 'no-cache');
    res.setHeader('Connection', 'keep-alive');
    res.flushHeaders();

    const heartbeat = setInterval(() => res.write(': heartbeat\n\n'), 30_000);

    const unsubscribe = this.taskEvents.subscribe((event) => {
      res.write(`data: ${JSON.stringify(event)}\n\n`);
    });

    req.on('close', () => {
      clearInterval(heartbeat);
      unsubscribe();
    });
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

  @Post(':id/skip')
  skip(@Param('id') id: string, @Body() body: { actor?: string }) {
    return this.stateMachine.transition(id, 'cancelled', body.actor || 'director', { skipped: true });
  }

  @Post(':id/resolve')
  async resolve(@Param('id') id: string, @Body() body: { actor?: string }) {
    const task = await this.prisma.task.findUnique({ where: { id } });
    if (!task) throw new BadRequestException('Task not found');
    if (task.status === 'completed') return task;
    // Force-complete: bypass state machine, mark as completed directly
    await this.prisma.taskTransition.create({
      data: { taskId: id, fromStatus: task.status, toStatus: 'completed', actor: body.actor || 'director', metadata: { resolved: true } },
    });
    return this.prisma.task.update({ where: { id }, data: { status: 'completed' } });
  }

  @Post(':id/promote')
  async promote(@Param('id') id: string, @Body() body: { targetEnv: string }) {
    const task = await this.prisma.task.findUnique({ where: { id } });
    if (!task) throw new BadRequestException('Task not found');
    if (task.status !== 'completed') throw new BadRequestException('Task must be completed before promotion');
    const validEnvs = ['dev', 'homolog', 'prod'];
    if (!validEnvs.includes(body.targetEnv)) {
      throw new BadRequestException(`Invalid target environment: ${body.targetEnv}`);
    }
    return this.prisma.task.update({ where: { id }, data: { promoteTo: body.targetEnv as any } });
  }

  // Create task from template
  @Post('from-template/:slug')
  async createFromTemplate(
    @Param('slug') slug: string,
    @Body() body: { values?: Record<string, string>; createdBy?: string; channel?: string; target?: string },
  ) {
    const template = await this.prisma.taskTemplate.findUnique({ where: { slug } });
    if (!template || !template.isActive) {
      throw new BadRequestException(`Template '${slug}' not found or inactive`);
    }

    // Interpolate template with values
    let command = template.commandTemplate;
    let description = template.descriptionTemplate || '';
    const values = body.values || {};

    for (const [key, value] of Object.entries(values)) {
      command = command.replace(new RegExp(`{{${key}}}`, 'g'), value);
      description = description.replace(new RegExp(`{{${key}}}`, 'g'), value);
    }

    return this.tasksService.create(
      command,
      description,
      body.createdBy || 'patricia',
      body.channel,
      body.target,
      template.repo,
    );
  }

  // Worker PATCH endpoint (backward compatible, with state machine validation)
  @Patch(':id')
  update(@Param('id') id: string, @Body() body: any) {
    return this.tasksService.update(id, body);
  }
}
