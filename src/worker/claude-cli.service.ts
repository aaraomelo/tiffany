import { Injectable, Logger } from '@nestjs/common';
import { execSync, spawn, ChildProcess } from 'child_process';
import { REPO_DIRS } from './worker.config';

interface SpecialistSession {
  proc: ChildProcess;
  ask: (prompt: string) => Promise<string>;
  close: () => void;
}

const CLAUDE_ENV = { ...process.env, HOME: '/root' };
const SHELL_OPT = '/bin/sh';

@Injectable()
export class ClaudeCliService {
  private readonly logger = new Logger('ClaudeCLI');
  private readonly activeSessions = new Map<string, SpecialistSession>();

  /**
   * Run Claude Code in write mode (~35 turns).
   * Allowed tools: Edit, Read, Write, Glob, Grep, Bash.
   */
  runClaude(prompt: string, cwd: string): string {
    try {
      const result = execSync(
        `claude -p --max-turns 35 --allowedTools "Edit,Read,Write,Glob,Grep,Bash"`,
        {
          input: prompt,
          encoding: 'utf-8',
          timeout: 600_000,
          maxBuffer: 50 * 1024 * 1024,
          cwd,
          env: CLAUDE_ENV,
          shell: SHELL_OPT,
        },
      );
      return result.trim();
    } catch (err: any) {
      const stderr = err.stderr?.toString()?.substring(0, 500) || '';
      const stdout = err.stdout?.toString()?.substring(0, 500) || '';
      throw new Error(`Claude CLI failed (exit ${err.status}): ${stderr || stdout || err.message}`);
    }
  }

  /**
   * Run Claude Code in read-only mode (~15 turns).
   * Allowed tools: Read, Glob, Grep, Bash.
   */
  runClaudeReadOnly(prompt: string, cwd: string): string {
    try {
      const result = execSync(
        `claude -p --max-turns 15 --allowedTools "Read,Glob,Grep,Bash"`,
        {
          input: prompt,
          encoding: 'utf-8',
          timeout: 600_000,
          maxBuffer: 50 * 1024 * 1024,
          cwd,
          env: CLAUDE_ENV,
          shell: SHELL_OPT,
        },
      );
      return result.trim();
    } catch (err: any) {
      const stderr = err.stderr?.toString()?.substring(0, 500) || '';
      const stdout = err.stdout?.toString()?.substring(0, 500) || '';
      throw new Error(`Claude CLI read-only failed (exit ${err.status}): ${stderr || stdout || err.message}`);
    }
  }

  /**
   * Infer which repository a task belongs to.
   * First tries regex keyword matching, then falls back to Claude Code read-only analysis.
   */
  inferRepo(command: string, description?: string): string {
    const text = `${command} ${description || ''}`.toLowerCase();

    // Keyword matching
    if (
      /\b(patria-api|endpoint|backend|prisma|migration|controller|service|nestjs|dto)\b/.test(text) ||
      /(?:^|\s)api(?:\s|$)/.test(text)
    ) {
      return 'patria-api';
    }
    if (
      /\b(patria-app|multi-tenant|tenant|dashboard)\b/.test(text) ||
      /(?:^|\s)app(?:\s|$)/.test(text)
    ) {
      return 'patria-app';
    }
    if (/\b(landpage|landing|componente|css|seção|secao)\b/.test(text)) {
      return 'landpage';
    }

    // Claude fallback — read-only in patria-api repo
    try {
      const prompt =
        'Analise esta tarefa e responda APENAS o nome do repositório ' +
        '(patria-api, patria-app ou landpage). ' +
        `Tarefa: ${command}. Descrição: ${description || 'N/A'}`;
      const result = this.runClaudeReadOnly(prompt, REPO_DIRS['patria-api']);
      const clean = result.toLowerCase().trim().replace(/[^a-z-]/g, '');
      if (['patria-api', 'patria-app', 'landpage'].includes(clean)) {
        return clean;
      }
    } catch (err: any) {
      this.logger.warn(`inferRepo Claude fallback failed: ${err.message}`);
    }

    return 'patria-api'; // default
  }

  /**
   * Detect if a task requires changes across multiple repositories.
   * Returns an array with 2+ repos, or an empty array if single-repo.
   */
  detectMultiRepo(command: string, description?: string): string[] {
    const text = `${command} ${description || ''}`.toLowerCase();
    const repos: string[] = [];

    if (/\b(endpoint|backend|prisma|migration|controller|service|dto|nestjs)\b/.test(text)) {
      repos.push('patria-api');
    }
    if (/\b(página|pagina|tela|componente|formulário|formulario|botão|botao|frontend)\b/.test(text)) {
      if (/\b(landing|landpage|institucional)\b/.test(text)) {
        repos.push('landpage');
      } else {
        repos.push('patria-app');
      }
    }

    return repos.length > 1 ? repos : [];
  }

  /**
   * Resolve which repos a task needs.
   * If the task already has a repo assigned, checks for multi-repo; otherwise infers.
   */
  resolveRepos(task: { repo?: string | null; command: string; description?: string | null }): string[] {
    if (task.repo) {
      const multi = this.detectMultiRepo(task.command, task.description || undefined);
      return multi.length > 0 ? multi : [task.repo];
    }
    const repo = this.inferRepo(task.command, task.description || undefined);
    return [repo];
  }

  // --- Specialist Sessions ---

  private getSessionKey(channel: string, target: string): string {
    return `${channel}:${target}`;
  }

  /**
   * Open a persistent specialist Claude process for interactive sessions.
   * Uses stream-json output format for structured event parsing.
   */
  openSpecialistProcess(repoDir: string, systemPrompt: string, channel: string, target: string): SpecialistSession {
    const key = this.getSessionKey(channel, target);

    // Close existing session if any
    const existing = this.activeSessions.get(key);
    if (existing) {
      existing.close();
      this.activeSessions.delete(key);
    }

    const proc = spawn(
      'claude',
      ['--output-format', 'stream-json', '--verbose', '--max-turns', '15', '--allowedTools', 'Read,Glob,Grep,Bash', '-p'],
      {
        cwd: repoDir,
        env: CLAUDE_ENV,
        shell: SHELL_OPT,
        stdio: ['pipe', 'pipe', 'pipe'],
      },
    );

    let buffer = '';

    const ask = (prompt: string): Promise<string> => {
      return new Promise<string>((resolve, reject) => {
        let result = '';
        let resolved = false;

        const onData = (chunk: Buffer) => {
          buffer += chunk.toString();
          const lines = buffer.split('\n');
          buffer = lines.pop() || '';

          for (const line of lines) {
            if (!line.trim()) continue;
            try {
              const event = JSON.parse(line);
              if (event.type === 'assistant' && event.message?.content) {
                for (const block of event.message.content) {
                  if (block.type === 'text') {
                    result += block.text;
                  }
                }
              } else if (event.type === 'result') {
                if (!resolved) {
                  resolved = true;
                  proc.stdout?.removeListener('data', onData);
                  resolve(result.trim() || event.result || '');
                }
              }
            } catch {
              // Not valid JSON, skip
            }
          }
        };

        const onError = (err: Error) => {
          if (!resolved) {
            resolved = true;
            proc.stdout?.removeListener('data', onData);
            reject(err);
          }
        };

        const onClose = (code: number | null) => {
          if (!resolved) {
            resolved = true;
            proc.stdout?.removeListener('data', onData);
            if (result) {
              resolve(result.trim());
            } else {
              reject(new Error(`Specialist process exited with code ${code}`));
            }
          }
        };

        proc.stdout?.on('data', onData);
        proc.once('error', onError);
        proc.once('close', onClose);

        // Send the prompt
        proc.stdin?.write(prompt + '\n');
      });
    };

    const close = () => {
      try {
        proc.stdin?.end();
        proc.kill('SIGTERM');
      } catch {
        // Process may already be dead
      }
    };

    const session: SpecialistSession = { proc, ask, close };
    this.activeSessions.set(key, session);
    this.logger.log(`Specialist session opened: ${key}`);
    return session;
  }

  /**
   * Get an existing specialist session by channel+target.
   */
  getSpecialistSession(channel: string, target: string): SpecialistSession | undefined {
    const key = this.getSessionKey(channel, target);
    return this.activeSessions.get(key);
  }

  /**
   * Close a specialist session. Returns true if a session was found and closed.
   */
  closeSpecialistProcess(channel: string, target: string): boolean {
    const key = this.getSessionKey(channel, target);
    const session = this.activeSessions.get(key);
    if (session) {
      session.close();
      this.activeSessions.delete(key);
      this.logger.log(`Specialist session closed: ${key}`);
      return true;
    }
    return false;
  }

  /**
   * Check if a specialist session exists for a channel+target.
   */
  hasSpecialistSession(channel: string, target: string): boolean {
    return this.activeSessions.has(this.getSessionKey(channel, target));
  }
}
