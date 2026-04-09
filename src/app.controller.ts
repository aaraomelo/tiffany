import { Controller, Get } from '@nestjs/common';
import { RequestContext } from './request-context';
import { DailySummaryService } from './daily-summary.service';
import { PrismaService } from './prisma.service';

@Controller('api')
export class AppController {
  constructor(private dailySummary: DailySummaryService, private prisma: PrismaService) {}

  @Get()
  getRoot() {
    return { name: 'patria-api', client: RequestContext.alias, status: 'ok' };
  }

  @Get('health')
  async getHealth() {
    const version = require('../package.json').version;
    const uptime = process.uptime();
    const taskCount = await this.prisma.task.count();

    let dbStatus: string;
    try {
      await this.prisma.$queryRaw`SELECT 1`;
      dbStatus = 'connected';
    } catch {
      dbStatus = 'disconnected';
    }

    return {
      status: 'healthy',
      timestamp: new Date().toISOString(),
      version,
      uptime,
      taskCount,
      db: dbStatus,
    };
  }

  @Get('status')
  getStatus() {
    return this.dailySummary.getSummary();
  }
}
