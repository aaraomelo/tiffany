import {
  Controller, Post, Get, Body, Query, BadRequestException, Logger,
} from '@nestjs/common';
import { ApiTags, ApiOperation, ApiSecurity } from '@nestjs/swagger';
import { MultiversoService } from './multiverso.service';

@ApiTags('Multiverso (Patrícia tool)')
@ApiSecurity('api-key')
@Controller('api/multiverso')
export class MultiversoController {
  private readonly logger = new Logger('MultiversoController');

  constructor(private readonly mv: MultiversoService) {}

  // -------- CONTROL --------
  @Post('control')
  @ApiOperation({
    summary: 'Despacha override de carga (factor, duration_sec) pra um voluntário via /ws/ingest',
  })
  async control(@Body() body: {
    target: string;
    factor: number;
    duration_sec: number;
    reason?: string;
  }) {
    if (!body?.target) throw new BadRequestException('target obrigatório');
    if (typeof body.factor !== 'number') throw new BadRequestException('factor obrigatório');
    if (typeof body.duration_sec !== 'number') throw new BadRequestException('duration_sec obrigatório');
    return this.mv.sendControl({
      target: body.target,
      factor: body.factor,
      durationSec: body.duration_sec,
      reason: body.reason,
    });
  }

  // -------- VOLUNTÁRIOS (catálogo) --------
  @Post('voluntarios/upsert')
  @ApiOperation({
    summary: 'Cria/atualiza voluntário no catálogo + embedding semântico',
  })
  async upsert(@Body() body: {
    vid: string;
    display_name: string;
    host?: string;
    role?: 'voluntary' | 'general' | 'coord';
    runtime?: 'python' | 'haskell-hs';
    is_lab?: boolean;
    description: string;
  }) {
    if (!body?.vid) throw new BadRequestException('vid obrigatório');
    if (!body?.display_name) throw new BadRequestException('display_name obrigatório');
    if (!body?.description) throw new BadRequestException('description obrigatório');
    return this.mv.upsertVoluntario({
      vid: body.vid,
      displayName: body.display_name,
      host: body.host,
      role: body.role,
      runtime: body.runtime,
      isLab: body.is_lab,
      description: body.description,
    });
  }

  @Get('voluntarios')
  @ApiOperation({ summary: 'Lista catálogo completo de voluntários' })
  async list() {
    return { voluntarios: await this.mv.listVoluntarios() };
  }

  @Get('voluntarios/search')
  @ApiOperation({
    summary: 'Busca semântica no catálogo (Patrícia: "qual o general Haskell?")',
  })
  async search(@Query('q') q: string, @Query('limit') limit?: string) {
    if (!q) throw new BadRequestException('q obrigatório');
    const lim = limit ? Math.max(1, Math.min(20, parseInt(limit, 10))) : 5;
    return { results: await this.mv.searchVoluntarios(q, lim) };
  }

  // -------- STATUS --------
  @Get('status')
  @ApiOperation({
    summary: 'Snapshot ao vivo do leaderboard do coord (opcional vid pra um único)',
  })
  async status(@Query('vid') vid?: string) {
    return this.mv.statusNow(vid);
  }
}
