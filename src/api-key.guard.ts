import { Injectable, CanActivate, ExecutionContext, UnauthorizedException } from '@nestjs/common';

const API_KEYS: Record<string, string> = {
  patricia: process.env.API_KEY_PATRICIA || 'pk_patricia_2026',
  worker: process.env.API_KEY_WORKER || 'pk_worker_2026',
  admin: process.env.API_KEY_ADMIN || 'pk_admin_2026',
};

// Endpoint prefixes that don't require auth (supports dynamic segments)
const PUBLIC_PREFIXES = [
  '/api/tenants/check-alias/',
  '/api/auth/github',
];

// Endpoints that don't require auth
const PUBLIC_PATHS = [
  '/api/tenants/register',
  '/api/health',
  '/api/webhooks/github',
  '/api/webhooks/deploy-diagnostic',
  '/api/webhooks/evolution',
  '/api/webhooks/telegram-inbound',
  '/api/tasks/events/stream',
  '/api/auth/login',
  '/api/auth/register',
  '/api', // root endpoint
  '/api/product',
];

@Injectable()
export class ApiKeyGuard implements CanActivate {
  canActivate(context: ExecutionContext): boolean {
    const req = context.switchToHttp().getRequest();
    const path = req.path;

    // Public endpoints
    if (PUBLIC_PATHS.includes(path)) return true;
    if (PUBLIC_PREFIXES.some(prefix => path.startsWith(prefix))) return true;

    // Check for serve-static (no /api prefix)
    if (!path.startsWith('/api')) return true;

    const apiKey = req.headers['x-api-key'];
    if (!apiKey) {
      throw new UnauthorizedException('Missing X-API-Key header');
    }

    const agent = Object.entries(API_KEYS).find(([, key]) => key === apiKey);
    if (!agent) {
      throw new UnauthorizedException('Invalid API key');
    }

    // Attach agent identity to request
    req.apiAgent = agent[0]; // 'patricia', 'worker', or 'admin'

    // Patricia must use gateway — block direct endpoint access
    if (req.apiAgent === 'patricia' && !path.startsWith('/api/patricia/')) {
      // Allow health, status, and search (read-only) for backwards compatibility
      const allowedDirect = ['/api/health', '/api/status', '/api/pending-actions'];
      if (!allowedDirect.includes(path)) {
        throw new UnauthorizedException('Patricia deve usar o gateway /api/patricia/action');
      }
    }

    return true;
  }
}
