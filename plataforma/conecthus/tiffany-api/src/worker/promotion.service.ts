import { Injectable, Logger } from '@nestjs/common';
import { PrismaService } from '../prisma.service';
import { GitService } from './git.service';
import { MemoryService } from '../messaging/memory.service';
import { REPO_DIRS, BRANCH_MAP, ENV_URLS, DEFAULT_CHANNEL, DEFAULT_TARGET, GROUP_TARGETS } from './worker.config';

@Injectable()
export class PromotionService {
  private readonly logger = new Logger('Promotion');
  private readonly apiKey = process.env.API_KEY_WORKER || process.env.API_KEY_PATRICIA || '';

  constructor(
    private prisma: PrismaService,
    private git: GitService,
    private memory: MemoryService,
  ) {}

  private resolveTarget(task: any): string {
    const tgt = task.target || DEFAULT_TARGET;
    return GROUP_TARGETS[tgt] || tgt;
  }

  private async notify(message: string, task: any): Promise<void> {
    const ch = task?.channel || DEFAULT_CHANNEL;
    const tgt = task ? this.resolveTarget(task) : DEFAULT_TARGET;
    try {
      const port = process.env.PORT || 8080;
      await fetch(`http://127.0.0.1:${port}/api/messaging/send`, {
        method: 'POST',
        headers: { 'Content-Type': 'application/json', 'X-API-Key': this.apiKey },
        body: JSON.stringify({ channel: ch, target: tgt, message }),
        signal: AbortSignal.timeout(30_000),
      });
      this.logger.log(`Notified via ${ch} → ${tgt}`);
    } catch (err) {
      this.logger.error(`Notification failed: ${err.message}`);
    }
  }

  private async transition(taskId: string, toStatus: string, actor: string, metadata?: any) {
    const task = await this.prisma.task.findUnique({ where: { id: taskId } });
    if (!task) return null;
    return this.prisma.$transaction(async (tx) => {
      const t = await tx.task.update({
        where: { id: taskId },
        data: { status: toStatus as any, ...(toStatus === 'replanning' && { replanCount: { increment: 1 } }) },
      });
      await tx.taskTransition.create({
        data: { taskId, fromStatus: task.status, toStatus, actor, metadata: metadata ?? undefined },
      });
      return t;
    });
  }

  private async saveExecution(taskId: string, phase: string, data: any) {
    const existing = await this.prisma.taskExecution.findFirst({
      where: { taskId, finishedAt: null },
      orderBy: { startedAt: 'desc' },
    });
    if (existing) return this.prisma.taskExecution.update({ where: { id: existing.id }, data });
    return this.prisma.taskExecution.create({ data: { taskId, phase, ...data } });
  }

  async promoteTask(task: any): Promise<void> {
    const repos = task.repo ? [task.repo] : ['patria-api'];
    const toBranch = BRANCH_MAP[task.promoteTo];
    const sourceBranch = task.branch;

    if (!sourceBranch) {
      this.logger.error(`No branch found for task ${task.id}`);
      await this.notify('Falha ao promover: branch da tarefa não encontrada. A tarefa pode já ter sido promovida.', task);
      await this.prisma.task.update({ where: { id: task.id }, data: { promoteTo: null } });
      return;
    }

    this.logger.log(`Promoting: ${task.command} (${sourceBranch} → ${toBranch})`);

    try {
      let lastSha = '';
      for (const repoName of repos) {
        const srcDir = REPO_DIRS[repoName];
        if (!srcDir) continue;
        try {
          await this.git.mergeBranch(srcDir, sourceBranch, toBranch);
          lastSha = await this.git.getHeadSha(srcDir);
          this.logger.log(`${repoName}: ${sourceBranch} → ${toBranch}`);

          if (task.promoteTo === 'prod') {
            await this.git.syncHomologWithMain(srcDir);
            await this.git.deleteBranch(srcDir, sourceBranch);
          }
        } catch (mergeErr) {
          this.logger.warn(`${repoName}: skipped (${mergeErr.message.substring(0, 80)})`);
        }
      }

      const targetEnv = task.promoteTo;
      await this.prisma.task.update({
        where: { id: task.id },
        data: { environment: targetEnv, promoteTo: null, branchDeleted: targetEnv === 'prod' },
      });
      await this.transition(task.id, 'deploying', 'worker', { promotion: true, to: targetEnv });
      await this.saveExecution(task.id, 'execution', { result: `Promoted → ${targetEnv}`, commitSha: lastSha });

      await this.notify(`Promovido para ${targetEnv.toUpperCase()}!\n\nTarefa: ${task.command}\nRepos: ${repos.join(', ')}\n\nDeploy em andamento...`, task);
      await this.memory.save('project', `Promoção: ${task.command}`, `Promovido para ${targetEnv.toUpperCase()}. Repos: ${repos.join(', ')}.`, 'short_term');
    } catch (err) {
      this.logger.error(`Promotion error: ${err.message}`);
      await this.notify(`Falha ao promover tarefa: ${task.command}\nErro: ${err.message.substring(0, 200)}`, task);
    }
  }

  async promoteProject(project: any): Promise<void> {
    const subtasks = await this.prisma.task.findMany({ where: { projectId: project.id }, orderBy: { sortOrder: 'asc' } });
    if (subtasks.length === 0) return;

    const toBranch = BRANCH_MAP[project.promoteTo];
    const sourceBranch = project.branch;
    if (!sourceBranch) {
      this.logger.error(`No branch found for project ${project.id}`);
      return;
    }

    const targetEnv = project.promoteTo;
    this.logger.log(`Promoting project: ${project.name} (${sourceBranch} → ${toBranch})`);

    try {
      const repos = [...new Set(subtasks.map((t) => t.repo).filter(Boolean))];
      let lastCommitSha = '';

      for (const repo of repos) {
        const repoDir = REPO_DIRS[repo];
        if (!repoDir) continue;
        try {
          await this.git.mergeBranch(repoDir, sourceBranch, toBranch);
          lastCommitSha = await this.git.getHeadSha(repoDir);
          this.logger.log(`Merged ${sourceBranch} → ${toBranch} in ${repo}`);

          if (targetEnv === 'prod') {
            await this.git.syncHomologWithMain(repoDir);
            await this.git.deleteBranch(repoDir, sourceBranch);
          }
        } catch {
          this.logger.warn(`No branch ${sourceBranch} in ${repo} (skipping)`);
        }
      }

      const deployedAtField = { dev: 'deployedDevAt', homolog: 'deployedHomologAt', prod: 'deployedProdAt' }[targetEnv];
      await this.prisma.project.update({
        where: { id: project.id },
        data: { environment: targetEnv, promoteTo: null, [deployedAtField]: new Date() },
      });
      await this.saveExecution(subtasks[0].id, 'execution', { result: `Project promoted → ${targetEnv}`, commitSha: lastCommitSha });
      await this.prisma.task.updateMany({ where: { projectId: project.id }, data: { environment: targetEnv } });
      await this.transition(subtasks[0].id, 'deploying', 'worker', { promotion: true });

      await this.notify(`Projeto promovido para ${targetEnv.toUpperCase()}!\n\nProjeto: ${project.name}\n\nDeploy em andamento...`, project);
      await this.memory.save('project', `Promoção: ${project.name}`, `Projeto promovido para ${targetEnv.toUpperCase()}. Repos: ${repos.join(', ')}.`, 'short_term');
    } catch (err) {
      this.logger.error(`Project promotion error: ${err.message}`);
      await this.notify(`Falha ao promover projeto: ${project.name}\nErro: ${err.message.substring(0, 200)}`, project);
    }
  }
}
