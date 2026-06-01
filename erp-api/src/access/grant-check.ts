import {
  resolveCondition,
  type PrismaWhere,
  type Resolved,
  type ScopeCondition,
} from './condition-resolver';
import { attenuatedDepth, depthFromStored } from './delegation';
import type { Action, RuleInput, Subject } from './access.types';

/**
 * Enforcement GENÉRICO de delegação: "qualquer um pode conceder a qualquer um
 * abaixo, mas nunca uma permissão maior que a sua" (não-escalada).
 * Ver doc/casl-propagation.tex §4 (atenuação). Verificado no ATO DE DELEGAR.
 *
 * canGrant(ator, proposta) é verdadeiro sse, para cada (ação,subject) da
 * proposta: (i) o ator detém (ação,subject); (ii) o extent proposto ⊆ extent
 * do ator (condição mais restritiva ou igual); (iii) a profundidade atenua
 * (δ_proposta ≤ δ_detida − 1). É CONSERVADOR: na dúvida sobre o ⊆, nega
 * (seguro — nunca permite escalada).
 */

export interface GrantActor {
  rules: RuleInput[];
  scope: ScopeCondition;
  isOperator?: boolean;
}

function asArray<T>(v: T | T[] | null | undefined): T[] {
  if (v == null) return [];
  return Array.isArray(v) ? v : [v];
}

/** Maior δ detido pelo ator para (subject, action) entre grants positivos. */
function maxHeldDepth(rules: RuleInput[], subject: Subject, action: Action): number {
  let max = Number.NEGATIVE_INFINITY;
  for (const r of rules) {
    if (r.inverted) continue;
    const subjs = asArray(r.subject);
    const acts = asArray(r.action);
    const subjOk = subjs.includes(subject) || subjs.includes('all');
    const actOk = acts.includes(action) || acts.includes('manage');
    if (subjOk && actOk) {
      max = Math.max(max, depthFromStored(r.propagationDepth));
    }
  }
  return max;
}

/** Cláusulas de igualdade atômicas de um where; null se contém forma não suportada. */
function atomicClauses(where: PrismaWhere): Array<[string, unknown]> | null {
  const out: Array<[string, unknown]> = [];
  for (const [k, v] of Object.entries(where)) {
    if (k === 'AND') {
      for (const sub of asArray(v as PrismaWhere | PrismaWhere[])) {
        const s = atomicClauses(sub);
        if (s === null) return null;
        out.push(...s);
      }
    } else if (k === 'OR' || k === 'NOT') {
      return null; // não decidível conservadoramente
    } else if (v !== null && typeof v === 'object') {
      return null; // operador ({in},{gt},...) → conservador
    } else {
      out.push([k, v]);
    }
  }
  return out;
}

/** Conservador: ⟦P⟧ ⊆ ⟦A⟧ se toda cláusula atômica de A aparece igual em P. */
function whereSubset(p: PrismaWhere, a: PrismaWhere): boolean {
  const ac = atomicClauses(a);
  const pc = atomicClauses(p);
  if (ac === null || pc === null) return false;
  const pmap = new Map(pc.map(([k, v]) => [k, v]));
  return ac.every(([k, v]) => pmap.has(k) && pmap.get(k) === v);
}

function extentSubset(proposed: Resolved, actor: Resolved): boolean {
  if (actor.kind === 'allow-all') return true; // ator tem autoridade plena p/ (s,a)
  if (actor.kind === 'deny') return false;
  if (proposed.kind === 'deny') return true; // ∅ ⊆ qualquer
  if (proposed.kind === 'allow-all') return false; // proposta ampla vs ator restrito
  return whereSubset(proposed.where, actor.where);
}

export function canGrant(actor: GrantActor, proposed: RuleInput): boolean {
  if (actor.isOperator) return true;
  const subjects = asArray(proposed.subject);
  const actions = asArray(proposed.action);
  const proposedDepth = depthFromStored(proposed.propagationDepth);

  for (const s of subjects) {
    for (const a of actions) {
      const actorEff = resolveCondition({
        subject: s,
        action: a,
        rules: actor.rules,
        inheritedScope: actor.scope,
        isTenantOwned: true,
      }).effective;
      if (actorEff.kind === 'deny') return false; // ator não detém (a,s)

      const held = maxHeldDepth(actor.rules, s, a);
      if (attenuatedDepth(held, proposedDepth) === null) return false;

      if (actorEff.kind === 'allow-all') continue;

      const proposedEff = resolveCondition({
        subject: s,
        action: a,
        rules: [proposed],
        inheritedScope: actor.scope,
        isTenantOwned: true,
      }).effective;
      if (!extentSubset(proposedEff, actorEff)) return false;
    }
  }
  return true;
}
