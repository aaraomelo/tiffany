import {
  Controller,
  Get,
  Param,
  Query,
  Header,
  Res,
  HttpException,
  HttpStatus,
} from '@nestjs/common';
import type { Response } from 'express';
import { PrismaService } from './prisma.service';

const ALLOWED_LANGS = new Set(['pt', 'en']);

function normLang(input: unknown, fallback = 'pt'): string {
  const v = typeof input === 'string' ? input.toLowerCase().slice(0, 2) : '';
  return ALLOWED_LANGS.has(v) ? v : fallback;
}

@Controller('api/papers')
export class PapersController {
  constructor(private readonly prisma: PrismaService) {}

  @Get()
  async list(@Query('lang') lang?: string) {
    const wantLang = normLang(lang);
    const papers = await this.prisma.paper.findMany({
      orderBy: { date: 'desc' },
      include: {
        i18n: true,
        files: { select: { lang: true, kind: true, sizeBytes: true } },
      },
    });

    return {
      papers: papers.map((p) => {
        const i18n =
          p.i18n.find((i) => i.lang === wantLang) ||
          p.i18n.find((i) => i.lang === 'pt') ||
          p.i18n[0];
        const fileLangs = (kind: string) =>
          Array.from(new Set(p.files.filter((f) => f.kind === kind).map((f) => f.lang)));
        const pdfLangs = fileLangs('pdf');
        const texLangs = fileLangs('tex');
        return {
          slug: p.slug,
          title: i18n?.title ?? p.slug,
          authors: p.authors,
          abstract: i18n?.abstract ?? '',
          date: p.date.toISOString().slice(0, 10),
          tags: p.tags,
          has_pdf: pdfLangs.length > 0,
          has_tex: texLangs.length > 0,
          pdf_langs: pdfLangs,
          tex_langs: texLangs,
          available_langs: Array.from(new Set(p.i18n.map((i) => i.lang))),
        };
      }),
      count: papers.length,
      lang: wantLang,
    };
  }

  @Get(':slug/meta')
  async meta(@Param('slug') slug: string, @Query('lang') lang?: string) {
    const wantLang = normLang(lang);
    const p = await this.prisma.paper.findUnique({
      where: { slug },
      include: {
        i18n: true,
        files: { select: { lang: true, kind: true, sizeBytes: true } },
      },
    });
    if (!p) throw new HttpException('paper not found', HttpStatus.NOT_FOUND);
    const i18n =
      p.i18n.find((i) => i.lang === wantLang) ||
      p.i18n.find((i) => i.lang === 'pt') ||
      p.i18n[0];
    const fileLangs = (kind: string) =>
      Array.from(new Set(p.files.filter((f) => f.kind === kind).map((f) => f.lang)));
    return {
      slug: p.slug,
      title: i18n?.title ?? slug,
      authors: p.authors,
      abstract: i18n?.abstract ?? '',
      date: p.date.toISOString().slice(0, 10),
      tags: p.tags,
      has_pdf: p.files.some((f) => f.kind === 'pdf'),
      has_tex: p.files.some((f) => f.kind === 'tex'),
      pdf_langs: fileLangs('pdf'),
      tex_langs: fileLangs('tex'),
      available_langs: Array.from(new Set(p.i18n.map((i) => i.lang))),
      lang_served: i18n?.lang ?? 'pt',
    };
  }

  @Get(':slug/source')
  @Header('Cache-Control', 'public, max-age=300')
  async source(
    @Param('slug') slug: string,
    @Query('lang') lang: string | undefined,
    @Res() res: Response,
  ) {
    const wantLang = normLang(lang);
    const file = await this.fetchFile(slug, wantLang, 'tex');
    if (!file) throw new HttpException('paper source not found', HttpStatus.NOT_FOUND);
    res.setHeader('Content-Type', 'text/x-tex; charset=utf-8');
    res.setHeader('Content-Length', file.sizeBytes.toString());
    res.setHeader('Content-Language', file.lang);
    res.send(file.data);
  }

  @Get(':slug/pdf')
  async pdf(
    @Param('slug') slug: string,
    @Query('lang') lang: string | undefined,
    @Res() res: Response,
  ) {
    const wantLang = normLang(lang);
    const file = await this.fetchFile(slug, wantLang, 'pdf');
    if (!file) throw new HttpException('paper pdf not found', HttpStatus.NOT_FOUND);
    res.setHeader('Content-Type', 'application/pdf');
    res.setHeader('Content-Length', file.sizeBytes.toString());
    res.setHeader('Content-Disposition', `inline; filename="${slug}.pdf"`);
    res.setHeader('Cache-Control', 'public, max-age=300');
    res.setHeader('Content-Language', file.lang);
    res.send(file.data);
  }

  private async fetchFile(slug: string, lang: string, kind: 'pdf' | 'tex') {
    const primary = await this.prisma.paperFile.findUnique({
      where: { paperSlug_lang_kind: { paperSlug: slug, lang, kind } },
    });
    if (primary) return primary;
    if (lang === 'pt') return null;
    const fallback = await this.prisma.paperFile.findUnique({
      where: { paperSlug_lang_kind: { paperSlug: slug, lang: 'pt', kind } },
    });
    return fallback;
  }
}
