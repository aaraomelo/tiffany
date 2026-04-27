import { Controller, Post, Get, Delete, Body, Query, Param, BadRequestException, Logger } from '@nestjs/common';
import { ApiTags, ApiOperation, ApiSecurity } from '@nestjs/swagger';
import { MemoryService } from '../messaging/memory.service';
import { PrismaService } from '../prisma.service';
import { SaveMemoryDto, BulkSaveDto } from './claude-brain.dto';

const THEORY_EMBEDDER_URL = process.env.THEORY_EMBEDDER_URL || 'http://127.0.0.1:9301';

const AARAO_PERSON_ID = '5b374ab4-8f2d-47a8-a4c9-eb64fcf2cb5f';
const DEFAULT_SOURCE_MODEL = 'claude-opus-4-7';
const DEFAULT_VISIBILITY = 'private';
const DEFAULT_PRIORITY = 'long_term';

@ApiTags('Claude Brain')
@ApiSecurity('api-key')
@Controller('api/claude-brain')
export class ClaudeBrainController {
  private readonly logger = new Logger('ClaudeBrain');

  constructor(
    private readonly memory: MemoryService,
    private readonly prisma: PrismaService,
  ) {}

  // --- Theory search ---

  @Get('theory/search')
  @ApiOperation({ summary: 'Busca semântica nos chunks .tex (theory_chunks)' })
  async theorySearch(
    @Query('q') q: string,
    @Query('limit') limit?: string,
    @Query('file') file?: string,
  ) {
    if (!q) throw new BadRequestException('q required');
    const limitN = limit ? Math.min(50, parseInt(limit, 10) || 8) : 8;
    let vector: number[];
    try {
      const res = await fetch(`${THEORY_EMBEDDER_URL}/embed-query`, {
        method: 'POST',
        headers: { 'Content-Type': 'application/json' },
        body: JSON.stringify({ text: q }),
        signal: AbortSignal.timeout(15_000),
      });
      const data = await res.json();
      if (!res.ok || !data.vector) throw new Error('embedder failed');
      vector = data.vector;
    } catch (e: any) {
      throw new BadRequestException(`embedder unavailable: ${e.message}`);
    }
    const vec = `[${vector.join(',')}]`;
    const fileFilter = file ? `AND file = $3` : '';
    const args: any[] = [vec, limitN];
    if (file) args.push(file);
    const rows: any[] = await this.prisma.$queryRawUnsafe(
      `SELECT id::text, file, section, chunk_idx, content,
              1 - (embedding <=> $1::vector) AS similarity
       FROM theory_chunks
       WHERE embedding IS NOT NULL ${fileFilter}
       ORDER BY embedding <=> $1::vector LIMIT $2`,
      ...args,
    );
    return { ok: true, count: rows.length, results: rows };
  }

  @Get('theory/stats')
  @ApiOperation({ summary: 'Stats: total chunks, embedded, pendentes, por file' })
  async theoryStats() {
    const totals: any[] = await this.prisma.$queryRawUnsafe(
      `SELECT COUNT(*)::int AS total,
              COUNT(*) FILTER (WHERE embedded_at IS NOT NULL)::int AS embedded,
              COUNT(*) FILTER (WHERE embedded_at IS NULL)::int AS pending
       FROM theory_chunks`
    );
    const byFile: any[] = await this.prisma.$queryRawUnsafe(
      `SELECT file, COUNT(*)::int AS chunks,
              COUNT(*) FILTER (WHERE embedded_at IS NOT NULL)::int AS embedded
       FROM theory_chunks
       GROUP BY file ORDER BY chunks DESC LIMIT 50`
    );
    return { ok: true, totals: totals[0], byFile };
  }

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
  async search(
    @Query('q') q: string,
    @Query('limit') limit?: string,
    @Query('priorities') priorities?: string,
    @Query('personId') personId?: string,
    @Query('accessLevel') accessLevel?: string,
  ) {
    if (!q) throw new BadRequestException('q required');
    const limitN = limit ? parseInt(limit, 10) : 8;
    const prioritiesArr = priorities
      ? priorities.split(',').map((s) => s.trim()).filter(Boolean)
      : ['core', 'long_term', 'short_term'];
    const results = await this.memory.searchByEmbedding(
      q,
      prioritiesArr,
      limitN,
      personId || AARAO_PERSON_ID,
      accessLevel || 'all',
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

  @Post('messages/backfill')
  @ApiOperation({ summary: 'Gerar embeddings pra person_messages sem embedding (busca semântica cross-channel)' })
  async backfillMessages(@Query('batch') batch?: string) {
    const max = batch ? Math.min(500, parseInt(batch, 10) || 200) : 200;
    const updated = await this.memory.backfillMessageEmbeddings(max);
    return { ok: true, updated };
  }

  @Get('memory/stats')
  @ApiOperation({ summary: 'Estatísticas (priority × state, acessos)' })
  async stats() {
    const stats = await this.memory.getStats();
    return { ok: true, stats };
  }
}
