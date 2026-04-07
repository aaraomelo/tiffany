import { Injectable, BadRequestException } from '@nestjs/common';
import { PrismaService } from './prisma.service';
import { Task, TaskStatus } from '@prisma/client';

interface AllowedTransition {
  to: TaskStatus;
  actors: string[];
  guard?: string;
}

const TRANSITIONS: Record<TaskStatus, AllowedTransition[]> = {
  pending: [
    { to: 'planning', actors: ['worker'] },
    { to: 'cancelled', actors: ['director'] },
  ],
  planning: [
    { to: 'needs_info', actors: ['worker'] },
    { to: 'awaiting_approval', actors: ['worker'] },
    { to: 'failed', actors: ['worker'] },
  ],
  needs_info: [
    { to: 'planning', actors: ['director'], guard: 'hasFeedback' },
    { to: 'cancelled', actors: ['director'] },
    { to: 'timed_out', actors: ['system'] },
  ],
  awaiting_approval: [
    { to: 'approved', actors: ['director'] },
    { to: 'replanning', actors: ['director'], guard: 'hasFeedback' },
    { to: 'rejected', actors: ['director'] },
    { to: 'cancelled', actors: ['director'] },
    { to: 'timed_out', actors: ['system'] },
  ],
  approved: [
    { to: 'executing', actors: ['worker'] },
    { to: 'cancelled', actors: ['director'] },
  ],
  replanning: [
    { to: 'needs_info', actors: ['worker'] },
    { to: 'awaiting_approval', actors: ['worker'] },
    { to: 'failed', actors: ['worker'] },
  ],
  executing: [
    { to: 'deploying', actors: ['worker'] },
    { to: 'completed', actors: ['worker'] },
    { to: 'failed', actors: ['worker'] },
  ],
  deploying: [
    { to: 'completed', actors: ['worker', 'system'] },
    { to: 'failed', actors: ['worker', 'system'] },
  ],
  completed: [],
  failed: [
    { to: 'pending', actors: ['director'] },
    { to: 'cancelled', actors: ['director'] },
  ],
  rejected: [],
  cancelled: [],
  timed_out: [
    { to: 'pending', actors: ['director'] },
    { to: 'cancelled', actors: ['director'] },
  ],
};

@Injectable()
export class TaskStateMachine {
  constructor(private prisma: PrismaService) {}

  async transition(
    taskId: string,
    toStatus: TaskStatus,
    actor: string,
    metadata?: Record<string, any>,
  ): Promise<Task> {
    const task = await this.prisma.task.findUnique({ where: { id: taskId } });
    if (!task) throw new BadRequestException(`Task ${taskId} not found`);

    const allowed = TRANSITIONS[task.status];
    const match = allowed.find((t) => t.to === toStatus);

    if (!match) {
      throw new BadRequestException(
        `Invalid transition: ${task.status} → ${toStatus}`,
      );
    }

    // Validate actor type
    const actorType = actor.startsWith('director') ? 'director' : actor;
    if (!match.actors.includes(actorType)) {
      throw new BadRequestException(
        `Actor '${actor}' cannot perform transition ${task.status} → ${toStatus}`,
      );
    }

    // Check guards
    if (match.guard === 'hasFeedback' && !metadata?.feedback) {
      throw new BadRequestException(
        `Transition ${task.status} → ${toStatus} requires feedback`,
      );
    }

    // Execute in transaction
    const updated = await this.prisma.$transaction(async (tx) => {
      const updatedTask = await tx.task.update({
        where: { id: taskId },
        data: {
          status: toStatus,
          ...(toStatus === 'replanning' && { replanCount: { increment: 1 } }),
        },
      });

      await tx.taskTransition.create({
        data: {
          taskId,
          fromStatus: task.status,
          toStatus,
          actor,
          metadata: metadata ?? undefined,
        },
      });

      return updatedTask;
    });

    return updated;
  }

  getValidTransitions(status: TaskStatus): TaskStatus[] {
    return TRANSITIONS[status].map((t) => t.to);
  }
}
