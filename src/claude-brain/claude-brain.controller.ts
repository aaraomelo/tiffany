import { Controller, Post, Get, Delete, Body, Query, Param, BadRequestException, Logger } from '@nestjs/common';
import { ApiTags, ApiOperation, ApiSecurity } from '@nestjs/swagger';
import { MemoryService } from '../messaging/memory.service';
import { SaveMemoryDto, SearchMemoryDto, BulkSaveDto } from './claude-brain.dto';

const AARAO_PERSON_ID = '5b374ab4-8f2d-47a8-a4c9-eb64fcf2cb5f';
const DEFAULT_SOURCE_MODEL = 'claude-opus-4-7';
const DEFAULT_VISIBILITY = 'private';
const DEFAULT_PRIORITY = 'long_term';

@ApiTags('Claude Brain')
@ApiSecurity('api-key')
@Controller('api/claude-brain')
export class ClaudeBrainController {
  private readonly logger = new Logger('ClaudeBrain');

  constructor(private readonly memory: MemoryService) {}

  @Post('memory')
  @ApiOperation({ summary: 'Salvar memória (com defaults: Aarão, private, claude-opus-4-7)' })
  async save(@Body() dto: SaveMemoryDto) {
    if (!dto.title || !dto.content || !dto.category) {
      throw new BadRequestException('title, content, category required');
    }
    const id = await this.memory.save(
      dto.category,
      dto.title,
      dto.content,
      dto.priority || DEFAULT_PRIORITY,
      dto.personId || AARAO_PERSON_ID,
      dto.visibility || DEFAULT_VISIBILITY,
      dto.sourceModel || DEFAULT_SOURCE_MODEL,
    );
    return { ok: true, id };
  }

  @Get('memory/search')
  @ApiOperation({ summary: 'Busca semântica (cosine) com fallback textual' })
  async search(@Query() q: SearchMemoryDto) {
    if (!q.q) throw new BadRequestException('q required');
    const limit = q.limit ? Number(q.limit) : 8;
    const priorities = q.priorities && q.priorities.length > 0
      ? q.priorities
      : ['core', 'long_term', 'short_term'];
    const results = await this.memory.searchByEmbedding(
      q.q,
      priorities,
      limit,
      q.personId || AARAO_PERSON_ID,
      q.accessLevel || 'all',
    );
    return { ok: true, results, count: results.length };
  }

  @Get('memory/context')
  @ApiOperation({ summary: 'Contexto formatado pra prompt (core + relevantes via embedding)' })
  async context(@Query('q') query: string, @Query('personId') personId?: string, @Query('accessLevel') accessLevel?: string) {
    if (!query) throw new BadRequestException('q required');
    const text = await this.memory.getContextForPerson(
      query,
      personId || AARAO_PERSON_ID,
      accessLevel || 'all',
    );
    return { ok: true, context: text, length: text.length };
  }

  @Delete('memory/:idOrTitle')
  @ApiOperation({ summary: 'Esquecer memória (archive — não deleta de fato)' })
  async forget(@Param('idOrTitle') idOrTitle: string, @Query('personId') personId?: string) {
    const ok = await this.memory.forget(idOrTitle, personId || AARAO_PERSON_ID, 'all');
    return { ok };
  }

  @Post('memory/bulk')
  @ApiOperation({ summary: 'Migração em lote — salva N memories de uma vez' })
  async bulk(@Body() dto: BulkSaveDto) {
    if (!dto.items || !Array.isArray(dto.items)) {
      throw new BadRequestException('items[] required');
    }
    const results: any[] = [];
    let saved = 0, errors = 0;
    for (const it of dto.items) {
      try {
        const id = await this.memory.save(
          it.category,
          it.title,
          it.content,
          it.priority || DEFAULT_PRIORITY,
          it.personId || AARAO_PERSON_ID,
          it.visibility || DEFAULT_VISIBILITY,
          it.sourceModel || DEFAULT_SOURCE_MODEL,
        );
        results.push({ title: it.title, id, ok: true });
        saved++;
      } catch (e: any) {
        results.push({ title: it.title, ok: false, error: e.message?.slice(0, 200) });
        errors++;
      }
    }
    this.logger.log(`Bulk save: ${saved} saved, ${errors} errors`);
    return { ok: true, saved, errors, results };
  }

  @Post('memory/backfill')
  @ApiOperation({ summary: 'Gerar embeddings pra memories sem embedding' })
  async backfill() {
    const updated = await this.memory.backfillEmbeddings();
    return { ok: true, updated };
  }

  @Get('memory/stats')
  @ApiOperation({ summary: 'Estatísticas (priority × state, acessos)' })
  async stats() {
    const stats = await this.memory.getStats();
    return { ok: true, stats };
  }
}
