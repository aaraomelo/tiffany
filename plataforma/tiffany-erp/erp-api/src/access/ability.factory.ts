import { AbilityBuilder, createMongoAbility } from '@casl/ability';
import type { Action, AppAbility, Subject } from './access.types';

interface RuleLike {
  action: unknown; // Json no Prisma: Action | Action[]
  subject: unknown; // Json: Subject | Subject[]
  fields?: unknown;
  conditions?: unknown;
  inverted?: boolean | null;
}

interface RoleLike {
  rules: RuleLike[];
}

/**
 * Reconstrói o Ability CASL a partir dos perfis (e suas regras) do usuário.
 * Cada regra é replayada via can()/cannot() com as sobrecargas de
 * fields/conditions, igual ao modelo do easysync.
 */
export function buildAbility(roles: RoleLike[]): AppAbility {
  const { can, cannot, build } = new AbilityBuilder<AppAbility>(
    createMongoAbility,
  );

  for (const role of roles) {
    for (const rule of role.rules) {
      const apply = rule.inverted ? cannot : can;
      const action = rule.action as Action | Action[];
      const subject = rule.subject as Subject | Subject[];
      const fields = (rule.fields ?? undefined) as
        | string
        | string[]
        | undefined;
      // conditions é JSON dinâmico (MongoQuery) — tipagem fraca de propósito
      // eslint-disable-next-line @typescript-eslint/no-explicit-any
      const conditions = (rule.conditions ?? undefined) as any;

      if (conditions && fields) {
        apply(action, subject, fields, conditions);
      } else if (conditions) {
        apply(action, subject, conditions);
      } else if (fields) {
        apply(action, subject, fields);
      } else {
        apply(action, subject);
      }
    }
  }

  return build();
}
