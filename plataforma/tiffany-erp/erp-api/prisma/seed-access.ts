// Seed dos perfis padrão de Controle de Acesso (RBAC/CASL) por tenant.
//
// Rodar:  npm run seed:access
//
// Idempotente: faz upsert dos perfis de sistema e recria suas regras. Atribui
// um perfil a cada usuário (mapeando o enum UserRole) APENAS se o usuário ainda
// não tiver nenhum perfil — preserva customizações feitas na tela.

import { PrismaClient } from '@prisma/client';
import { ROLE_BY_ENUM, SYSTEM_ROLES } from '../src/access/system-roles';

const prisma = new PrismaClient();

const ROLES = SYSTEM_ROLES;

async function seedTenant(tenantId: string, alias: string) {
  const idByName: Record<string, string> = {};

  for (const r of ROLES) {
    const role = await prisma.accessRole.upsert({
      where: { tenantId_name: { tenantId, name: r.name } },
      create: {
        tenantId,
        name: r.name,
        description: r.description,
        isSystem: true,
      },
      update: { description: r.description, isSystem: true },
    });
    // recria as regras pra refletir a definição atual
    await prisma.accessRule.deleteMany({ where: { roleId: role.id } });
    await prisma.accessRule.createMany({
      data: r.rules.map((rule) => ({
        roleId: role.id,
        action: rule.action,
        subject: rule.subject,
        inverted: rule.inverted ?? false,
      })),
    });
    idByName[r.name] = role.id;
  }

  // atribui perfil só a quem ainda não tem nenhum
  const users = await prisma.tenantUser.findMany({
    where: { tenantId },
    select: { id: true, role: true, _count: { select: { accessRoles: true } } },
  });
  let assigned = 0;
  for (const u of users) {
    if (u._count.accessRoles > 0) continue;
    const roleName = ROLE_BY_ENUM[u.role];
    const roleId = idByName[roleName];
    if (!roleId) continue;
    await prisma.tenantUser.update({
      where: { id: u.id },
      data: { accessRoles: { connect: { id: roleId } } },
    });
    assigned++;
  }

  console.log(`· tenant ${alias}: ${ROLES.length} perfis · ${assigned} usuário(s) atribuído(s)`);
}

async function main() {
  const tenants = await prisma.tenant.findMany({
    select: { id: true, alias: true },
  });
  if (tenants.length === 0) {
    console.log('· sem tenants');
    return;
  }
  for (const t of tenants) await seedTenant(t.id, t.alias);
  console.log('✓ perfis de acesso semeados');
}

main()
  .catch((e) => {
    console.error(e);
    process.exit(1);
  })
  .finally(async () => {
    await prisma.$disconnect();
  });
