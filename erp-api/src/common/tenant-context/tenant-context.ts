import { AsyncLocalStorage } from 'node:async_hooks';

export interface TenantContext {
  tenantId: string | null;
  userId: string | null;
  role: string | null;
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
