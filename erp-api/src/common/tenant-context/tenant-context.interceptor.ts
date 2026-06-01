import {
  CallHandler,
  ExecutionContext,
  Injectable,
  NestInterceptor,
} from '@nestjs/common';
import { Observable } from 'rxjs';
import { PrismaService } from '../../prisma/prisma.service';
import type { Action, RuleInput, Subject } from '../../access/access.types';
import { TenantContext, tenantContextStorage } from './tenant-context';

interface RequestWithUser {
  user?: { tenantId?: string; sub?: string; role?: string; email?: string };
  headers: Record<string, string | string[] | undefined>;
}

/** Emails de operador de plataforma (admin geral, ⊤ — vê todos os tenants). */
function platformOperatorEmails(): Set<string> {
  return new Set(
    (process.env.PLATFORM_OPERATOR_EMAILS ?? '')
      .split(',')
      .map((e) => e.trim().toLowerCase())
      .filter(Boolean),
  );
}

// eslint-disable-next-line @typescript-eslint/no-explicit-any
function toRuleInput(r: any): RuleInput {
  return {
    action: r.action as Action | Action[],
    subject: r.subject as Subject | Subject[],
    fields: (r.fields ?? null) as string | string[] | null,
    conditions: (r.conditions ?? null) as Record<string, unknown> | null,
    inverted: !!r.inverted,
    propagationDepth: r.propagationDepth ?? null,
  };
}

@Injectable()
export class TenantContextInterceptor implements NestInterceptor {
  constructor(private readonly prisma: PrismaService) {}

  async intercept(
    context: ExecutionContext,
    next: CallHandler,
  ): Promise<Observable<unknown>> {
    const req = context.switchToHttp().getRequest<RequestWithUser>();
    const headerTenant = req.headers['x-tenant'];
    const tenantIdFromHeader = Array.isArray(headerTenant)
      ? headerTenant[0]
      : headerTenant;
    const tenantId = req.user?.tenantId ?? tenantIdFromHeader ?? null;
    const userId = req.user?.sub ?? null;

    const email = req.user?.email?.toLowerCase();
    const isOperator = !!email && platformOperatorEmails().has(email);

    const ctx: TenantContext = {
      tenantId,
      userId,
      role: req.user?.role ?? null,
      isPlatformOperator: isOperator, // vale mesmo com RLS off (p/ o grant-check)
    };

    // RLS só liga com RLS_MODE=shadow|enforce. Sem isso, accessRules fica
    // undefined → a PrismaService passa direto (push inerte).
    const mode = process.env.RLS_MODE;
    if ((mode === 'shadow' || mode === 'enforce') && userId) {
      try {
        if (isOperator) {
          // operador de plataforma = ⊤: manage all + escopo irrestrito
          ctx.accessRules = [{ action: 'manage', subject: 'all' }];
          ctx.inheritedScope = { kind: 'all' };
        } else {
          ctx.accessRules = await this.loadRules(userId);
          ctx.inheritedScope = tenantId
            ? { kind: 'where', where: { tenantId } }
            : { kind: 'all' };
        }
        ctx.rlsMode = mode;
      } catch {
        // falha ao carregar regras → não liga RLS (fail-safe: passa direto)
      }
    }

    return new Observable((subscriber) => {
      tenantContextStorage.run(ctx, () => {
        next.handle().subscribe({
          next: (value) => subscriber.next(value),
          error: (err) => subscriber.error(err),
          complete: () => subscriber.complete(),
        });
      });
    });
  }

  private async loadRules(userId: string): Promise<RuleInput[]> {
    const user = await this.prisma.tenantUser.findUnique({
      where: { id: userId },
      include: { accessRoles: { include: { rules: true } } },
    });
    return (user?.accessRoles ?? [])
      .flatMap((role) => role.rules)
      .map(toRuleInput);
  }
}
