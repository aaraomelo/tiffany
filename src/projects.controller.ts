import { Controller, Get, Post, Patch, Body, Param, Query, BadRequestException } from '@nestjs/common';
import { ProjectsService } from './projects.service';

@Controller('api/projects')
export class ProjectsController {
  constructor(private readonly projectsService: ProjectsService) {}

  @Post()
  create(@Body() body: {
    name: string;
    description?: string;
    autoApprove?: boolean;
    createdBy?: string;
    channel?: string;
    target?: string;
  }) {
    return this.projectsService.create(body);
  }

  @Get()
  findAll(@Query('status') status?: string) {
    return this.projectsService.findAll(status);
  }

  @Get(':id')
  findOne(@Param('id') id: string) {
    return this.projectsService.findOne(id);
  }

  @Get(':id/planning')
  getPlanning(@Param('id') id: string) {
    return this.projectsService.getPlanning(id);
  }

  @Post(':id/discuss')
  discuss(@Param('id') id: string, @Body() body: { message: string }) {
    return this.projectsService.addDiscussion(id, body.message);
  }

  @Post(':id/approve')
  approve(@Param('id') id: string) {
    return this.projectsService.approve(id);
  }

  @Post(':id/pause')
  pause(@Param('id') id: string) {
    return this.projectsService.pause(id);
  }

  @Post(':id/resume')
  resume(@Param('id') id: string) {
    return this.projectsService.resume(id);
  }

  @Post(':id/cancel')
  cancel(@Param('id') id: string) {
    return this.projectsService.cancel(id);
  }

  @Post(':id/complete')
  async complete(@Param('id') id: string) {
    // Only allow completion if all subtasks are done
    const pending = await this.projectsService.countPendingSubtasks(id);
    if (pending > 0) {
      throw new BadRequestException(`Cannot complete: ${pending} subtask(s) still pending. Use /approve to start execution.`);
    }
    return this.projectsService.transition(id, 'completed');
  }

  @Post(':id/reopen')
  reopen(@Param('id') id: string) {
    return this.projectsService.reopen(id);
  }

  @Post(':id/promote')
  async promote(@Param('id') id: string, @Body() body: { targetEnv: string }) {
    const project = await this.projectsService.findOne(id);
    if (!project) throw new BadRequestException('Project not found');
    const validEnvs = ['dev', 'homolog', 'prod'];
    if (!validEnvs.includes(body.targetEnv)) {
      throw new BadRequestException(`Invalid target environment: ${body.targetEnv}`);
    }
    return this.projectsService.setPromoteTo(id, body.targetEnv);
  }

  @Post(':id/add-task')
  addTask(@Param('id') id: string, @Body() body: { command: string; description?: string; project?: string }) {
    return this.projectsService.addSubtask(id, body);
  }

  @Get(':id/commits')
  async getCommits(@Param('id') id: string) {
    const executions = await this.projectsService.getCommits(id);
    return executions;
  }

  @Patch(':id')
  update(@Param('id') id: string, @Body() body: any) {
    return this.projectsService.transition(id, body.status);
  }
}
