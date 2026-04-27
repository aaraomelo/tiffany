import { Controller, Get, Post, Query, Body, BadRequestException } from '@nestjs/common';
import { ApiTags, ApiOperation, ApiSecurity } from '@nestjs/swagger';
import { OrganismEventsService } from './organism-events.service';

@ApiTags('Organism Events (autenticado — Claude consumer)')
@ApiSecurity('api-key')
@Controller('api/claude-brain/organism-events')
export class OrganismEventsController {
  constructor(private readonly events: OrganismEventsService) {}

  @Get('unconsumed')
  @ApiOperation({ summary: 'Eventos não consumidos pelo claude_watch (ordenado por ts DESC)' })
  async unconsumed(@Query('limit') limit?: string) {
    const n = limit ? Math.min(200, parseInt(limit, 10) || 50) : 50;
    const events = await this.events.loadUnconsumed(n);
    return { ok: true, count: events.length, events };
  }

  @Post('consume')
  @ApiOperation({ summary: 'Marca lista de event ids como consumidos' })
  async consume(@Body() body: { ids: string[]; by?: string }) {
    if (!body.ids || !Array.isArray(body.ids)) {
      throw new BadRequestException('ids[] required');
    }
    const updated = await this.events.markConsumed(body.ids, body.by || 'claude_watch');
    return { ok: true, updated };
  }

  @Get('stats')
  @ApiOperation({ summary: 'Estatísticas dos eventos das últimas 24h por kind' })
  async stats() {
    const stats = await this.events.stats();
    return { ok: true, stats };
  }
}
