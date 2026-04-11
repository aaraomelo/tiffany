import { Injectable, BadRequestException } from '@nestjs/common';
import { PrismaService } from './prisma.service';
import { ClaudeService } from './claude.service';
import { ProjectStatus } from '@prisma/client';
import { RequestContext } from './request-context';

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
  constructor(private prisma: PrismaService, private claude: ClaudeService) {}

  async create(data: {
    name: string;
    description?: string;
    autoApprove?: boolean;
    createdBy?: string;
    channel?: string;
    target?: string;
  }) {
    const project = await this.prisma.project.create({
      data: {
        name: data.name,
        description: data.description,
        autoApprove: true,
        createdBy: data.createdBy || 'patricia',
        channel: data.channel || 'whatsapp',
        target: data.target || '+5511977808883',
        tenantId: RequestContext.alias,
      },
    });
    this.claude.embedProject(project.id, project.name, project.description).catch(() => {});
    return project;
  }

  async findAll(status?: string) {
    const tenantId = RequestContext.alias;
    const where = status ? { tenantId, status: status as ProjectStatus } : { tenantId };
    return this.prisma.project.findMany({
      where,
      include: { subtasks: { orderBy: { sortOrder: 'asc' } } },
      orderBy: { createdAt: 'desc' },
    });
  }

  async findOne(id: string) {
    return this.prisma.project.findFirst({
      where: { id, tenantId: RequestContext.alias },
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
    const project = await this.prisma.project.findFirst({ where: { id, tenantId: RequestContext.alias } });
    if (!project) throw new BadRequestException('Project not found');

    return this.prisma.projectPlanning.create({
      data: { projectId: id, role, content: message },
    });
  }

  async transition(id: string, toStatus: ProjectStatus) {
    const project = await this.prisma.project.findFirst({ where: { id, tenantId: RequestContext.alias } });
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

  async forceComplete(id: string) {
    // Cancel all non-completed subtasks and mark project as completed
    await this.prisma.$transaction(async (tx) => {
      await tx.task.updateMany({
        where: {
          projectId: id,
          status: { notIn: ['completed', 'cancelled'] },
        },
        data: { status: 'cancelled' },
      });
      const done = await tx.task.count({ where: { projectId: id, status: 'completed' } });
      const total = await tx.task.count({ where: { projectId: id } });
      await tx.project.update({
        where: { id },
        data: { status: 'completed', doneSubtasks: done, totalSubtasks: total },
      });
    });
    return this.findOne(id);
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
    const proj = await this.prisma.project.findFirst({ where: { id: projectId, tenantId: RequestContext.alias } });
    if (!proj) throw new BadRequestException('Project not found');

    // Get next sortOrder
    const lastTask = await this.prisma.task.findFirst({
      where: { projectId },
      orderBy: { sortOrder: 'desc' },
    });
    const sortOrder = (lastTask?.sortOrder || 0) + 1;

    // Use repo from caller if provided, otherwise infer
    let repo = data.repo || await this.claude.inferRepo(data.command, data.description, projectId);
    if (!repo) {
      // Fallback: keyword-based
      const text = `${data.command} ${data.description || ''}`.toLowerCase();
      if (text.includes('api') || text.includes('endpoint') || text.includes('backend') || text.includes('prisma')) {
        repo = 'patria-api';
      } else if (text.includes('app') || text.includes('multi-tenant')) {
        repo = 'patria-app';
      } else {
        repo = 'landpage';
      }
    }

    const task = await this.prisma.task.create({
      data: {
        command: data.command,
        description: data.description,
        repo,
        projectId,
        sortOrder,
        createdBy: 'director',
        channel: proj.channel,
        target: proj.target,
        tenantId: RequestContext.alias,
      },
    });

    // Generate embedding in background
    this.claude.embedTask(task.id, data.command, data.description, projectId).catch(() => {});

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

  async countPendingSubtasks(projectId: string) {
    return this.prisma.task.count({
      where: {
        projectId,
        status: { notIn: ['completed', 'cancelled'] },
      },
    });
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
