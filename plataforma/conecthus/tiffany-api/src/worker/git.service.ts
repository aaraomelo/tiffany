import { Injectable, Logger } from '@nestjs/common';
import { bridgeCall } from './bridge-client';
import { REPO_DIRS } from './worker.config';

@Injectable()
export class GitService {
  private readonly logger = new Logger('Git');

  getRepoDir(repo: string): string {
    const dir = REPO_DIRS[repo];
    if (!dir) throw new Error(`Unknown repo: ${repo}`);
    return dir;
  }

  async gitSync(srcDir: string, branch = 'develop'): Promise<void> {
    await bridgeCall('/git/sync', { dir: srcDir, branch }).catch((err) => {
      this.logger.error(`gitSync failed for ${srcDir} (${branch}): ${err.message}`);
      throw err;
    });
  }

  async createBranch(srcDir: string, branchName: string, fromBranch = 'develop'): Promise<void> {
    await bridgeCall('/git/create-branch', { dir: srcDir, branch: branchName, from: fromBranch }).catch((err) => {
      this.logger.error(`createBranch failed: ${branchName} from ${fromBranch} in ${srcDir}: ${err.message}`);
      throw err;
    });
    this.logger.log(`Branch created: ${branchName} from ${fromBranch}`);
  }

  async mergeBranch(srcDir: string, fromBranch: string, toBranch: string): Promise<void> {
    await bridgeCall('/git/merge', { dir: srcDir, from: fromBranch, to: toBranch }, 90_000).catch((err) => {
      this.logger.error(`mergeBranch failed: ${fromBranch} → ${toBranch} in ${srcDir}: ${err.message}`);
      throw err;
    });
    this.logger.log(`Merged ${fromBranch} → ${toBranch}`);
  }

  async deleteBranch(srcDir: string, branchName: string): Promise<void> {
    await bridgeCall('/git/delete-branch', { dir: srcDir, branch: branchName }).catch((err) => {
      this.logger.warn(`deleteBranch failed: ${branchName}: ${err.message}`);
    });
    this.logger.log(`Branch deleted: ${branchName}`);
  }

  async hasChanges(srcDir: string): Promise<string> {
    const data = await bridgeCall<{ changes: string }>('/git/has-changes', { dir: srcDir }).catch((err) => {
      this.logger.error(`hasChanges failed: ${err.message}`);
      throw err;
    });
    return data.changes;
  }

  async getHeadSha(srcDir: string): Promise<string> {
    const data = await bridgeCall<{ sha: string }>('/git/head-sha', { dir: srcDir }).catch((err) => {
      this.logger.error(`getHeadSha failed: ${err.message}`);
      throw err;
    });
    return data.sha;
  }

  async commitAll(srcDir: string, message: string): Promise<void> {
    await bridgeCall('/git/commit', { dir: srcDir, message }).catch((err) => {
      this.logger.error(`commitAll failed: ${err.message}`);
      throw err;
    });
  }

  async pushBranch(srcDir: string, branch: string): Promise<void> {
    await bridgeCall('/git/push', { dir: srcDir, branch }, 90_000).catch((err) => {
      this.logger.error(`pushBranch failed: ${branch}: ${err.message}`);
      throw err;
    });
  }

  async checkSchemaWithoutMigration(srcDir: string): Promise<boolean> {
    const data = await bridgeCall<{ needsMigration: boolean }>('/git/check-schema', { dir: srcDir }).catch(() => ({ needsMigration: false }));
    return data.needsMigration;
  }

  async createAutoMigration(srcDir: string): Promise<void> {
    await bridgeCall('/git/create-migration', { dir: srcDir }, 60_000).catch((err) => {
      this.logger.error(`createAutoMigration failed: ${err.message}`);
      throw err;
    });
    this.logger.log('Migration created and amended to commit');
  }

  async syncHomologWithMain(srcDir: string): Promise<void> {
    try {
      await this.mergeBranch(srcDir, 'main', 'homolog');
      this.logger.log('Homolog synced with main');
    } catch (err) {
      this.logger.error(`Failed to sync homolog: ${err.message}`);
    }
  }
}
