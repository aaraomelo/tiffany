import { Controller, Post, Req, Res, Headers } from '@nestjs/common';
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
