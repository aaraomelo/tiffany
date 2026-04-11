import { Controller, Post, Req, Res, Headers, Body } from '@nestjs/common';
import { Request, Response } from 'express';
import { createHmac } from 'crypto';
import { PrismaService } from './prisma.service';
import { TaskStateMachine } from './task-state-machine';

const WEBHOOK_SECRET = process.env.GITHUB_WEBHOOK_SECRET || 'patria-webhook-2026';

@Controller('api/webhooks')
export class WebhookController {
  constructor(
    private prisma: PrismaService,
    private stateMachine: TaskStateMachine,
  ) {}

  @Post('deploy-diagnostic')
  async handleDeployDiagnostic(
    @Body() body: { commitSha: string; diagnostic: string },
    @Res() res: Response,
  ) {
    const { commitSha, diagnostic } = body;
    if (!commitSha || !diagnostic) {
      return res.status(400).json({ error: 'commitSha and diagnostic required' });
    }

    console.log(`[deploy-diagnostic] sha=${commitSha} diagnostic=${diagnostic.substring(0, 100)}...`);

    const execution = await this.prisma.taskExecution.findFirst({
      where: { commitSha },
      include: { task: true },
    });

    if (!execution || execution.task.status !== 'deploying') {
      console.log(`[deploy-diagnostic] No deploying task for sha=${commitSha}`);
      return res.status(200).json({ ok: true, noTask: true });
    }

    const task = execution.task;

    // Store diagnostic in the execution
    await this.prisma.taskExecution.update({
      where: { id: execution.id },
      data: { feedback: `[DEPLOY_FAILURE]\n${diagnostic}` },
    });

    // If under replan limit, send to replanning; otherwise fail
    if (task.replanCount < 3) {
      try {
        await this.stateMachine.transition(task.id, 'replanning', 'system', {
          feedback: `[DEPLOY_FAILURE]\n${diagnostic}`,
          conclusion: 'failure',
        });
        console.log(`[deploy-diagnostic] Task ${task.id.substring(0, 8)} → replanning (attempt ${task.replanCount + 1}/3)`);
        return res.status(200).json({ ok: true, taskId: task.id, status: 'replanning', attempt: task.replanCount + 1 });
      } catch (err) {
        console.log(`[deploy-diagnostic] Transition error: ${err.message}`);
        // Fallback to failed
        await this.stateMachine.transition(task.id, 'failed', 'system', {
          feedback: `[DEPLOY_FAILURE]\n${diagnostic}`,
          conclusion: 'failure',
        });
        return res.status(200).json({ ok: true, taskId: task.id, status: 'failed', error: err.message });
      }
    } else {
      await this.stateMachine.transition(task.id, 'failed', 'system', {
        feedback: `[DEPLOY_FAILURE] Max retries exceeded\n${diagnostic}`,
        conclusion: 'failure',
      });
      console.log(`[deploy-diagnostic] Task ${task.id.substring(0, 8)} → failed (max replans reached: ${task.replanCount})`);
      return res.status(200).json({ ok: true, taskId: task.id, status: 'failed', reason: 'max_replans' });
    }
  }

  @Post('github')
  async handleGithub(
    @Req() req: Request,
    @Res() res: Response,
    @Headers('x-hub-signature-256') signature: string,
    @Headers('x-github-event') event: string,
  ) {
    const body = JSON.stringify(req.body);
    console.log(`[webhook] event=${event} action=${req.body?.action} signature=${signature ? 'present' : 'missing'}`);

    // Verify signature (skip if no signature header - for testing)
    if (signature) {
      const expected = 'sha256=' + createHmac('sha256', WEBHOOK_SECRET).update(body).digest('hex');
      if (signature !== expected) {
        console.log(`[webhook] Signature mismatch`);
        return res.status(401).json({ error: 'Invalid signature' });
      }
    }

    // Only handle workflow_run completed events
    if (event !== 'workflow_run' || req.body?.action !== 'completed') {
      return res.status(200).json({ ok: true, skipped: true });
    }

    const payload = req.body;
    const conclusion = payload.workflow_run?.conclusion;
    const headSha = payload.workflow_run?.head_sha;
    const repoName = payload.repository?.name;

    console.log(`[webhook] workflow_run completed: repo=${repoName} sha=${headSha} conclusion=${conclusion}`);

    if (!headSha) {
      return res.status(200).json({ ok: true, skipped: true });
    }

    // Find task in deploying state that matches this commit
    const execution = await this.prisma.taskExecution.findFirst({
      where: { commitSha: headSha },
      include: { task: true },
    });

    if (!execution || execution.task.status !== 'deploying') {
      console.log(`[webhook] No deploying task for sha=${headSha}`);
      return res.status(200).json({ ok: true, noTask: true });
    }

    const task = execution.task;
    // If failure was already handled by deploy-diagnostic (task moved to replanning), skip
    if (conclusion === 'failure' && task.status !== 'deploying') {
      console.log(`[webhook] Task ${task.id.substring(0, 8)} already transitioned (${task.status}), skipping`);
      return res.status(200).json({ ok: true, alreadyHandled: true });
    }
    const toStatus = conclusion === 'success' ? 'completed' : 'failed';

    try {
      await this.stateMachine.transition(task.id, toStatus as any, 'system', {
        conclusion,
        repo: repoName,
        commitSha: headSha,
      });
      console.log(`[webhook] Task ${task.id.substring(0, 8)} → ${toStatus}`);
      return res.status(200).json({ ok: true, taskId: task.id, status: toStatus });
    } catch (err) {
      console.log(`[webhook] Transition error: ${err.message}`);
      return res.status(200).json({ ok: true, error: err.message });
    }
  }
}
