import { Injectable, Logger, OnModuleInit } from '@nestjs/common';
import { Interval } from '@nestjs/schedule';
import { PrismaService } from '../prisma.service';
import { TaskExecutionService } from './task-execution.service';
import { ProjectExecutionService } from './project-execution.service';
import { PromotionService } from './promotion.service';
import { DeployMonitorService } from './deploy-monitor.service';
import { ClaudeCliService } from './claude-cli.service';
import { GitService } from './git.service';
import { MemoryService } from '../messaging/memory.service';
import { REPO_DIRS, DEFAULT_CHANNEL, DEFAULT_TARGET, GROUP_TARGETS } from './worker.config';

const ENABLED = process.env.ENABLE_WORKER === 'true';

@Injectable()
export class WorkerService implements OnModuleInit {
  private readonly logger = new Logger('Worker');
  private processing = false;
  private readonly apiKey = process.env.API_KEY_WORKER || process.env.API_KEY_PATRICIA || '';

  constructor(
    private prisma: PrismaService,
    private taskExec: TaskExecutionService,
    private projectExec: ProjectExecutionService,
    private promotion: PromotionService,
    private deployMonitor: DeployMonitorService,
    private claude: ClaudeCliService,
    private git: GitService,
    private memory: MemoryService,
  ) {}

  onModuleInit() {
    if (!ENABLED) {
      this.logger.warn('Worker DISABLED (set ENABLE_WORKER=true to enable)');
      return;
    }
    this.logger.log('Worker enabled — polling every 30s');
    // First poll after 10s startup delay
    setTimeout(() => this.processQueue(), 10_000);
  }

  @Interval(30_000)
  async scheduledPoll() {
    if (!ENABLED) return;
    await this.processQueue();
  }

  private async processQueue(): Promise<void> {
    if (this.processing) return;
    this.processing = true;

    try {
      // === Standalone tasks ===
      const pending = await this.prisma.task.findMany({ where: { status: 'pending', projectId: null } });
      for (const task of pending) await this.taskExec.planTask(task);

      const replanning = await this.prisma.task.findMany({ where: { status: 'replanning', projectId: null } });
      for (const task of replanning) {
        if (await this.deployMonitor.isDeployFailureReplan(task.id)) {
          await this.deployMonitor.createFixTask(task);
        } else {
          await this.taskExec.planTask(task, true);
        }
      }

      const approved = await this.prisma.task.findMany({ where: { status: 'approved', projectId: null } });
      for (const task of approved) await this.taskExec.executeTask(task);

      // === Projects ===
      const planningProjects = await this.prisma.project.findMany({ where: { status: 'planning' } });
      for (const p of planningProjects) await this.projectExec.decomposeProject(p);

      const reviewProjects = await this.prisma.project.findMany({ where: { status: 'awaiting_review' } });
      for (const p of reviewProjects) await this.projectExec.handleProjectDiscussion(p);

      const activeProjects = await this.prisma.project.findMany({
        where: { status: { in: ['approved', 'executing'] } },
      });
      for (const p of activeProjects) await this.projectExec.processProjectSubtask(p);

      // === Retry blocked tasks ===
      await this.deployMonitor.retryBlockedTasks();

      // === Promotions ===
      const tasksToPromote = await this.prisma.task.findMany({
        where: { status: 'completed', promoteTo: { not: null }, projectId: null },
      });
      for (const task of tasksToPromote) await this.promotion.promoteTask(task);

      const projectsToPromote = await this.prisma.project.findMany({
        where: { promoteTo: { not: null } },
      });
      for (const p of projectsToPromote) await this.promotion.promoteProject(p);

      // === Specialist queries ===
      await this.processSpecialistQueries();

      // === Deploy notifications ===
      await this.deployMonitor.handleDeployNotifications();
    } catch (err) {
      this.logger.error(`Process queue error: ${err.message}`);
    } finally {
      this.processing = false;
    }
  }

  private async processSpecialistQueries(): Promise<void> {
    const pendingQueries = await this.prisma.specialistQuery.findMany({ where: { status: 'pending' } });

    for (const query of pendingQueries) {
      try {
        await this.prisma.specialistQuery.update({ where: { id: query.id }, data: { status: 'processing' } });
        this.logger.log(`Processing specialist query (${query.type}): ${query.question.substring(0, 60)}...`);

        const repoDir = REPO_DIRS[query.repo] || REPO_DIRS['patria-api'];
        const agentContext = {
          'patria-api': { role: 'técnico de backend', stack: 'NestJS, TypeScript, Prisma, PostgreSQL' },
          'patria-app': { role: 'técnico de frontend', stack: 'React 19, Redux, Tailwind, Vite' },
          landpage: { role: 'técnico de frontend', stack: 'React 19, CSS puro, Vite' },
        }[query.repo] || { role: 'técnico', stack: 'NestJS + React' };

        let answer: string;

        if (query.type === 'specialist_session') {
          let session = await this.claude.getSpecialistSession(query.channel, query.target);

          if (!session) {
            await this.git.gitSync(repoDir);
            const allProjects = await this.prisma.project.findMany({
              select: { name: true, status: true, environment: true },
              orderBy: { createdAt: 'desc' },
              take: 10,
            });
            const projectContext = '\nProjetos:\n' + allProjects.map((p) => `- ${p.name} (${p.status}, ${p.environment})`).join('\n');

            session = await this.claude.openSpecialistProcess(repoDir, '', query.channel, query.target);
            answer = await session.ask(
              `Você é o ${agentContext.role} da Patria Technology, especialista em ${agentContext.stack}.\n\nLeia PRODUCT.md se existir. NÃO altere nenhum arquivo.\n${projectContext}\n\nPergunta: ${query.question}\n\nResponda em português, máximo 15 linhas.`,
            );
          } else {
            answer = await session.ask(query.question);
          }
        } else {
          await this.git.gitSync(repoDir);
          const allProjects = await this.prisma.project.findMany({
            select: { name: true, status: true, environment: true },
            orderBy: { createdAt: 'desc' },
            take: 10,
          });
          let projectContext = '\nProjetos existentes:\n' + allProjects.map((p) => `- ${p.name} (${p.status}, ${p.environment})`).join('\n');

          if (query.projectId) {
            const project = await this.prisma.project.findUnique({ where: { id: query.projectId } });
            if (project) projectContext += `\n\nProjeto em foco: ${project.name} (${project.status}, ${project.environment})`;
          }

          const promptType = query.type === 'diagnose'
            ? 'Analise o código, identifique a causa (cite arquivo e linha), e sugira a correção.'
            : 'Sugira os próximos passos baseado no PRODUCT.md e no que já foi implementado. NÃO sugira o que já está concluído.';

          answer = await this.claude.runClaudeReadOnly(
            `Você é o ${agentContext.role} da Patria Technology, especialista em ${agentContext.stack}.\n\nLeia PRODUCT.md se existir. NÃO altere nenhum arquivo.\n${projectContext}\n\nPergunta: ${query.question}\n\n${promptType}\nResponda em português, máximo 15 linhas.`,
            repoDir,
          );
        }

        await this.prisma.specialistQuery.update({
          where: { id: query.id },
          data: { status: 'completed', answer },
        });

        const specialistLabel = query.repo === 'patria-api' ? 'Backend' : 'Frontend';
        const cleanAnswer = answer.replace(/[`*#_~]/g, '').substring(0, 1000);
        await this.notify(`Diagnóstico do Técnico ${specialistLabel}:\n\n${cleanAnswer}`, { channel: query.channel, target: query.target });
        this.logger.log('Specialist query answered and notified');
      } catch (err) {
        await this.prisma.specialistQuery.update({ where: { id: query.id }, data: { status: 'failed', answer: err.message } });
        this.logger.error(`Specialist query error: ${err.message}`);
        await this.notify(
          `Não consegui completar o diagnóstico.\n\nPergunta: ${query.question}\nErro: ${err.message.substring(0, 200)}\n\nTente novamente em alguns minutos.`,
          { channel: query.channel, target: query.target },
        );
      }
    }
  }

  private async notify(message: string, task: any): Promise<void> {
    const ch = task?.channel || DEFAULT_CHANNEL;
    const tgt = GROUP_TARGETS[task?.target] || task?.target || DEFAULT_TARGET;
    try {
      const port = process.env.PORT || 8080;
      await fetch(`http://127.0.0.1:${port}/api/messaging/send`, {
        method: 'POST',
        headers: { 'Content-Type': 'application/json', 'X-API-Key': this.apiKey },
        body: JSON.stringify({ channel: ch, target: tgt, message }),
        signal: AbortSignal.timeout(30_000),
      });
    } catch (err) {
      this.logger.error(`Notification failed: ${err.message}`);
    }
  }
}
