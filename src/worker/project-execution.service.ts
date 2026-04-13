import { Injectable, Logger, Inject, forwardRef } from '@nestjs/common';
import { PrismaService } from '../prisma.service';
import { GitService } from './git.service';
import { ClaudeCliService } from './claude-cli.service';
import { TaskExecutionService } from './task-execution.service';
import { DeployMonitorService } from './deploy-monitor.service';
import { REPO_DIRS, REPO_PRIORITY } from './worker.config';

@Injectable()
export class ProjectExecutionService {
  private readonly logger = new Logger('ProjectExecution');

  constructor(
    private prisma: PrismaService,
    private git: GitService,
    private claude: ClaudeCliService,
    @Inject(forwardRef(() => TaskExecutionService))
    private taskExecution: TaskExecutionService,
    private deployMonitor: DeployMonitorService,
  ) {}

  // ── Decompose Project ──────────────────────────────────────────────

  async decomposeProject(project: any): Promise<void> {
    this.logger.log(`Decomposing project: ${project.name}`);

    try {
      const defaultRepo = 'patria-api';
      const srcDir = REPO_DIRS[defaultRepo];
      const projectBranch = `project/${project.id.substring(0, 8)}`;

      const response = this.claude.runClaude(
        `Analise este projeto e decomponha em subtarefas executáveis.

ANTES DE DECOMPOR:
1. Leia PRODUCT.md na raiz — contém o modelo de dados, regras de negócio e convenções do produto.
2. Leia CLAUDE.md — contém convenções técnicas obrigatórias (Swagger, DTOs, validação).
3. Leia prisma/schema.prisma — entenda o que já existe no banco.

Projeto: ${project.name}
Descrição: ${project.description || ''}

REGRAS DE DECOMPOSIÇÃO:
- Cada subtarefa DEVE ser PEQUENA: máximo 3 arquivos criados/alterados por subtarefa.
- Se uma feature precisa de migration + service + controller + dto, quebre em 2 subtarefas: (1) migration + model, (2) service + controller + dto.
- Prefira 5-8 subtarefas pequenas em vez de 2-4 grandes. Subtarefas grandes falham.
- Cada subtarefa deve resultar em UM commit funcional.
- Cada subtarefa deve ser autocontida — funcionar sozinha após executada.
- Backend (patria-api) SEMPRE antes de frontend (patria-app/landpage).
- Cada subtarefa DEVE especificar o repo correto:
  - "patria-api" para backend (NestJS, endpoints, banco, Prisma)
  - "patria-app" para app multi-tenant (React, dashboard, formulários)
  - "landpage" para landing page institucional

FORMATO: Retorne EXATAMENTE um JSON array, sem texto antes ou depois.
Cada item: {"command": "título curto", "description": "resumo do que fazer (1-2 frases)", "project": "landpage|patria-api|patria-app", "sortOrder": N, "dependsOnPrevious": true/false}

Responda APENAS o JSON array.`,
        srcDir,
      );

      // Parse subtasks from Claude response
      let subtasks: any[];
      try {
        const jsonMatch = response.match(/\[[\s\S]*\]/);
        if (!jsonMatch) throw new Error('No JSON array found');
        subtasks = JSON.parse(jsonMatch[0]);
      } catch (parseErr) {
        this.logger.error(`Failed to parse subtasks: ${parseErr.message}`);
        await this.prisma.projectPlanning.create({
          data: { projectId: project.id, role: 'claude', content: `Erro ao decompor: ${response.substring(0, 500)}` },
        });
        return;
      }

      this.logger.log(`Found ${subtasks.length} subtasks`);

      // Create project branch in each repo that has subtasks
      const repos = [...new Set(subtasks.map((s) => s.project).filter(Boolean))] as string[];
      for (const repo of repos) {
        const repoDir = REPO_DIRS[repo];
        if (repoDir) {
          try {
            this.git.createBranch(repoDir, projectBranch, 'main');
            this.logger.log(`Project branch created in ${repo}`);
          } catch {
            this.logger.warn(`Branch already exists in ${repo}`);
          }
        }
      }

      // Save project branch
      await this.prisma.project.update({ where: { id: project.id }, data: { branch: projectBranch } });

      // Create subtask records with dependsOnId chaining
      let prevTaskId: string | null = null;
      for (const sub of subtasks) {
        const task = await this.prisma.task.create({
          data: {
            command: sub.command,
            description: sub.description,
            repo: sub.project || defaultRepo,
            projectId: project.id,
            sortOrder: sub.sortOrder || 0,
            dependsOnId: prevTaskId,
            createdBy: 'worker',
            channel: project.channel,
            target: project.target,
          },
        });
        prevTaskId = task.id;
      }

      // Save planning message
      const summary = subtasks.map((s, i) => `${i + 1}. ${s.command}`).join('\n');
      await this.prisma.projectPlanning.create({
        data: {
          projectId: project.id,
          role: 'claude',
          content: `Projeto decomposto em ${subtasks.length} subtarefas:\n\n${summary}`,
        },
      });

      // Update project to awaiting_review
      await this.prisma.project.update({
        where: { id: project.id },
        data: { status: 'awaiting_review', totalSubtasks: subtasks.length },
      });

      // Notify
      const cleanSummary = summary.substring(0, 800);
      await this.taskExecution.notify(
        `Projeto decomposto em ${subtasks.length} subtarefas:\n\n${project.name}\n\n${cleanSummary}\n\nResponda "aprova o projeto" ou discuta ajustes.`,
        project,
      );

      // Save memory about new project
      await this.taskExecution.saveMemory(
        'project',
        `Projeto: ${project.name}`,
        `Novo projeto com ${subtasks.length} subtarefas. Status: aguardando aprovação.`,
        'short_term',
      );

      this.logger.log(`Project decomposed -> awaiting_review`);
    } catch (err) {
      this.logger.error(`Decomposition error: ${err.message}`);
      await this.prisma.projectPlanning.create({
        data: { projectId: project.id, role: 'claude', content: `Erro: ${err.message}` },
      });
    }
  }

  // ── Process Project Subtask ────────────────────────────────────────

  async processProjectSubtask(project: any): Promise<void> {
    // Find next pending/approved/replanning subtask, sorted by: backend first, then sortOrder
    const candidates = await this.prisma.task.findMany({
      where: {
        projectId: project.id,
        status: { in: ['pending', 'approved', 'replanning'] },
      },
      orderBy: { sortOrder: 'asc' },
    });

    // Sort: backend (patria-api) first, then app, then landpage, then by sortOrder
    candidates.sort((a, b) => {
      const pa = REPO_PRIORITY[a.repo] ?? 9;
      const pb = REPO_PRIORITY[b.repo] ?? 9;
      return pa !== pb ? pa - pb : a.sortOrder - b.sortOrder;
    });

    const nextTask = candidates[0] || null;

    if (!nextTask) {
      // Check if all subtasks done
      const remaining = await this.prisma.task.count({
        where: { projectId: project.id, status: { notIn: ['completed', 'cancelled'] } },
      });
      if (remaining === 0 && project.status === 'executing') {
        await this.prisma.project.update({
          where: { id: project.id },
          data: { status: 'awaiting_review', doneSubtasks: project.totalSubtasks, environment: 'dev' },
        });
        this.logger.log(`Project all subtasks done: ${project.name}`);
        await this.taskExecution.saveMemory(
          'project',
          `Projeto: ${project.name}`,
          `Todas ${project.totalSubtasks} subtarefas concluídas. Deployado em DEV. Aguardando revisão do diretor.`,
          'short_term',
        );
      }
      return;
    }

    // Check dependency (dependsOnId must be completed)
    if (nextTask.dependsOnId) {
      const dep = await this.prisma.task.findUnique({ where: { id: nextTask.dependsOnId } });
      if (dep && dep.status !== 'completed') {
        return;
      }
    }

    // Update project status to executing if was approved
    if (project.status === 'approved') {
      await this.prisma.project.update({
        where: { id: project.id },
        data: { status: 'executing' },
      });
    }

    // --- Handle replanning ---
    if (nextTask.status === 'replanning') {
      const isDeployFailure = await this.deployMonitor.isDeployFailureReplan(nextTask.id);
      if (isDeployFailure) {
        await this.deployMonitor.createFixTask(nextTask);
        return;
      }

      await this.taskExecution.planTask(nextTask, true);

      if (await this.isProjectPaused(project.id)) return;

      // Auto-approve if worker-created and autoApprove enabled
      if (project.autoApprove && nextTask.createdBy !== 'director') {
        const refreshed = await this.prisma.task.findUnique({ where: { id: nextTask.id } });
        if (refreshed && refreshed.status === 'awaiting_approval') {
          await this.taskExecution.transition(nextTask.id, 'approved', 'worker');
        }
      }
    }

    // --- Handle pending ---
    if (nextTask.status === 'pending') {
      await this.taskExecution.planTask(nextTask);

      // Check if paused between plan and approve
      if (await this.isProjectPaused(project.id)) {
        this.logger.log(`Project paused after planning: ${project.name}`);
        await this.taskExecution.notify(
          `Projeto pausado.\n\nSubtarefa "${nextTask.command}" foi planejada e ficará aguardando.\nQuando quiser retomar, é só me avisar.`,
          project,
        );
        return;
      }

      // Auto-approve only subtasks from original decomposition (createdBy: worker)
      // Subtasks added later by director need explicit approval
      if (project.autoApprove && nextTask.createdBy !== 'director') {
        const refreshed = await this.prisma.task.findUnique({ where: { id: nextTask.id } });
        if (refreshed && refreshed.status === 'awaiting_approval') {
          await this.taskExecution.transition(nextTask.id, 'approved', 'worker');
        }
      }

      // Check again before execution
      if (await this.isProjectPaused(project.id)) {
        this.logger.log(`Project paused before execution: ${project.name}`);
        await this.taskExecution.notify(
          `Projeto pausado.\n\nSubtarefa "${nextTask.command}" está aprovada e pronta para executar.\nQuando quiser retomar, é só me avisar.`,
          project,
        );
        return;
      }
    }

    // --- Handle approved ---
    const currentStatus = nextTask.status === 'approved'
      ? 'approved'
      : (await this.prisma.task.findUnique({ where: { id: nextTask.id } }))?.status;

    if (currentStatus === 'approved') {
      // Final pause check before execution
      if (await this.isProjectPaused(project.id)) {
        this.logger.log(`Project paused before execution: ${project.name}`);
        return;
      }

      await this.taskExecution.executeTask(nextTask);

      // Update progress after execution
      const done = await this.prisma.task.count({
        where: { projectId: project.id, status: 'completed' },
      });
      const total = project.totalSubtasks;
      await this.prisma.project.update({
        where: { id: project.id },
        data: { doneSubtasks: done },
      });

      // Check if paused before starting next subtask
      if (await this.isProjectPaused(project.id)) {
        this.logger.log(`Project paused after subtask ${done}/${total}: ${project.name}`);
        await this.taskExecution.notify(
          `Projeto pausado após subtarefa ${done}/${total}.\n\nÚltima concluída: "${nextTask.command}"\nQuando quiser retomar, é só me avisar.`,
          project,
        );
        return;
      }

      // Notify only at milestones: halfway
      const halfway = Math.ceil(total / 2);
      if (done === halfway) {
        await this.taskExecution.notify(
          `Progresso: ${done}/${total} subtarefas concluidas.\n\nProjeto: ${project.name}`,
          project,
        );
      }
    }
  }

  // ── Handle Project Discussion ──────────────────────────────────────

  async handleProjectDiscussion(project: any): Promise<void> {
    // Get all planning messages
    const plannings = await this.prisma.projectPlanning.findMany({
      where: { projectId: project.id },
      orderBy: { createdAt: 'asc' },
    });

    if (plannings.length === 0) return;

    // Check if last message is from director (needs Claude response)
    const last = plannings[plannings.length - 1];
    if (last.role !== 'director') return;

    this.logger.log(`Discussing project: ${project.name}`);
    this.logger.log(`Director said: ${last.content.substring(0, 80)}...`);

    try {
      // Determine repo from existing subtasks
      const subtasks = await this.prisma.task.findMany({
        where: { projectId: project.id },
        orderBy: { sortOrder: 'asc' },
      });

      const repoProject = subtasks[0]?.repo || 'landpage';
      const srcDir = REPO_DIRS[repoProject] || REPO_DIRS.landpage;

      this.git.gitSync(srcDir);

      // Build conversation history
      const history = plannings.map((p) => `[${p.role.toUpperCase()}]: ${p.content}`).join('\n\n');
      const subtaskList = subtasks
        .map((t, i) => `${i + 1}. [${t.status}] ${t.command}: ${t.description || ''}`)
        .join('\n');

      const response = this.claude.runClaude(
        `Você está ajudando a planejar um projeto. O diretor fez uma pergunta ou pediu ajuste.

Projeto: ${project.name}
Descrição: ${project.description || ''}

Subtarefas atuais:
${subtaskList || 'Nenhuma ainda'}

Histórico da conversa:
${history}

Responda a última mensagem do diretor. Se ele pediu ajustes nas subtarefas, descreva as mudanças sugeridas.

Se precisar RECRIAR as subtarefas (adicionar, remover, reordenar), retorne um bloco JSON com o novo array:
\`\`\`json
[{"command": "...", "description": "...", "project": "${repoProject}", "sortOrder": N, "dependsOnPrevious": true/false}]
\`\`\`

Se NÃO precisar mudar as subtarefas, apenas responda a pergunta normalmente sem JSON.

Responda em português, de forma concisa.`,
        srcDir,
      );

      // Save Claude's response
      await this.prisma.projectPlanning.create({
        data: { projectId: project.id, role: 'claude', content: response },
      });

      // Check if Claude returned new subtasks (redecomposition)
      const jsonMatch = response.match(/```json\s*(\[[\s\S]*?\])\s*```/);
      if (jsonMatch) {
        try {
          const newSubtasks = JSON.parse(jsonMatch[1]);

          // Delete old subtasks that haven't completed
          await this.prisma.task.deleteMany({
            where: { projectId: project.id, status: { in: ['pending', 'failed', 'cancelled'] } },
          });

          // Create new subtasks — chain to last existing subtask
          const lastExisting = await this.prisma.task.findFirst({
            where: { projectId: project.id, status: 'completed' },
            orderBy: { sortOrder: 'desc' },
          });
          let prevTaskId = lastExisting?.id || null;
          for (const sub of newSubtasks) {
            const task = await this.prisma.task.create({
              data: {
                command: sub.command,
                description: sub.description,
                repo: sub.project || repoProject,
                projectId: project.id,
                sortOrder: sub.sortOrder || 0,
                dependsOnId: prevTaskId,
                createdBy: 'worker',
                channel: project.channel,
                target: project.target,
              },
            });
            prevTaskId = task.id;
          }

          await this.prisma.project.update({
            where: { id: project.id },
            data: { totalSubtasks: newSubtasks.length },
          });

          this.logger.log(`Subtasks updated: ${newSubtasks.length}`);
        } catch {
          // JSON parse failed silently — response was conversational
        }
      }

      // Notify director
      const cleanResponse = response
        .replace(/```json[\s\S]*?```/g, '')
        .replace(/[`*#_~]/g, '')
        .substring(0, 800);
      await this.taskExecution.notify(
        `Resposta sobre o projeto: ${project.name}\n\n${cleanResponse}\n\nResponda "aprova o projeto" ou continue discutindo.`,
        project,
      );
      this.logger.log(`Discussion response sent`);
    } catch (err) {
      this.logger.error(`Discussion error: ${err.message}`);
    }
  }

  // ── Is Project Paused ──────────────────────────────────────────────

  async isProjectPaused(projectId: string): Promise<boolean> {
    const p = await this.prisma.project.findUnique({ where: { id: projectId } });
    return p?.status === 'paused';
  }
}
