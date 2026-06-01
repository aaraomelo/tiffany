import type { TenantContext } from '../common/tenant-context/tenant-context';

/**
 * Extensão da RLS NATIVA (Fase B): envolve cada op de model numa transação que
 * faz `SET LOCAL app.tenant_id` (+ bypass do operador), para a policy nativa do
 * PostgreSQL confinar a query — inclusive o piso que pega qualquer rota.
 * Ver doc/casl-propagation.tex §7 e a migration 20260601070000_native_rls.
 *
 * Só tem efeito quando o app conecta como role NÃO-superusuário (erp_app);
 * como superusuário a RLS é ignorada. Atrás da flag RLS_NATIVE.
 */

// eslint-disable-next-line @typescript-eslint/no-explicit-any
type RawClient = any;

/** bypass quando operador de plataforma ou sem tenant ativo (rotas públicas/sistema). */
export function bypassFor(ctx: TenantContext): 'on' | 'off' {
  return ctx.isPlatformOperator || !ctx.tenantId ? 'on' : 'off';
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
          const ctx = getContext();
          const res: unknown[] = await base.$transaction([
            base.$executeRawUnsafe(
              `SELECT set_config('app.tenant_id', $1, true)`,
              ctx.tenantId ?? '',
            ),
            base.$executeRawUnsafe(
              `SELECT set_config('app.bypass_rls', $1, true)`,
              bypassFor(ctx),
            ),
            query(args),
          ]);
          return res[res.length - 1];
        },
      },
    },
  };
}
