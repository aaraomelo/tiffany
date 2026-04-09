import { Controller, Get } from '@nestjs/common';
import * as fs from 'fs';
import * as path from 'path';
import { RequestContext } from './request-context';
import { DailySummaryService } from './daily-summary.service';
import { PrismaService } from './prisma.service';

@Controller('api')
export class AppController {
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
    const pkg = JSON.parse(fs.readFileSync(path.join(process.cwd(), 'package.json'), 'utf8'));

    let task_count = 0;
    let db_status = 'ok';
    try {
      await this.prisma.$queryRaw`SELECT 1`;
      task_count = await this.prisma.task.count();
    } catch {
      db_status = 'error';
    }

    return {
      status: 'healthy',
      timestamp: new Date().toISOString(),
      version: pkg.version,
      uptime: process.uptime(),
      task_count,
      db_status,
    };
  }

  @Get('status')
  getStatus() {
    return this.dailySummary.getSummary();
  }
}
