import {
  Injectable,
  Logger,
  OnModuleDestroy,
  OnModuleInit,
} from '@nestjs/common';
import { Prisma, PrismaClient } from '@prisma/client';
import { getTenantContext } from '../common/tenant-context/tenant-context';
import { createRowLevelSecurityExtension } from './prisma-rls.extension';
import { createNativeRlsExtension, gucParams } from './prisma-rls-native';
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
      // Seta o GUC no início de cada $transaction (o tx interativo não passa
      // pela extensão). Só quando RLS_NATIVE=on; senão usa o $transaction real.
      wrapTransaction: (base) => {
        if (process.env.RLS_NATIVE !== 'on') return null;
        const tx = (base as PrismaClient).$transaction.bind(base as PrismaClient);
        // eslint-disable-next-line @typescript-eslint/no-explicit-any
        return (arg: any, options?: any) => {
          const { tenantId, bypass } = gucParams(getTenantContext());
          // eslint-disable-next-line @typescript-eslint/no-explicit-any
          const setT = (db: any) =>
            db.$executeRawUnsafe(`SELECT set_config('app.tenant_id', $1, true)`, tenantId);
          // eslint-disable-next-line @typescript-eslint/no-explicit-any
          const setB = (db: any) =>
            db.$executeRawUnsafe(`SELECT set_config('app.bypass_rls', $1, true)`, bypass);
          if (typeof arg === 'function') {
            // eslint-disable-next-line @typescript-eslint/no-explicit-any
            return tx(async (t: any) => {
              await setT(t);
              await setB(t);
              return arg(t);
            }, options);
          }
          // forma array: prefixa os set_config e descarta seus 2 resultados
          return tx([setT(base), setB(base), ...arg], options).then(
            // eslint-disable-next-line @typescript-eslint/no-explicit-any
            (res: any[]) => res.slice(2),
          );
        };
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
