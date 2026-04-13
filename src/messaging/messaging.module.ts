import { Module, forwardRef } from '@nestjs/common';
import { PrismaService } from '../prisma.service';
import { PeopleModule } from '../people/people.module';
import { WorkerModule } from '../worker/worker.module';
import { PatriciaGatewayService } from '../patricia-gateway.service';
import { TasksService } from '../tasks.service';
import { ProjectsService } from '../projects.service';
import { ClaudeService } from '../claude.service';
import { TaskStateMachine } from '../task-state-machine';
import { TaskEventsService } from '../task-events.service';
import { MessagingService } from './messaging.service';
import { EvolutionApiService } from './evolution-api.service';
import { TelegramService } from './telegram.service';
import { PatriciaLlmService } from './patricia-llm.service';
import { MemoryService } from './memory.service';
import { MessagingWebhookController } from './messaging-webhook.controller';
import { MessagingSenderController } from './messaging-sender.controller';

@Module({
  imports: [PeopleModule, forwardRef(() => WorkerModule)],
  controllers: [MessagingWebhookController, MessagingSenderController],
  providers: [
    MessagingService,
    EvolutionApiService,
    TelegramService,
    PatriciaLlmService,
    MemoryService,
    PrismaService,
    PatriciaGatewayService,
    TasksService,
    ProjectsService,
    ClaudeService,
    TaskStateMachine,
    TaskEventsService,
  ],
  exports: [MessagingService, MemoryService],
})
export class MessagingModule {}
