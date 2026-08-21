import { Injectable, Logger } from '@nestjs/common';
import { PrismaService } from '../prisma.service';
import { GitService } from './git.service';
import { ClaudeCliService } from './claude-cli.service';
import { MemoryService } from '../messaging/memory.service';
import { EmbeddingService } from './embedding.service';
import {
  REPO_DIRS,
  REPO_PRIORITY,
  ENV_URLS,
  PROJECT_URLS,
  DEFAULT_CHANNEL,
  DEFAULT_TARGET,
  GROUP_TARGETS,
  WHATSAPP_GROUP_ID,
  PORT_MAP,
} from './worker.config';

const API_KEY = process.env.API_KEY_WORKER || process.env.API_KEY_PATRICIA || '';

@Injectable()
export class TaskExecutionService {
  private readonly logger = new Logger('TaskExecution');

  constructor(
    private readonly prisma: PrismaService,
    private readonly git: GitService,
    private readonly claude: ClaudeCliService,
    private readonly memory: MemoryService,
    private readonly embedding: EmbeddingService,
  ) {}

  // ---------------------------------------------------------------------------
  // Helpers
  // ---------------------------------------------------------------------------

  resolveTarget(task: any): string {
    const tgt = task.target || DEFAULT_TARGET;
    return GROUP_TARGETS[tgt] || tgt;
  }

  async notify(message: string, task: any): Promise<void> {
    const ch = task?.channel || DEFAULT_CHANNEL;
    const tgt = task ? this.resolveTarget(task) : DEFAULT_TARGET;
    const port = process.env.PORT || 8080;
    try {
      const res = await fetch(`http://127.0.0.1:${port}/api/messaging/send`, {
        method: 'POST',
        headers: { 'Content-Type': 'application/json', 'X-API-Key': API_KEY },
        body: JSON.stringify({ channel: ch, target: tgt, message }),
        signal: AbortSignal.timeout(30_000),
      });
      if (!res.ok) {
        const body = await res.text();
        throw new Error(`HTTP ${res.status}: ${body}`);
      }
      this.logger.log(`Notified via ${ch} -> ${tgt}`);
    } catch (err: any) {
      this.logger.error(`Notification failed: ${err.message}`);
    }
  }

  async saveMemory(category: string, title: string, content: string, priority = 'short_term'): Promise<void> {
    try {
      await this.memory.save(category, title, content, priority);
    } catch (err: any) {
      this.logger.warn(`saveMemory failed: ${err.message}`);
    }
  }

  isTransientError(msg: string): boolean {
    return (
      msg.includes('fetch failed') ||
      msg.includes('ENOENT') ||
      msg.includes('timeout') ||
      msg.includes('ECONNREFUSED')
    );
  }

  // ---------------------------------------------------------------------------
  // DB helpers
  // ---------------------------------------------------------------------------

  async transition(taskId: string, toStatus: string, actor: string, metadata?: any): Promise<any> {
    const task = await this.prisma.task.findUnique({ where: { id: taskId } });
    if (!task) return null;
    const updated = await this.prisma.$transaction(async (tx) => {
      const t = await tx.task.update({
        where: { id: taskId },
        data: {
          status: toStatus as any,
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
      return t;
    });
    return updated;
  }

  async getLatestExecution(taskId: string): Promise<any> {
    return this.prisma.taskExecution.findFirst({
      where: { taskId },
      orderBy: { startedAt: 'desc' },
    });
  }

  async saveExecution(taskId: string, phase: string, data: any): Promise<any> {
    const existing = await this.prisma.taskExecution.findFirst({
      where: { taskId, finishedAt: null },
      orderBy: { startedAt: 'desc' },
    });
    if (existing) {
      return this.prisma.taskExecution.update({ where: { id: existing.id }, data });
    }
    return this.prisma.taskExecution.create({ data: { taskId, phase, ...data } });
  }

  // ---------------------------------------------------------------------------
  // Smoke test (inline)
  // ---------------------------------------------------------------------------

  async smokeTest(env: string): Promise<{ passed: boolean; results: any[]; failed: any[] }> {
    const port = PORT_MAP[env] || 8080;
    const baseUrl = `http://127.0.0.1:${port}`;

    // Wait for container to stabilize
    await new Promise((r) => setTimeout(r, 5000));

    const checks = [
      { name: 'health', url: `${baseUrl}/api/health`, needsKey: false },
      { name: 'status', url: `${baseUrl}/api/status`, needsKey: true },
    ];

    const results: any[] = [];
    for (const check of checks) {
      try {
        const headers: Record<string, string> = check.needsKey ? { 'X-API-Key': API_KEY } : {};
        const res = await fetch(check.url, { headers, signal: AbortSignal.timeout(10_000) });
        results.push({ name: check.name, status: res.status, ok: res.ok });
      } catch (err: any) {
        results.push({ name: check.name, status: 0, ok: false, error: err.message });
      }
    }

    const failed = results.filter((r) => !r.ok);
    return { passed: failed.length === 0, results, failed };
  }

  // ---------------------------------------------------------------------------
  // Plan Task
  // ---------------------------------------------------------------------------

  async planTask(task: any, isReplan = false, retryCount = 0): Promise<void> {
    const label = isReplan ? 'Replanning' : 'Planning';
    this.logger.log(`${label}: ${task.command} (projectId: ${task.projectId || 'standalone'})`);
    await this.transition(task.id, isReplan ? 'replanning' : 'planning', 'worker');

    try {
      const repos = await this.claude.resolveRepos(task);
      const isMultiRepo = repos.length > 1;
      if (isMultiRepo) this.logger.log(`Multi-repo plan: ${repos.join(' + ')}`);

      // Get previous plan/feedback for replanning
      let replanContext = '';
      if (isReplan) {
        const exec = await this.getLatestExecution(task.id);
        replanContext =
          `\nPLANO ANTERIOR (rejeitado): ${exec?.plan || ''}` +
          `\nMOTIVO DA REJEIÇÃO / FEEDBACK: ${exec?.feedback || ''}` +
          `\n\nCrie um NOVO plano levando em conta o feedback acima.\n`;
      }

      // Build context from completed subtasks
      let prevSubtaskContext = '';
      if (task.projectId) {
        const completedSubtasks = await this.prisma.task.findMany({
          where: {
            projectId: task.projectId,
            status: 'completed',
            sortOrder: { lt: task.sortOrder },
          },
          orderBy: { sortOrder: 'asc' },
        });
        if (completedSubtasks.length > 0) {
          const summaries: string[] = [];
          for (const prev of completedSubtasks) {
            const prevExec = await this.prisma.taskExecution.findFirst({
              where: { taskId: prev.id },
              orderBy: { startedAt: 'desc' },
            });
            summaries.push(
              `Subtarefa ${prev.sortOrder}: ${prev.command}\nResultado: ${prevExec?.result || prevExec?.plan || 'concluída'}`,
            );
          }
          prevSubtaskContext = `\nSubtarefas anteriores já concluídas:\n${summaries.join('\n\n')}\n`;
        }
      }

      // Plan in each repo
      const allPlans: string[] = [];
      let prevRepoPlan = '';
      repos.sort((a, b) => (REPO_PRIORITY[a] ?? 9) - (REPO_PRIORITY[b] ?? 9));

      for (const repoName of repos) {
        const srcDir = REPO_DIRS[repoName];
        if (!srcDir) continue;
        this.logger.log(`Project: ${repoName} (${srcDir})`);
        await this.git.gitSync(srcDir);

        const multiRepoHint = isMultiRepo
          ? `\nEsta tarefa será executada em MÚLTIPLOS repositórios: ${repos.join(', ')}. Você está planejando a parte do ${repoName}.${prevRepoPlan ? `\n\nPlano já feito em outro repo:\n${prevRepoPlan}` : ''}\n`
          : '';

        const repoPlan = await this.claude.runClaudeReadOnly(
          `Analise o projeto e crie um PLANO DETALHADO para esta tarefa. NÃO execute nada, apenas descreva o que precisa ser feito.

IMPORTANTE: Trabalhe APENAS com arquivos versionados no git. IGNORE dist/, build/, node_modules/. NÃO execute git checkout ou git pull — o código já está atualizado.

ANTES DE PLANEJAR:
1. Leia PRODUCT.md na raiz do projeto (se existir) — contém a visão do produto, modelo de dados e regras de negócio. SIGA as definições de campos, entidades e convenções descritas lá.
2. Leia CLAUDE.md — contém convenções técnicas obrigatórias (Swagger, DTOs, validação, etc.)
3. Leia o código existente (schemas, DTOs, controllers, componentes) para garantir compatibilidade.
${replanContext}${prevSubtaskContext}${multiRepoHint}
Tarefa: ${task.command}
Detalhes: ${task.description || ''}

Responda em português com:
1. Quais arquivos serão criados ou alterados neste repo (${repoName})
2. O que será feito em cada arquivo (campos, validações, endpoints, componentes)
3. Modelo de dados: campos com tipos, obrigatórios vs opcionais
4. Endpoints: método, rota, payload, resposta
5. Dependências entre esta tarefa e outras

Se precisar de informação que não tem, comece a resposta EXATAMENTE com "PRECISO_INFO:" seguido da pergunta.`,
          srcDir,
        );

        allPlans.push(`**${repoName}:**\n${repoPlan}`);
        prevRepoPlan = `[${repoName}] ${repoPlan}`;
      }

      const plan = allPlans.join('\n\n');
      await this.saveExecution(task.id, isReplan ? 'replanning' : 'planning', { plan });

      if (plan.startsWith('PRECISO_INFO:')) {
        const question = plan.replace('PRECISO_INFO:', '').trim();
        this.logger.log(`Needs info: ${question.substring(0, 80)}...`);
        await this.transition(task.id, 'needs_info', 'worker');
        await this.saveExecution(task.id, 'planning', { question });
        await this.notify(
          `Preciso de uma informacao para continuar:\n\nTarefa: ${task.command}\n\nPergunta: ${question.replace(/[`*#_~]/g, '').substring(0, 500)}`,
          task,
        );
        return;
      }

      this.logger.log(`Plan created: ${plan.substring(0, 100)}...`);
      await this.transition(task.id, 'awaiting_approval', 'worker');

      // Only notify for standalone tasks (not project subtasks)
      if (!task.projectId) {
        const cleanPlan = plan.replace(/[`*#_~]/g, '').replace(/^Plano:\s*/i, '').substring(0, 1000);
        const note = isReplan ? ' (replano apos rejeicao)' : '';
        await this.notify(
          `Plano pronto para aprovacao${note}\n\nTarefa: ${task.command}\n\n${cleanPlan}\n\nResponda "aprova" ou "rejeita"`,
          task,
        );
      }
      this.logger.log('Awaiting approval');
    } catch (err: any) {
      // Auto-retry once for transient errors
      if (retryCount === 0 && this.isTransientError(err.message)) {
        this.logger.log('Transient error, retrying in 10s...');
        await new Promise((r) => setTimeout(r, 10_000));
        return this.planTask(task, isReplan, 1);
      }
      this.logger.error(`${label} error: ${err.message}`);
      await this.transition(task.id, 'failed', 'worker');
      await this.saveExecution(task.id, 'planning', { result: `Erro no ${label.toLowerCase()}: ${err.message}` });
      if (task.projectId) {
        await this.prisma.project.update({ where: { id: task.projectId }, data: { status: 'paused' } });
        await this.notify(
          `Subtarefa falhou. Projeto pausado.\n\nTarefa: ${task.command}\nErro: ${err.message.substring(0, 200)}\n\nResponda "retenta", "pula" ou "cancela o projeto".`,
          task,
        );
      } else {
        await this.notify(`Falha ao planejar tarefa: ${task.command}\nErro: ${err.message.substring(0, 200)}`, task);
      }
    }
  }

  // ---------------------------------------------------------------------------
  // Execute in Repo
  // ---------------------------------------------------------------------------

  async executeInRepo(
    repoName: string,
    srcDir: string,
    taskBranch: string,
    baseBranch: string,
    task: any,
    exec: any,
    previousContext: string,
    prevRepoResult: string,
  ): Promise<{ result: string; hasChanges: boolean }> {
    let multiRepoContext = '';
    if (prevRepoResult) {
      multiRepoContext =
        `\n\nCONTEXTO — Alterações já feitas em outro repositório nesta mesma tarefa:\n${prevRepoResult}` +
        `\n\nGaranta que seu código é COMPATÍVEL com essas alterações (campos, tipos, payloads, rotas).\n`;
    }

    const isBackend = repoName === 'patria-api';
    const dbInstructions = isBackend
      ? `
Se precisar alterar o banco de dados (adicionar tabela, campo, relação):
1. Edite prisma/schema.prisma
2. Execute via Bash: npx prisma migrate dev --create-only --name descricao_curta
3. Execute via Bash: npx prisma generate
4. Inclua schema.prisma E prisma/migrations/ no resultado`
      : '';

    const compatInstructions = isBackend
      ? `- Prisma schema (prisma/schema.prisma) — campos obrigatórios vs opcionais
- DTOs existentes — validações, tipos, campos
- Endpoints existentes — payloads esperados, respostas`
      : `- Componentes frontend — props, estado, chamadas de API
- Store/slices Redux — actions, selectors
- Rotas existentes — paths, componentes`;

    const result = await this.claude.runClaude(
      `Execute esta tarefa no projeto (repositório: ${repoName}). Faça as alterações necessárias nos arquivos.

IMPORTANTE: Altere APENAS arquivos versionados no git. NÃO toque em dist/, build/, node_modules/. NÃO execute git checkout ou git pull — o código já está atualizado.
IMPORTANTE: Se instalar dependências (npm install), execute também: git add package-lock.json

ANTES DE IMPLEMENTAR:
1. Leia PRODUCT.md na raiz do projeto (se existir) — contém modelo de dados e regras de negócio. SIGA os campos e convenções definidos lá.
2. Leia CLAUDE.md — contém convenções técnicas obrigatórias.
3. Leia o código existente para garantir compatibilidade:
${compatInstructions}
Se algo que você está criando consome ou depende de código anterior, VERIFIQUE que os tipos, campos e payloads estão alinhados.
${dbInstructions}${previousContext}${multiRepoContext}
Tarefa: ${task.command}
Detalhes: ${task.description || ''}
Plano aprovado: ${exec?.plan || ''}

Após concluir, NÃO faça build ou deploy. Apenas confirme o que foi alterado em português, máximo 5 linhas.`,
      srcDir,
    );

    // Commit changes
    const hasChanges = await this.git.hasChanges(srcDir);
    if (hasChanges) {
      await this.git.commitAll(srcDir, `${task.command} [${repoName}]\n\nTask #${task.id}`);

      // Migration fallback for backend
      if (isBackend) {
        try {
          if (await this.git.checkSchemaWithoutMigration(srcDir)) {
            this.logger.warn('Schema changed without migration, creating one...');
            await this.git.createAutoMigration(srcDir);
            this.logger.log('Migration created and amended to commit');
          }
        } catch (migErr: any) {
          this.logger.error(`Migration fallback failed: ${migErr.message}`);
        }
      }

      await this.git.pushBranch(srcDir, taskBranch);
      return { result, hasChanges: true };
    }
    return { result, hasChanges: false };
  }

  // ---------------------------------------------------------------------------
  // Execute Task
  // ---------------------------------------------------------------------------

  async executeTask(task: any, retryCount = 0): Promise<void> {
    this.logger.log(`Executing: ${task.command}`);
    await this.transition(task.id, 'executing', 'worker');

    try {
      const repos = await this.claude.resolveRepos(task);
      const exec = await this.getLatestExecution(task.id);
      const isSubtask = !!task.projectId;
      const taskBranch = `task/${task.id.substring(0, 8)}`;
      let baseBranch = 'main';

      if (isSubtask) {
        const project = await this.prisma.project.findUnique({ where: { id: task.projectId } });
        baseBranch = project?.branch || 'develop';
      }

      // Build context from previous subtasks
      let previousContext = '';
      if (isSubtask) {
        const completedSubtasks = await this.prisma.task.findMany({
          where: {
            projectId: task.projectId,
            status: 'completed',
            sortOrder: { lt: task.sortOrder },
          },
          orderBy: { sortOrder: 'asc' },
        });
        if (completedSubtasks.length > 0) {
          const summaries: string[] = [];
          for (const prev of completedSubtasks) {
            const prevExec = await this.prisma.taskExecution.findFirst({
              where: { taskId: prev.id },
              orderBy: { startedAt: 'desc' },
            });
            summaries.push(
              `Subtarefa ${prev.sortOrder}: ${prev.command}\nResultado: ${prevExec?.result || prevExec?.plan || 'concluída'}`,
            );
          }
          previousContext =
            `\n\nCONTEXTO — Subtarefas anteriores já concluídas:\n${summaries.join('\n\n')}` +
            `\n\nUse os nomes de arquivos, classes e funções criadas nas subtarefas anteriores. Eles já existem no código.\n`;
        }
      }

      const isMultiRepo = repos.length > 1;
      if (isMultiRepo) this.logger.log(`Multi-repo task: ${repos.join(' + ')}`);

      // Sort repos: backend first (REPO_PRIORITY)
      repos.sort((a, b) => (REPO_PRIORITY[a] ?? 9) - (REPO_PRIORITY[b] ?? 9));

      let anyChanges = false;
      let prevRepoResult = '';
      let lastSha = '';
      const allResults: string[] = [];

      for (const repoName of repos) {
        const srcDir = REPO_DIRS[repoName];
        if (!srcDir) continue;
        this.logger.log(`Executing in ${repoName} (${srcDir})`);

        // Create branch in this repo
        await this.git.createBranch(srcDir, taskBranch, baseBranch);

        const { result, hasChanges } = await this.executeInRepo(
          repoName,
          srcDir,
          taskBranch,
          baseBranch,
          task,
          exec,
          previousContext,
          prevRepoResult,
        );

        allResults.push(`[${repoName}] ${result}`);
        if (hasChanges) {
          anyChanges = true;
          prevRepoResult += `\nRepo ${repoName}:\n${result}\n`;

          // Merge task branch -> base branch in this repo (keep branch for promotion)
          const mergeTo = isSubtask ? baseBranch : 'develop';
          await this.git.mergeBranch(srcDir, taskBranch, mergeTo);
          // Do NOT delete branch — kept alive for promotion to homolog/prod

          if (isSubtask) {
            await this.git.mergeBranch(srcDir, baseBranch, 'develop');
          }

          lastSha = await this.git.getHeadSha(srcDir);
          this.logger.log(`${repoName} done (${lastSha.substring(0, 7)})`);
        } else {
          this.logger.log(`${repoName} (no changes)`);
        }
      }

      await this.prisma.task.update({ where: { id: task.id }, data: { branch: taskBranch } });
      const combinedResult = allResults.join('\n');
      const changedRepos = repos.filter((r, i) => allResults[i]?.includes('[' + r + ']'));

      if (anyChanges) {
        await this.prisma.task.update({ where: { id: task.id }, data: { environment: 'dev' } });
        await this.transition(task.id, 'deploying', 'worker');
        await this.saveExecution(task.id, 'execution', {
          result: combinedResult,
          commitSha: lastSha,
          finishedAt: new Date(),
        });

        // For multi-repo: poll until all deploys complete (max 5 min)
        if (isMultiRepo && changedRepos.length > 1) {
          this.logger.log(`Multi-repo (${changedRepos.join(' + ')}) -> deploying (DEV), polling for completion`);
          const multiTask = task;
          const multiRepos = changedRepos;
          let attempt = 0;
          const maxAttempts = 10; // 10 x 30s = 5 min
          const pollInterval = setInterval(async () => {
            attempt++;
            try {
              const t = await this.prisma.task.findUnique({ where: { id: multiTask.id } });
              if (!t || t.status === 'failed') {
                clearInterval(pollInterval);
                return;
              }
              const env = t.environment || 'dev';
              const smoke = await this.smokeTest(env);
              if (!smoke.passed) {
                if (attempt >= maxAttempts) {
                  clearInterval(pollInterval);
                  this.logger.error(`Multi-repo smoke failed after ${attempt} attempts`);
                  await this.notify(
                    `Deploy multi-repo com problema em ${env.toUpperCase()}!\n\nTarefa: ${multiTask.command}\nRepos: ${multiRepos.join(' + ')}\nFalhas: ${smoke.failed.map((f: any) => `${f.name} (${f.error || f.status})`).join(', ')}`,
                    multiTask,
                  );
                }
                return; // Keep polling
              }
              // All healthy!
              clearInterval(pollInterval);
              const urls = multiRepos.map((r) => ENV_URLS[env]?.[r]).filter(Boolean);
              let projectInfo = '';
              if (multiTask.projectId) {
                const proj = await this.prisma.project.findUnique({ where: { id: multiTask.projectId } });
                if (proj) projectInfo = `\nProjeto: ${proj.name} (${proj.doneSubtasks}/${proj.totalSubtasks} subtarefas)`;
              }
              await this.notify(
                `Deploy concluido em ${env.toUpperCase()}!\n\nTarefa: ${multiTask.command}\nRepos: ${multiRepos.join(' + ')}${projectInfo}\n\n${urls.map((u) => `Veja: ${u}`).join('\n')}\n\nQuando quiser promover, é só me avisar.`,
                multiTask,
              );
              this.logger.log(`Multi-repo deploy verified after ${attempt * 30}s for ${multiTask.id.substring(0, 8)}`);
            } catch (err: any) {
              this.logger.error(`Multi-repo poll error: ${err.message}`);
              if (attempt >= maxAttempts) clearInterval(pollInterval);
            }
          }, 30_000); // Check every 30s
        } else {
          if (!isSubtask) {
            const cleanResult = combinedResult.replace(/[`*#_~]/g, '').substring(0, 500);
            await this.notify(
              `Codigo alterado e enviado!\n\nTarefa: ${task.command}\n\nResultado:\n${cleanResult}\n\nDeploy em DEV em andamento...`,
              task,
            );
          }
          this.logger.log(`${isSubtask ? 'Subtask' : 'Pushed'} -> deploying (DEV)`);
        }
      } else {
        await this.transition(task.id, 'completed', 'worker');
        await this.saveExecution(task.id, 'execution', { result: combinedResult, finishedAt: new Date() });
        if (!isSubtask) {
          await this.notify(`Tarefa concluida (sem alteracoes necessarias).\n\nTarefa: ${task.command}`, task);
        }
        this.logger.log('Completed (no changes)');
      }
    } catch (err: any) {
      if (retryCount === 0 && this.isTransientError(err.message)) {
        this.logger.log('Transient error, retrying in 10s...');
        await new Promise((r) => setTimeout(r, 10_000));
        return this.executeTask(task, 1);
      }
      this.logger.error(`Execution error: ${err.message}`);
      await this.transition(task.id, 'failed', 'worker');
      await this.saveExecution(task.id, 'execution', { result: `Erro na execução: ${err.message}` });
      if (task.projectId) {
        await this.prisma.project.update({ where: { id: task.projectId }, data: { status: 'paused' } });
        await this.notify(
          `Subtarefa falhou. Projeto pausado.\n\nTarefa: ${task.command}\nErro: ${err.message.substring(0, 200)}\n\nResponda "retenta", "pula" ou "cancela o projeto".`,
          task,
        );
      } else {
        await this.notify(`Falha ao executar tarefa: ${task.command}\nErro: ${err.message.substring(0, 200)}`, task);
      }
    }
  }
}
