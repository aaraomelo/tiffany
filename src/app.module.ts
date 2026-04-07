import { Module } from '@nestjs/common';
import { ServeStaticModule } from '@nestjs/serve-static';
import { join } from 'path';
import { AppController } from './app.controller';
import { TasksController } from './tasks.controller';
import { WebhookController } from './webhook.controller';
import { TasksService } from './tasks.service';
import { TaskStateMachine } from './task-state-machine';
import { PrismaService } from './prisma.service';

@Module({
  imports: [
    ServeStaticModule.forRoot({
      rootPath: join(__dirname, '..', 'public'),
    }),
  ],
  controllers: [AppController, TasksController, WebhookController],
  providers: [PrismaService, TasksService, TaskStateMachine],
})
export class AppModule {}
