import { Injectable, Logger } from '@nestjs/common';
import { PrismaService } from './prisma.service';
import { TasksService } from './tasks.service';
import { ProjectsService } from './projects.service';
import { ClaudeService } from './claude.service';
import { ConversationPhase } from '@prisma/client';

const PHASE_ACTIONS: Record<string, string[]> = {
  idle: ['create_task', 'create_project', 'status', 'search_project', 'search_task', 'promote'],
  creating_task: ['status'],
  discussing_project: ['discuss', 'approve_project', 'cancel_project', 'status', 'search_project'],
  monitoring: ['status', 'cancel_task', 'cancel_project', 'pause', 'diagnose', 'search_project', 'search_task'],
  reviewing: ['approve_task', 'reject_task', 'promote', 'diagnose', 'status', 'complete_project', 'force_complete', 'add_subtask', 'resolve_task', 'cancel_task'],
  diagnosing: ['create_task', 'add_subtask', 'cancel_task', 'status'],
};

@Injectable()
export class PatriciaGatewayService {
  private readonly logger = new Logger(PatriciaGatewayService.name);

  constructor(
    private prisma: PrismaService,
    private tasks: TasksService,
    private projects: ProjectsService,
    private claude: ClaudeService,
  ) {}

  async getOrCreateSession(channel: string, target: string) {
    let session = await this.prisma.conversationSession.findUnique({
      where: { channel_target: { channel, target } },
    });
    if (!session) {
      session = await this.prisma.conversationSession.create({
        data: { channel, target },
      });
    }
    return session;
  }

  async refreshPhase(session: any) {
    // Lazy sync: check real state of active project/task and adjust phase
    if (session.activeProjectId) {
      const project = await this.prisma.project.findUnique({
        where: { id: session.activeProjectId },
        include: { subtasks: true },
      });
      if (!project || project.status === 'completed' || project.status === 'cancelled') {
        return this.updatePhase(session, 'idle', null, null);
      }
      if (project.status === 'awaiting_review') {
        const hasPending = project.subtasks.some(t =>
          ['awaiting_approval', 'needs_info'].includes(t.status),
        );
        if (hasPending) return this.updatePhase(session, 'reviewing', project.id, null);
        return this.updatePhase(session, 'discussing_project', project.id, null);
      }
      if (project.status === 'planning') {
        return this.updatePhase(session, 'discussing_project', project.id, null);
      }
      if (['approved', 'executing'].includes(project.status)) {
        const needsReview = project.subtasks.some(t =>
          ['awaiting_approval', 'needs_info'].includes(t.status),
        );
        if (needsReview) return this.updatePhase(session, 'reviewing', project.id, null);
        return this.updatePhase(session, 'monitoring', project.id, null);
      }
    }
    if (session.activeTaskId) {
      const task = await this.prisma.task.findUnique({ where: { id: session.activeTaskId } });
      if (!task || ['completed', 'cancelled', 'failed'].includes(task.status)) {
        return this.updatePhase(session, 'idle', null, null);
      }
      if (['awaiting_approval', 'needs_info'].includes(task.status)) {
        return this.updatePhase(session, 'reviewing', null, task.id);
      }
      return this.updatePhase(session, 'monitoring', null, task.id);
    }
    return session;
  }

  private async updatePhase(session: any, phase: ConversationPhase, projectId?: string | null, taskId?: string | null) {
    const data: any = { phase, lastActionAt: new Date() };
    if (projectId !== undefined) data.activeProjectId = projectId;
    if (taskId !== undefined) data.activeTaskId = taskId;
    return this.prisma.conversationSession.update({
      where: { id: session.id },
      data,
    });
  }

  getValidActions(phase: string): string[] {
    return PHASE_ACTIONS[phase] || PHASE_ACTIONS.idle;
  }

  private detectMultiRepo(command: string, description?: string): string[] | null {
    const text = `${command} ${description || ''}`.toLowerCase();
    const hasApi = /\b(api|backend|endpoint|controller|service|prisma|migration|dto)\b/.test(text);
    const hasFront = /\b(frontend|componente|tela|login|dashboard|css|tailwind|redux|react-router|formulário|formulario)\b/.test(text);
    const hasApp = /\b(app|patria-app|multi-tenant)\b/.test(text);
    const hasLandpage = /\b(landpage|landing)\b/.test(text);
    const repos: string[] = [];
    if (hasApi) repos.push('patria-api');
    if (hasApp || (hasFront && !hasLandpage)) repos.push('patria-app');
    if (hasLandpage) repos.push('landpage');
    return repos.length > 1 ? repos : null;
  }

  private async runGuards(action: string, session: any, params: any): Promise<{ allowed: boolean; reason?: string }> {
    // Guard: multi-repo detection for tasks and subtasks
    if (['create_task', 'add_subtask'].includes(action)) {
      const multiRepo = this.detectMultiRepo(params.command, params.description);
      if (multiRepo) {
        // Multi-repo is allowed for standalone tasks (worker handles it)
        // But subtasks should be single-repo
        if (action === 'add_subtask') {
          return { allowed: false, reason: `Subtarefa abrange múltiplos repos (${multiRepo.join(', ')}). Crie uma subtarefa separada para cada repo.` };
        }
        // For standalone tasks, multi-repo is fine (worker handles it)
      }
    }

    // Guard: add_subtask blocked during planning
    if (action === 'add_subtask') {
      const projectId = params.projectId || session.activeProjectId;
      if (projectId) {
        const project = await this.prisma.project.findUnique({ where: { id: projectId } });
        if (project && ['planning', 'awaiting_review'].includes(project.status)) {
          return { allowed: false, reason: `Não é possível adicionar subtarefas enquanto o projeto está em "${project.status}". Aguarde a decomposição ou aprove primeiro.` };
        }
        // Guard: diagnose-first when project has failed subtasks
        const failedCount = await this.prisma.task.count({
          where: { projectId, status: 'failed' },
        });
        if (failedCount > 0 && session.phase !== 'diagnosing') {
          return { allowed: false, reason: `Projeto tem ${failedCount} subtarefa(s) com falha. Use "diagnose" primeiro para analisar o problema antes de adicionar subtarefas.` };
        }
      }
    }

    // Guard: promote requires completed status
    if (action === 'promote') {
      if (params.taskId) {
        const task = await this.prisma.task.findUnique({ where: { id: params.taskId } });
        if (task && task.status !== 'completed') {
          return { allowed: false, reason: `Tarefa não está concluída (status: ${task.status}). Só é possível promover tarefas concluídas.` };
        }
      }
      if (params.projectId) {
        const project = await this.prisma.project.findUnique({ where: { id: params.projectId } });
        if (project && !['completed', 'awaiting_review'].includes(project.status)) {
          return { allowed: false, reason: `Projeto não está pronto para promoção (status: ${project.status}).` };
        }
      }
    }

    return { allowed: true };
  }

  buildSessionState(session: any) {
    return {
      phase: session.phase,
      activeProjectId: session.activeProjectId,
      activeTaskId: session.activeTaskId,
      validActions: this.getValidActions(session.phase),
    };
  }

  async logAction(sessionId: string, action: string, params: any, responseCode: number, blocked = false, blockReason?: string) {
    await this.prisma.conversationAction.create({
      data: {
        sessionId,
        action,
        requestBody: params,
        responseCode,
        blocked,
        blockReason,
      },
    });
  }

  async executeAction(action: string, channel: string, target: string, params: any = {}) {
    let session = await this.getOrCreateSession(channel, target);
    session = await this.refreshPhase(session);

    // Validate action is allowed in current phase
    const validActions = this.getValidActions(session.phase);
    if (!validActions.includes(action)) {
      const reason = `Ação "${action}" não permitida na fase "${session.phase}". Ações válidas: ${validActions.join(', ')}`;
      await this.logAction(session.id, action, params, 403, true, reason);
      return { allowed: false, error: reason, sessionState: this.buildSessionState(session) };
    }

    // Run action-specific guards
    const guard = await this.runGuards(action, session, params);
    if (!guard.allowed) {
      await this.logAction(session.id, action, params, 403, true, guard.reason);
      return { allowed: false, error: guard.reason, sessionState: this.buildSessionState(session) };
    }

    try {
      const result = await this.dispatch(action, session, params, channel, target);
      session = await this.advancePhase(session, action, result);
      await this.logAction(session.id, action, params, 200);

      return {
        allowed: true,
        result,
        sessionState: this.buildSessionState(session),
      };
    } catch (err) {
      await this.logAction(session.id, action, params, 400, false, err.message);
      return {
        allowed: false,
        error: err.message,
        sessionState: this.buildSessionState(session),
      };
    }
  }

  private async dispatch(action: string, session: any, params: any, channel: string, target: string) {
    switch (action) {
      case 'create_task':
        return this.tasks.create(
          params.command, params.description || '', 'patricia',
          channel, target, params.repo,
        );

      case 'create_project':
        return this.projects.create({
          name: params.name,
          description: params.description,
          channel,
          target,
        });

      case 'approve_task': {
        const taskId = params.taskId || session.activeTaskId;
        if (!taskId) throw new Error('taskId required');
        // Use task state machine via tasks service update
        return this.prisma.task.update({ where: { id: taskId }, data: { status: 'approved' } });
      }

      case 'reject_task': {
        const taskId = params.taskId || session.activeTaskId;
        if (!taskId) throw new Error('taskId required');
        return this.prisma.task.update({ where: { id: taskId }, data: { status: 'rejected' } });
      }

      case 'approve_project': {
        const projectId = params.projectId || session.activeProjectId;
        if (!projectId) throw new Error('projectId required');
        return this.projects.approve(projectId);
      }

      case 'cancel_project': {
        const projectId = params.projectId || session.activeProjectId;
        if (!projectId) throw new Error('projectId required');
        return this.projects.cancel(projectId);
      }

      case 'cancel_task': {
        const taskId = params.taskId || session.activeTaskId;
        if (!taskId) throw new Error('taskId required');
        return this.prisma.task.update({ where: { id: taskId }, data: { status: 'cancelled' } });
      }

      case 'add_subtask': {
        const projectId = params.projectId || session.activeProjectId;
        if (!projectId) throw new Error('projectId required');
        return this.projects.addSubtask(projectId, {
          command: params.command,
          description: params.description,
          repo: params.repo,
        });
      }

      case 'discuss': {
        const projectId = params.projectId || session.activeProjectId;
        if (!projectId) throw new Error('projectId required');
        return this.projects.addDiscussion(projectId, params.message);
      }

      case 'promote': {
        if (params.projectId || session.activeProjectId) {
          const projectId = params.projectId || session.activeProjectId;
          return this.projects.setPromoteTo(projectId, params.targetEnv);
        }
        if (params.taskId || session.activeTaskId) {
          const taskId = params.taskId || session.activeTaskId;
          return this.prisma.task.update({
            where: { id: taskId },
            data: { promoteTo: params.targetEnv },
          });
        }
        throw new Error('projectId or taskId required');
      }

      case 'complete_project': {
        const projectId = params.projectId || session.activeProjectId;
        if (!projectId) throw new Error('projectId required');
        return this.projects.transition(projectId, 'completed');
      }

      case 'force_complete': {
        const projectId = params.projectId || session.activeProjectId;
        if (!projectId) throw new Error('projectId required');
        return this.projects.forceComplete(projectId);
      }

      case 'pause': {
        const projectId = params.projectId || session.activeProjectId;
        if (!projectId) throw new Error('projectId required');
        return this.projects.pause(projectId);
      }

      case 'resume': {
        const projectId = params.projectId || session.activeProjectId;
        if (!projectId) throw new Error('projectId required');
        return this.projects.resume(projectId);
      }

      case 'resolve_task': {
        const taskId = params.taskId || session.activeTaskId;
        if (!taskId) throw new Error('taskId required');
        await this.prisma.taskTransition.create({
          data: { taskId, fromStatus: 'any', toStatus: 'completed', actor: 'director', metadata: { resolved: true } },
        });
        return this.prisma.task.update({ where: { id: taskId }, data: { status: 'completed' } });
      }

      case 'diagnose': {
        const projectId = params.projectId || session.activeProjectId;
        if (!projectId) throw new Error('projectId required');
        const project = await this.projects.findOne(projectId);
        const repo = params.repo || project?.subtasks?.[0]?.repo || 'patria-api';
        return { diagnosis: await this.claude.diagnose(params.question, repo, projectId) };
      }

      case 'status': {
        const status = await this.getFullStatus();
        const pendingActions = await this.getPendingActions();
        return { status, pendingActions };
      }

      case 'search_project': {
        return this.claude.searchProjects(params.q, params.limit);
      }

      case 'search_task': {
        return this.claude.searchTasks(params.q, params.limit);
      }

      default:
        throw new Error(`Unknown action: ${action}`);
    }
  }

  private async advancePhase(session: any, action: string, result: any) {
    let newPhase = session.phase;
    let projectId = session.activeProjectId;
    let taskId = session.activeTaskId;

    switch (action) {
      case 'create_task':
        newPhase = 'monitoring';
        taskId = result?.id || null;
        break;
      case 'create_project':
        newPhase = 'discussing_project';
        projectId = result?.id || null;
        break;
      case 'approve_project':
        newPhase = 'monitoring';
        break;
      case 'complete_project':
      case 'force_complete':
      case 'cancel_project':
        newPhase = 'idle';
        projectId = null;
        break;
      case 'cancel_task':
        if (!session.activeProjectId) { newPhase = 'idle'; taskId = null; }
        break;
      case 'diagnose':
        newPhase = 'diagnosing';
        break;
      case 'search_project':
        // If results found, set active project to first result
        if (Array.isArray(result) && result.length > 0 && result[0].id) {
          projectId = result[0].id;
        }
        break;
      case 'search_task':
        if (Array.isArray(result) && result.length > 0 && result[0].id) {
          taskId = result[0].id;
        }
        break;
    }

    return this.updatePhase(session, newPhase as ConversationPhase, projectId, taskId);
  }

  private async getFullStatus() {
    const now = new Date();
    const oneDayAgo = new Date(now.getTime() - 86400000);

    const [completedLast24h, failedLast24h, pendingTasks, deployingTasks, activeProjects, deployingDetails] = await Promise.all([
      this.prisma.task.count({ where: { status: 'completed', updatedAt: { gte: oneDayAgo } } }),
      this.prisma.task.count({ where: { status: 'failed', updatedAt: { gte: oneDayAgo } } }),
      this.prisma.task.count({ where: { status: 'pending' } }),
      this.prisma.task.count({ where: { status: 'deploying' } }),
      this.prisma.project.findMany({
        where: { status: { notIn: ['completed', 'cancelled'] } },
        select: { id: true, name: true, status: true, doneSubtasks: true, totalSubtasks: true, environment: true },
      }),
      this.prisma.task.findMany({
        where: { status: 'deploying' },
        select: { id: true, command: true, environment: true },
      }),
    ]);

    // Explicit warnings for deploying tasks
    const warnings: string[] = [];
    if (deployingDetails.length > 0) {
      warnings.push(`ATENÇÃO: ${deployingDetails.length} tarefa(s) EM DEPLOY. NÃO diga que foram concluídas.`);
      for (const t of deployingDetails) {
        warnings.push(`- "${t.command}" em ${t.environment.toUpperCase()} (EM ANDAMENTO)`);
      }
    }

    return { completedLast24h, failedLast24h, pendingTasks, deployingTasks, activeProjects, warnings };
  }

  private async getPendingActions() {
    const actions: any[] = [];

    const awaitingReview = await this.prisma.project.findMany({
      where: { status: 'awaiting_review' },
      select: { id: true, name: true, environment: true },
    });
    for (const p of awaitingReview) {
      actions.push({ type: 'project', action: 'approve_or_promote', id: p.id, name: p.name, environment: p.environment });
    }

    const awaitingApproval = await this.prisma.task.findMany({
      where: { status: 'awaiting_approval', projectId: null },
      select: { id: true, command: true },
    });
    for (const t of awaitingApproval) {
      actions.push({ type: 'task', action: 'approve', id: t.id, name: t.command });
    }

    const promotable = await this.prisma.task.findMany({
      where: { status: 'completed', projectId: null, promoteTo: null, environment: { in: ['dev', 'homolog'] } },
      select: { id: true, command: true, environment: true },
    });
    for (const t of promotable) {
      actions.push({ type: 'task', action: 'promote', id: t.id, name: t.command, environment: t.environment });
    }

    return actions;
  }

  async getContext(channel: string, target: string): Promise<string> {
    let session = await this.getOrCreateSession(channel, target);
    session = await this.refreshPhase(session);
    const validActions = this.getValidActions(session.phase);

    let context = `## Estado atual\nFase: ${session.phase}\nAções válidas: ${validActions.join(', ')}\n`;

    if (session.activeProjectId) {
      const project = await this.prisma.project.findUnique({
        where: { id: session.activeProjectId },
        include: { subtasks: true },
      });
      if (project) {
        context += `\nProjeto ativo: "${project.name}" (${project.status}, ${project.environment})\n`;
        context += `Subtarefas: ${project.subtasks.filter(t => t.status === 'completed').length}/${project.subtasks.length}\n`;
        const deploying = project.subtasks.filter(t => t.status === 'deploying');
        if (deploying.length) {
          context += `EM DEPLOY (NÃO DIGA CONCLUÍDO): ${deploying.map(t => t.command).join(', ')}\n`;
        }
      }
    }

    context += `\n## Regras\n1. Use APENAS as ações listadas acima.\n2. NUNCA diga "deploy concluido" — só o sistema notifica.\n3. Respostas curtas em português.\n`;

    return context;
  }
}
