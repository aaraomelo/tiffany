import { createRlsPrismaProxy, toRlsContext } from './prisma-rls-proxy';
import type { TenantContext } from '../common/tenant-context/tenant-context';

const T = '11111111-1111-1111-1111-111111111111';

function fakeBase() {
  return {
    student: {
      findMany: jest.fn(async () => 'base-find'),
      findFirst: jest.fn(),
      create: jest.fn(async () => 'base-create'),
    },
    $queryRaw: jest.fn(async () => 'raw'),
    $transaction: jest.fn(),
    _internal: { secret: 1 },
  };
}

describe('toRlsContext', () => {
  it('sem accessRules → null (passa direto)', () => {
    expect(toRlsContext({ tenantId: T, userId: 'u', role: null })).toBeNull();
  });
  it('com regras + tenant → escopo de tenant, modo default shadow', () => {
    const tc: TenantContext = { tenantId: T, userId: 'u', role: null, accessRules: [] };
    const rls = toRlsContext(tc)!;
    expect(rls.inheritedScope).toEqual({ kind: 'where', where: { tenantId: T } });
    expect(rls.mode).toBe('shadow');
  });
  it('operador → escopo all', () => {
    const tc: TenantContext = { tenantId: null, userId: 'op', role: null, accessRules: [], isPlatformOperator: true };
    expect(toRlsContext(tc)!.inheritedScope).toEqual({ kind: 'all' });
  });
});

describe('createRlsPrismaProxy', () => {
  it('buildExtended retorna null → chama o client base (passthrough)', async () => {
    const base = fakeBase();
    const buildExtended = jest.fn(() => null);
    const proxy = createRlsPrismaProxy(base as any, {
      getContext: () => ({ tenantId: T, userId: 'u', role: null }),
      buildExtended,
    });
    const r = await (proxy as any).student.findMany();
    expect(r).toBe('base-find');
    expect(base.student.findMany).toHaveBeenCalled();
    expect(buildExtended).toHaveBeenCalled();
  });

  it('buildExtended retorna client → roteia pelo estendido (e cacheia)', async () => {
    const base = fakeBase();
    const extended = { student: { findMany: jest.fn(async () => 'ext-find') } };
    const buildExtended = jest.fn(() => extended);
    const ctx: TenantContext = { tenantId: T, userId: 'u', role: null, accessRules: [] };
    const proxy = createRlsPrismaProxy(base as any, {
      getContext: () => ctx,
      buildExtended: buildExtended as any,
    });
    const r1 = await (proxy as any).student.findMany();
    const r2 = await (proxy as any).student.findMany();
    expect(r1).toBe('ext-find');
    expect(r2).toBe('ext-find');
    expect(base.student.findMany).not.toHaveBeenCalled();
    expect(buildExtended).toHaveBeenCalledTimes(1); // cacheado por contexto
  });

  it('métodos não-delegate ($queryRaw) passam direto', async () => {
    const base = fakeBase();
    const proxy = createRlsPrismaProxy(base as any, {
      getContext: () => ({ tenantId: T, userId: 'u', role: null, accessRules: [] }),
      buildExtended: () => ({}) as any,
    });
    const r = await (proxy as any).$queryRaw();
    expect(r).toBe('raw');
    expect(base.$queryRaw).toHaveBeenCalled();
  });

  it('objetos internos (não-delegate) passam direto', () => {
    const base = fakeBase();
    const proxy = createRlsPrismaProxy(base as any, {
      getContext: () => ({ tenantId: T, userId: 'u', role: null }),
      buildExtended: () => null,
    });
    expect((proxy as any)._internal).toBe(base._internal);
  });
});
