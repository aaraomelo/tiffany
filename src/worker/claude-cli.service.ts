import { Injectable, Logger } from '@nestjs/common';
import { bridgeCall, BridgeError } from './bridge-client';
import { MatcherService } from '../matcher.service';
import { REPO_DIRS } from './worker.config';

@Injectable()
export class ClaudeCliService {
  private readonly logger = new Logger('ClaudeCLI');

  constructor(private matcher: MatcherService) {}

  // --- Claude CLI execution via bridge ---

  runClaude(prompt: string, cwd: string): Promise<string> {
    return bridgeCall<{ result: string }>('/claude/run', { prompt, cwd }, 660_000)
      .then((d) => d.result)
      .catch((err) => {
        this.logger.error(`Claude write failed in ${cwd}: ${err.message}`);
        throw new Error(`Claude CLI failed: ${err.message}`);
      });
  }

  runClaudeReadOnly(prompt: string, cwd: string): Promise<string> {
    return bridgeCall<{ result: string }>('/claude/read-only', { prompt, cwd }, 660_000)
      .then((d) => d.result)
      .catch((err) => {
        this.logger.error(`Claude read-only failed in ${cwd}: ${err.message}`);
        throw new Error(`Claude CLI read-only failed: ${err.message}`);
      });
  }

  // --- Repo inference (runs locally, no bridge needed) ---

  async inferRepo(command: string, description?: string): Promise<string> {
    const text = `${command} ${description || ''}`;
    const repo = this.matcher.detectRepo(text);
    if (repo !== 'patria-api') return repo; // keyword matched non-default

    // Claude fallback for ambiguous cases
    try {
      const result = await this.runClaudeReadOnly(
        `Analise esta tarefa e responda APENAS o nome do repositório (patria-api, patria-app ou landpage). Tarefa: ${command}. Descrição: ${description || 'N/A'}`,
        REPO_DIRS['patria-api'],
      );
      const clean = result.toLowerCase().trim().replace(/[^a-z-]/g, '');
      if (['patria-api', 'patria-app', 'landpage'].includes(clean)) return clean;
    } catch (err) {
      this.logger.warn(`inferRepo Claude fallback failed: ${err.message}`);
    }

    return 'patria-api';
  }

  detectMultiRepo(command: string, description?: string): string[] {
    return this.matcher.detectMultiRepo(`${command} ${description || ''}`);
  }

  async resolveRepos(task: { repo?: string | null; command: string; description?: string | null }): Promise<string[]> {
    if (task.repo) {
      const multi = this.detectMultiRepo(task.command, task.description || undefined);
      return multi.length > 0 ? multi : [task.repo];
    }
    const repo = await this.inferRepo(task.command, task.description || undefined);
    return [repo];
  }

  // --- Specialist sessions via bridge ---

  async openSpecialistProcess(repoDir: string, _systemPrompt: string, channel: string, target: string): Promise<{ ask: (prompt: string) => Promise<string> }> {
    await bridgeCall('/specialist/open', { repoDir, channel, target }).catch((err) => {
      this.logger.error(`Failed to open specialist session: ${err.message}`);
      throw err;
    });

    return {
      ask: async (prompt: string) => {
        const data = await bridgeCall<{ answer: string }>('/specialist/ask', { channel, target, prompt }, 300_000).catch((err) => {
          this.logger.error(`Specialist ask failed: ${err.message}`);
          throw err;
        });
        return data.answer;
      },
    };
  }

  async getSpecialistSession(channel: string, target: string): Promise<{ ask: (prompt: string) => Promise<string> } | null> {
    const data = await bridgeCall<{ exists: boolean }>('/specialist/has', { channel, target }).catch(() => ({ exists: false }));
    if (!data.exists) return null;

    return {
      ask: async (prompt: string) => {
        const res = await bridgeCall<{ answer: string }>('/specialist/ask', { channel, target, prompt }, 300_000);
        return res.answer;
      },
    };
  }

  async closeSpecialistProcess(channel: string, target: string): Promise<boolean> {
    const data = await bridgeCall<{ closed: boolean }>('/specialist/close', { channel, target }).catch(() => ({ closed: false }));
    return data.closed;
  }

  async hasSpecialistSession(channel: string, target: string): Promise<boolean> {
    const data = await bridgeCall<{ exists: boolean }>('/specialist/has', { channel, target }).catch(() => ({ exists: false }));
    return data.exists;
  }
}
