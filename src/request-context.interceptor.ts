import { Injectable, NestInterceptor, ExecutionContext, CallHandler } from '@nestjs/common';
import { Observable, from, switchMap } from 'rxjs';
import { RequestContext } from './request-context';
import { PrismaService } from './prisma.service';

@Injectable()
export class RequestContextInterceptor implements NestInterceptor {
  constructor(private prisma: PrismaService) {}

  intercept(context: ExecutionContext, next: CallHandler<any>): Observable<any> {
    const req = context.switchToHttp().getRequest();
    const alias = req.headers['x-tenant'] ?? 'patria';

    const resolve = async () => {
      const tenant = await this.prisma.tenant.findUnique({ where: { alias } });
      const resolvedAlias = tenant?.alias ?? 'patria';
      return RequestContext.run(resolvedAlias, () => next.handle());
    };

    return from(resolve()).pipe(switchMap((obs) => obs));
  }
}
