import { Injectable } from '@nestjs/common';
import { PrismaService } from './prisma.service';
import { readFileSync } from 'fs';
import { join } from 'path';

@Injectable()
export class AppService {
  private readonly startTime: Date;

  constructor(private prisma: PrismaService) {
    this.startTime = new Date();
  }

  async getDetailedHealth() {
    const pkg = JSON.parse(readFileSync(join(__dirname, '..', 'package.json'), 'utf-8'));
    const uptime_seconds = Math.floor((Date.now() - this.startTime.getTime()) / 1000);

    const grouped = await this.prisma.task.groupBy({
      by: ['status'],
      _count: { status: true },
    });

    const by_status: Record<string, number> = {};
    let total = 0;
    for (const row of grouped) {
      by_status[row.status] = row._count.status;
      total += row._count.status;
    }

    let dbStatus: 'connected' | 'disconnected' = 'disconnected';
    try {
      await this.prisma.$queryRaw`SELECT 1`;
      dbStatus = 'connected';
    } catch {
      dbStatus = 'disconnected';
    }

    return {
      version: pkg.version,
      uptime_seconds,
      tasks: { total, by_status },
      database: { status: dbStatus },
    };
  }
}
