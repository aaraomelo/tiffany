import { SetMetadata } from '@nestjs/common';

export const REQUIRES_MODULE_KEY = 'requiresModule';

/**
 * Marca um controller ou handler como pertencente a um módulo opcional.
 * O ModuleEnabledGuard lê esta metadata e retorna 403 se o tenant atual
 * não tiver o módulo ativo (TenantModule.enabled = false).
 */
export const RequiresModule = (slug: string) =>
  SetMetadata(REQUIRES_MODULE_KEY, slug);
