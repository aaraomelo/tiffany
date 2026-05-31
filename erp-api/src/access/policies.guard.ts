import {
  CanActivate,
  ExecutionContext,
  ForbiddenException,
  Injectable,
} from '@nestjs/common';
import { Reflector } from '@nestjs/core';
import { PrismaService } from '../prisma/prisma.service';
import { buildAbility } from './ability.factory';
import {
  CHECK_POLICIES_KEY,
  type RequiredPolicy,
} from './check-policies.decorator';

interface RequestWithUser {
  user?: { sub?: string };
}

/**
 * Guard de RBAC. Lê as policies do handler/controller (@CheckPolicies),
 * carrega os perfis+regras do usuário, monta o Ability CASL e exige que
 * todas as policies sejam permitidas. Sem policies declaradas → libera
 * (enforcement incremental).
 */
@Injectable()
export class PoliciesGuard implements CanActivate {
  constructor(
    private readonly reflector: Reflector,
    private readonly prisma: PrismaService,
  ) {}

  async canActivate(context: ExecutionContext): Promise<boolean> {
    const policies =
      this.reflector.getAllAndOverride<RequiredPolicy[] | undefined>(
        CHECK_POLICIES_KEY,
        [context.getHandler(), context.getClass()],
      ) ?? [];
    if (policies.length === 0) return true;

    const req = context.switchToHttp().getRequest<RequestWithUser>();
    const userId = req.user?.sub;
    if (!userId) {
      throw new ForbiddenException('autenticação necessária');
    }

    const user = await this.prisma.tenantUser.findUnique({
      where: { id: userId },
      include: { accessRoles: { include: { rules: true } } },
    });

    const ability = buildAbility(user?.accessRoles ?? []);

    const ok = policies.every((p) => ability.can(p.action, p.subject));
    if (!ok) {
      throw new ForbiddenException({
        code: 'FORBIDDEN',
        message: 'você não tem permissão para esta ação',
      });
    }
    return true;
  }
}
