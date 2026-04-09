import { Injectable, Logger } from '@nestjs/common';

@Injectable()
export class ClaudeService {
  private readonly logger = new Logger(ClaudeService.name);
  private readonly workerUrl = process.env.WORKER_URL || 'http://host.docker.internal:9090';
  private readonly workerSecret = process.env.WORKER_SECRET || 'wk_infer_patria_2026';

  async inferRepo(command: string, description?: string, projectId?: string): Promise<string | null> {
    try {
      const res = await fetch(`${this.workerUrl}/infer-repo`, {
        method: 'POST',
        headers: { 'Content-Type': 'application/json', 'X-Worker-Key': this.workerSecret },
        body: JSON.stringify({ command, description, projectId }),
        signal: AbortSignal.timeout(35_000),
      });

      if (!res.ok) {
        this.logger.warn(`Worker inferRepo error: ${res.status}`);
        return null;
      }

      const data = await res.json();
      return data.repo || null;
    } catch (err) {
      this.logger.warn(`Worker inferRepo failed: ${err.message}`);
      return null;
    }
  }
}
