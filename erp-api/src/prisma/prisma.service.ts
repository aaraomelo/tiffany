import {
  Injectable,
  Logger,
  OnModuleDestroy,
  OnModuleInit,
} from '@nestjs/common';
import { Prisma, PrismaClient } from '@prisma/client';
import { getTenantContext } from '../common/tenant-context/tenant-context';
import { createRowLevelSecurityExtension } from './prisma-rls.extension';
import { createRlsPrismaProxy } from './prisma-rls-proxy';

@Injectable()
export class PrismaService
  extends PrismaClient
  implements OnModuleInit, OnModuleDestroy
{
  private readonly logger = new Logger('RLS');

  constructor() {
    super();

    // Envolve o client num Proxy por-request: chamadas a delegates de model são
    // roteadas por um client estendido com a RLS (quando o contexto tem regras,
    // i.e. RLS_MODE ligado); senão passam direto. Métodos/$transação/raw passam
    // direto. Ver doc/casl-propagation.tex §7.
    const proxy = createRlsPrismaProxy<PrismaClient>(this, {
      getContext: getTenantContext,
      buildExtended: (rls, base) =>
        base.$extends(
          Prisma.defineExtension(
            createRowLevelSecurityExtension(rls, base as never),
          ),
        ) as unknown as PrismaClient,
      onShadowDeny: (info) =>
        this.logger.warn(
          `[shadow] negaria ${info.action} ${info.model} (user ${info.userId ?? '-'})`,
        ),
    });
    return proxy as unknown as this;
  }

  async onModuleInit() {
    await this.$connect();
  }

  async onModuleDestroy() {
    await this.$disconnect();
  }
}
