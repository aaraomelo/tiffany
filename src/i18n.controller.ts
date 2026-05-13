import { Controller, Get, Param, Header, BadRequestException } from '@nestjs/common';
import { PrismaService } from './prisma.service';

const ALLOWED_LANGS = new Set(['pt', 'en']);

@Controller('api/i18n')
export class I18nController {
  // Cache em memória por lang. Invalida explicitamente via update.
  private cache: Record<string, { strings: Record<string, string>; etag: string; built: number }> = {};
  private readonly TTL_MS = 5 * 60_000; // refetch DB a cada 5min se nada invalidar

  constructor(private readonly prisma: PrismaService) {}

  @Get(':lang')
  @Header('Cache-Control', 'public, max-age=300, s-maxage=300')
  async getStrings(@Param('lang') lang: string) {
    if (!ALLOWED_LANGS.has(lang)) {
      throw new BadRequestException(`lang must be one of: ${[...ALLOWED_LANGS].join(', ')}`);
    }
    const now = Date.now();
    const c = this.cache[lang];
    if (c && now - c.built < this.TTL_MS) {
      return { lang, strings: c.strings, etag: c.etag, cached: true };
    }
    const rows = await this.prisma.i18nString.findMany({
      where: { lang },
      select: { key: true, value: true, updatedAt: true },
    });
    const strings: Record<string, string> = {};
    let maxUpdated = 0;
    for (const r of rows) {
      strings[r.key] = r.value;
      const u = r.updatedAt.getTime();
      if (u > maxUpdated) maxUpdated = u;
    }
    const etag = `${rows.length}-${maxUpdated}`;
    this.cache[lang] = { strings, etag, built: now };
    return { lang, strings, etag, cached: false };
  }
}
