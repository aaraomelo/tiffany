import { Injectable, BadRequestException } from '@nestjs/common';
import { PrismaService } from './prisma.service';
import { ProjectStatus } from '@prisma/client';

const PROJECT_TRANSITIONS: Record<string, string[]> = {
  planning: ['awaiting_review', 'cancelled'],
  awaiting_review: ['planning', 'approved', 'completed', 'cancelled'],
  approved: ['executing', 'paused', 'cancelled'],
  executing: ['paused', 'completed', 'failed', 'cancelled'],
  paused: ['executing', 'cancelled'],
  completed: [],
  failed: ['approved', 'cancelled'],
  cancelled: [],
};

@Injectable()
export class ProjectsService {
  constructor(private prisma: PrismaService) {}

  async create(data: {
    name: string;
    description?: string;
    autoApprove?: boolean;
    createdBy?: string;
    channel?: string;
    target?: string;
  }) {
    return this.prisma.project.create({
      data: {
        name: data.name,
        description: data.description,
        autoApprove: data.autoApprove ?? true,
        createdBy: data.createdBy || 'patricia',
        channel: data.channel || 'whatsapp',
        target: data.target || '+5511977808883',
      },
    });
  }

  async findAll(status?: string) {
    const where = status ? { status: status as ProjectStatus } : {};
    return this.prisma.project.findMany({
      where,
      include: { subtasks: { orderBy: { sortOrder: 'asc' } } },
      orderBy: { createdAt: 'desc' },
    });
  }

  async findOne(id: string) {
    return this.prisma.project.findUnique({
      where: { id },
      include: {
        subtasks: { orderBy: { sortOrder: 'asc' } },
        plannings: { orderBy: { createdAt: 'asc' } },
      },
    });
  }

  async getPlanning(id: string) {
    return this.prisma.projectPlanning.findMany({
      where: { projectId: id },
      orderBy: { createdAt: 'asc' },
    });
  }

  async addDiscussion(id: string, message: string, role: string = 'director') {
    const project = await this.prisma.project.findUnique({ where: { id } });
    if (!project) throw new BadRequestException('Project not found');

    return this.prisma.projectPlanning.create({
      data: { projectId: id, role, content: message },
    });
  }

  async transition(id: string, toStatus: ProjectStatus) {
    const project = await this.prisma.project.findUnique({ where: { id } });
    if (!project) throw new BadRequestException('Project not found');

    const allowed = PROJECT_TRANSITIONS[project.status] || [];
    if (!allowed.includes(toStatus)) {
      throw new BadRequestException(
        `Invalid project transition: ${project.status} → ${toStatus}`,
      );
    }

    return this.prisma.project.update({
      where: { id },
      data: { status: toStatus },
    });
  }

  async approve(id: string) {
    return this.transition(id, 'approved');
  }

  async pause(id: string) {
    return this.transition(id, 'paused');
  }

  async resume(id: string) {
    return this.transition(id, 'executing');
  }

  async cancel(id: string) {
    // Cancel project and all pending subtasks
    await this.prisma.$transaction(async (tx) => {
      await tx.project.update({
        where: { id },
        data: { status: 'cancelled' },
      });
      await tx.task.updateMany({
        where: {
          projectId: id,
          status: { in: ['pending', 'planning', 'awaiting_approval', 'approved'] },
        },
        data: { status: 'cancelled' },
      });
    });
    return this.findOne(id);
  }

  async updateProgress(id: string) {
    const subtasks = await this.prisma.task.findMany({
      where: { projectId: id },
    });
    const total = subtasks.length;
    const done = subtasks.filter((t) => t.status === 'completed').length;

    return this.prisma.project.update({
      where: { id },
      data: {
        totalSubtasks: total,
        doneSubtasks: done,
      },
    });
  }
}
