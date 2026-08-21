import { SetMetadata } from '@nestjs/common';
import type { Action, Subject } from './access.types';

export interface RequiredPolicy {
  action: Action;
  subject: Subject;
}

export const CHECK_POLICIES_KEY = 'check_policies';

/**
 * Exige uma ou mais permissões (action+subject) no handler/controller.
 * O PoliciesGuard checa todas via ability.can(). Handlers sem este decorator
 * passam livremente (enforcement incremental).
 *
 * Ex.: @CheckPolicies({ action: 'manage', subject: 'Role' })
 */
export const CheckPolicies = (...policies: RequiredPolicy[]) =>
  SetMetadata(CHECK_POLICIES_KEY, policies);
