import { Module } from '@nestjs/common';
import { ServeStaticModule } from '@nestjs/serve-static';
import { ScheduleModule } from '@nestjs/schedule';
import { join } from 'path';
import { AppController } from './app.controller';
import { TasksController } from './tasks.controller';
import { TemplatesController } from './templates.controller';
import { ProjectsController } from './projects.controller';
import { WebhookController } from './webhook.controller';
import { TasksService } from './tasks.service';
import { TaskStateMachine } from './task-state-machine';
import { TaskEventsService } from './task-events.service';
import { TaskTimeoutService } from './task-timeout.service';
import { DailySummaryService } from './daily-summary.service';
import { ProjectsService } from './projects.service';
import { PrismaService } from './prisma.service';

@Module({
  imports: [
    ServeStaticModule.forRoot({
      rootPath: join(__dirname, '..', 'public'),
    }),
    ScheduleModule.forRoot(),
  ],
  controllers: [AppController, TasksController, TemplatesController, ProjectsController, WebhookController],
  providers: [PrismaService, TasksService, TaskStateMachine, TaskEventsService, TaskTimeoutService, DailySummaryService, ProjectsService],
})
export class AppModule {}
