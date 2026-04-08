import { Controller, Get } from '@nestjs/common';
import { RequestContext } from './request-context';

@Controller('api')
export class AppController {
  @Get()
  getRoot() {
    return { name: 'patria-api', client: RequestContext.alias, status: 'ok' };
  }

  @Get('health')
  getHealth() {
    return { status: 'healthy', timestamp: new Date().toISOString() };
  }
}
