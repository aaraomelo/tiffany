import { Injectable, Logger } from '@nestjs/common';
import { PrismaService } from '../prisma.service';
import { MemoryService } from '../messaging/memory.service';
import { PORT_MAP, ENV_URLS, PROJECT_URLS, DEFAULT_CHANNEL, DEFAULT_TARGET, GROUP_TARGETS } from './worker.config';

@Injectable()
export class DeployMonitorService {
  private readonly logger = new Logger('DeployMonitor');
  private readonly notifiedDeploys = new Set<string>();
  private readonly apiKey = process.env.API_KEY_WORKER || process.env.API_KEY_PATRICIA || '';

  constructor(
    private prisma: PrismaService,
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

  async smokeTest(env: string): Promise<{ passed: boolean; results: any[]; failed: any[] }> {
    const port = PORT_MAP[env] || 8080;
    const baseUrl = `http://127.0.0.1:${port}`;
    await new Promise((r) => setTimeout(r, 5000));

    const checks = [
      { name: 'health', url: `${baseUrl}/api/health`, needsKey: false },
      { name: 'status', url: `${baseUrl}/api/status`, needsKey: true },
    ];

    const results: any[] = [];
    for (const check of checks) {
      try {
        const headers: any = check.needsKey ? { 'X-API-Key': this.apiKey } : {};
        const res = await fetch(check.url, { headers, signal: AbortSignal.timeout(10_000) });
        results.push({ name: check.name, status: res.status, ok: res.ok });
      } catch (err) {
        results.push({ name: check.name, status: 0, ok: false, error: err.message });
      }
    }

    const failed = results.filter((r) => !r.ok);
    return { passed: failed.length === 0, results, failed };
  }

  async isDeployFailureReplan(taskId: string): Promise<boolean> {
    const exec = await this.prisma.taskExecution.findFirst({
      where: { taskId },
      orderBy: { startedAt: 'desc' },
    });
    return exec?.feedback?.startsWith('[DEPLOY_FAILURE]') || false;
  }

  async getFixChainDepth(task: any): Promise<number> {
    let depth = 0;
    let current = task;
    while (current.command?.startsWith('Corrigir falha de deploy:')) {
      depth++;
      const match = current.description?.match(/TAREFA ORIGINAL: ([a-f0-9-]+)/);
      if (!match) break;
      current = await this.prisma.task.findUnique({ where: { id: match[1] } });
      if (!current) break;
    }
    return depth;
  }

  async createFixTask(task: any): Promise<void> {
    this.logger.log(`Creating fix task for deploy failure: ${task.command}`);

    try {
      const depth = await this.getFixChainDepth(task);
      if (depth >= 3) {
        this.logger.error(`Fix chain depth limit reached (${depth}/3)`);
        await this.transition(task.id, 'failed', 'worker');
        await this.saveExecution(task.id, 'execution', {
          result: `[DEPLOY_FAILURE] Max fix chain depth reached (${depth}/3). Manual intervention required.`,
        });
        await this.notify(`Deploy falhou e o limite de correções automáticas foi atingido (${depth}/3).\n\nTarefa: ${task.command}\n\nPrecisa de intervenção manual.`, task);
        return;
      }

      const exec = await this.prisma.taskExecution.findFirst({ where: { taskId: task.id }, orderBy: { startedAt: 'desc' } });
      const diagnostic = (exec?.feedback || '').replace('[DEPLOY_FAILURE]\n', '').trim();

      await this.transition(task.id, 'failed', 'worker');
      await this.saveExecution(task.id, 'execution', {
        result: `[DEPLOY_FAILURE] Pipeline validation failed. Fix task will be created.\nDiagnostic: ${diagnostic}`,
      });

      const fixCommand = `Corrigir falha de deploy: ${task.command}`;
      const fixDescription = `A tarefa "${task.command}" falhou na validação pre-deploy do pipeline.\n\nDIAGNÓSTICO DO ERRO:\n${diagnostic}\n\nTAREFA ORIGINAL: ${task.id}\nREPO ORIGINAL: ${task.repo || 'auto'}\n\nAnalise o diagnóstico, identifique a causa raiz (pode estar em qualquer repositório), e corrija o código.`;

      const fixData: any = {
        command: fixCommand,
        description: fixDescription,
        repo: null,
        channel: task.channel,
        target: task.target,
        createdBy: 'worker',
      };

      if (task.projectId) {
        fixData.projectId = task.projectId;
        fixData.sortOrder = task.sortOrder || 0;
        const lastCompleted = await this.prisma.task.findFirst({
          where: { projectId: task.projectId, status: 'completed' },
          orderBy: { sortOrder: 'desc' },
        });
        fixData.dependsOnId = lastCompleted?.id || null;

        const fixTask = await this.prisma.task.create({ data: fixData });
        const total = await this.prisma.task.count({ where: { projectId: task.projectId } });
        await this.prisma.project.update({ where: { id: task.projectId }, data: { totalSubtasks: total } });

        this.logger.log(`Fix subtask created: ${fixTask.id.substring(0, 8)}`);
        await this.notify(`Deploy falhou para "${task.command}".\n\nCriei uma tarefa de correção no projeto (prioridade alta).\nDiagnóstico: ${diagnostic.substring(0, 200)}`, task);
      } else {
        const fixTask = await this.prisma.task.create({ data: fixData });
        await this.prisma.task.update({ where: { id: task.id }, data: { dependsOnId: fixTask.id } });

        this.logger.log(`Fix task created: ${fixTask.id.substring(0, 8)}`);
        await this.notify(`Deploy falhou para "${task.command}".\n\nCriei uma tarefa de correção: ${fixTask.id.substring(0, 8)}\nDiagnóstico: ${diagnostic.substring(0, 200)}\n\nQuando a correção for concluída, a tarefa original será retentada automaticamente.`, task);
      }

      await this.retryBlockedTasks();
    } catch (err) {
      this.logger.error(`Create fix task error: ${err.message}`);
      await this.transition(task.id, 'failed', 'worker').catch(() => {});
      await this.notify(`Erro ao criar tarefa de correção:\n\n${err.message.substring(0, 200)}`, task);
    }
  }

  async retryBlockedTasks(): Promise<void> {
    const blocked = await this.prisma.task.findMany({
      where: { status: 'failed', dependsOnId: { not: null }, projectId: null },
      include: { dependsOn: true },
    });

    for (const task of blocked) {
      if ((task as any).dependsOn?.status === 'completed') {
        this.logger.log(`Fix task completed, retrying: ${task.id.substring(0, 8)}`);
        await this.prisma.task.update({ where: { id: task.id }, data: { dependsOnId: null } });
        await this.transition(task.id, 'approved', 'worker');
        await this.notify(`A correção foi concluída! Retentando a tarefa original (sem re-planejar): ${task.command}`, task);
      }
    }
  }

  async handleDeployNotifications(): Promise<void> {
    const envLabel: Record<string, string> = { dev: 'DEV', homolog: 'HOMOLOG', prod: 'PROD' };

    for (const s of ['completed', 'failed'] as const) {
      const tasks = await this.prisma.task.findMany({ where: { status: s } });
      for (const task of tasks) {
        const lt = await this.prisma.taskTransition.findFirst({
          where: { taskId: task.id },
          orderBy: { createdAt: 'desc' },
        });
        if (!lt || lt.fromStatus !== 'deploying' || lt.actor !== 'system') continue;
        const age = Date.now() - new Date(lt.createdAt).getTime();
        if (age > 120_000) continue;
        const deployKey = `${task.id}:${lt.createdAt.toISOString()}`;
        if (this.notifiedDeploys.has(deployKey)) continue;

        let env = task.environment || 'dev';
        if (task.projectId) {
          const proj = await this.prisma.project.findUnique({ where: { id: task.projectId } });
          if (proj) env = proj.environment;
        }

        if (s === 'completed') {
          this.logger.log(`Running smoke test for ${env}...`);
          const smoke = await this.smokeTest(env);
          if (!smoke.passed) {
            this.logger.error(`Smoke test failed: ${smoke.failed.map((f) => f.name).join(', ')}`);
            try { await this.transition(task.id, 'failed', 'system', { smokeTest: smoke.failed }); } catch {}
            await this.notify(`Deploy passou mas app com problema em ${env.toUpperCase()}!\n\nTarefa: ${task.command}\nFalhas: ${smoke.failed.map((f) => `${f.name} (${f.error || f.status})`).join(', ')}`, task);
            this.notifiedDeploys.add(deployKey);
            continue;
          }
          this.logger.log('Smoke test passed');
        }

        let msg: string;
        if (s === 'completed') {
          let projectInfo = '';
          if (task.projectId) {
            const proj = await this.prisma.project.findUnique({ where: { id: task.projectId } });
            if (proj) projectInfo = `\nProjeto: ${proj.name} (${proj.doneSubtasks}/${proj.totalSubtasks} subtarefas)`;
          }
          const url = ENV_URLS[env]?.[task.repo] || PROJECT_URLS[task.repo] || 'https://patriatechnology.com';
          msg = `Deploy concluido em ${envLabel[env]}!\n\nTarefa: ${task.command}${projectInfo}\n\nVeja: ${url}`;
          if (env !== 'prod') msg += '\n\nQuando quiser promover, é só me avisar.';
        } else {
          msg = `Deploy falhou em ${envLabel[env]}!\n\nTarefa: ${task.command}\nVerifique o pipeline no GitHub.`;
        }

        await this.notify(msg, task);
        if (s === 'completed') {
          await this.memory.save('project', `Deploy: ${task.command}`, `Deployado em ${envLabel[env]}. Repo: ${task.repo || 'auto'}.`, 'short_term');
        }
        this.notifiedDeploys.add(deployKey);
        this.logger.log(`Deploy ${s} notification for ${task.id.substring(0, 8)}`);
      }
    }
  }
}
