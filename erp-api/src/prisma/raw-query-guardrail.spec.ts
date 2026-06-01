import { readdirSync, readFileSync, statSync } from 'fs';
import { join, relative } from 'path';

// Guardrail: queries raw ($queryRaw/$executeRaw) escapam da extensão RLS de
// aplicação. Só são permitidas as que escopam o tenant manualmente e estão
// nesta allowlist (com justificativa). Surgiu uma nova → CI vermelho, forçando
// escopo explícito até a RLS nativa do Postgres (Fase B).
const ALLOWLIST = new Set<string>([
  'product/product.service.ts', // pgvector: WHERE "tenantId" = $2 (parametrizado)
  'dashboard/dashboard.service.ts', // estoque baixo: WHERE "tenantId" = $1
  'assistant/assistant-memory.service.ts', // pgvector da assistente (escopo tenant no SELECT)
  'embedding/embedding.service.ts', // só menção em comentário (helper toVectorLiteral)
]);

const SRC = join(__dirname, '..');
const RAW = /\$(queryRaw|executeRaw)/;

function walk(dir: string): string[] {
  const out: string[] = [];
  for (const entry of readdirSync(dir)) {
    const full = join(dir, entry);
    if (statSync(full).isDirectory()) {
      out.push(...walk(full));
    } else if (entry.endsWith('.ts') && !entry.endsWith('.spec.ts')) {
      out.push(full);
    }
  }
  return out;
}

describe('guardrail de raw queries', () => {
  it('nenhuma raw query fora da allowlist escopada', () => {
    const offenders = walk(SRC)
      .filter((f) => RAW.test(readFileSync(f, 'utf8')))
      .map((f) => relative(SRC, f))
      .filter((rel) => !ALLOWLIST.has(rel));
    expect(offenders).toEqual([]);
  });
});
