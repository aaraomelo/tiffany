import { Injectable, Logger } from '@nestjs/common';
import { bridgeCall, BridgeError } from './bridge-client';
import { REPO_DIRS } from './worker.config';

@Injectable()
export class ClaudeCliService {
  private readonly logger = new Logger('ClaudeCLI');

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
    const text = `${command} ${description || ''}`.toLowerCase();

    if (
      /\b(patria-api|endpoint|backend|prisma|migration|controller|service|nestjs|dto)\b/.test(text) ||
      /(?:^|\s)api(?:\s|$)/.test(text)
    )
      return 'patria-api';

    if (
      /\b(patria-app|multi-tenant|tenant|dashboard)\b/.test(text) ||
      /(?:^|\s)app(?:\s|$)/.test(text)
    )
      return 'patria-app';

    if (/\b(landpage|landing|componente|css|seção|secao)\b/.test(text)) return 'landpage';

    // Claude fallback
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
    const text = `${command} ${description || ''}`.toLowerCase();
    const repos: string[] = [];

    if (/\b(endpoint|backend|prisma|migration|controller|service|dto|nestjs)\b/.test(text)) repos.push('patria-api');
    if (/\b(página|pagina|tela|componente|formulário|formulario|botão|botao|frontend)\b/.test(text)) {
      if (/\b(landing|landpage|institucional)\b/.test(text)) repos.push('landpage');
      else repos.push('patria-app');
    }

    return repos.length > 1 ? repos : [];
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
