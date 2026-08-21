import { resolveCondition, type ScopeCondition } from './condition-resolver';
import type { RuleInput } from './access.types';

const TENANT = '11111111-1111-1111-1111-111111111111';
const tenantScope: ScopeCondition = { kind: 'where', where: { tenantId: TENANT } };
const operatorScope: ScopeCondition = { kind: 'all' };

const manageAll: RuleInput = { action: 'manage', subject: 'all' };

describe('condition-resolver (propagação c_eff)', () => {
  it('operador (escopo ⊤) + manage all → allow-all (vê tudo)', () => {
    const r = resolveCondition({
      subject: 'Student',
      action: 'read',
      rules: [manageAll],
      inheritedScope: operatorScope,
      isTenantOwned: true,
    });
    expect(r.effective).toEqual({ kind: 'allow-all' });
  });

  it('dono (escopo tenant) + manage all → confinado ao tenant', () => {
    const r = resolveCondition({
      subject: 'Student',
      action: 'create',
      rules: [manageAll],
      inheritedScope: tenantScope,
      isTenantOwned: true,
    });
    // meet(where{tenantId}, allow-all) = where{tenantId}
    expect(r.effective).toEqual({ kind: 'where', where: { tenantId: TENANT } });
  });

  it('grant com condição → AND com o escopo herdado (refinamento)', () => {
    const r = resolveCondition({
      subject: 'Order',
      action: 'read',
      rules: [{ action: 'read', subject: 'Order', conditions: { sellerId: 'u1' } }],
      inheritedScope: tenantScope,
      isTenantOwned: true,
    });
    expect(r.effective).toEqual({
      kind: 'where',
      where: { AND: [{ tenantId: TENANT }, { sellerId: 'u1' }] },
    });
  });

  it('sem grant positivo → deny (bottom); scopeOnly preserva o tenant; wouldDeny=true', () => {
    const r = resolveCondition({
      subject: 'Order',
      action: 'read',
      rules: [{ action: 'read', subject: 'Student' }], // grant de outro subject
      inheritedScope: tenantScope,
      isTenantOwned: true,
    });
    expect(r.effective.kind).toBe('deny');
    expect(r.scopeOnly).toEqual({ kind: 'where', where: { tenantId: TENANT } });
    expect(r.wouldDeny).toBe(true);
  });

  it('múltiplos grants positivos → OR (união, não interseção)', () => {
    const rules: RuleInput[] = [
      { action: 'read', subject: 'Order', conditions: { branchId: 'A' } },
      { action: 'read', subject: 'Order', conditions: { branchId: 'B' } },
    ];
    const r = resolveCondition({
      subject: 'Order',
      action: 'read',
      rules,
      inheritedScope: tenantScope,
      isTenantOwned: true,
    });
    expect(r.effective).toEqual({
      kind: 'where',
      where: { AND: [{ tenantId: TENANT }, { OR: [{ branchId: 'A' }, { branchId: 'B' }] }] },
    });
  });

  it('proibição (inverted) subtrai com AND NOT', () => {
    const rules: RuleInput[] = [
      { action: 'manage', subject: 'all' },
      { action: 'read', subject: 'Tuition', conditions: { status: 'CANCELLED' }, inverted: true },
    ];
    const r = resolveCondition({
      subject: 'Tuition',
      action: 'read',
      rules,
      inheritedScope: tenantScope,
      isTenantOwned: true,
    });
    // manage all → allow-all, menos NOT{status:CANCELLED}, sob o tenant
    expect(r.effective).toEqual({
      kind: 'where',
      where: { AND: [{ tenantId: TENANT }, { NOT: { status: 'CANCELLED' } }] },
    });
  });

  it('model sem subject CASL, mas de tenant → só o escopo herdado confina', () => {
    const r = resolveCondition({
      subject: null,
      action: 'read',
      rules: [],
      inheritedScope: tenantScope,
      isTenantOwned: true,
    });
    expect(r.effective).toEqual({ kind: 'where', where: { tenantId: TENANT } });
    expect(r.wouldDeny).toBe(false);
  });

  it('model global (sem tenantId) e sem subject → allow-all (referência)', () => {
    const r = resolveCondition({
      subject: null,
      action: 'read',
      rules: [],
      inheritedScope: tenantScope,
      isTenantOwned: false,
    });
    expect(r.effective).toEqual({ kind: 'allow-all' });
  });

  it('monotonicidade: o efetivo nunca é mais amplo que o scopeOnly', () => {
    // com grant condicional, o efetivo carrega o tenant + refinamento;
    // scopeOnly é só o tenant → efetivo ⊆ scopeOnly (estruturalmente AND)
    const r = resolveCondition({
      subject: 'Order',
      action: 'update',
      rules: [{ action: 'update', subject: 'Order', conditions: { sellerId: 'u1' } }],
      inheritedScope: tenantScope,
      isTenantOwned: true,
    });
    expect(r.effective.kind).toBe('where');
    if (r.effective.kind === 'where') {
      expect(r.effective.where).toHaveProperty('AND');
      // o primeiro fator do AND é exatamente o scopeOnly (tenant)
      expect((r.effective.where as { AND: unknown[] }).AND[0]).toEqual({ tenantId: TENANT });
    }
  });
});
