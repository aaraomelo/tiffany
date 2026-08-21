import type { Action, RuleInput, Subject } from './access.types';

/**
 * Resolver de condição efetiva (c_eff) do modelo de propagação.
 *
 * c_eff(s,a) = (escopo herdado da hierarquia) ∧ ( ⋃ grants ∖ ⋃ proibições )
 *
 * Ver doc/casl-propagation.tex §3–§5. A extensão RLS empurra o resultado para
 * o `where` do Prisma. O resolver é puro (sem I/O) para ser 100% testável.
 */

// Condição vinda dos ancestrais (no backbone: do contexto do tenant).
export type ScopeCondition =
  | { kind: 'all' } // operador no topo: extent = R
  | { kind: 'where'; where: PrismaWhere };

export type PrismaWhere = Record<string, unknown>;

export type Resolved =
  | { kind: 'allow-all' } // sem restrição de linha
  | { kind: 'deny'; reason: string } // bottom: extent vazio
  | { kind: 'where'; where: PrismaWhere };

export interface ResolveInput {
  /** subject CASL do model consultado; null se o model não é protegido por CASL */
  subject: Subject | null;
  action: Action;
  /** regras (achatadas) do principal */
  rules: RuleInput[];
  /** condição herdada da hierarquia (tenant, no backbone) */
  inheritedScope: ScopeCondition;
  /** o model tem coluna tenantId? (confina mesmo sem subject CASL) */
  isTenantOwned: boolean;
}

export interface ResolveResult {
  /** resultado estrito do modelo (usado em modo enforce) */
  effective: Resolved;
  /** só o escopo herdado — fallback seguro usado em modo shadow */
  scopeOnly: Resolved;
  /** true quando o modelo estrito negaria mas o subject é protegido (p/ log shadow) */
  wouldDeny: boolean;
}

function asArray<T>(v: T | T[] | null | undefined): T[] {
  if (v == null) return [];
  return Array.isArray(v) ? v : [v];
}

function ruleMatches(rule: RuleInput, subject: Subject, action: Action): boolean {
  const subjects = asArray(rule.subject);
  const actions = asArray(rule.action);
  const subjOk = subjects.includes(subject) || subjects.includes('all');
  const actOk = actions.includes(action) || actions.includes('manage');
  return subjOk && actOk;
}

function scopeToResolved(scope: ScopeCondition): Resolved {
  return scope.kind === 'all'
    ? { kind: 'allow-all' }
    : { kind: 'where', where: scope.where };
}

/** AND de dois Resolved (meet na álgebra). allow-all é neutro; deny é absorvente. */
function meet(a: Resolved, b: Resolved): Resolved {
  if (a.kind === 'deny') return a;
  if (b.kind === 'deny') return b;
  if (a.kind === 'allow-all') return b;
  if (b.kind === 'allow-all') return a;
  return { kind: 'where', where: { AND: [a.where, b.where] } };
}

/**
 * Camada CASL para (subject, action): ⋃ grants ∖ ⋃ proibições.
 * - sem grant positivo → deny (bottom)
 * - grant incondicional (ex.: manage all sem conditions) → allow-all
 * - grants com conditions → OR; proibições → AND NOT
 */
function grantLayer(
  rules: RuleInput[],
  subject: Subject,
  action: Action,
): Resolved {
  const matching = rules.filter((r) => ruleMatches(r, subject, action));
  const positives = matching.filter((r) => !r.inverted);
  const negatives = matching.filter((r) => r.inverted);

  if (positives.length === 0) {
    return { kind: 'deny', reason: `sem grant para ${action} ${subject}` };
  }

  const hasUnconditionalGrant = positives.some(
    (r) => r.conditions == null || Object.keys(r.conditions).length === 0,
  );
  const positiveConds = positives
    .map((r) => r.conditions)
    .filter((c): c is Record<string, unknown> => c != null && Object.keys(c).length > 0);
  const negativeConds = negatives
    .map((r) => r.conditions)
    .filter((c): c is Record<string, unknown> => c != null && Object.keys(c).length > 0);

  // base = união dos grants positivos (allow-all se algum é incondicional)
  let base: Resolved;
  if (hasUnconditionalGrant) {
    base = { kind: 'allow-all' };
  } else if (positiveConds.length === 1) {
    base = { kind: 'where', where: positiveConds[0] };
  } else {
    base = { kind: 'where', where: { OR: positiveConds } };
  }

  if (negativeConds.length === 0) return base;

  // subtrai proibições: base ∧ NOT(⋃ neg)
  const notClause = {
    NOT: negativeConds.length === 1 ? negativeConds[0] : { OR: negativeConds },
  };
  if (base.kind === 'allow-all') return { kind: 'where', where: notClause };
  return { kind: 'where', where: { AND: [base.where, notClause] } };
}

export function resolveCondition(input: ResolveInput): ResolveResult {
  const { subject, action, rules, inheritedScope, isTenantOwned } = input;
  const scopeOnly = scopeToResolved(inheritedScope);

  // Model sem subject CASL: só o escopo herdado confina (se for de tenant);
  // dados de referência globais ficam livres.
  if (subject == null) {
    const r: Resolved = isTenantOwned ? scopeOnly : { kind: 'allow-all' };
    return { effective: r, scopeOnly: r, wouldDeny: false };
  }

  const grant = grantLayer(rules, subject, action);
  const effective = meet(scopeOnly, grant);
  return {
    effective,
    scopeOnly,
    wouldDeny: grant.kind === 'deny',
  };
}
