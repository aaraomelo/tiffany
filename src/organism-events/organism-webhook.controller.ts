import { Controller, Post, Body, Headers, BadRequestException, UnauthorizedException, Logger } from '@nestjs/common';
import { ApiTags, ApiOperation } from '@nestjs/swagger';
import { OrganismEventsService } from './organism-events.service';

const ORGANISM_TOKEN = process.env.ORGANISM_TOKEN;

@ApiTags('Organism Webhook (público — token)')
@Controller('api/webhooks/organism')
export class OrganismWebhookController {
  private readonly logger = new Logger('OrganismWebhook');

  constructor(private readonly events: OrganismEventsService) {}

  @Post()
  @ApiOperation({ summary: 'Recebe evento aferente do GEX44 (corpo do organismo)' })
  async receive(
    @Headers('x-organism-token') token: string,
    @Body() body: { source: string; kind: string; severity?: number; data?: any; ts?: string },
  ) {
    if (!ORGANISM_TOKEN) {
      this.logger.error('ORGANISM_TOKEN não configurado no env');
      throw new UnauthorizedException('not configured');
    }
    if (token !== ORGANISM_TOKEN) {
      throw new UnauthorizedException('invalid token');
    }
    if (!body.source || !body.kind) {
      throw new BadRequestException('source and kind required');
    }
    const r = await this.events.record(body);
    return { ok: true, event_id: r.id };
  }
}
