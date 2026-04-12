import { Module } from '@nestjs/common';
import { PeopleController } from './people.controller';
import { PeopleService } from './people.service';
import { PrismaService } from '../prisma.service';

@Module({
  controllers: [PeopleController],
  providers: [PeopleService, PrismaService],
  exports: [PeopleService],
})
export class PeopleModule {}
