import { Injectable, Logger } from '@nestjs/common';
import { ClaudeCliService } from './worker/claude-cli.service';
import { EmbeddingService } from './worker/embedding.service';
import { GitService } from './worker/git.service';
import { REPO_DIRS } from './worker/worker.config';

@Injectable()
export class ClaudeService {
  private readonly logger = new Logger(ClaudeService.name);

  constructor(
    private claude: ClaudeCliService,
    private embedding: EmbeddingService,
    private git: GitService,
  ) {}

  async inferRepo(command: string, description?: string): Promise<string | null> {
    try {
      return this.claude.inferRepo(command, description);
    } catch (err) {
      this.logger.warn(`inferRepo failed: ${err.message}`);
      return null;
    }
  }

  async embedTask(taskId: string, command: string, description?: string, projectId?: string): Promise<void> {
    try {
      await this.embedding.saveTaskEmbedding(taskId, command, description, projectId);
    } catch (err) {
      this.logger.warn(`Embed task failed: ${err.message}`);
    }
  }

  async embedProject(projectId: string, name: string, description?: string): Promise<void> {
    try {
      await this.embedding.saveProjectEmbedding(projectId, name, description);
    } catch (err) {
      this.logger.warn(`Embed project failed: ${err.message}`);
    }
  }

  async searchProjects(query: string, limit = 5): Promise<any[]> {
    try {
      return await this.embedding.searchProjects(query, limit);
    } catch {
      return [];
    }
  }

  async searchTasks(query: string, limit = 5): Promise<any[]> {
    try {
      return await this.embedding.searchTasks(query, limit);
    } catch {
      return [];
    }
  }

  async diagnose(question: string, repo: string, projectId?: string): Promise<string> {
    try {
      const repoDir = REPO_DIRS[repo];
      if (!repoDir) return `Repo desconhecido: ${repo}`;

      this.git.gitSync(repoDir);
      return this.claude.runClaudeReadOnly(
        `Você é o técnico da Patria Technology.\n\nNÃO altere nenhum arquivo. Apenas leia o código e diagnostique.\n\nProblema: ${question}\n\nResponda em português, máximo 10 linhas.`,
        repoDir,
      );
    } catch (err) {
      return `Erro: ${err.message}`;
    }
  }
}
