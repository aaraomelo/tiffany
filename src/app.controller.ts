import { Controller, Get } from '@nestjs/common';
import { readFileSync } from 'fs';
import { join } from 'path';
import { RequestContext } from './request-context';
import { PrismaService } from './prisma.service';

@Controller('api')
export class AppController {
  constructor(private readonly prisma: PrismaService) {}
  @Get()
  getRoot() {
    return { name: 'patria-api', client: RequestContext.alias, status: 'ok' };
  }

  @Get('health')
  getHealth() {
    return { status: 'healthy', timestamp: new Date().toISOString() };
  }

  @Get('health/detailed')
  async getHealthDetailed() {
    const pkg = JSON.parse(
      readFileSync(join(__dirname, '..', 'package.json'), 'utf-8'),
    );
    let db: { status: 'connected' | 'disconnected'; taskCount: number | null };
    try {
      await this.prisma.$queryRaw`SELECT 1`;
      const taskCount = await this.prisma.task.count();
      db = { status: 'connected', taskCount };
    } catch {
      db = { status: 'disconnected', taskCount: null };
    }
    return { version: pkg.version, uptime: process.uptime(), db };
  }
}
