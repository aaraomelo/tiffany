import {
  CanActivate,
  ExecutionContext,
  ForbiddenException,
  Injectable,
} from '@nestjs/common';
import { Reflector } from '@nestjs/core';
import { ModulesService } from '../../modules/modules.service';
import { REQUIRES_MODULE_KEY } from '../decorators/requires-module.decorator';

interface RequestWithUser {
  user?: { tenantId?: string };
  headers: Record<string, string | string[] | undefined>;
}

@Injectable()
export class ModuleEnabledGuard implements CanActivate {
  constructor(
    private readonly reflector: Reflector,
    private readonly modules: ModulesService,
  ) {}

  async canActivate(context: ExecutionContext): Promise<boolean> {
    const slug = this.reflector.getAllAndOverride<string | undefined>(
      REQUIRES_MODULE_KEY,
      [context.getHandler(), context.getClass()],
    );
    if (!slug) return true;

    // Guards rodam antes do TenantContextInterceptor — lemos o tenantId
    // direto da request (mesma lógica do interceptor).
    const req = context.switchToHttp().getRequest<RequestWithUser>();
    const headerTenant = req.headers['x-tenant'];
    const tenantIdFromHeader = Array.isArray(headerTenant)
      ? headerTenant[0]
      : headerTenant;
    const tenantId = req.user?.tenantId ?? tenantIdFromHeader ?? null;

    if (!tenantId) return true; // rotas públicas/sem tenant — outros guards cuidam

    const enabled = await this.modules.isEnabled(tenantId, slug);
    if (!enabled) {
      throw new ForbiddenException({
        code: 'MODULE_DISABLED',
        module: slug,
        message: `módulo '${slug}' não está ativo neste tenant`,
      });
    }
    return true;
  }
}
