// Upsert genérico de um arquivo (PDF ou TeX) numa linguagem qualquer.
// Útil quando você gera uma nova versão traduzida e quer subir pro banco.
//
// Uso (dentro do container patria-api):
//   docker cp /tmp/lopes_en.pdf patria-api:/tmp/lopes_en.pdf
//   docker exec patria-api node /app/scripts/upsert_paper_file.js \
//     --slug lopes --lang en --kind pdf --src /tmp/lopes_en.pdf
//
// Args (todos obrigatórios):
//   --slug  <paper slug>
//   --lang  <pt|en>
//   --kind  <pdf|tex>
//   --src   <path do arquivo>

const { PrismaClient } = require('@prisma/client');
const { readFileSync, existsSync } = require('fs');

function arg(name) {
  const i = process.argv.indexOf(`--${name}`);
  if (i < 0 || i + 1 >= process.argv.length) return null;
  return process.argv[i + 1];
}

async function main() {
  const slug = arg('slug');
  const lang = arg('lang');
  const kind = arg('kind');
  const src = arg('src');
  for (const [k, v] of [['slug', slug], ['lang', lang], ['kind', kind], ['src', src]]) {
    if (!v) { console.error(`missing --${k}`); process.exit(2); }
  }
  if (!['pt', 'en'].includes(lang)) { console.error('lang must be pt or en'); process.exit(2); }
  if (!['pdf', 'tex'].includes(kind)) { console.error('kind must be pdf or tex'); process.exit(2); }
  if (!existsSync(src)) { console.error(`src not found: ${src}`); process.exit(2); }

  const prisma = new PrismaClient();
  const data = readFileSync(src);
  await prisma.paperFile.upsert({
    where: { paperSlug_lang_kind: { paperSlug: slug, lang, kind } },
    update: { data, sizeBytes: data.length },
    create: { paperSlug: slug, lang, kind, data, sizeBytes: data.length },
  });
  console.log(`upsert ${slug}.${kind} ${lang}: ${data.length} bytes`);
  await prisma.$disconnect();
}

main().catch((e) => { console.error(e); process.exit(1); });
