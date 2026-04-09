import { Injectable, forwardRef, Inject } from '@nestjs/common';
import { PrismaService } from './prisma.service';
import { TaskStateMachine } from './task-state-machine';
import { TaskEventsService } from './task-events.service';
import { Task, TaskStatus } from '@prisma/client';

@Injectable()
export class TasksService {
  constructor(
    private prisma: PrismaService,
    @Inject(forwardRef(() => TaskStateMachine)) private stateMachine: TaskStateMachine,
    private taskEvents: TaskEventsService,
  ) {}

  async create(
    command: string,
    description: string,
    createdBy: string,
    channel?: string,
    target?: string,
    repo?: string,
  ): Promise<Task> {
    const task = await this.prisma.task.create({
      data: {
        command,
        description,
        createdBy,
        channel: channel || 'whatsapp',
        target: target || '+5511977808883',
        repo,
      },
    });

    await this.prisma.taskTransition.create({
      data: {
        taskId: task.id,
        fromStatus: '',
        toStatus: 'pending',
        actor: createdBy,
      },
    });

    this.taskEvents.emit({ type: 'created', taskId: task.id, status: 'pending' });

    return task;
  }

  async findAll(status?: string): Promise<Task[]> {
    if (status) {
      return this.prisma.task.findMany({
        where: { status: status as TaskStatus },
        orderBy: { createdAt: 'desc' },
      });
    }
    return this.prisma.task.findMany({ orderBy: { createdAt: 'desc' } });
  }

  async findOne(id: string): Promise<Task | null> {
    return this.prisma.task.findUnique({ where: { id } });
  }

  async update(
    id: string,
    data: {
      status?: string;
      result?: string;
      plan?: string;
      question?: string;
      feedback?: string;
      channel?: string;
      target?: string;
      repo?: string;
      commitSha?: string;
    },
  ): Promise<Task | null> {
    const task = await this.prisma.task.findUnique({ where: { id } });
    if (!task) return null;

    const oldStatus = task.status;
    const updateData: any = {};

    if (data.status) updateData.status = data.status as TaskStatus;
    if (data.channel !== undefined) updateData.channel = data.channel;
    if (data.target !== undefined) updateData.target = data.target;
    if (data.repo !== undefined) updateData.repo = data.repo;

    // Save commitSha on latest execution
    if (data.commitSha) {
      const latestExec = await this.prisma.taskExecution.findFirst({
        where: { taskId: id },
        orderBy: { startedAt: 'desc' },
      });
      if (latestExec) {
        await this.prisma.taskExecution.update({
          where: { id: latestExec.id },
          data: { commitSha: data.commitSha },
        });
      }
    }

    // Store execution-related data in task_executions
    if (data.plan !== undefined || data.question !== undefined || data.result !== undefined || data.feedback !== undefined) {
      const phase = data.status === 'replanning' ? 'replanning' :
                    data.result !== undefined ? 'execution' : 'planning';

      // Find existing execution for this phase or create new
      const existingExecution = await this.prisma.taskExecution.findFirst({
        where: { taskId: id, finishedAt: null },
        orderBy: { startedAt: 'desc' },
      });

      if (existingExecution) {
        await this.prisma.taskExecution.update({
          where: { id: existingExecution.id },
          data: {
            ...(data.plan !== undefined && { plan: data.plan }),
            ...(data.question !== undefined && { question: data.question }),
            ...(data.feedback !== undefined && { feedback: data.feedback }),
            ...(data.result !== undefined && { result: data.result }),
            ...(data.result !== undefined && { finishedAt: new Date() }),
          },
        });
      } else {
        await this.prisma.taskExecution.create({
          data: {
            taskId: id,
            phase: phase as any,
            ...(data.plan !== undefined && { plan: data.plan }),
            ...(data.question !== undefined && { question: data.question }),
            ...(data.feedback !== undefined && { feedback: data.feedback }),
            ...(data.result !== undefined && { result: data.result }),
            ...(data.result !== undefined && { finishedAt: new Date() }),
          },
        });
      }
    }

    // Use state machine for status transitions
    let updated: Task;
    if (data.status && data.status !== oldStatus) {
      // Non-status fields first
      if (Object.keys(updateData).length > 1 || !updateData.status) {
        const { status, ...rest } = updateData;
        if (Object.keys(rest).length > 0) {
          await this.prisma.task.update({ where: { id }, data: rest });
        }
      }
      // Infer actor: director actions vs worker actions
      const directorStatuses = ['approved', 'rejected', 'replanning', 'cancelled', 'pending'];
      const isDirectorAction = directorStatuses.includes(data.status) &&
        ['awaiting_approval', 'needs_info', 'failed', 'timed_out'].includes(oldStatus);
      const actor = isDirectorAction ? 'director' : 'worker';

      updated = await this.stateMachine.transition(
        id,
        data.status as TaskStatus,
        actor,
        data.feedback ? { feedback: data.feedback } : undefined,
      );
    } else {
      updated = await this.prisma.task.update({
        where: { id },
        data: updateData,
      });
    }

    // Return flattened response for backward compatibility with worker
    const latestExecution = await this.prisma.taskExecution.findFirst({
      where: { taskId: id },
      orderBy: { startedAt: 'desc' },
    });

    return {
      ...updated,
      plan: latestExecution?.plan ?? null,
      question: latestExecution?.question ?? null,
      feedback: latestExecution?.feedback ?? null,
      result: latestExecution?.result ?? null,
    } as any;
  }

  async getHistory(id: string) {
    return this.prisma.taskTransition.findMany({
      where: { taskId: id },
      orderBy: { createdAt: 'asc' },
    });
  }
}
