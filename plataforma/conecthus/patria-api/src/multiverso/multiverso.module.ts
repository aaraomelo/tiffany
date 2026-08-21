import { Module } from '@nestjs/common';
import { PrismaService } from '../prisma.service';
import { MultiversoService } from './multiverso.service';
import { MultiversoController } from './multiverso.controller';

@Module({
  controllers: [MultiversoController],
  providers: [MultiversoService, PrismaService],
  exports: [MultiversoService],
})
export class MultiversoModule {}
