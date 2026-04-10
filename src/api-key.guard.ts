import { Injectable, CanActivate, ExecutionContext, UnauthorizedException } from '@nestjs/common';

const API_KEYS: Record<string, string> = {
  patricia: process.env.API_KEY_PATRICIA || 'pk_patricia_2026',
  worker: process.env.API_KEY_WORKER || 'pk_worker_2026',
  admin: process.env.API_KEY_ADMIN || 'pk_admin_2026',
};

// Endpoints that don't require auth
const PUBLIC_PATHS = [
  '/api/health',
  '/api/webhooks/github',
  '/api/tasks/events/stream',
  '/api/auth/login',
  '/api', // root endpoint
];

@Injectable()
export class ApiKeyGuard implements CanActivate {
  canActivate(context: ExecutionContext): boolean {
    const req = context.switchToHttp().getRequest();
    const path = req.path;

    // Public endpoints
    if (PUBLIC_PATHS.includes(path)) return true;

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
    return true;
  }
}
