import { Injectable, BadRequestException } from '@nestjs/common';
import { PrismaService } from './prisma.service';
import { ProjectStatus } from '@prisma/client';

const PROJECT_TRANSITIONS: Record<string, string[]> = {
  planning: ['awaiting_review', 'cancelled'],
  awaiting_review: ['planning', 'approved', 'executing', 'completed', 'cancelled'],
  approved: ['executing', 'paused', 'cancelled'],
  executing: ['paused', 'completed', 'failed', 'cancelled'],
  paused: ['executing', 'cancelled'],
  completed: ['executing'],
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

  async addSubtask(projectId: string, data: {
    command: string;
    description?: string;
    repo?: string;
  }) {
    const proj = await this.prisma.project.findUnique({ where: { id: projectId } });
    if (!proj) throw new BadRequestException('Project not found');

    // Get next sortOrder
    const lastTask = await this.prisma.task.findFirst({
      where: { projectId },
      orderBy: { sortOrder: 'desc' },
    });
    const sortOrder = (lastTask?.sortOrder || 0) + 1;

    const task = await this.prisma.task.create({
      data: {
        command: data.command,
        description: data.description,
        repo: data.repo || 'landpage',
        projectId,
        sortOrder,
        createdBy: 'director',
        channel: proj.channel,
        target: proj.target,
      },
    });

    // Update total and reactivate project if needed
    const total = await this.prisma.task.count({ where: { projectId } });
    const updateData: any = { totalSubtasks: total };

    // If project is completed or awaiting_review, move back to executing
    if (proj.status === 'completed' || proj.status === 'awaiting_review') {
      updateData.status = 'executing';
    }

    await this.prisma.project.update({ where: { id: projectId }, data: updateData });

    return task;
  }

  async reopen(id: string) {
    return this.transition(id, 'executing');
  }

  async setPromoteTo(id: string, targetEnv: string) {
    return this.prisma.project.update({
      where: { id },
      data: { promoteTo: targetEnv as any },
    });
  }

  async getCommits(projectId: string) {
    return this.prisma.taskExecution.findMany({
      where: {
        task: { projectId },
        commitSha: { not: null },
      },
      select: { commitSha: true, result: true, finishedAt: true, task: { select: { command: true } } },
      orderBy: { finishedAt: 'asc' },
    });
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
