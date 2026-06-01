import type { TenantContext } from '../common/tenant-context/tenant-context';
import type { ScopeCondition } from '../access/condition-resolver';
import type { RlsContext } from './prisma-rls.extension';

/**
 * Proxy por-request sobre o PrismaClient (modelo do easypay-api, adaptado):
 * intercepta chamadas aos delegates de model (prisma.student.findMany) e as
 * roteia por um client estendido com a RLS, construído sob demanda e cacheado
 * por request. Métodos (raw queries, transações, connect) e internos passam
 * direto. Se o contexto não tem regras (RLS_MODE desligado), também passa
 * direto — o push fica inerte até ligarmos no servidor.
 */

export interface RlsProxyDeps<T extends object> {
  getContext: () => TenantContext;
  buildExtended: (rls: RlsContext, base: T) => T;
  onShadowDeny?: RlsContext['onShadowDeny'];
}

/** Constrói a RlsContext a partir do TenantContext, ou null (→ passa direto). */
export function toRlsContext(
  tc: TenantContext,
  onShadowDeny?: RlsContext['onShadowDeny'],
): RlsContext | null {
  if (!tc.accessRules) return null;
  const inheritedScope: ScopeCondition =
    tc.inheritedScope ??
    (tc.isPlatformOperator || !tc.tenantId
      ? { kind: 'all' }
      : { kind: 'where', where: { tenantId: tc.tenantId } });
  return {
    rules: tc.accessRules,
    inheritedScope,
    mode: tc.rlsMode ?? 'shadow',
    tenantId: tc.tenantId,
    userId: tc.userId,
    onShadowDeny,
  };
}

/** Um delegate de model do Prisma tem findMany/findFirst. */
function isModelDelegate(o: unknown): boolean {
  return (
    !!o &&
    typeof o === 'object' &&
    typeof (o as Record<string, unknown>).findMany === 'function' &&
    typeof (o as Record<string, unknown>).findFirst === 'function'
  );
}

export function createRlsPrismaProxy<T extends object>(
  base: T,
  deps: RlsProxyDeps<T>,
): T {
  const cache = new WeakMap<object, T>();

  return new Proxy(base, {
    get(target, prop, receiver) {
      const original = Reflect.get(target, prop, receiver);
      if (!isModelDelegate(original)) return original;

      const modelName = prop as string;
      return new Proxy(original as object, {
        get(modelTarget, modelProp) {
          const modelOriginal = Reflect.get(modelTarget, modelProp);
          if (typeof modelOriginal !== 'function') return modelOriginal;

          return (...args: unknown[]) => {
            const tc = deps.getContext();
            const rls = toRlsContext(tc, deps.onShadowDeny);
            if (!rls) {
              return (modelOriginal as (...a: unknown[]) => unknown).apply(
                modelTarget,
                args,
              );
            }
            let extended = cache.get(tc as object);
            if (!extended) {
              extended = deps.buildExtended(rls, base);
              cache.set(tc as object, extended);
            }
            const extModel = (extended as Record<string, unknown>)[modelName];
            const extMethod = (extModel as Record<string, unknown>)[
              modelProp as string
            ] as (...a: unknown[]) => unknown;
            return extMethod.apply(extModel, args);
          };
        },
      });
    },
  });
}
