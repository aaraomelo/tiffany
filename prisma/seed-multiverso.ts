/**
 * Seed inicial dos 14 voluntários do multiverso distribuído.
 *
 * Roda assim (no servidor patria, depois de prisma migrate deploy):
 *   npx ts-node prisma/seed-multiverso.ts
 *
 * Idempotente: usa upsertVoluntario que atualiza se já existir.
 */
import { PrismaClient } from '@prisma/client';

const EMBEDDER_URL = process.env.THEORY_EMBEDDER_URL || 'http://127.0.0.1:9301';
const prisma = new PrismaClient();

interface Vol {
  vid: string;
  displayName: string;
  host: string;
  role: 'voluntary' | 'general' | 'coord';
  runtime: 'python' | 'haskell-hs';
  isLab: boolean;
  description: string;
}

// Catálogo inicial: só paubrasil (laboratório do general Haskell).
// Os outros 13 voluntários Python entram no catálogo conforme migrarem
// pra volunteer-hs — atualiza este array e roda o seed de novo.
const VOLUNTARIOS: Vol[] = [
  {
    vid: 'paubrasil-srv-tools',
    displayName: 'Pau Brasil',
    host: 'paubrasil',
    role: 'general',
    runtime: 'haskell-hs',
    isLab: true,
    description:
      'Pau Brasil — laboratório oficial do general Haskell. 8 cores, 20 GiB RAM. ' +
      'Roda volunteer-hs (reescrita Haskell completa: TorchSave + Coord + Train + canal WS de controle). ' +
      'Primeiro voluntário a usar a versão Haskell em produção e bancada de testes ' +
      'pra qualquer feature nova antes do rolling deploy do exército. ' +
      'Suporta override de carga via canal WS (factor de 0 a 1, duration_sec). ' +
      'Os outros 13 voluntários ainda rodam volunteer.py em Python e vão migrar gradualmente.',
  },
];

async function embed(text: string): Promise<number[] | null> {
  try {
    const res = await fetch(`${EMBEDDER_URL}/embed-query`, {
      method: 'POST',
      headers: { 'Content-Type': 'application/json' },
      body: JSON.stringify({ text }),
      signal: AbortSignal.timeout(15000),
    });
    if (!res.ok) {
      console.error(`embedder HTTP ${res.status}`);
      return null;
    }
    const data: any = await res.json();
    return data.vector || null;
  } catch (e) {
    console.error('embed falhou:', String(e));
    return null;
  }
}

async function main() {
  console.log(`Seedando ${VOLUNTARIOS.length} voluntários...`);
  for (const v of VOLUNTARIOS) {
    const baseData = {
      vid: v.vid, displayName: v.displayName, host: v.host,
      role: v.role, runtime: v.runtime, isLab: v.isLab,
      description: v.description,
    };
    await prisma.multiversoVoluntario.upsert({
      where: { vid: v.vid },
      update: baseData,
      create: baseData,
    });
    const emb = await embed(v.description);
    if (emb) {
      await prisma.$executeRawUnsafe(
        `UPDATE multiverso_voluntarios SET embedding = $1::vector, embedded_at = NOW() WHERE vid = $2`,
        `[${emb.join(',')}]`,
        v.vid,
      );
      console.log(`  ✓ ${v.displayName} (${v.vid})`);
    } else {
      console.log(`  ⚠ ${v.displayName} (${v.vid}) — sem embedding`);
    }
  }
  console.log('Done.');
  await prisma.$disconnect();
}

main().catch((e) => {
  console.error(e);
  process.exit(1);
});
