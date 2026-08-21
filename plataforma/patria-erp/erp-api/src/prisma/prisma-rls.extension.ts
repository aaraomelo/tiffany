import { ForbiddenException } from '@nestjs/common';
import {
  resolveCondition,
  type PrismaWhere,
  type ScopeCondition,
} from '../access/condition-resolver';
import { SUBJECT_BY_MODEL } from '../access/access.types';
import type { Action, RuleInput } from '../access/access.types';

/**
 * Extensão de Row-Level Security: empurra a condição efetiva (c_eff) do modelo
 * de propagação para o `where` de toda query. Ver doc/casl-propagation.tex §5.
 *
 * Endurecimentos vs. o protótipo original: (a) sem `tenantId` hardcoded — a
 * condição vem do resolver; (b) grants compõem com OR; (c) fail-closed (deny →
 * Forbidden) em enforce; (d) duas velocidades — o piso de escopo (tenant) vale
 * já em shadow, a camada CASL só nega em enforce.
 */

export type RlsMode = 'shadow' | 'enforce';

export interface RlsContext {
  rules: RuleInput[];
  inheritedScope: ScopeCondition;
  mode: RlsMode;
  /** tenant ativo (para guardar create e log); null = operador */
  tenantId: string | null;
  userId?: string | null;
  /** callback de log para negações em shadow (default: console.warn) */
  onShadowDeny?: (info: { model: string; action: Action; userId?: string | null }) => void;
}

export interface RlsBaseClient {
  _runtimeDataModel?: { models: Record<string, { fields: Array<{ name: string }> }> };
}

function opToAction(op: string): Action | null {
  switch (op) {
    case 'findUnique':
    case 'findUniqueOrThrow':
    case 'findFirst':
    case 'findFirstOrThrow':
    case 'findMany':
    case 'count':
    case 'aggregate':
    case 'groupBy':
      return 'read';
    case 'create':
    case 'createMany':
    case 'createManyAndReturn':
      return 'create';
    case 'update':
    case 'updateMany':
    case 'updateManyAndReturn':
    case 'upsert':
      return 'update';
    case 'delete':
    case 'deleteMany':
      return 'delete';
    default:
      return null;
  }
}

/** Junta a condição injetada preservando o seletor único no topo (Prisma 6 extendedWhereUnique). */
function mergeWhere(orig: unknown, inject: PrismaWhere): PrismaWhere {
  const base = (orig ?? {}) as PrismaWhere;
  const existing = Array.isArray(base.AND)
    ? base.AND
    : base.AND
      ? [base.AND]
      : [];
  return { ...base, AND: [...existing, inject] };
}

function scopeTenantId(scope: ScopeCondition): string | null {
  if (scope.kind === 'where' && typeof scope.where.tenantId === 'string') {
    return scope.where.tenantId;
  }
  return null;
}

/** Models com coluna `tenantId` no DMMF → recebem escopo de linha. */
function buildTenantOwnedSet(base: RlsBaseClient): Set<string> {
  const out = new Set<string>();
  const models = base._runtimeDataModel?.models ?? {};
  for (const [name, meta] of Object.entries(models)) {
    if (meta.fields?.some((f) => f.name === 'tenantId')) out.add(name);
  }
  return out;
}

export const createRowLevelSecurityExtension = (
  ctx: RlsContext,
  base: RlsBaseClient,
) => {
  const tenantOwned = buildTenantOwnedSet(base);
  const tid = scopeTenantId(ctx.inheritedScope);
  const warn =
    ctx.onShadowDeny ??
    ((info: { model: string; action: Action }) =>
      // eslint-disable-next-line no-console
      console.warn(`[RLS shadow] negaria ${info.action} ${info.model}`));

  return {
    query: {
      $allModels: {
        // eslint-disable-next-line @typescript-eslint/no-explicit-any
        async $allOperations({ model, operation, args, query }: any) {
          const action = opToAction(operation);
          if (!action) return query(args);

          const subject = SUBJECT_BY_MODEL[model] ?? null;
          const owned = tenantOwned.has(model);
          // dado de referência global, sem subject → sem escopo
          if (!owned && subject == null) return query(args);

          const { effective, wouldDeny } = resolveCondition({
            subject,
            action,
            rules: ctx.rules,
            inheritedScope: ctx.inheritedScope,
            isTenantOwned: owned,
          });

          // SHADOW: nunca altera args nem nega — só observa e loga o que
          // aconteceria. Comportamento idêntico ao atual (rollout seguro).
          if (ctx.mode === 'shadow') {
            if (wouldDeny) warn({ model, action, userId: ctx.userId });
            if (action === 'create' && tid) {
              const data = args.data;
              const check = (row: Record<string, unknown>) => {
                if (row?.tenantId != null && row.tenantId !== tid) {
                  warn({ model, action, userId: ctx.userId });
                }
              };
              if (Array.isArray(data)) data.forEach(check);
              else if (data) check(data);
            }
            return query(args);
          }

          // ENFORCE: aplica c_eff
          if (effective.kind === 'deny') {
            throw new ForbiddenException({
              code: 'RLS_DENIED',
              message: `acesso negado para ${action} ${model}`,
            });
          }

          // create/createMany: garante que a linha nasce no tenant ativo
          if (action === 'create' && tid) {
            const data = args.data;
            const guard = (row: Record<string, unknown>) => {
              if (row.tenantId == null) {
                row.tenantId = tid;
              } else if (row.tenantId !== tid) {
                throw new ForbiddenException({
                  code: 'RLS_DENIED',
                  message: `não pode criar ${model} em outro tenant`,
                });
              }
            };
            if (Array.isArray(data)) data.forEach(guard);
            else if (data) guard(data);
            return query(args);
          }
          if (action === 'create') return query(args);

          // demais ops: injeta a condição no where (allow-all = sem injeção)
          if (effective.kind === 'where') {
            args.where = mergeWhere(args.where, effective.where);
            // upsert também cria: guarda o ramo de create
            if (operation === 'upsert' && tid && args.create) {
              if (args.create.tenantId == null) args.create.tenantId = tid;
            }
          }
          return query(args);
        },
      },
    },
  };
};
