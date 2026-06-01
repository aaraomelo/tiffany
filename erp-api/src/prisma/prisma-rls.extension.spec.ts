import { ForbiddenException } from '@nestjs/common';
import {
  createRowLevelSecurityExtension,
  type RlsContext,
} from './prisma-rls.extension';
import type { ScopeCondition } from '../access/condition-resolver';

const T = '11111111-1111-1111-1111-111111111111';
const OTHER = '22222222-2222-2222-2222-222222222222';

const base = {
  _runtimeDataModel: {
    models: {
      Student: { fields: [{ name: 'id' }, { name: 'tenantId' }, { name: 'name' }] },
      Order: { fields: [{ name: 'id' }, { name: 'tenantId' }, { name: 'sellerId' }] },
      // referência global: sem tenantId e sem subject CASL
      ModulePack: { fields: [{ name: 'slug' }, { name: 'name' }] },
    },
  },
};

const tenantScope: ScopeCondition = { kind: 'where', where: { tenantId: T } };
const operatorScope: ScopeCondition = { kind: 'all' };

function ctxOwner(mode: 'shadow' | 'enforce' = 'enforce', onShadowDeny?: jest.Mock): RlsContext {
  return { rules: [{ action: 'manage', subject: 'all' }], inheritedScope: tenantScope, mode, tenantId: T, onShadowDeny };
}
function ctxOperator(): RlsContext {
  return { rules: [{ action: 'manage', subject: 'all' }], inheritedScope: operatorScope, mode: 'enforce', tenantId: null };
}
function ctxAssociate(mode: 'shadow' | 'enforce' = 'enforce', onShadowDeny?: jest.Mock): RlsContext {
  // associado: só lê Order do próprio (sellerId)
  return {
    rules: [{ action: 'read', subject: 'Order', conditions: { sellerId: 'u1' } }],
    inheritedScope: tenantScope,
    mode,
    tenantId: T,
    userId: 'u1',
    onShadowDeny,
  };
}

const run = (ctx: RlsContext, model: string, operation: string, args: Record<string, unknown>) => {
  const ext = createRowLevelSecurityExtension(ctx, base);
  const query = jest.fn(async (a: unknown) => a);
  return { query, result: ext.query.$allModels.$allOperations({ model, operation, args, query }) };
};

describe('prisma-rls.extension', () => {
  it('dono: findMany injeta o piso de tenant no where', async () => {
    const { query } = run(ctxOwner(), 'Student', 'findMany', { where: { name: 'x' } });
    await Promise.resolve();
    expect(query).toHaveBeenCalledWith({ where: { name: 'x', AND: [{ tenantId: T }] } });
  });

  it('operador (escopo ⊤ + manage all): nenhuma injeção (vê tudo)', async () => {
    const { query } = run(ctxOperator(), 'Student', 'findMany', { where: { name: 'x' } });
    await Promise.resolve();
    expect(query).toHaveBeenCalledWith({ where: { name: 'x' } });
  });

  it('associado: grant condicional → AND de tenant + condição do grant', async () => {
    const { query } = run(ctxAssociate(), 'Order', 'findMany', {});
    await Promise.resolve();
    expect(query).toHaveBeenCalledWith({
      where: { AND: [{ AND: [{ tenantId: T }, { sellerId: 'u1' }] }] },
    });
  });

  it('associado sem grant para o subject (enforce) → Forbidden', async () => {
    const { result } = run(ctxAssociate(), 'Student', 'findMany', {});
    await expect(result).rejects.toBeInstanceOf(ForbiddenException);
  });

  it('associado sem grant (shadow) → não nega, aplica só o piso de tenant e loga', async () => {
    const onShadowDeny = jest.fn();
    const { query, result } = run(ctxAssociate('shadow', onShadowDeny), 'Student', 'findMany', {});
    await result;
    expect(onShadowDeny).toHaveBeenCalledWith(expect.objectContaining({ model: 'Student', action: 'read' }));
    expect(query).toHaveBeenCalledWith({ where: { AND: [{ tenantId: T }] } });
  });

  it('create: preenche tenantId quando ausente', async () => {
    const { query } = run(ctxOwner(), 'Student', 'create', { data: { name: 'Ana' } });
    await Promise.resolve();
    expect(query).toHaveBeenCalledWith({ data: { name: 'Ana', tenantId: T } });
  });

  it('create cross-tenant (enforce) → Forbidden', async () => {
    const { result } = run(ctxOwner(), 'Student', 'create', { data: { name: 'Ana', tenantId: OTHER } });
    await expect(result).rejects.toBeInstanceOf(ForbiddenException);
  });

  it('create cross-tenant (shadow) → loga e deixa passar', async () => {
    const onShadowDeny = jest.fn();
    const { query, result } = run(ctxOwner('shadow', onShadowDeny), 'Student', 'create', {
      data: { name: 'Ana', tenantId: OTHER },
    });
    await result;
    expect(onShadowDeny).toHaveBeenCalled();
    expect(query).toHaveBeenCalled();
  });

  it('model global (sem tenantId/sem subject) → passthrough', async () => {
    const { query } = run(ctxOwner(), 'ModulePack', 'findMany', { where: { slug: 's' } });
    await Promise.resolve();
    expect(query).toHaveBeenCalledWith({ where: { slug: 's' } });
  });

  it('findUnique: injeta preservando o seletor único no topo', async () => {
    const { query } = run(ctxOwner(), 'Student', 'findUnique', { where: { id: 'abc' } });
    await Promise.resolve();
    expect(query).toHaveBeenCalledWith({ where: { id: 'abc', AND: [{ tenantId: T }] } });
  });
});
