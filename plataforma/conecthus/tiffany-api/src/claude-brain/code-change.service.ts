import { Injectable, Logger, BadRequestException } from '@nestjs/common';
import { execSync } from 'child_process';
import { writeFileSync, readFileSync, existsSync } from 'fs';
import { join, dirname } from 'path';
import { mkdirSync } from 'fs';

const MIRROR_DIR = process.env.MULTIVERSO_MIRROR_DIR || '/root/multiverso-code-mirror';
const SSH_KEY = process.env.MULTIVERSO_SSH_KEY || '/root/.ssh/id_ed25519_multiverso_code';
const GEX44_HOST = process.env.GEX44_HOST_FOR_SCP || 'claude@78.46.19.151';
const GEX44_SSH_KEY = process.env.GEX44_SSH_KEY || '/root/.ssh/id_ed25519_gex44';
const REPO_HTTPS = 'https://github.com/aaraomelo/multiverso-code';

const ALLOWED_PREFIXES = ['bus/', 'sandbox/'];
const FORBIDDEN_FILES = [/secrets/i, /\.env/i, /id_ed25519/, /\.key$/];

@Injectable()
export class CodeChangeService {
  private readonly logger = new Logger('CodeChange');

  private gitEnv() {
    return {
      ...process.env,
      GIT_SSH_COMMAND: `ssh -i ${SSH_KEY} -o StrictHostKeyChecking=accept-new`,
    };
  }

  private validateFile(file: string) {
    if (!ALLOWED_PREFIXES.some((p) => file.startsWith(p))) {
      throw new BadRequestException(`file must start with one of: ${ALLOWED_PREFIXES.join(', ')}`);
    }
    if (file.includes('..')) throw new BadRequestException('path traversal');
    for (const re of FORBIDDEN_FILES) {
      if (re.test(file)) throw new BadRequestException(`forbidden file pattern: ${file}`);
    }
  }

  private gitPull() {
    execSync('git pull --ff-only origin main', {
      cwd: MIRROR_DIR, env: this.gitEnv(), encoding: 'utf-8',
    });
  }

  readFile(file: string): string {
    this.validateFile(file);
    const fullPath = join(MIRROR_DIR, file);
    if (!existsSync(fullPath)) {
      throw new BadRequestException(`file not found: ${file}`);
    }
    return readFileSync(fullPath, 'utf-8');
  }

  /** Aplica mudança. Se urgent, push em main + scp pro GEX44.
   *  Senão, cria branch + push (caller mostra URL pra criar PR).
   */
  async proposeChange(opts: {
    file: string;
    newContent: string;
    message: string;
    urgent: boolean;
    reason: string;
    actor: string; // person.name ou slug
  }): Promise<any> {
    this.validateFile(opts.file);
    if (!opts.newContent || opts.newContent.length < 10) {
      throw new BadRequestException('new_content too short');
    }
    if (opts.newContent.length > 200_000) {
      throw new BadRequestException('new_content too large (>200KB)');
    }

    // 1. pull repo
    try {
      this.gitPull();
    } catch (e: any) {
      this.logger.warn(`pull failed (continuing): ${e.message?.slice(0, 100)}`);
    }

    // 2. escreve arquivo
    const fullPath = join(MIRROR_DIR, opts.file);
    mkdirSync(dirname(fullPath), { recursive: true });
    const oldContent = existsSync(fullPath) ? readFileSync(fullPath, 'utf-8') : '';
    if (oldContent === opts.newContent) {
      return { ok: true, noop: true, reason: 'conteúdo idêntico ao atual' };
    }
    writeFileSync(fullPath, opts.newContent, 'utf-8');

    // 3. branch ou main
    const ts = new Date().toISOString().replace(/[:.]/g, '-').slice(0, 19);
    const safeFile = opts.file.replace(/[^a-zA-Z0-9]/g, '_');
    const branch = opts.urgent ? 'main' : `claude/${safeFile}-${ts}`;
    const commitMsg =
      `${opts.urgent ? '[URGENT] ' : ''}${opts.message}\n\n` +
      `Reason: ${opts.reason}\n` +
      `Actor: ${opts.actor}\n` +
      `(via Patrícia code-change tool)`;

    try {
      if (!opts.urgent) {
        // checkout main pra branchar de lá; reset pra evitar restos
        execSync(`git checkout main && git reset --hard origin/main`, {
          cwd: MIRROR_DIR, env: this.gitEnv(), stdio: 'pipe',
        });
        // re-aplica edição (foi perdida no reset)
        writeFileSync(fullPath, opts.newContent, 'utf-8');
        execSync(`git checkout -b ${JSON.stringify(branch)}`, {
          cwd: MIRROR_DIR, env: this.gitEnv(), stdio: 'pipe',
        });
      }
      execSync(`git add ${JSON.stringify(opts.file)}`, {
        cwd: MIRROR_DIR, env: this.gitEnv(), stdio: 'pipe',
      });
      execSync(
        `git -c user.email=patria-mc@noreply -c user.name=patria-multiverso-code-bot ` +
        `commit -m ${JSON.stringify(commitMsg)}`,
        { cwd: MIRROR_DIR, env: this.gitEnv(), stdio: 'pipe' },
      );
      const sha = execSync('git rev-parse HEAD', {
        cwd: MIRROR_DIR, env: this.gitEnv(), encoding: 'utf-8',
      }).trim();
      execSync(`git push origin ${JSON.stringify(branch)}`, {
        cwd: MIRROR_DIR, env: this.gitEnv(), stdio: 'pipe',
      });
      // restaura pra main pra próximas operações
      if (!opts.urgent) {
        execSync('git checkout main', {
          cwd: MIRROR_DIR, env: this.gitEnv(), stdio: 'pipe',
        });
      }

      // 4. urgent → scp pro GEX44 (aplica de fato)
      let scpResult: any = null;
      if (opts.urgent) {
        try {
          execSync(
            `scp -i ${GEX44_SSH_KEY} -o StrictHostKeyChecking=accept-new ` +
            `${JSON.stringify(fullPath)} ${GEX44_HOST}:/home/claude/${opts.file}`,
            { encoding: 'utf-8', stdio: 'pipe' },
          );
          scpResult = { scpd: true, target: `/home/claude/${opts.file}` };
        } catch (e: any) {
          scpResult = { scpd: false, error: e.message?.slice(0, 200) };
        }
      }

      return {
        ok: true,
        urgent: opts.urgent,
        branch,
        commit_sha: sha,
        commit_message: opts.message,
        diff_chars: Math.abs(opts.newContent.length - oldContent.length),
        pr_url: opts.urgent ? null : `${REPO_HTTPS}/compare/main...${encodeURIComponent(branch)}?expand=1`,
        gex44_applied: scpResult,
        repo_url: `${REPO_HTTPS}/tree/${opts.urgent ? 'main' : branch}/${opts.file}`,
      };
    } catch (e: any) {
      // tenta limpar
      try {
        execSync('git checkout main && git reset --hard origin/main', {
          cwd: MIRROR_DIR, env: this.gitEnv(), stdio: 'pipe',
        });
      } catch {}
      throw new BadRequestException(`git op failed: ${e.message?.slice(0, 300)}`);
    }
  }
}
