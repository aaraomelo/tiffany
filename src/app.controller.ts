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
    let dbStatus = 'disconnected';
    let taskCount = 0;
    try {
      await this.prisma.$queryRaw`SELECT 1`;
      dbStatus = 'connected';
      taskCount = await this.prisma.task.count();
    } catch {
      // db remains disconnected
    }
    return {
      version: require('../package.json').version,
      uptime: process.uptime(),
      db: {
        status: dbStatus,
        taskCount,
      },
    };
  }
}
