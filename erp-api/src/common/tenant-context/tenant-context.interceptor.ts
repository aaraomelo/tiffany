import {
  CallHandler,
  ExecutionContext,
  Injectable,
  NestInterceptor,
} from '@nestjs/common';
import { Observable } from 'rxjs';
import {
  TenantContext,
  tenantContextStorage,
} from './tenant-context';

interface RequestWithUser {
  user?: { tenantId?: string; sub?: string; role?: string };
  headers: Record<string, string | string[] | undefined>;
}

@Injectable()
export class TenantContextInterceptor implements NestInterceptor {
  intercept(context: ExecutionContext, next: CallHandler): Observable<unknown> {
    const req = context.switchToHttp().getRequest<RequestWithUser>();
    const headerTenant = req.headers['x-tenant'];
    const tenantIdFromHeader = Array.isArray(headerTenant)
      ? headerTenant[0]
      : headerTenant;

    const ctx: TenantContext = {
      tenantId: req.user?.tenantId ?? tenantIdFromHeader ?? null,
      userId: req.user?.sub ?? null,
      role: req.user?.role ?? null,
    };

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
}
