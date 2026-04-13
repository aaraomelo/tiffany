import { Injectable, Logger } from '@nestjs/common';
import { execSync } from 'child_process';
import { REPO_DIRS, SH_OPTS, SH_LONG } from './worker.config';

@Injectable()
export class GitService {
  private readonly logger = new Logger('Git');

  getRepoDir(repo: string): string {
    const dir = REPO_DIRS[repo];
    if (!dir) throw new Error(`Unknown repo: ${repo}`);
    return dir;
  }

  gitSync(srcDir: string, branch = 'develop'): void {
    execSync(`cd ${srcDir} && git checkout . && git clean -fd && git checkout ${branch} && git pull origin ${branch}`, SH_OPTS);
  }

  createBranch(srcDir: string, branchName: string, fromBranch = 'develop'): void {
    execSync(`cd ${srcDir} && git checkout . && git clean -fd`, SH_OPTS);
    execSync(`cd ${srcDir} && git checkout ${fromBranch} && git pull origin ${fromBranch}`, SH_OPTS);
    try { execSync(`cd ${srcDir} && git branch -D ${branchName}`, SH_OPTS); } catch {}
    execSync(`cd ${srcDir} && git checkout -b ${branchName}`, SH_OPTS);
    execSync(`cd ${srcDir} && git push -u origin ${branchName} --force-with-lease`, SH_LONG);
    this.logger.log(`Branch created: ${branchName} from ${fromBranch}`);
  }

  mergeBranch(srcDir: string, fromBranch: string, toBranch: string): void {
    execSync(`cd ${srcDir} && git checkout ${toBranch} && git pull origin ${toBranch}`, SH_OPTS);
    execSync(`cd ${srcDir} && git checkout ${fromBranch} && git pull origin ${fromBranch}`, SH_OPTS);
    try {
      execSync(`cd ${srcDir} && git rebase ${toBranch}`, SH_LONG);
      execSync(`cd ${srcDir} && git push origin ${fromBranch} --force-with-lease`, SH_LONG);
      this.logger.log(`Rebased ${fromBranch} on ${toBranch}`);
    } catch {
      try { execSync(`cd ${srcDir} && git rebase --abort`, SH_OPTS); } catch {}
      this.logger.warn(`Rebase failed, falling back to merge`);
      execSync(`cd ${srcDir} && git checkout ${toBranch}`, SH_OPTS);
    }

    execSync(`cd ${srcDir} && git checkout ${toBranch}`, SH_OPTS);
    try {
      execSync(`cd ${srcDir} && git merge ${fromBranch} --no-edit`, SH_OPTS);
    } catch {
      this.logger.warn(`Merge conflict ${fromBranch} → ${toBranch}, resolving...`);
      try {
        execSync(`cd ${srcDir} && git checkout --theirs . && git add -A && git commit --no-edit`, SH_OPTS);
      } catch {
        execSync(`cd ${srcDir} && git merge --abort`, SH_OPTS);
        throw new Error(`Merge conflict ${fromBranch} → ${toBranch} could not be resolved`);
      }
    }
    execSync(`cd ${srcDir} && git push origin ${toBranch}`, SH_LONG);
    this.logger.log(`Merged ${fromBranch} → ${toBranch}`);
  }

  deleteBranch(srcDir: string, branchName: string): void {
    try {
      execSync(`cd ${srcDir} && git push origin --delete ${branchName}`, SH_OPTS);
      execSync(`cd ${srcDir} && git branch -D ${branchName}`, SH_OPTS);
      this.logger.log(`Branch deleted: ${branchName}`);
    } catch {}
  }

  hasChanges(srcDir: string): string {
    return execSync(`cd ${srcDir} && git status --porcelain`, { encoding: 'utf-8', timeout: 10_000, shell: '/bin/sh' }).trim();
  }

  getHeadSha(srcDir: string): string {
    return execSync(`cd ${srcDir} && git rev-parse HEAD`, { encoding: 'utf-8', timeout: 5_000, shell: '/bin/sh' }).trim();
  }

  commitAll(srcDir: string, message: string): void {
    const { writeFileSync, unlinkSync } = require('fs');
    const commitFile = '/tmp/patria-worker-commit.txt';
    writeFileSync(commitFile, message);
    execSync(`cd ${srcDir} && git add -A && git commit -F ${commitFile}`, SH_OPTS);
    unlinkSync(commitFile);
  }

  pushBranch(srcDir: string, branch: string): void {
    execSync(`cd ${srcDir} && git push origin ${branch}`, SH_LONG);
  }

  checkSchemaWithoutMigration(srcDir: string): boolean {
    try {
      const schemaChanged = execSync(`cd ${srcDir} && git diff --name-only HEAD~1 HEAD | grep schema.prisma || true`, { encoding: 'utf-8', shell: '/bin/sh' }).trim();
      const migrationCreated = execSync(`cd ${srcDir} && git diff --name-only HEAD~1 HEAD | grep prisma/migrations/ || true`, { encoding: 'utf-8', shell: '/bin/sh' }).trim();
      return !!(schemaChanged && !migrationCreated);
    } catch {
      return false;
    }
  }

  createAutoMigration(srcDir: string): void {
    execSync(`cd ${srcDir} && npx prisma migrate dev --create-only --name auto_migration`, { stdio: 'pipe', timeout: 30_000, shell: '/bin/sh' });
    execSync(`cd ${srcDir} && git add prisma/ && git commit --amend --no-edit`, SH_OPTS);
    this.logger.log('Migration created and amended to commit');
  }

  syncHomologWithMain(srcDir: string): void {
    try {
      this.mergeBranch(srcDir, 'main', 'homolog');
      this.logger.log('Homolog synced with main');
    } catch (err) {
      this.logger.error(`Failed to sync homolog: ${err.message}`);
    }
  }
}
