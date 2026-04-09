import { Injectable, Logger } from '@nestjs/common';

@Injectable()
export class ClaudeService {
  private readonly logger = new Logger(ClaudeService.name);
  private readonly workerUrl = process.env.WORKER_URL || 'http://host.docker.internal:9090';

  async inferRepo(command: string, description?: string): Promise<string | null> {
    try {
      const res = await fetch(`${this.workerUrl}/infer-repo`, {
        method: 'POST',
        headers: { 'Content-Type': 'application/json' },
        body: JSON.stringify({ command, description }),
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
