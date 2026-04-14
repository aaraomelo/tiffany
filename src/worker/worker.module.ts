import { Module, forwardRef } from '@nestjs/common';
import { WorkerService } from './worker.service';
import { ClaudeCliService } from './claude-cli.service';
import { GitService } from './git.service';
import { EmbeddingService } from './embedding.service';
import { TaskExecutionService } from './task-execution.service';
import { ProjectExecutionService } from './project-execution.service';
import { PromotionService } from './promotion.service';
import { DeployMonitorService } from './deploy-monitor.service';
import { PrismaService } from '../prisma.service';
import { MatcherService } from '../matcher.service';
import { MessagingModule } from '../messaging/messaging.module';

@Module({
  imports: [forwardRef(() => MessagingModule)],
  providers: [
    PrismaService,
    MatcherService,
    WorkerService,
    ClaudeCliService,
    GitService,
    EmbeddingService,
    TaskExecutionService,
    ProjectExecutionService,
    PromotionService,
    DeployMonitorService,
  ],
  exports: [ClaudeCliService, EmbeddingService, GitService],
})
export class WorkerModule {}
