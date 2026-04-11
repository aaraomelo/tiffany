import { Module } from '@nestjs/common';
import { ServeStaticModule } from '@nestjs/serve-static';
import { ScheduleModule } from '@nestjs/schedule';
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
import { PatriciaGatewayService } from './patricia-gateway.service';
import { PrismaService } from './prisma.service';
import { RequestContextInterceptor } from './request-context.interceptor';
import { AuthModule } from './auth/auth.module';
import { TenantsModule } from './tenants/tenants.module';
import { MessagingModule } from './messaging/messaging.module';

@Module({
  imports: [
    ServeStaticModule.forRoot({
      rootPath: join(__dirname, '..', 'public'),
    }),
    ScheduleModule.forRoot(),
    AuthModule,
    TenantsModule,
    MessagingModule,
  ],
  controllers: [AppController, TasksController, TemplatesController, ProjectsController, WebhookController, ContactsController, PatriciaGatewayController],
  providers: [PrismaService, RequestContextInterceptor, TasksService, TaskStateMachine, TaskEventsService, TaskTimeoutService, DailySummaryService, ProjectsService, ClaudeService, PatriciaGatewayService],
})
export class AppModule {}
