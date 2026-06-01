import type { TenantContext } from '../common/tenant-context/tenant-context';

/**
 * Extensão da RLS NATIVA (Fase B): envolve cada op de model numa transação que
 * faz `SET LOCAL app.tenant_id` (+ bypass), para a policy nativa do PostgreSQL
 * confinar a query — inclusive o piso que pega qualquer rota/raw.
 * Ver doc/casl-propagation.tex §7 e a migration 20260601070000_native_rls.
 *
 * Só tem efeito como role NÃO-superusuário (erp_app) + RLS_NATIVE=on.
 */

// eslint-disable-next-line @typescript-eslint/no-explicit-any
type RawClient = any;

export interface GucParams {
  tenantId: string; // UUID do tenant autenticado, ou '' (→ NULL na policy)
  bypass: 'on' | 'off';
}

/**
 * Parâmetros do GUC a partir do contexto. Regra: só confina quem está
 * AUTENTICADO (userId presente) — login, rotas públicas e webhooks operam sem
 * tenant confinado e usam bypass (escopo explícito do service). Operador =
 * bypass. tenantId só vai pro GUC quando autenticado (é um UUID válido); senão
 * '' (a policy usa NULLIF('','') = NULL, evitando erro de cast).
 */
export function gucParams(ctx: TenantContext): GucParams {
  const authed = !!ctx.userId;
  const tenantId = authed && ctx.tenantId ? ctx.tenantId : '';
  const bypass: 'on' | 'off' =
    ctx.isPlatformOperator || !authed ? 'on' : 'off';
  return { tenantId, bypass };
}

export function createNativeRlsExtension(
  base: RawClient,
  getContext: () => TenantContext,
) {
  return {
    query: {
      $allModels: {
        // eslint-disable-next-line @typescript-eslint/no-explicit-any
        async $allOperations({ args, query }: any) {
          const { tenantId, bypass } = gucParams(getContext());
          const res: unknown[] = await base.$transaction([
            base.$executeRawUnsafe(
              `SELECT set_config('app.tenant_id', $1, true)`,
              tenantId,
            ),
            base.$executeRawUnsafe(
              `SELECT set_config('app.bypass_rls', $1, true)`,
              bypass,
            ),
            query(args),
          ]);
          return res[res.length - 1];
        },
      },
    },
  };
}

/**
 * Helper para raw queries: roda `fn(db)` com o GUC do tenant setado na MESMA
 * transação (senão a RLS nativa bloqueia o raw). Inerte quando RLS_NATIVE!=on
 * (roda direto, sem transação). Os call sites de $queryRaw usam isto.
 */
export async function withTenantScope<T>(
  prisma: RawClient,
  getContext: () => TenantContext,
  fn: (db: RawClient) => Promise<T>,
): Promise<T> {
  if (process.env.RLS_NATIVE !== 'on') return fn(prisma);
  const { tenantId, bypass } = gucParams(getContext());
  return prisma.$transaction(async (tx: RawClient) => {
    await tx.$executeRawUnsafe(
      `SELECT set_config('app.tenant_id', $1, true)`,
      tenantId,
    );
    await tx.$executeRawUnsafe(
      `SELECT set_config('app.bypass_rls', $1, true)`,
      bypass,
    );
    return fn(tx);
  });
}
