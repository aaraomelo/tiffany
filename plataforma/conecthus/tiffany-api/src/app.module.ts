import { Module } from '@nestjs/common';
import { APP_GUARD } from '@nestjs/core';
import { ServeStaticModule } from '@nestjs/serve-static';
import { ScheduleModule } from '@nestjs/schedule';
import { ApiKeyGuard } from './api-key.guard';
import { join } from 'path';
import { AppController } from './app.controller';
import { TasksController } from './tasks.controller';
import { TemplatesController } from './templates.controller';
import { ProjectsController } from './projects.controller';
import { WebhookController } from './webhook.controller';
import { ContactsController } from './contacts.controller';
import { TasksService } from './tasks.service';
import { TaskStateMachine } from './task-state-machine';
import { TaskEventsService } from './task-events.service';
import { TaskTimeoutService } from './task-timeout.service';
import { DailySummaryService } from './daily-summary.service';
import { ProjectsService } from './projects.service';
import { ClaudeService } from './claude.service';
import { PatriciaGatewayController } from './patricia-gateway.controller';
import { NcoController } from './nco.controller';
import { PapersController } from './papers.controller';
import { OutboundController } from "./outbound.controller";
import { LLMController } from "./llm.controller";
import { I18nController } from "./i18n.controller";
import { BillingModule } from "./billing/billing.module";
import { EmailService } from './email.service';
import { PatriciaGatewayService } from './patricia-gateway.service';
import { PrismaService } from './prisma.service';
import { RequestContextInterceptor } from './request-context.interceptor';
import { AuthModule } from './auth/auth.module';
import { TenantsModule } from './tenants/tenants.module';
import { MessagingModule } from './messaging/messaging.module';
import { PeopleModule } from './people/people.module';
import { WorkerModule } from './worker/worker.module';
import { ClaudeBrainModule } from './claude-brain/claude-brain.module';
import { OrganismEventsModule } from './organism-events/organism-events.module';
import { MultiversoModule } from './multiverso/multiverso.module';
import { ProfileService } from './profile.service';
import { MatcherService } from './matcher.service';

@Module({
  imports: [
    ServeStaticModule.forRoot({
      rootPath: join(__dirname, '..', 'public'),
    }),
    ScheduleModule.forRoot(),
    AuthModule,
    TenantsModule,
    MessagingModule,
    PeopleModule,
    WorkerModule,
    ClaudeBrainModule,
    OrganismEventsModule,
    MultiversoModule,
    BillingModule,
  ],
  controllers: [
    AppController,
    TasksController,
    TemplatesController,
    ProjectsController,
    WebhookController,
    ContactsController,
    PatriciaGatewayController,
    NcoController,
    PapersController,
    I18nController,
    LLMController,
    OutboundController,
  ],
  providers: [
    { provide: APP_GUARD, useClass: ApiKeyGuard },
    PrismaService,
    RequestContextInterceptor,
    EmailService,
    TasksService,
    TaskStateMachine,
    TaskEventsService,
    TaskTimeoutService,
    DailySummaryService,
    ProjectsService,
    ClaudeService,
    PatriciaGatewayService,
    ProfileService,
    MatcherService,
  ],
})
export class AppModule {}
