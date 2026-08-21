import { canGrant, type GrantActor } from './grant-check';
import type { ScopeCondition } from './condition-resolver';
import type { RuleInput } from './access.types';

const T = '11111111-1111-1111-1111-111111111111';
const tenantScope: ScopeCondition = { kind: 'where', where: { tenantId: T } };

const operator: GrantActor = { rules: [{ action: 'manage', subject: 'all' }], scope: { kind: 'all' }, isOperator: true };
const owner: GrantActor = { rules: [{ action: 'manage', subject: 'all' }], scope: tenantScope };
const orderReader: GrantActor = { rules: [{ action: 'read', subject: 'Order' }], scope: tenantScope };
const branchA: GrantActor = {
  rules: [{ action: 'read', subject: 'Order', conditions: { branchId: 'A' } }],
  scope: tenantScope,
};

describe('canGrant (não-escalada na delegação)', () => {
  it('operador concede qualquer coisa', () => {
    expect(canGrant(operator, { action: 'manage', subject: 'all' })).toBe(true);
    expect(canGrant(operator, { action: 'delete', subject: 'Order', conditions: { tenantId: 'outro' } })).toBe(true);
  });

  it('dono (manage all) concede dentro do tenant', () => {
    expect(canGrant(owner, { action: 'read', subject: 'Order', conditions: { sellerId: 'u1' } })).toBe(true);
    expect(canGrant(owner, { action: 'manage', subject: 'Student' })).toBe(true);
  });

  it('não concede ação/subject que não detém', () => {
    expect(canGrant(orderReader, { action: 'delete', subject: 'Order' })).toBe(false); // só read
    expect(canGrant(orderReader, { action: 'read', subject: 'Customer' })).toBe(false); // não tem Customer
  });

  it('concede refinamento, mas não pode ampliar', () => {
    // ator: read Order branch=A
    expect(canGrant(branchA, { action: 'read', subject: 'Order', conditions: { branchId: 'A', sellerId: 'u' } })).toBe(true); // mais restrito
    expect(canGrant(branchA, { action: 'read', subject: 'Order', conditions: { branchId: 'B' } })).toBe(false); // outro branch
    expect(canGrant(branchA, { action: 'read', subject: 'Order' })).toBe(false); // sem condição = mais amplo
  });

  it('respeita a profundidade δ (orçamento de re-delegação)', () => {
    const holderD1: GrantActor = {
      rules: [{ action: 'read', subject: 'Order', propagationDepth: 1 }],
      scope: tenantScope,
    };
    expect(canGrant(holderD1, { action: 'read', subject: 'Order', propagationDepth: 0 })).toBe(true);
    expect(canGrant(holderD1, { action: 'read', subject: 'Order', propagationDepth: 1 })).toBe(false); // precisa ≤0

    const holderD0: GrantActor = {
      rules: [{ action: 'read', subject: 'Order', propagationDepth: 0 }],
      scope: tenantScope,
    };
    expect(canGrant(holderD0, { action: 'read', subject: 'Order', propagationDepth: 0 })).toBe(false); // δ=0 não re-delega
  });

  it('subject/ação em lote: todos precisam ser concedíveis', () => {
    expect(canGrant(orderReader, { action: ['read'], subject: ['Order'] })).toBe(true);
    expect(canGrant(orderReader, { action: ['read', 'delete'], subject: ['Order'] })).toBe(false); // delete não
  });
});
