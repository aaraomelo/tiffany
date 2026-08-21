import { Module } from '@nestjs/common';
import { PrismaService } from '../prisma.service';
import { OrganismEventsService } from './organism-events.service';
import { OrganismEventsController } from './organism-events.controller';
import { OrganismWebhookController } from './organism-webhook.controller';

@Module({
  controllers: [OrganismEventsController, OrganismWebhookController],
  providers: [OrganismEventsService, PrismaService],
  exports: [OrganismEventsService],
})
export class OrganismEventsModule {}
