import { Injectable, Logger } from '@nestjs/common';
import { Cron, CronExpression } from '@nestjs/schedule';
import { PrismaService } from '../prisma/prisma.service';

/**
 * Marca mensalidades vencidas (PENDING com dueDate no passado) como OVERDUE.
 * Roda fora de contexto de request — varre todos os tenants de uma vez.
 */
@Injectable()
export class TuitionScheduler {
  private readonly logger = new Logger(TuitionScheduler.name);

  constructor(private readonly prisma: PrismaService) {}

  @Cron(CronExpression.EVERY_DAY_AT_1AM, { name: 'tuition-overdue' })
  async markOverdue() {
    const today = new Date();
    today.setHours(0, 0, 0, 0);

    const res = await this.prisma.tuition.updateMany({
      where: { status: 'PENDING', dueDate: { lt: today } },
      data: { status: 'OVERDUE' },
    });

    if (res.count > 0) {
      this.logger.log(`${res.count} mensalidade(s) marcada(s) como OVERDUE`);
    }
  }
}
