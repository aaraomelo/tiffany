// Cria um tenant de demonstração da Escola de Futebol já no pack correto,
// com um usuário OWNER para login local.
//
// Rodar:  npx ts-node -r tsconfig-paths/register prisma/seed-login.ts
//
// Idempotente: usa upsert em Tenant/TenantUser e aplica o pack football-school.

import { PrismaClient } from '@prisma/client';
import * as bcrypt from 'bcrypt';

const prisma = new PrismaClient();

const ALIAS = 'escola';
const EMAIL = 'admin@escola.test';
const PASSWORD = 'escola123';
const PACK = 'football-school';

async function applyPack(tenantId: string, packSlug: string) {
  const pack = await prisma.modulePack.findUnique({
    where: { slug: packSlug },
    include: { items: true },
  });
  if (!pack) throw new Error(`pack '${packSlug}' não encontrado — rode npm run seed:modules antes`);

  const allModules = await prisma.module.findMany({
    select: { slug: true, isCore: true },
  });
  const packSlugs = new Set(pack.items.map((i) => i.moduleSlug));

  await prisma.tenant.update({ where: { id: tenantId }, data: { packSlug } });

  // replace: este é o único segmento ativo do tenant
  await prisma.tenantPack.deleteMany({ where: { tenantId } });
  await prisma.tenantPack.create({ data: { tenantId, packSlug } });

  for (const m of allModules) {
    const shouldEnable = m.isCore || packSlugs.has(m.slug);
    await prisma.tenantModule.upsert({
      where: { tenantId_moduleSlug: { tenantId, moduleSlug: m.slug } },
      create: {
        tenantId,
        moduleSlug: m.slug,
        enabled: shouldEnable,
        disabledAt: shouldEnable ? null : new Date(),
      },
      update: {
        enabled: shouldEnable,
        disabledAt: shouldEnable ? null : new Date(),
      },
    });
  }
}

async function main() {
  const tenant = await prisma.tenant.upsert({
    where: { alias: ALIAS },
    create: {
      alias: ALIAS,
      name: 'Escola de Futebol Demo',
      packSlug: PACK,
      status: 'ACTIVE',
    },
    update: { name: 'Escola de Futebol Demo' },
  });
  console.log(`✓ tenant '${tenant.alias}' (${tenant.id})`);

  await applyPack(tenant.id, PACK);
  console.log(`✓ pack '${PACK}' aplicado`);

  const passwordHash = await bcrypt.hash(PASSWORD, 10);
  await prisma.tenantUser.upsert({
    where: { tenantId_email: { tenantId: tenant.id, email: EMAIL } },
    create: {
      tenantId: tenant.id,
      email: EMAIL,
      passwordHash,
      name: 'Admin Escola',
      role: 'OWNER',
      active: true,
    },
    update: { passwordHash, active: true, role: 'OWNER' },
  });

  console.log('\n✓ credenciais de login:');
  console.log(`   alias:  ${ALIAS}`);
  console.log(`   email:  ${EMAIL}`);
  console.log(`   senha:  ${PASSWORD}`);
}

main()
  .catch((e) => {
    console.error(e);
    process.exit(1);
  })
  .finally(async () => {
    await prisma.$disconnect();
  });
