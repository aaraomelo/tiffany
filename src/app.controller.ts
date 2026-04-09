import { Controller, Get } from '@nestjs/common';
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
    const version = require('../../package.json').version;
    const uptime = process.uptime();

    let status = 'disconnected';
    let taskCount = 0;
    try {
      await this.prisma.$queryRaw`SELECT 1`;
      status = 'connected';
      taskCount = await this.prisma.task.count();
    } catch {
      // status remains 'disconnected'
    }

    return { version, uptime, database: { status, taskCount } };
  }
}
