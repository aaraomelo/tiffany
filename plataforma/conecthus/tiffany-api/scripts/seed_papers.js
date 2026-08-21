// Popula tabelas Paper + PaperI18n + PaperFile a partir de:
// - Metadados hardcoded neste arquivo
// - PDFs/TeX em /app/data/papers/<slug>.{pdf,tex} (lang='pt')
//
// Uso (dentro do container patria-api):
//   docker exec patria-api node /app/scripts/seed_papers.js
//
// Idempotente: usa upsert. Re-rodar é seguro.

const { PrismaClient } = require('@prisma/client');
const { readFileSync, existsSync } = require('fs');

const PAPERS_DIR = '/app/data/papers';
const prisma = new PrismaClient();

const seedPapers = [
  {
    slug: 'lopes',
    date: new Date('2026-05-09'),
    authors: ['Patria Technology', 'Gentil (UFRR)'],
    tags: ['scaling laws', 'method', 'NCO', 'max-cut'],
    i18n: [
      {
        lang: 'pt',
        title:
          'Método Lopes/Gentil: identificação empírica de leis de escala via comparação multi-modelo',
        abstract:
          'Apresentamos um procedimento prático para identificar a lei de escala empírica que melhor descreve uma sequência de medições V(n). Combina comparação multi-modelo (Lopes, Patria Technology, 2026) com formas híbrida e dupolinomial motivadas pelas sequências aritméticas e geométricas de ordem m de Gentil (UFRR, 2020). As 4 formas (polinomial, geométrica, dupolinomial, híbrida) são ajustadas por mínimos quadrados em log V e selecionadas pelo menor RMSE. Aplicamos à validação experimental da lei n^{3/2} em otimização combinatorial neural (max-cut) e obtemos expoente empírico 1,51 em treinamento, atravessando a fronteira teórica de Edwards (1973).',
      },
      {
        lang: 'en',
        title:
          'Lopes/Gentil Method: empirical identification of scaling laws via multi-model comparison',
        abstract:
          'We present a practical procedure for identifying the empirical scaling law that best describes a sequence of measurements V(n). It combines multi-model comparison (Lopes, Patria Technology, 2026) with hybrid and dupolynomial forms motivated by the order-m arithmetic and geometric sequences of Gentil (UFRR, 2020). The 4 forms (polynomial, geometric, dupolynomial, hybrid) are fit by least squares on log V and selected by lowest RMSE. We apply it to the experimental validation of the n^{3/2} law in neural combinatorial optimization (max-cut) and obtain empirical exponent 1.51 in training, crossing the theoretical Edwards (1973) boundary.',
      },
    ],
  },
];

async function main() {
  for (const p of seedPapers) {
    await prisma.paper.upsert({
      where: { slug: p.slug },
      update: { date: p.date, authors: p.authors, tags: p.tags },
      create: { slug: p.slug, date: p.date, authors: p.authors, tags: p.tags },
    });
    for (const i of p.i18n) {
      await prisma.paperI18n.upsert({
        where: { paperSlug_lang: { paperSlug: p.slug, lang: i.lang } },
        update: { title: i.title, abstract: i.abstract },
        create: { paperSlug: p.slug, lang: i.lang, title: i.title, abstract: i.abstract },
      });
    }
    // Seed do PDF/TeX em PT a partir do filesystem (legado).
    // Versões EN são subidas separadamente via upsert_paper_file.js.
    for (const kind of ['pdf', 'tex']) {
      const path = `${PAPERS_DIR}/${p.slug}.${kind}`;
      if (!existsSync(path)) {
        console.log(`skip ${p.slug}.${kind}: not found`);
        continue;
      }
      const data = readFileSync(path);
      await prisma.paperFile.upsert({
        where: { paperSlug_lang_kind: { paperSlug: p.slug, lang: 'pt', kind } },
        update: { data, sizeBytes: data.length },
        create: { paperSlug: p.slug, lang: 'pt', kind, data, sizeBytes: data.length },
      });
      console.log(`upsert ${p.slug}.${kind} pt: ${data.length} bytes`);
    }
  }
  console.log('done');
  await prisma.$disconnect();
}

main().catch((e) => { console.error(e); process.exit(1); });
