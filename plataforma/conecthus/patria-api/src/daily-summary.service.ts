import { Injectable } from '@nestjs/common';
import { Cron } from '@nestjs/schedule';
import { PrismaService } from './prisma.service';

@Injectable()
export class DailySummaryService {
  constructor(private prisma: PrismaService) {}

  // Every day at 9:00 AM (UTC-3 = 12:00 UTC)
  @Cron('0 12 * * *')
  async sendDailySummary() {
    const yesterday = new Date(Date.now() - 24 * 60 * 60 * 1000);

    const completedTasks = await this.prisma.task.count({
      where: { status: 'completed', updatedAt: { gte: yesterday } },
    });
    const failedTasks = await this.prisma.task.count({
      where: { status: 'failed', updatedAt: { gte: yesterday } },
    });
    const activeProjects = await this.prisma.project.findMany({
      where: { status: { in: ['executing', 'awaiting_review', 'approved'] } },
    });
    const pendingTasks = await this.prisma.task.count({
      where: { status: { in: ['pending', 'awaiting_approval'] } },
    });

    if (completedTasks === 0 && failedTasks === 0 && activeProjects.length === 0 && pendingTasks === 0) {
      return; // Nothing to report
    }

    const lines = ['📊 Resumo diário — Patria Technology\n'];

    if (completedTasks > 0) lines.push(`✅ ${completedTasks} tarefa(s) concluída(s) ontem`);
    if (failedTasks > 0) lines.push(`❌ ${failedTasks} tarefa(s) falharam`);
    if (pendingTasks > 0) lines.push(`⏳ ${pendingTasks} tarefa(s) pendente(s)`);

    if (activeProjects.length > 0) {
      lines.push(`\n📁 Projetos ativos:`);
      for (const p of activeProjects) {
        lines.push(`  • ${p.name} (${p.doneSubtasks}/${p.totalSubtasks} subtarefas)`);
      }
    }

    console.log(`[daily-summary] ${lines.join(' | ')}`);
    // Summary is available via GET /api/summary — Patrícia or cron can send it
  }

  async getSummary() {
    const yesterday = new Date(Date.now() - 24 * 60 * 60 * 1000);

    const completedTasks = await this.prisma.task.count({
      where: { status: 'completed', updatedAt: { gte: yesterday } },
    });
    const failedTasks = await this.prisma.task.count({
      where: { status: 'failed', updatedAt: { gte: yesterday } },
    });
    const activeProjects = await this.prisma.project.findMany({
      where: { status: { in: ['executing', 'awaiting_review', 'approved'] } },
      select: { id: true, name: true, status: true, doneSubtasks: true, totalSubtasks: true },
    });
    const pendingTasks = await this.prisma.task.count({
      where: { status: { in: ['pending', 'awaiting_approval'] }, projectId: null },
    });
    const deployingTasks = await this.prisma.task.count({
      where: { status: 'deploying' },
    });

    return {
      completedLast24h: completedTasks,
      failedLast24h: failedTasks,
      pendingTasks,
      deployingTasks,
      activeProjects,
    };
  }
}
