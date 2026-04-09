import { Controller, Get } from '@nestjs/common';
import * as fs from 'fs';
import * as path from 'path';
import { RequestContext } from './request-context';
import { DailySummaryService } from './daily-summary.service';
import { PrismaService } from './prisma.service';

@Controller('api')
export class AppController {
  private startTime = Date.now();

  constructor(
    private dailySummary: DailySummaryService,
    private prisma: PrismaService,
  ) {}

  @Get()
  getRoot() {
    return { name: 'patria-api', client: RequestContext.alias, status: 'ok' };
  }

  @Get('health')
  async getHealth() {
    const pkg = JSON.parse(
      fs.readFileSync(path.join(__dirname, '..', 'package.json'), 'utf8'),
    );
    const version = pkg.version;

    let dbStatus: 'connected' | 'disconnected' = 'disconnected';
    let taskCount = 0;
    try {
      await this.prisma.$queryRaw`SELECT 1`;
      dbStatus = 'connected';
      taskCount = await this.prisma.task.count();
    } catch {
      dbStatus = 'disconnected';
    }

    return {
      status: 'healthy',
      version,
      uptimeSeconds: Math.floor((Date.now() - this.startTime) / 1000),
      database: { status: dbStatus },
      tasks: { total: taskCount },
    };
  }

  @Get('status')
  getStatus() {
    return this.dailySummary.getSummary();
  }
}
