import { AsyncLocalStorage } from 'node:async_hooks';
import type { RuleInput } from '../../access/access.types';
import type { ScopeCondition } from '../../access/condition-resolver';

export interface TenantContext {
  tenantId: string | null;
  userId: string | null;
  role: string | null;
  // Carregados pela RLS quando RLS_MODE está ligado (senão undefined → a
  // PrismaService passa direto, sem extensão). Ver prisma-rls.extension.ts.
  accessRules?: RuleInput[];
  inheritedScope?: ScopeCondition;
  isPlatformOperator?: boolean;
  rlsMode?: 'shadow' | 'enforce';
}

export const tenantContextStorage = new AsyncLocalStorage<TenantContext>();

export function getTenantContext(): TenantContext {
  return (
    tenantContextStorage.getStore() ?? {
      tenantId: null,
      userId: null,
      role: null,
    }
  );
}

export function requireTenantId(): string {
  const ctx = getTenantContext();
  if (!ctx.tenantId) {
    throw new Error('TenantContext.tenantId is required but missing');
  }
  return ctx.tenantId;
}
