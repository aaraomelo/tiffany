import { Controller, Get, Post, Patch, Delete, Body, Param, Query } from '@nestjs/common';
import { PrismaService } from './prisma.service';

@Controller('api/templates')
export class TemplatesController {
  constructor(private prisma: PrismaService) {}

  @Post()
  async create(@Body() body: {
    name: string;
    slug: string;
    commandTemplate: string;
    descriptionTemplate?: string;
    project?: string;
    fields?: any;
    createdBy?: string;
  }) {
    return this.prisma.taskTemplate.create({
      data: {
        name: body.name,
        slug: body.slug,
        commandTemplate: body.commandTemplate,
        descriptionTemplate: body.descriptionTemplate,
        project: body.project,
        fields: body.fields || [],
        createdBy: body.createdBy || 'admin',
      },
    });
  }

  @Get()
  async findAll(@Query('active') active?: string) {
    const where = active === 'false' ? {} : { isActive: true };
    return this.prisma.taskTemplate.findMany({ where, orderBy: { name: 'asc' } });
  }

  @Get(':slug')
  async findOne(@Param('slug') slug: string) {
    return this.prisma.taskTemplate.findUnique({ where: { slug } });
  }

  @Patch(':slug')
  async update(@Param('slug') slug: string, @Body() body: any) {
    return this.prisma.taskTemplate.update({ where: { slug }, data: body });
  }

  @Delete(':slug')
  async deactivate(@Param('slug') slug: string) {
    return this.prisma.taskTemplate.update({
      where: { slug },
      data: { isActive: false },
    });
  }
}
