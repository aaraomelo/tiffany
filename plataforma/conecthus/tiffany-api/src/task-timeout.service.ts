import { Injectable } from '@nestjs/common';
import { Cron } from '@nestjs/schedule';
import { PrismaService } from './prisma.service';
import { TaskStateMachine } from './task-state-machine';

@Injectable()
export class TaskTimeoutService {
  constructor(
    private prisma: PrismaService,
    private stateMachine: TaskStateMachine,
  ) {}

  // Run every hour
  @Cron('0 * * * *')
  async checkTimeouts() {
    const now = new Date();

    // awaiting_approval > 48h → timed_out
    const staleApprovals = await this.prisma.task.findMany({
      where: {
        status: 'awaiting_approval',
        updatedAt: { lt: new Date(now.getTime() - 48 * 60 * 60 * 1000) },
      },
    });

    for (const task of staleApprovals) {
      try {
        await this.stateMachine.transition(task.id, 'timed_out', 'system', {
          reason: 'Approval timeout (48h)',
        });
        console.log(`[timeout] Task ${task.id.substring(0, 8)} timed out (awaiting_approval > 48h)`);
      } catch (err) {
        console.error(`[timeout] Failed to timeout task ${task.id.substring(0, 8)}: ${err.message}`);
      }
    }

    // needs_info > 72h → timed_out
    const staleInfo = await this.prisma.task.findMany({
      where: {
        status: 'needs_info',
        updatedAt: { lt: new Date(now.getTime() - 72 * 60 * 60 * 1000) },
      },
    });

    for (const task of staleInfo) {
      try {
        await this.stateMachine.transition(task.id, 'timed_out', 'system', {
          reason: 'Info timeout (72h)',
        });
        console.log(`[timeout] Task ${task.id.substring(0, 8)} timed out (needs_info > 72h)`);
      } catch (err) {
        console.error(`[timeout] Failed to timeout task ${task.id.substring(0, 8)}: ${err.message}`);
      }
    }

    if (staleApprovals.length || staleInfo.length) {
      console.log(`[timeout] Timed out ${staleApprovals.length} approvals, ${staleInfo.length} info requests`);
    }
  }
}
