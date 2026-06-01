import {
  Injectable,
  Logger,
  OnModuleDestroy,
  OnModuleInit,
} from '@nestjs/common';
import { Prisma, PrismaClient } from '@prisma/client';
import { getTenantContext } from '../common/tenant-context/tenant-context';
import { createRowLevelSecurityExtension } from './prisma-rls.extension';
import { createNativeRlsExtension } from './prisma-rls-native';
import { createRlsPrismaProxy, toRlsContext } from './prisma-rls-proxy';

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
      buildExtended: (tc, base) => {
        // app-level (RLS_MODE): injeção de where dirigida por CASL
        const rls = toRlsContext(tc, (info) =>
          this.logger.warn(
            `[shadow] negaria ${info.action} ${info.model} (user ${info.userId ?? '-'})`,
          ),
        );
        const nativeOn = process.env.RLS_NATIVE === 'on';
        if (!rls && !nativeOn) return null; // nada ativo → passa direto

        let client = base as PrismaClient;
        if (rls) {
          client = client.$extends(
            Prisma.defineExtension(
              createRowLevelSecurityExtension(rls, base as never),
            ),
          ) as unknown as PrismaClient;
        }
        // Fase B: piso nativo — seta o GUC do tenant por op (TODA query, mesmo
        // não-autenticada). Inerte sem RLS_NATIVE=on / como superusuário.
        if (nativeOn) {
          client = client.$extends(
            Prisma.defineExtension(
              createNativeRlsExtension(base, getTenantContext),
            ),
          ) as unknown as PrismaClient;
        }
        return client;
      },
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
