import { Controller, Get, Post, Patch, Body, Param, Query } from '@nestjs/common';
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

  @Patch(':id')
  update(@Param('id') id: string, @Body() body: any) {
    return this.projectsService.transition(id, body.status);
  }
}
