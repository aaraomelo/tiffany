import { Injectable, CanActivate, ExecutionContext, HttpException, HttpStatus, UnauthorizedException } from '@nestjs/common';
import { PrismaService } from './prisma.service';

const API_KEYS: Record<string, string> = {
  patricia: process.env.API_KEY_PATRICIA || 'pk_patricia_2026',
  worker: process.env.API_KEY_WORKER || 'pk_worker_2026',
  admin: process.env.API_KEY_ADMIN || 'pk_admin_2026',
};

const DEMO_API_KEY = process.env.DEMO_API_KEY || 'pk_demo_playground_2026';

// throttle in-memory para demo: 1 req / 3s por IP
const DEMO_LAST_HIT = new Map<string, number>();
const DEMO_THROTTLE_MS = 3000;
function demoThrottle(ip: string): boolean {
  const now = Date.now();
  const last = DEMO_LAST_HIT.get(ip) || 0;
  if (now - last < DEMO_THROTTLE_MS) return false;
  DEMO_LAST_HIT.set(ip, now);
  // GC simples: limpa entradas antigas a cada 1000 hits
  if (DEMO_LAST_HIT.size > 1000) {
    for (const [k, t] of DEMO_LAST_HIT) {
      if (now - t > 60000) DEMO_LAST_HIT.delete(k);
    }
  }
  return true;
}

const PUBLIC_PREFIXES = [
  '/api/outbound', // controller faz auth via X-Admin-Token
  '/api/llm', // público (rate-limit por IP cobre abuso)
  '/api/tenants/check-alias/',
  '/api/auth/github',
  '/api/papers/',
  '/api/papers',
];

const PUBLIC_PATHS = [
  '/api/tenants/register',
  '/api/health',
  '/api/nco/health',
  '/api/webhooks/github',
  '/api/webhooks/deploy-diagnostic',
  '/api/webhooks/evolution',
  '/api/webhooks/telegram-inbound',
  '/api/webhooks/pagbank',
  '/api/nco/bench/trigger',
  '/api/nco/bench-events/emit',
  '/api/nco/bench-events/inject-external',
  '/api/billing/health',
  '/api/billing/plans',
  '/api/i18n/pt',
  '/api/i18n/en',
  '/api/billing/events',
  '/api/tasks/events/stream',
  '/api/auth/login',
  '/api/auth/register',
  '/api/auth/signup',
  '/api/auth/verify',
  '/api/auth/resend-verification',
  '/api/auth/logout',
  '/api',
  '/api/product',
  '/api/sandbox/activate',
  '/api/sandbox/sync',
  '/api/sandbox/deactivate',
  '/api/sandbox/stream',
];

const NCO_GET_PUBLIC = new Set([
  '/api/nco',
  '/api/nco/maxcut',
  '/api/nco/coloring',
  '/api/nco/mis',
  '/api/nco/health',
  '/api/nco/public-stats',
  '/api/nco/scaling-bench',
  '/api/nco/bench-evolution',
  '/api/nco/bench/status',
  '/api/nco/bench-events',
  '/api/nco/ckpt-info',
  '/api/nco/per-net-exponents',
]);

const PATRICIA_DIRECT_OK = new Set([
  '/api/health',
  '/api/nco/health',
  '/api/nco/maxcut',
  '/api/status',
  '/api/pending-actions',
]);

@Injectable()
export class ApiKeyGuard implements CanActivate {
  constructor(private readonly prisma: PrismaService) {}

  async canActivate(context: ExecutionContext): Promise<boolean> {
    const req = context.switchToHttp().getRequest();
    const path = req.path;

    if (PUBLIC_PATHS.includes(path)) return true;
    if (req.method === 'GET' && NCO_GET_PUBLIC.has(path)) return true;
    if (PUBLIC_PREFIXES.some((prefix) => path.startsWith(prefix))) return true;
    if (!path.startsWith('/api')) return true;

    const apiKey = req.headers['x-api-key'] as string | undefined;
    if (!apiKey) {
      throw new UnauthorizedException('Missing X-API-Key header');
    }

    // 1. Tenta chaves internas (patricia/worker/admin)
    const internalAgent = Object.entries(API_KEYS).find(([, key]) => key === apiKey);
    if (internalAgent) {
      req.apiAgent = internalAgent[0];
      if (req.apiAgent === 'patricia' && !path.startsWith('/api/patricia/') && !PATRICIA_DIRECT_OK.has(path)) {
        throw new UnauthorizedException('Patricia deve usar o gateway /api/patricia/action');
      }
      return true;
    }

    // 2. Demo público (playground) — só endpoints NCO POST, com throttle
    if (apiKey === DEMO_API_KEY) {
      if (!path.startsWith('/api/nco/') || req.method !== 'POST') {
        throw new UnauthorizedException('demo key só pode chamar POST /api/nco/*');
      }
      const ip = (req.headers['x-forwarded-for'] as string)?.split(',')[0].trim() || req.ip || 'unknown';
      if (!demoThrottle(ip)) {
        throw new HttpException('Calma — demo aceita 1 chamada a cada 3s. Crie conta grátis pra rodar à vontade.', HttpStatus.TOO_MANY_REQUESTS);
      }
      req.apiAgent = 'demo';
      return true;
    }

    // 3. Tenta apiKey de tenant cadastrado (signup)
    if (apiKey.startsWith('pk_')) {
      const tenant = await this.prisma.tenant.findUnique({
        where: { apiKey },
        select: { id: true, alias: true },
      });
      if (tenant) {
        req.apiAgent = `tenant:${tenant.alias}`;
        req.tenantId = tenant.id;
        return true;
      }
    }

    throw new UnauthorizedException('Invalid API key');
  }
}
