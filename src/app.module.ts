import { Module } from '@nestjs/common';
import { ServeStaticModule } from '@nestjs/serve-static';
import { ScheduleModule } from '@nestjs/schedule';
import { join } from 'path';
import { AppController } from './app.controller';
import { TasksController } from './tasks.controller';
import { TemplatesController } from './templates.controller';
import { WebhookController } from './webhook.controller';
import { TasksService } from './tasks.service';
import { TaskStateMachine } from './task-state-machine';
import { TaskEventsService } from './task-events.service';
import { TaskTimeoutService } from './task-timeout.service';
import { PrismaService } from './prisma.service';

@Module({
  imports: [
    ServeStaticModule.forRoot({
      rootPath: join(__dirname, '..', 'public'),
    }),
    ScheduleModule.forRoot(),
  ],
  controllers: [AppController, TasksController, TemplatesController, WebhookController],
  providers: [PrismaService, TasksService, TaskStateMachine, TaskEventsService, TaskTimeoutService],
})
export class AppModule {}
