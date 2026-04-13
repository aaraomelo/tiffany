import { Injectable, Logger } from '@nestjs/common';
import { PrismaService } from './prisma.service';
import { TasksService } from './tasks.service';
import { ProjectsService } from './projects.service';
import { ClaudeService } from './claude.service';
import { PeopleService } from './people/people.service';
import { ProfileService } from './profile.service';
import { ConversationPhase } from '@prisma/client';

// Actions available in ALL phases
const COMMON_ACTIONS = [
  'status', 'search_project', 'search_task',
  'approve_task', 'reject_task', 'approve_project',
  'cancel_task', 'cancel_project',
  'promote', 'resolve_task',
  'create_task', 'create_project',
  'diagnose', 'followup',
  'task_detail', 'project_detail',
  'complete_project', 'force_complete',
  'consult', 'ask_specialist',
  'update_subtask', 'discuss',
  'resume', 'pause',
  'save_memory', 'forget_memory',
  'open_specialist',
  'add_contact', 'send_message', 'check_contact', 'update_contact', 'check_sent', 'send_recado', 'retry_task',
  'toggle_privacy', 'set_password', 'switch_model', 'list_models', 'manage_models',
  'simulate_person', 'preview_message',
];

// Phase-specific EXTRA actions (on top of common)
const PHASE_EXTRA: Record<string, string[]> = {
  idle: [],
  creating_task: [],
  discussing_project: ['discuss'],
  monitoring: ['pause', 'diagnose'],
  reviewing: ['complete_project', 'force_complete', 'add_subtask', 'diagnose'],
  diagnosing: ['add_subtask'],
};

const PHASE_ACTIONS: Record<string, string[]> = Object.fromEntries(
  Object.entries(PHASE_EXTRA).map(([phase, extra]) => [phase, [...COMMON_ACTIONS, ...extra]])
);

// Phase-specific instructions — loaded dynamically into prompt per conversation state
const PHASE_INSTRUCTIONS: Record<string, string> = {
  idle: `
## Instruções
- Respostas curtas em português.
- Use as ações listadas acima quando o contexto pedir.
- Para criar tarefa: precisa de um título claro.
- Para criar projeto: precisa de nome e descrição.
- Saudações e conversa casual: responda naturalmente sem chamar tools.`,

  creating_task: `
## Instruções (criando tarefa)
- Uma tarefa foi criada e está sendo processada.
- Aguarde o técnico planejar. NÃO diga que está pronto até receber confirmação.
- Se perguntarem status, use a tool status.`,

  discussing_project: `
## Instruções (planejando projeto)
- Projeto em planejamento. O diretor pode discutir ajustes.
- Use "discuss" para adicionar mensagens ao planejamento.
- Quando o diretor aprovar, use "approve_project".
- Mostre as subtarefas quando pedirem detalhes.`,

  monitoring: `
## Instruções (monitorando)
- Projeto ou tarefa em execução. O técnico está trabalhando.
- NUNCA diga "deploy concluído" — só o sistema notifica via webhook.
- Se perguntarem status, use a tool status ou project_detail.
- O diretor pode pausar, promover, ou pedir diagnóstico.`,

  reviewing: `
## Instruções (revisando)
- Projeto/tarefa aguardando revisão do diretor.
- Mostre detalhes quando pedirem. Use task_detail ou project_detail.
- Diretor pode aprovar, rejeitar com feedback, adicionar subtarefas, ou pedir diagnóstico.
- Se rejeitar com feedback, o técnico vai replanejar.`,

  diagnosing: `
## Instruções (diagnosticando)
- Diagnóstico técnico em andamento.
- Aguarde resultado. Se já chegou, apresente de forma clara.
- O diretor pode adicionar subtarefas baseado no diagnóstico.`,
};

@Injectable()
export class PatriciaGatewayService {
  private readonly logger = new Logger(PatriciaGatewayService.name);

  constructor(
    private prisma: PrismaService,
    private tasks: TasksService,
    private projects: ProjectsService,
    private claude: ClaudeService,
    private peopleService: PeopleService,
    private profileService: ProfileService,
  ) {}

  private isDirector(profile: any): boolean { return this.profileService.isDirector(profile); }
  private isDirectorSlug(slug: string): boolean { return this.profileService.isDirectorSlug(slug); }

  // Injected lazily to avoid circular dependency
  private _memory: any;
  private getMemory() {
    if (!this._memory) {
      // Dynamic import to avoid circular deps
      const { MemoryService } = require('./messaging/memory.service');
      this._memory = new MemoryService(this.prisma);
    }
    return this._memory;
  }

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
    const hasApi = /\b(patria-api|backend|endpoint|controller|service|prisma|migration|dto)\b/.test(text)
      || /(?:^|\s)api(?:\s|$)/.test(text);
    const hasFront = /\b(frontend|componente|tela de|css|tailwind|redux|react-router|formulário|formulario|patria-app)\b/.test(text);
    const hasApp = /\b(patria-app|multi-tenant)\b/.test(text)
      || /(?:^|\s)app(?:\s|$)/.test(text);
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

    // Guard: don't approve project right after discuss — ask director first
    if (action === 'approve_project') {
      const lastAction = await this.prisma.conversationAction.findFirst({
        where: { sessionId: session.id, action: 'discuss' },
        orderBy: { createdAt: 'desc' },
      });
      if (lastAction) {
        const timeSinceDiscuss = Date.now() - new Date(lastAction.createdAt).getTime();
        // If discussed less than 2 minutes ago, block auto-approve
        if (timeSinceDiscuss < 120_000) {
          // Check if director explicitly said "aprova" (not Patricia auto-approving)
          const lastApproveAttempt = await this.prisma.conversationAction.findFirst({
            where: { sessionId: session.id, action: 'approve_project', blocked: true },
            orderBy: { createdAt: 'desc' },
          });
          const alreadyBlocked = lastApproveAttempt && (Date.now() - new Date(lastApproveAttempt.createdAt).getTime()) < 120_000;
          if (!alreadyBlocked) {
            return { allowed: false, reason: 'Há uma discussão recente no projeto. Pergunte ao diretor se o plano está ok antes de aprovar.' };
          }
        }
      }
    }

    return { allowed: true };
  }

  async resolveTaskId(params: any, session: any): Promise<string> {
    // 1. Direct ID
    if (params.taskId) {
      const task = await this.prisma.task.findUnique({ where: { id: params.taskId } });
      if (task) return task.id;
    }

    // 2. By index (1-based) in active project
    const projectId = params.projectId || session.activeProjectId;
    if (params.index && projectId) {
      const subtasks = await this.prisma.task.findMany({ where: { projectId }, orderBy: { sortOrder: 'asc' } });
      const target = subtasks[params.index - 1];
      if (target) return target.id;
    }

    // 3. By search text in active project
    if (params.search && projectId) {
      const subtasks = await this.prisma.task.findMany({ where: { projectId }, orderBy: { sortOrder: 'asc' } });
      const match = subtasks.find(t => t.command.toLowerCase().includes(params.search.toLowerCase()));
      if (match) return match.id;
    }

    // 4. Session active task
    if (session.activeTaskId) return session.activeTaskId;

    throw new Error('Tarefa não encontrada. Use taskId, index (número da subtarefa) ou search (texto).');
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
    // Normalize channel/target — Patrícia sometimes swaps them
    const validChannels = ['whatsapp', 'telegram'];
    if (!validChannels.includes(channel)) {
      // channel looks like a phone number, swap
      const tmp = channel;
      channel = 'whatsapp';
      target = tmp;
    }

    // Auto-detect target: use most recently active session
    if (!target || target === 'undefined' || target === 'auto') {
      const recent = await this.prisma.conversationSession.findFirst({
        where: { channel },
        orderBy: { lastActionAt: 'desc' },
      });
      target = recent?.target || '+5511977808883';
      console.log(`[gateway] Auto-detected target: ${target}`);
    }

    let session = await this.getOrCreateSession(channel, target);
    session = await this.refreshPhase(session);

    // Validate action is allowed by caller's PROFILE
    const callerContactGw = await this.prisma.messagingContact.findFirst({
      where: { channelType: channel as any, remoteId: target },
      include: { person: { include: { profile: true } } },
    }).catch(() => null);
    const callerProfile = (callerContactGw as any)?.person?.profile;
    if (callerProfile) {
      const profileTools: string[] = callerProfile.allowedTools || [];
      if (profileTools.length > 0 && !profileTools.includes(action)) {
        const reason = `Ação "${action}" não disponível no perfil "${callerProfile.name}".`;
        this.logger.warn(`[gateway] Blocked ${action} for profile ${callerProfile.slug} (${target})`);
        await this.logAction(session.id, action, params, 403, true, reason);
        return { allowed: false, error: reason };
      }
    }

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
      // Enrich log with resolved values (e.g. detected repo for diagnose)
      const logParams = { ...params };
      if (action === 'diagnose' && (result as any)?.repo) logParams.repo = (result as any).repo;
      await this.logAction(session.id, action, logParams, 200);

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
      case 'create_task': {
        const result = await this.tasks.create(
          params.command, params.description || '', 'patricia',
          channel, target, params.repo,
        );
        return { _summary: `Tarefa criada: ${params.command}`, ...result };
      }

      case 'create_project': {
        const result = await this.projects.create({
          name: params.name,
          description: params.description,
          channel,
          target,
        });
        return { _summary: `Projeto criado: ${params.name}`, ...result };
      }

      case 'approve_task': {
        const taskId = params.taskId || session.activeTaskId;
        if (!taskId) throw new Error('taskId required');
        await this.prisma.taskTransition.create({
          data: { taskId, fromStatus: 'awaiting_approval', toStatus: 'approved', actor: 'director' },
        });
        const result = await this.prisma.task.update({ where: { id: taskId }, data: { status: 'approved' } });
        return { _summary: 'Tarefa aprovada', ...result };
      }

      case 'reject_task': {
        const taskId = params.taskId || session.activeTaskId;
        if (!taskId) throw new Error('taskId required');
        const taskToReject = await this.prisma.task.findUnique({ where: { id: taskId } });
        if (!taskToReject) throw new Error('Task not found');
        // With feedback → replanning, without → rejected
        const targetStatus = params.feedback ? 'replanning' : 'rejected';
        await this.prisma.taskTransition.create({
          data: { taskId, fromStatus: taskToReject.status, toStatus: targetStatus, actor: 'director', metadata: params.feedback ? { feedback: params.feedback } : undefined },
        });
        if (params.feedback) {
          await this.prisma.taskExecution.updateMany({
            where: { taskId, finishedAt: null },
            data: { feedback: params.feedback },
          });
          // Also save feedback on the latest execution that has a plan
          const latestExec = await this.prisma.taskExecution.findFirst({
            where: { taskId },
            orderBy: { startedAt: 'desc' },
          });
          if (latestExec) {
            await this.prisma.taskExecution.update({
              where: { id: latestExec.id },
              data: { feedback: params.feedback },
            });
          }
        }
        const result = await this.prisma.task.update({ where: { id: taskId }, data: { status: targetStatus } });
        return { _summary: params.feedback ? 'Tarefa rejeitada com feedback' : 'Tarefa rejeitada', ...result };
      }

      case 'approve_project': {
        const projectId = params.projectId || session.activeProjectId;
        if (!projectId) throw new Error('projectId required');
        const result = await this.projects.approve(projectId);
        return { _summary: 'Projeto aprovado e em execução', ...result };
      }

      case 'cancel_project': {
        const projectId = params.projectId || session.activeProjectId;
        if (!projectId) throw new Error('projectId required');
        const result = await this.projects.cancel(projectId);
        return { _summary: 'Projeto cancelado', ...result };
      }

      case 'cancel_task': {
        const taskId = params.taskId || session.activeTaskId;
        if (!taskId) throw new Error('taskId required');
        const result = await this.prisma.task.update({ where: { id: taskId }, data: { status: 'cancelled' } });
        return { _summary: 'Tarefa cancelada', ...result };
      }

      case 'add_subtask': {
        const projectId = params.projectId || session.activeProjectId;
        if (!projectId) throw new Error('projectId required');
        const result = await this.projects.addSubtask(projectId, {
          command: params.command,
          description: params.description,
          repo: params.repo,
        });
        return { _summary: `Subtarefa adicionada: ${params.command}`, ...result };
      }

      case 'discuss': {
        const projectId = params.projectId || session.activeProjectId;
        if (!projectId) throw new Error('projectId required');
        const result = await this.projects.addDiscussion(projectId, params.message);
        return { _summary: 'Mensagem adicionada ao planejamento', ...result };
      }

      case 'promote': {
        const promoteProjectId = params.projectId || session.activeProjectId;
        const promoteTaskId = params.taskId || session.activeTaskId;

        if (promoteProjectId) {
          // Auto-complete project if approved and all subtasks done
          const proj = await this.prisma.project.findUnique({ where: { id: promoteProjectId } });
          if (proj && (proj.status === 'approved' || proj.status === 'awaiting_review')) {
            const remaining = await this.prisma.task.count({
              where: { projectId: promoteProjectId, status: { notIn: ['completed', 'cancelled'] } },
            });
            if (remaining === 0) {
              await this.prisma.project.update({
                where: { id: promoteProjectId },
                data: { status: 'completed', doneSubtasks: proj.totalSubtasks },
              });
            }
          }
          await this.projects.setPromoteTo(promoteProjectId, params.targetEnv);
          return { _summary: `Promoção iniciada para ${params.targetEnv}`, promoted: true, projectId: promoteProjectId, targetEnv: params.targetEnv };
        }
        if (promoteTaskId) {
          await this.prisma.task.update({
            where: { id: promoteTaskId },
            data: { promoteTo: params.targetEnv },
          });
          return { _summary: `Promoção iniciada para ${params.targetEnv}`, promoted: true, taskId: promoteTaskId, targetEnv: params.targetEnv };
        }
        throw new Error('projectId or taskId required');
      }

      case 'complete_project': {
        const projectId = params.projectId || session.activeProjectId;
        if (!projectId) throw new Error('projectId required');
        const result = await this.projects.transition(projectId, 'completed');
        return { _summary: 'Projeto finalizado', ...result };
      }

      case 'force_complete': {
        const projectId = params.projectId || session.activeProjectId;
        if (!projectId) throw new Error('projectId required');
        const result = await this.projects.forceComplete(projectId);
        return { _summary: 'Projeto fechado (force)', ...result };
      }

      case 'pause': {
        const projectId = params.projectId || session.activeProjectId;
        if (!projectId) throw new Error('projectId required');
        const result = await this.projects.pause(projectId);
        return { _summary: 'Projeto pausado', ...result };
      }

      case 'resume': {
        const projectId = params.projectId || session.activeProjectId;
        if (!projectId) throw new Error('projectId required');
        const result = await this.projects.resume(projectId);
        return { _summary: 'Projeto retomado', ...result };
      }

      case 'resolve_task': {
        const taskId = params.taskId || session.activeTaskId;
        if (!taskId) throw new Error('taskId required');
        await this.prisma.taskTransition.create({
          data: { taskId, fromStatus: 'any', toStatus: 'completed', actor: 'director', metadata: { resolved: true } },
        });
        const result = await this.prisma.task.update({ where: { id: taskId }, data: { status: 'completed' } });
        return { _summary: 'Tarefa marcada como concluída', ...result };
      }

      case 'diagnose': {
        if (!params.question) throw new Error('question required');
        const projectId = params.projectId || session.activeProjectId;
        let repo = params.repo;
        if (!repo && projectId) {
          const project = await this.projects.findOne(projectId);
          repo = project?.subtasks?.[0]?.repo;
        }
        if (!repo) {
          // Detect from question text
          const q = params.question.toLowerCase();
          if (q.includes('frontend') || q.includes('tela') || q.includes('componente') || q.includes('react') || q.includes('login') || q.includes('rota')) repo = 'patria-app';
          else if (q.includes('landpage') || q.includes('landing')) repo = 'landpage';
          else repo = 'patria-api';
        }
        const specialistMap = {
          'patria-api': 'Técnico Backend (NestJS/Prisma)',
          'patria-app': 'Técnico Frontend (React/Redux)',
          'landpage': 'Técnico Frontend (Landpage/CSS)',
        };
        const specialist = specialistMap[repo] || 'Técnico';
        const diagnosis = await this.claude.diagnose(params.question, repo, projectId);
        return { _summary: 'Diagnóstico em andamento', specialist, repo, diagnosis };
      }

      case 'followup': {
        // Follow-up question to the same specialist (uses last diagnose context)
        if (!params.question) throw new Error('question required');
        const lastDiagnose = await this.prisma.conversationAction.findFirst({
          where: { sessionId: session.id, action: 'diagnose' },
          orderBy: { createdAt: 'desc' },
        });
        const prevContext = lastDiagnose?.requestBody as any || {};
        const repo = params.repo || prevContext?.repo || prevContext?.params?.repo || 'patria-api';
        const projectId = params.projectId || session.activeProjectId || prevContext?.projectId;
        const prevQuestion = prevContext?.question || prevContext?.params?.question || '';
        const fullQuestion = `Contexto da pergunta anterior: ${prevQuestion}\n\nPergunta de follow-up: ${params.question}`;
        const specialistMap = {
          'patria-api': 'Técnico Backend',
          'patria-app': 'Técnico Frontend',
          'landpage': 'Técnico Frontend (Landpage)',
        };
        const diagnosis = await this.claude.diagnose(fullQuestion, repo, projectId);
        return { _summary: 'Follow-up respondido', specialist: specialistMap[repo] || 'Técnico', repo, diagnosis };
      }

      case 'save_memory': {
        if (!params.title || !params.content || !params.category) {
          throw new Error('title, content, and category required');
        }

        // Resolve person from current conversation
        const memContact = await this.prisma.messagingContact.findFirst({
          where: { channelType: channel as any, remoteId: target },
          include: { person: { include: { profile: true } } },
        }).catch(() => null);

        const memPerson = memContact?.person;
        const memProfile = memPerson?.profile;

        // Check if privacy mode is active in session
        const memSession = await this.getOrCreateSession(channel, target);
        const isPrivacyMode = (memSession.metadata as any)?.privacyMode === true;

        // Determine visibility
        // Work categories always stay global (business decisions are shared)
        // Privacy mode seals personal categories only
        const workCategories = ['decision', 'technical', 'project', 'product'];
        const isWork = workCategories.includes(params.category);
        const visibility = isWork ? 'global' : (isPrivacyMode ? 'sealed' : 'private');

        const memId = await this.getMemory().save(
          params.category,
          params.title,
          params.content,
          params.priority || 'short_term',
          memPerson?.id,
          visibility,
        );
        return { _summary: `Memória salva: ${params.title}`, saved: true, memoryId: memId, title: params.title, priority: params.priority || 'short_term', visibility };
      }

      case 'add_contact': {
        if (!params.name || !params.phone) throw new Error('name and phone required');

        // Normalize phone: remove non-digits, add +55 default, add 9 for mobile
        let phone = params.phone.replace(/\D/g, '');
        if (!phone.startsWith('55') && phone.length <= 11) phone = '55' + phone;
        // Brazilian mobile: +55 + 2 digit DDD + 9 + 8 digits = 13 digits total
        // If 12 digits (missing the 9), add it after DDD
        if (phone.startsWith('55') && phone.length === 12) {
          phone = phone.substring(0, 4) + '9' + phone.substring(4);
        }
        phone = '+' + phone;

        const profileRow = params.profile
          ? await this.prisma.profile.findUnique({ where: { slug: params.profile } })
          : null;

        // Check if person already exists by phone
        const existing = await this.prisma.person.findFirst({
          where: { phone },
        });
        if (existing) {
          return { _summary: `${existing.name} já está cadastrado(a)`, added: false, personId: existing.id, phone, message: `${existing.name} já está cadastrado(a)` };
        }

        const newPerson = await this.prisma.person.create({
          data: {
            name: params.name,
            phone,
            description: params.description || null,
            role: params.role || 'member',
            profileId: profileRow?.id || null,
          },
        });

        // Create messaging contact for WhatsApp
        const digits = phone.replace(/\D/g, '');
        await this.prisma.messagingContact.create({
          data: {
            channelType: 'whatsapp',
            remoteId: `${digits}@s.whatsapp.net`,
            displayName: params.name,
            phone,
            personId: newPerson.id,
          },
        });

        // Generate embedding for semantic search (name + description)
        if (params.description) {
          try {
            const GEMINI_KEY = process.env.GEMINI_API_KEY;
            if (GEMINI_KEY) {
              const embText = `${params.name} ${params.description}`;
              const embRes = await fetch(
                `https://generativelanguage.googleapis.com/v1beta/models/gemini-embedding-001:embedContent?key=${GEMINI_KEY}`,
                {
                  method: 'POST',
                  headers: { 'Content-Type': 'application/json' },
                  body: JSON.stringify({ content: { parts: [{ text: embText }] }, outputDimensionality: 768 }),
                  signal: AbortSignal.timeout(10_000),
                },
              );
              if (embRes.ok) {
                const embData = await embRes.json();
                const vec = `[${embData.embedding.values.join(',')}]`;
                await this.prisma.$executeRawUnsafe(`UPDATE people SET embedding = $1::vector WHERE id = $2`, vec, newPerson.id);
              }
            }
          } catch {}
        }

        return {
          _summary: `Contato adicionado: ${params.name}`,
          added: true,
          personId: newPerson.id,
          name: params.name,
          phone,
          profile: params.profile || 'sem perfil',
        };
      }

      case 'send_message': {
        if (!params.to || !params.message) throw new Error('to and message required');
        const ch = params.channel || 'whatsapp';

        // Resolve recipient with profile and contacts
        let targetId = params.to;
        const person = await this.prisma.person.findFirst({
          where: {
            OR: [
              { name: { contains: params.to, mode: 'insensitive' } },
              { phone: { contains: params.to } },
            ],
          },
          include: { profile: true, contacts: true },
        });

        // Check profile — warn if message doesn't match recipient's profile
        const recipientProfile = (person as any)?.profile?.slug;
        if (recipientProfile && !this.isDirectorSlug(recipientProfile)) {
          const msgLower = params.message.toLowerCase();
          if (msgLower.includes('patria technology') || msgLower.includes('gerente de projetos') || msgLower.includes('gestora')) {
            return {
              _summary: `Mensagem bloqueada: perfil "${recipientProfile}" incompatível`,
              sent: false,
              error: `O destinatário ${person.name} tem perfil "${recipientProfile}". NÃO mencione empresa ou cargo técnico. Reformule a mensagem de forma adequada ao perfil.`,
              profile: recipientProfile,
            };
          }
        }

        if (person && ch === 'whatsapp') {
          // Prefer @lid contact (new WhatsApp format) over @s.whatsapp.net
          const lidContact = (person as any).contacts?.find((c: any) => c.channelType === 'whatsapp' && c.remoteId.endsWith('@lid'));
          const waContact = (person as any).contacts?.find((c: any) => c.channelType === 'whatsapp');
          if (lidContact) {
            targetId = lidContact.remoteId;
          } else if (waContact) {
            targetId = waContact.remoteId;
          } else if (person.phone) {
            const digits = person.phone.replace(/\D/g, '');
            targetId = `${digits}@s.whatsapp.net`;
          }
        } else if (person?.phone) {
          targetId = person.phone.replace(/\D/g, '');
        }

        // Adapt message to recipient's profile via LLM
        let finalMessage = params.message;
        if (person?.profile) {
          try {
            const { bridgeCall } = require('./worker/bridge-client');
            const personModel = (person as any).context?.model || 'gemini-2.5-flash';
            const adapted = await bridgeCall('/llm/chat', {
              model: personModel,
              max_tokens: 512,
              system: `Você é a Patrícia. Reescreva a mensagem abaixo adaptando o tom para o perfil "${recipientProfile}" da pessoa "${person.name}" (${(person as any).description || ''}).
Mantenha o conteúdo e as informações — mude apenas o tom e a forma.
Se a mensagem já estiver adequada, retorne exatamente como está.
Responda APENAS com a mensagem final, sem explicações.`,
              messages: [{ role: 'user', content: params.message }],
            }, 30_000);
            const adaptedText = adapted?.content?.find((b: any) => b.type === 'text')?.text;
            if (adaptedText) finalMessage = adaptedText.trim();
          } catch {}
        }

        // Send via HTTP
        try {
          const sendRes = await fetch(`http://127.0.0.1:${process.env.PORT || 8080}/api/messaging/send`, {
            method: 'POST',
            headers: {
              'Content-Type': 'application/json',
              'X-API-Key': process.env.API_KEY_WORKER || 'wk_eb7128f2614831a3ffdd101699fffd05fe57a1fe437c6ccb',
            },
            body: JSON.stringify({ channel: ch, target: targetId, message: finalMessage }),
            signal: AbortSignal.timeout(30_000),
          });
          if (!sendRes.ok) {
            const err = await sendRes.text();
            return { _summary: 'Falha ao enviar mensagem', sent: false, error: err };
          }
          // Save sent message to recipient's session history so Patricia remembers
          if (person) {
            try {
              const recipientContact = (person as any).contacts?.find((c: any) => c.channelType === ch);
              if (recipientContact) {
                const recipientSession = await this.prisma.conversationSession.findFirst({
                  where: { channel: ch, target: recipientContact.remoteId },
                });
                if (recipientSession) {
                  const meta = (recipientSession.metadata as any) || {};
                  const history = meta.history || [];
                  history.push({ role: 'assistant', content: finalMessage });
                  await this.prisma.conversationSession.update({
                    where: { id: recipientSession.id },
                    data: { metadata: { ...meta, history: history.slice(-20) } },
                  });
                }
              }
            } catch {}
          }

          return { _summary: `Mensagem enviada para ${person?.name || params.to}`, sent: true, to: person?.name || params.to, channel: ch, profile: recipientProfile || 'unknown' };
        } catch (err) {
          return { _summary: 'Falha ao enviar mensagem', sent: false, error: err.message };
        }
      }

      case 'check_contact': {
        if (!params.name) throw new Error('name required');

        // Resolve caller's identity and access level
        const callerContact = await this.prisma.messagingContact.findFirst({
          where: { channelType: channel as any, remoteId: target },
          include: { person: { include: { profile: true } } },
        }).catch(() => null);
        const callerPerson = callerContact?.person;
        const callerAccess = callerPerson?.profile?.memoryAccess || 'own';

        // Find target person via PeopleService (searches name, email, phone)
        const results = await this.peopleService.search(params.name);
        if (results.length === 0) return { _summary: `Nenhum contato encontrado para "${params.name}"`, found: false, message: `Nenhum contato encontrado para "${params.name}"` };
        if (results.length > 1) {
          const names = results.map((p: any) => p.name).join(', ');
          return { _summary: `Encontrados ${results.length} contatos: ${names}`, found: false, message: `Encontrei ${results.length} contatos: ${names}. Seja mais específico.` };
        }
        const personBase = results[0];
        const person = await this.prisma.person.findFirst({
          where: { id: personBase.id },
          include: { profile: true, contacts: true },
        });
        if (!person) return { _summary: `Nenhum contato encontrado para "${params.name}"`, found: false, message: `Nenhum contato encontrado para "${params.name}"` };

        // Non-directors can only check themselves
        const isSelf = callerPerson?.id === person.id;
        if (callerAccess !== 'all' && !isSelf) {
          return { _summary: `${person.name}, acesso restrito`, found: true, name: person.name, restricted: true, message: 'Você não tem permissão para ver informações de outros contatos.' };
        }

        // Get recent messages (respect privacy — skip [mensagem privada] content for non-self)
        const contactIds = person.contacts.map((c: any) => c.id);
        const recentMessages = contactIds.length > 0
          ? await this.prisma.messageLog.findMany({
              where: { contactId: { in: contactIds }, direction: 'inbound' },
              orderBy: { createdAt: 'desc' },
              take: 5,
              select: { content: true, createdAt: true },
            })
          : [];

        // Get memories (directors see non-sealed, self sees own)
        const memoryFilter: any = { personId: person.id, state: 'active' };
        if (!isSelf) memoryFilter.visibility = { notIn: ['sealed', 'private'] };
        const personMemories = await this.prisma.patriciaMemory.findMany({
          where: memoryFilter,
          take: 5,
          orderBy: { updatedAt: 'desc' },
          select: { title: true, content: true, category: true },
        });

        return {
          _summary: `${person.name}, perfil ${person.profile?.name || 'sem perfil'}, última msg ${person.contacts[0]?.lastSeenAt ? new Date(person.contacts[0].lastSeenAt).toLocaleDateString('pt-BR') : 'desconhecida'}`,
          found: true,
          name: person.name,
          description: person.description || null,
          role: person.role,
          phone: person.phone,
          profile: person.profile?.name || 'sem perfil',
          channels: person.contacts.map((c: any) => c.channelType),
          lastSeen: person.contacts[0]?.lastSeenAt || null,
          recentMessages: recentMessages.map((m: any) => ({
            text: m.content.substring(0, 200),
            at: m.createdAt,
          })),
          memories: personMemories.map((m: any) => ({
            title: m.title,
            content: m.content.substring(0, 200),
            category: m.category,
          })),
        };
      }

      case 'update_contact': {
        if (!params.name) throw new Error('name required');
        const updatePerson = await this.prisma.person.findFirst({
          where: { name: { contains: params.name, mode: 'insensitive' } },
        });
        if (!updatePerson) return { _summary: `Contato "${params.name}" não encontrado`, updated: false, message: `Contato "${params.name}" não encontrado` };

        const updateData: any = {};
        if (params.description) updateData.description = params.description;
        if (params.phone) {
          let ph = params.phone.replace(/\D/g, '');
          if (!ph.startsWith('55') && ph.length <= 11) ph = '55' + ph;
          if (ph.startsWith('55') && ph.length === 12) ph = ph.substring(0, 4) + '9' + ph.substring(4);
          updateData.phone = '+' + ph;
        }
        if (params.profile) {
          const prof = await this.prisma.profile.findUnique({ where: { slug: params.profile } });
          if (prof) updateData.profileId = prof.id;
        }

        await this.prisma.person.update({ where: { id: updatePerson.id }, data: updateData });

        // Regenerate embedding if description changed
        if (params.description) {
          try {
            const GEMINI_KEY = process.env.GEMINI_API_KEY;
            if (GEMINI_KEY) {
              const embText = `${updatePerson.name} ${params.description}`;
              const embRes = await fetch(
                `https://generativelanguage.googleapis.com/v1beta/models/gemini-embedding-001:embedContent?key=${GEMINI_KEY}`,
                { method: 'POST', headers: { 'Content-Type': 'application/json' },
                  body: JSON.stringify({ content: { parts: [{ text: embText }] }, outputDimensionality: 768 }),
                  signal: AbortSignal.timeout(10_000) },
              );
              if (embRes.ok) {
                const embData = await embRes.json();
                const vec = `[${embData.embedding.values.join(',')}]`;
                await this.prisma.$executeRawUnsafe(`UPDATE people SET embedding = $1::vector WHERE id = $2`, vec, updatePerson.id);
              }
            }
          } catch {}
        }

        return { _summary: `Contato atualizado: ${updatePerson.name}`, updated: true, name: updatePerson.name, changes: updateData };
      }

      case 'retry_task': {
        const retryTaskId = params.taskId || session.activeTaskId;
        if (!retryTaskId) throw new Error('taskId required');
        const retryTask = await this.prisma.task.findUnique({ where: { id: retryTaskId } });
        if (!retryTask) throw new Error('Task not found');
        if (retryTask.status !== 'failed' && retryTask.status !== 'timed_out') {
          throw new Error(`Task status is ${retryTask.status}, not failed/timed_out`);
        }
        await this.prisma.task.update({ where: { id: retryTaskId }, data: { status: 'pending' } });
        await this.prisma.taskTransition.create({
          data: { taskId: retryTaskId, fromStatus: retryTask.status, toStatus: 'pending', actor: 'director' },
        });
        // Resume project if paused
        if (retryTask.projectId) {
          await this.prisma.project.update({
            where: { id: retryTask.projectId },
            data: { status: 'executing' },
          }).catch(() => {});
        }
        return { _summary: 'Tarefa retentada', retried: true, taskId: retryTaskId, command: retryTask.command };
      }

      case 'send_recado': {
        if (!params.message || !params.from) throw new Error('message and from required');

        // Find Aarão's preferred contact (try @lid first)
        const aarao = await this.prisma.person.findFirst({
          where: { name: { contains: 'Aarão', mode: 'insensitive' } },
          include: { contacts: true },
        });
        if (!aarao) return { _summary: 'Diretor não encontrado', sent: false, error: 'Diretor não encontrado' };

        const lidContact = aarao.contacts.find((c: any) => c.remoteId.endsWith('@lid') && c.channelType === 'whatsapp');
        const anyContact = aarao.contacts[0];
        const aaraoTarget = lidContact?.remoteId || anyContact?.remoteId;
        const aaraoCh = lidContact?.channelType || anyContact?.channelType || 'whatsapp';

        if (!aaraoTarget) return { _summary: 'Sem canal de contato do diretor', sent: false, error: 'Sem canal de contato do diretor' };

        const recadoText = `📩 Recado de ${params.from}:\n\n${params.message}`;
        try {
          await fetch(`http://127.0.0.1:${process.env.PORT || 8080}/api/messaging/send`, {
            method: 'POST',
            headers: { 'Content-Type': 'application/json', 'X-API-Key': process.env.API_KEY_WORKER || 'wk_eb7128f2614831a3ffdd101699fffd05fe57a1fe437c6ccb' },
            body: JSON.stringify({ channel: aaraoCh, target: aaraoTarget, message: recadoText }),
            signal: AbortSignal.timeout(30_000),
          });
          return { _summary: 'Recado enviado para Aarão', sent: true, to: 'Aarão', from: params.from };
        } catch (err) {
          return { _summary: 'Falha ao enviar recado', sent: false, error: err.message };
        }
      }

      case 'check_sent': {
        if (!params.name) throw new Error('name required');
        const sentPerson = await this.prisma.person.findFirst({
          where: { name: { contains: params.name, mode: 'insensitive' } },
          include: { contacts: true },
        });
        if (!sentPerson) return { _summary: `Nenhum contato "${params.name}"`, found: false, message: `Nenhum contato "${params.name}"` };

        const contactIds = sentPerson.contacts.map((c: any) => c.id);
        if (contactIds.length === 0) return { _summary: `Nenhuma mensagem enviada para ${sentPerson.name}`, found: true, messages: [] };

        const sent = await this.prisma.messageLog.findMany({
          where: { contactId: { in: contactIds }, direction: 'outbound' },
          orderBy: { createdAt: 'desc' },
          take: params.limit || 5,
          select: { content: true, createdAt: true },
        });

        return {
          _summary: `Últimas ${sent.length} mensagens enviadas para ${sentPerson.name}`,
          found: true,
          name: sentPerson.name,
          messages: sent.map((m: any) => ({ text: m.content.substring(0, 300), at: m.createdAt })),
        };
      }

      case 'toggle_privacy': {
        const privSession = await this.getOrCreateSession(channel, target);
        const privMeta = (privSession.metadata as any) || {};
        const enabled = params.enabled === true || params.enabled === 'true';

        // Resolve person
        const privContact = await this.prisma.messagingContact.findFirst({
          where: { channelType: channel as any, remoteId: target },
          include: { person: true },
        }).catch(() => null);
        const privPerson = (privContact as any)?.person;

        if (enabled) {
          if (channel === 'telegram') {
            // Telegram: send Mini App button — activation happens via initData
            const appUrl = `https://${process.env.APP_DOMAIN || 'patria.patriatechnology.com'}/sandbox.html`;
            return {
              _summary: 'Toque no botão abaixo pra ativar o modo privado de forma segura.',
              action: 'send_webapp_button',
              buttonText: '🔒 Ativar Modo Privado',
              webAppUrl: appUrl,
            };
          }

          // WhatsApp/other: password-based activation
          if (!privPerson?.passwordHash) {
            return { _summary: 'Você precisa criar uma senha antes. Diga "cria minha senha" seguido da senha desejada.', error: 'no_password' };
          }
          if (!params.password) {
            return { _summary: 'Informe sua senha pra ativar o modo privado.', error: 'password_required' };
          }
          const crypto = require('crypto');
          const inputHash = crypto.createHash('sha256').update(params.password).digest('hex');
          if (inputHash !== privPerson.passwordHash) {
            return { _summary: 'Senha incorreta.', error: 'wrong_password' };
          }

          const encKey = crypto.createHash('sha256').update(params.password + privSession.id).digest('hex');
          if (!(global as any).__sandboxKeys) (global as any).__sandboxKeys = new Map();
          (global as any).__sandboxKeys.set(privSession.id, encKey);

          await this.prisma.conversationSession.update({
            where: { id: privSession.id },
            data: { metadata: { ...privMeta, privacyMode: true, sandboxHistory: [] } },
          });
          return { _summary: 'Modo privado ativado. Conversa criptografada. Zero rastro ao sair.', privacyMode: true };
        } else {
          // Deactivate — clear key from memory + clear metadata
          if ((global as any).__sandboxKeys) (global as any).__sandboxKeys.delete(privSession.id);
          await this.prisma.conversationSession.update({
            where: { id: privSession.id },
            data: { metadata: { ...privMeta, privacyMode: false, sandboxHistory: [] } },
          });
          return { _summary: 'Modo privado desativado. Chave descartada. Zero rastro.', privacyMode: false };
        }
      }

      case 'set_password': {
        if (!params.password || params.password.length < 4) {
          return { _summary: 'Senha precisa ter pelo menos 4 caracteres.', error: true };
        }
        const setpwContact = await this.prisma.messagingContact.findFirst({
          where: { channelType: channel as any, remoteId: target },
          select: { personId: true },
        }).catch(() => null);
        if (!setpwContact?.personId) return { _summary: 'Pessoa não encontrada.', error: true };

        const crypto = require('crypto');
        const hash = crypto.createHash('sha256').update(params.password).digest('hex');
        await this.prisma.$queryRawUnsafe(
          `UPDATE people SET password_hash = $1 WHERE id = $2`,
          hash, setpwContact.personId,
        );
        return { _summary: 'Senha criada. Agora você pode ativar o modo privado.', set: true };
      }

      case 'open_specialist': {
        // Set specialist flag on session
        const specSession = await this.getOrCreateSession(channel, target);
        const specMeta = (specSession.metadata as any) || {};
        await this.prisma.conversationSession.update({
          where: { id: specSession.id },
          data: { metadata: { ...specMeta, specialistActive: true, specialistHistory: [] } },
        });
        return { _summary: 'Especialista conectado', opened: true, message: 'Especialista conectado. Pergunte diretamente.' };
      }

      case 'forget_memory': {
        if (!params.title) throw new Error('title required');

        // Resolve person + access level
        const forgetContact = await this.prisma.messagingContact.findFirst({
          where: { channelType: channel as any, remoteId: target },
          include: { person: { include: { profile: true } } },
        }).catch(() => null);

        const forgetPerson = forgetContact?.person;
        const forgetAccess = forgetPerson?.profile?.memoryAccess || 'own';

        const forgotten = await this.getMemory().forget(params.title, forgetPerson?.id, forgetAccess);
        return forgotten
          ? { _summary: 'Memória esquecida', forgotten: true, title: params.title }
          : { _summary: 'Memória não encontrada', forgotten: false, error: 'Memória não encontrada, não é sua, ou é conhecimento base (protegido)' };
      }

      case 'ask_specialist': {
        // ALL specialist questions are async — always queue for worker
        if (!params.question) throw new Error('question required');
        const q = params.question.toLowerCase();

        // Detect repo from question
        let repo = params.repo;
        if (!repo) {
          if (/frontend|tela|componente|react|login|rota|css/.test(q)) repo = 'patria-app';
          else if (/landpage|landing/.test(q)) repo = 'landpage';
          else repo = 'patria-api';
        }

        // Detect type
        const isDiagnose = /\b(bug|erro|falha|500|404|crash|quebrou|debug)\b/.test(q);
        const queryType = isDiagnose ? 'diagnose' : 'consult';

        const query = await this.prisma.specialistQuery.create({
          data: {
            question: params.question,
            type: queryType,
            repo,
            projectId: params.projectId || session.activeProjectId,
            channel,
            target,
          },
        });

        return {
          _summary: `Consulta enviada ao técnico ${repo === 'patria-api' ? 'Backend' : 'Frontend'}`,
          queued: true,
          queryId: query.id,
          message: `Técnico ${repo === 'patria-api' ? 'Backend' : 'Frontend'} analisando. Você será notificado quando o diagnóstico estiver pronto.`,
        };
      }

      case 'consult': {
        // General specialist consultation using Gemini Flash (fast, ~5s)
        if (!params.question) throw new Error('question required');

        // Build context from DB
        const allProjects = await this.prisma.project.findMany({
          select: { name: true, status: true, environment: true, totalSubtasks: true, doneSubtasks: true },
          orderBy: { createdAt: 'desc' },
          take: 10,
        });
        const recentTasks = await this.prisma.task.findMany({
          where: { projectId: null },
          select: { command: true, status: true, environment: true },
          orderBy: { createdAt: 'desc' },
          take: 10,
        });

        // Read PRODUCT.md from DB endpoint
        let productVision = '';
        try {
          const fs = await import('fs');
          const path = await import('path');
          productVision = fs.readFileSync(path.join(__dirname, '..', 'PRODUCT.md'), 'utf8');
        } catch {}

        let context = '';
        if (allProjects.length > 0) {
          context += '\nProjetos:\n' + allProjects.map(p => `- ${p.name} (${p.status}, ${p.environment})`).join('\n');
        }
        if (recentTasks.length > 0) {
          context += '\nTarefas recentes:\n' + recentTasks.map(t => `- ${t.command} (${t.status})`).join('\n');
        }

        // Use Gemini Flash directly (fast, no Claude Code CLI)
        const GEMINI_KEY = process.env.GEMINI_API_KEY;
        if (!GEMINI_KEY) throw new Error('GEMINI_API_KEY not configured');

        const prompt = `Você é consultor técnico da Patria Technology.

PRODUCT.md:
${productVision.substring(0, 3000)}

Estado atual:
${context}

Pergunta: ${params.question}

Responda em português, prático e objetivo. Máximo 10 linhas.
Se for sobre próximos passos, sugira módulos do PRODUCT.md que NÃO aparecem nos projetos concluídos.`;

        const geminiRes = await fetch(
          `https://generativelanguage.googleapis.com/v1beta/models/gemini-2.5-flash:generateContent?key=${GEMINI_KEY}`,
          {
            method: 'POST',
            headers: { 'Content-Type': 'application/json' },
            body: JSON.stringify({
              contents: [{ parts: [{ text: prompt }] }],
              generationConfig: { maxOutputTokens: 500 },
            }),
            signal: AbortSignal.timeout(15_000),
          },
        );
        if (!geminiRes.ok) throw new Error(`Gemini API error: ${geminiRes.status}`);
        const geminiData = await geminiRes.json();
        const answer = geminiData.candidates?.[0]?.content?.parts?.[0]?.text || 'Sem resposta';

        return { _summary: 'Consulta respondida', specialist: 'Consultor Técnico', answer };
      }

      case 'status': {
        const status = await this.getFullStatus();
        const pendingActions = await this.getPendingActions();
        return { _summary: `${status.completedLast24h} tarefas concluídas, ${status.pendingTasks} pendentes, ${status.activeProjects.length} projetos ativos`, status, pendingActions };
      }

      case 'task_detail': {
        const taskId = params.taskId || session.activeTaskId;
        if (!taskId) throw new Error('taskId required');
        const task = await this.prisma.task.findUnique({
          where: { id: taskId },
          include: { executions: { orderBy: { startedAt: 'desc' }, take: 1 }, transitions: { orderBy: { createdAt: 'desc' }, take: 5 } },
        });
        if (!task) throw new Error('Task not found');
        return {
          _summary: `Tarefa: ${task.command} — status: ${task.status}`,
          id: task.id,
          command: task.command,
          description: task.description,
          status: task.status,
          repo: task.repo,
          environment: task.environment,
          branch: task.branch,
          plan: task.executions[0]?.plan || null,
          result: task.executions[0]?.result || null,
          feedback: task.executions[0]?.feedback || null,
          recentTransitions: task.transitions.map(t => ({ from: t.fromStatus, to: t.toStatus, actor: t.actor, at: t.createdAt })),
        };
      }

      case 'project_detail': {
        const projectId = params.projectId || session.activeProjectId;
        if (!projectId) throw new Error('projectId required');
        const project = await this.prisma.project.findUnique({
          where: { id: projectId },
          include: { subtasks: { orderBy: { sortOrder: 'asc' } } },
        });
        if (!project) throw new Error('Project not found');
        const doneCount = project.subtasks.filter(t => t.status === 'completed').length;
        return {
          _summary: `Projeto: ${project.name} — ${doneCount}/${project.subtasks.length} subtarefas, status: ${project.status}`,
          id: project.id,
          name: project.name,
          description: project.description,
          status: project.status,
          environment: project.environment,
          subtasks: project.subtasks.map(t => ({
            id: t.id,
            command: t.command,
            description: t.description,
            status: t.status,
            repo: t.repo,
          })),
        };
      }

      case 'update_subtask': {
        const taskId = await this.resolveTaskId(params, session);
        const updateData: any = {};
        if (params.command) updateData.command = params.command;
        if (params.description) updateData.description = params.description;
        if (params.repo) updateData.repo = params.repo;
        const result = await this.prisma.task.update({ where: { id: taskId }, data: updateData });
        return { _summary: `Subtarefa atualizada`, ...result };
      }

      case 'search_project': {
        const results = await this.claude.searchProjects(params.q, params.limit);
        const count = Array.isArray(results) ? results.length : 0;
        return { _summary: `Encontrados ${count} resultados`, results };
      }

      case 'search_task': {
        const results = await this.claude.searchTasks(params.q, params.limit);
        const count = Array.isArray(results) ? results.length : 0;
        return { _summary: `Encontrados ${count} resultados`, results };
      }

      case 'switch_model': {
        // Resolve who: caller or target person (director can switch for others)
        const callerContact = await this.prisma.messagingContact.findFirst({
          where: { channelType: channel as any, remoteId: target },
          include: { person: { include: { profile: true } } },
        }).catch(() => null);
        const caller = (callerContact as any)?.person;
        const callerIsDirector = this.isDirector(caller?.profile);

        let targetPersonId = caller?.id;
        let targetProfile = caller?.profile;

        // If director wants to switch someone else's model
        if (params.person && callerIsDirector) {
          const found = await this.prisma.$queryRawUnsafe(
            `SELECT p.id, pr.slug as profile_slug FROM people p LEFT JOIN profiles pr ON p.profile_id = pr.id WHERE LOWER(p.name) LIKE LOWER($1) LIMIT 1`,
            `%${params.person}%`,
          ).catch(() => []) as any;
          if (found[0]) {
            targetPersonId = found[0].id;
            targetProfile = { slug: found[0].profile_slug };
          } else {
            return { _summary: `Pessoa "${params.person}" não encontrada`, switched: false, error: `Pessoa "${params.person}" não encontrada` };
          }
        }

        // Validate model is in target's allowed list (directors skip)
        if (targetProfile && !this.isDirector(targetProfile)) {
          const allowedResult = await this.prisma.$queryRawUnsafe(
            `SELECT value FROM patricia_config WHERE key = $1`, `models:${targetProfile.slug}`,
          ).catch(() => []) as any;
          const allowedModels: string[] = allowedResult[0]?.value ? JSON.parse(allowedResult[0].value) : ['gemini-2.5-flash', 'claude-haiku-4-5'];
          if (!allowedModels.includes(params.model)) {
            return { _summary: `Modelo "${params.model}" não disponível`, switched: false, error: `Modelo "${params.model}" não disponível para ${params.person || 'você'}. Modelos: ${allowedModels.join(', ')}` };
          }
        }

        // Save model per person
        if (targetPersonId) {
          await this.prisma.$queryRawUnsafe(
            `UPDATE people SET context = COALESCE(context, '{}'::jsonb) || jsonb_build_object('model', $1::text) WHERE id = $2`,
            params.model, targetPersonId,
          );
        }
        return { _summary: `Modelo trocado para ${params.model}${params.person ? ' (' + params.person + ')' : ''}`, switched: true, model: params.model, person: params.person || 'você' };
      }

      case 'list_models': {
        const listContact = await this.prisma.messagingContact.findFirst({
          where: { channelType: channel as any, remoteId: target },
          include: { person: { include: { profile: true } } },
        }).catch(() => null);
        const listProfile = (listContact as any)?.person?.profile;
        const listPerson = (listContact as any)?.person;

        // If director asks about someone else
        if (params.person && this.isDirector(listProfile)) {
          const targetPerson = await this.prisma.$queryRawUnsafe(
            `SELECT p.name, p.context, pr.slug as profile_slug FROM people p LEFT JOIN profiles pr ON p.profile_id = pr.id WHERE LOWER(p.name) LIKE LOWER($1)`,
            `%${params.person}%`,
          ).catch(() => []);
          if ((targetPerson as any).length > 0) {
            const tp = (targetPerson as any)[0];
            const modelsResult = await this.prisma.$queryRawUnsafe(
              `SELECT value FROM patricia_config WHERE key = $1`, `models:${tp.profile_slug}`,
            ).catch(() => []);
            const mr = modelsResult as any;
            const available = mr[0]?.value ? JSON.parse(mr[0].value) : ['gemini-2.5-flash', 'claude-haiku-4-5'];
            return { _summary: `${tp.name} usa ${tp.context?.model || 'default (flash)'}. Disponíveis: ${available.join(', ')}`, person: tp.name, profile: tp.profile_slug, currentModel: tp.context?.model || 'default (flash)', availableModels: available };
          }
          return { _summary: `Pessoa "${params.person}" não encontrada`, error: `Pessoa "${params.person}" não encontrada` };
        }

        // Own models
        const slug = listProfile?.slug || 'amiga';
        const modelsResult = await this.prisma.$queryRawUnsafe(
          `SELECT value FROM patricia_config WHERE key = $1`, `models:${slug}`,
        ).catch(() => []);
        const mr2 = modelsResult as any;
        const available = this.isDirectorSlug(slug)
          ? ['gemini-2.5-flash', 'claude-haiku-4-5', 'claude-sonnet-4-6', 'gpt-4o-mini']
          : (mr2[0]?.value ? JSON.parse(mr2[0].value) : ['gemini-2.5-flash', 'claude-haiku-4-5']);
        const currentModel = listPerson?.context?.model || 'default (flash)';
        return { _summary: `Você usa ${currentModel}. Disponíveis: ${available.join(', ')}`, currentModel, availableModels: available, profile: slug };
      }

      case 'manage_models': {
        // Director only
        const manageContact = await this.prisma.messagingContact.findFirst({
          where: { channelType: channel as any, remoteId: target },
          include: { person: { include: { profile: true } } },
        }).catch(() => null);
        const manageProfile = (manageContact as any)?.person?.profile;
        if (!this.isDirector(manageProfile)) {
          return { _summary: 'Permissão negada', error: 'Somente diretores podem gerenciar modelos de perfis' };
        }

        // Resolve profile slug: from person name or direct slug
        let profileSlug = params.profile;
        if (params.person && !profileSlug) {
          const found = await this.prisma.$queryRawUnsafe(
            `SELECT pr.slug FROM people p JOIN profiles pr ON p.profile_id = pr.id WHERE LOWER(p.name) LIKE LOWER($1) LIMIT 1`,
            `%${params.person}%`,
          ).catch(() => []) as any;
          profileSlug = found[0]?.slug;
          if (!profileSlug) return { _summary: `Pessoa "${params.person}" não encontrada`, error: `Pessoa "${params.person}" não encontrada ou sem perfil` };
        }
        if (!profileSlug) return { _summary: 'Informe a pessoa ou o perfil', error: 'Informe a pessoa ou o perfil' };

        // Can't manage other directors
        if (this.isDirectorSlug(profileSlug)) {
          return { _summary: 'Não é possível alterar modelos de diretores', error: 'Não é possível alterar modelos de outros diretores' };
        }

        const configKey = `models:${profileSlug}`;
        const existing = await this.prisma.$queryRawUnsafe(
          `SELECT value FROM patricia_config WHERE key = $1`, configKey,
        ).catch(() => []);
        const ex = existing as any;
        let models: string[] = ex[0]?.value ? JSON.parse(ex[0].value) : ['gemini-2.5-flash', 'claude-haiku-4-5'];

        if (params.action === 'add') {
          if (!models.includes(params.model)) models.push(params.model);
        } else if (params.action === 'remove') {
          models = models.filter((m) => m !== params.model);
          if (models.length === 0) models = ['gemini-2.5-flash']; // Always keep at least flash
        }

        await this.prisma.$queryRawUnsafe(
          `INSERT INTO patricia_config (key, value, updated_at) VALUES ($1, $2, NOW())
           ON CONFLICT (key) DO UPDATE SET value = $2, updated_at = NOW()`,
          configKey, JSON.stringify(models),
        );
        return { _summary: `Modelo ${params.model} ${params.action === 'add' ? 'adicionado ao' : 'removido do'} perfil ${profileSlug}`, profile: params.profile, models, action: params.action, model: params.model };
      }

      case 'simulate_person': {
        const simSession = await this.getOrCreateSession(channel, target);
        const simMeta = (simSession.metadata as any) || {};

        // Deactivate
        if (params.active === false) {
          await this.prisma.conversationSession.update({
            where: { id: simSession.id },
            data: { metadata: { ...simMeta, simulationActive: false, simulationPerson: null, simulationHistory: [] } },
          });
          return { _summary: 'Simulação encerrada. Voltei ao modo normal.', active: false };
        }

        // Resolve target person
        const simPerson = await this.prisma.$queryRawUnsafe(
          `SELECT p.id, p.name, p.role, p.description, p.context,
                  pr.slug as profile_slug, pr.system_prompt as profile_prompt,
                  pr.allowed_tools as allowed_tools, pr.memory_access as memory_access
           FROM people p LEFT JOIN profiles pr ON p.profile_id = pr.id
           WHERE LOWER(p.name) LIKE LOWER($1) LIMIT 1`,
          `%${params.person}%`,
        ).catch(() => []) as any;

        if (!simPerson[0]) return { _summary: `Pessoa "${params.person}" não encontrada`, error: true };
        const sp = simPerson[0];
        const personModel = sp.context?.model || 'gemini-2.5-flash';

        await this.prisma.conversationSession.update({
          where: { id: simSession.id },
          data: {
            metadata: {
              ...simMeta,
              simulationActive: true,
              simulationHistory: [],
              simulationPerson: {
                id: sp.id,
                name: sp.name,
                role: sp.role,
                description: sp.description,
                profileSlug: sp.profile_slug,
                profilePrompt: sp.profile_prompt,
                allowedTools: sp.allowed_tools || [],
                memoryAccess: sp.memory_access || 'own',
                model: personModel,
              },
            },
          },
        });

        return {
          _summary: `Simulando conversa com ${sp.name} (perfil: ${sp.profile_slug}, modelo: ${personModel}). Mande mensagens como se fosse ${sp.name}. Diga "sai da simulação" para encerrar.`,
          person: sp.name,
          profile: sp.profile_slug,
          model: personModel,
        };
      }

      case 'preview_message': {
        if (!params.to || !params.message) throw new Error('to and message required');

        const prevPerson = await this.prisma.person.findFirst({
          where: { name: { contains: params.to, mode: 'insensitive' } },
          include: { profile: true },
        });
        if (!prevPerson) return { _summary: `Pessoa "${params.to}" não encontrada`, error: true };

        const prevProfile = (prevPerson as any).profile?.slug || 'unknown';
        const prevModel = (prevPerson as any).context?.model || 'gemini-2.5-flash';

        try {
          const { bridgeCall } = require('./worker/bridge-client');
          const adapted = await bridgeCall('/llm/chat', {
            model: prevModel,
            max_tokens: 512,
            system: `Você é a Patrícia. Reescreva a mensagem abaixo adaptando o tom para o perfil "${prevProfile}" da pessoa "${prevPerson.name}" (${(prevPerson as any).description || ''}).
Mantenha o conteúdo e as informações — mude apenas o tom e a forma.
Se a mensagem já estiver adequada, retorne exatamente como está.
Responda APENAS com a mensagem final, sem explicações.`,
            messages: [{ role: 'user', content: params.message }],
          }, 30_000);
          const adaptedText = adapted?.content?.find((b: any) => b.type === 'text')?.text?.trim() || params.message;
          return {
            _summary: `Preview para ${prevPerson.name} (${prevProfile}): "${adaptedText}"`,
            preview: adaptedText,
            original: params.message,
            person: prevPerson.name,
            profile: prevProfile,
            model: prevModel,
            sent: false,
          };
        } catch (err) {
          return {
            _summary: `Preview sem adaptação: "${params.message}"`,
            preview: params.message,
            person: prevPerson.name,
            profile: prevProfile,
            adapted: false,
          };
        }
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
      case 'search_project': {
        // If results found, set active project to first result
        const projResults = result?.results || result;
        if (Array.isArray(projResults) && projResults.length > 0 && projResults[0].id) {
          projectId = projResults[0].id;
        }
        break;
      }
      case 'search_task': {
        const taskResults = result?.results || result;
        if (Array.isArray(taskResults) && taskResults.length > 0 && taskResults[0].id) {
          taskId = taskResults[0].id;
        }
        break;
      }
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

    // Projects completed but not in prod yet — ready to promote
    const readyToPromote = await this.prisma.project.findMany({
      where: { status: 'completed', environment: { not: 'prod' } },
      select: { id: true, name: true, environment: true },
    });

    return { completedLast24h, failedLast24h, pendingTasks, deployingTasks, activeProjects, readyToPromote, warnings };
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

    // Phase-specific instructions — only load what's relevant
    context += PHASE_INSTRUCTIONS[session.phase] || PHASE_INSTRUCTIONS.idle;

    return context;
  }
}
