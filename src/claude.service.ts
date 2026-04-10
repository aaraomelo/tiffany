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

  async embedTask(taskId: string, command: string, description?: string): Promise<void> {
    try {
      await fetch(`${this.workerUrl}/embed-task`, {
        method: 'POST',
        headers: { 'Content-Type': 'application/json', 'X-Worker-Key': this.workerSecret },
        body: JSON.stringify({ taskId, command, description }),
        signal: AbortSignal.timeout(10_000),
      });
    } catch (err) {
      this.logger.warn(`Embed task failed: ${err.message}`);
    }
  }

  async embedProject(projectId: string, name: string, description?: string): Promise<void> {
    try {
      await fetch(`${this.workerUrl}/embed-project`, {
        method: 'POST',
        headers: { 'Content-Type': 'application/json', 'X-Worker-Key': this.workerSecret },
        body: JSON.stringify({ projectId, name, description }),
        signal: AbortSignal.timeout(10_000),
      });
    } catch (err) {
      this.logger.warn(`Embed project failed: ${err.message}`);
    }
  }

  async searchProjects(query: string, limit = 5): Promise<any[]> {
    try {
      const res = await fetch(`${this.workerUrl}/search-projects`, {
        method: 'POST',
        headers: { 'Content-Type': 'application/json', 'X-Worker-Key': this.workerSecret },
        body: JSON.stringify({ query, limit }),
        signal: AbortSignal.timeout(10_000),
      });
      if (!res.ok) return [];
      const data = await res.json();
      return data.results || [];
    } catch {
      return [];
    }
  }

  async searchTasks(query: string, limit = 5): Promise<any[]> {
    try {
      const res = await fetch(`${this.workerUrl}/search-tasks`, {
        method: 'POST',
        headers: { 'Content-Type': 'application/json', 'X-Worker-Key': this.workerSecret },
        body: JSON.stringify({ query, limit }),
        signal: AbortSignal.timeout(10_000),
      });
      if (!res.ok) return [];
      const data = await res.json();
      return data.results || [];
    } catch {
      return [];
    }
  }
}
