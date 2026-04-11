import { Module } from '@nestjs/common';
import { JwtModule } from '@nestjs/jwt';
import { TenantsController } from './tenants.controller';
import { TenantsService } from './tenants.service';
import { PrismaService } from '../prisma.service';

@Module({
  imports: [
    JwtModule.register({
      secret: process.env.JWT_SECRET || 'jwt_secret_patria_2026',
      signOptions: { expiresIn: '7d' },
    }),
  ],
  controllers: [TenantsController],
  providers: [TenantsService, PrismaService],
  exports: [TenantsService],
})
export class TenantsModule {}
