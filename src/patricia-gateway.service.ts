import { Injectable, Logger } from '@nestjs/common';
import { PrismaService } from './prisma.service';
import { TasksService } from './tasks.service';
import { ProjectsService } from './projects.service';
import { ClaudeService } from './claude.service';
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
  'consult', 'ask',
  'update_subtask',
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
    if (!target || target === 'undefined') target = '+5511977808883';

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
        await this.prisma.taskTransition.create({
          data: { taskId, fromStatus: 'awaiting_approval', toStatus: 'approved', actor: 'director' },
        });
        return this.prisma.task.update({ where: { id: taskId }, data: { status: 'approved' } });
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
        return this.prisma.task.update({ where: { id: taskId }, data: { status: targetStatus } });
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
        // Explicit params take priority over session
        if (params.taskId) {
          return this.prisma.task.update({
            where: { id: params.taskId },
            data: { promoteTo: params.targetEnv },
          });
        }
        if (params.projectId) {
          return this.projects.setPromoteTo(params.projectId, params.targetEnv);
        }
        // Fallback to session — prefer task if both are set
        if (session.activeTaskId) {
          return this.prisma.task.update({
            where: { id: session.activeTaskId },
            data: { promoteTo: params.targetEnv },
          });
        }
        if (session.activeProjectId) {
          return this.projects.setPromoteTo(session.activeProjectId, params.targetEnv);
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
        return { specialist, repo, diagnosis };
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
        return { specialist: specialistMap[repo] || 'Técnico', repo, diagnosis };
      }

      case 'ask': {
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

        return { specialist: 'Consultor Técnico', answer };
      }

      case 'status': {
        const status = await this.getFullStatus();
        const pendingActions = await this.getPendingActions();
        return { status, pendingActions };
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
        return {
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
        const taskId = params.taskId;
        if (!taskId) throw new Error('taskId required');
        const task = await this.prisma.task.findUnique({ where: { id: taskId } });
        if (!task) throw new Error('Task not found');
        const updateData: any = {};
        if (params.command) updateData.command = params.command;
        if (params.description) updateData.description = params.description;
        if (params.repo) updateData.repo = params.repo;
        return this.prisma.task.update({ where: { id: taskId }, data: updateData });
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
