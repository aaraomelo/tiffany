// Seed dos perfis padrão de Controle de Acesso (RBAC/CASL) por tenant.
//
// Rodar:  npm run seed:access
//
// Idempotente: faz upsert dos perfis de sistema e recria suas regras. Atribui
// um perfil a cada usuário (mapeando o enum UserRole) APENAS se o usuário ainda
// não tiver nenhum perfil — preserva customizações feitas na tela.

import { PrismaClient, UserRole } from '@prisma/client';

const prisma = new PrismaClient();

type Rule = {
  action: string | string[];
  subject: string | string[];
  inverted?: boolean;
};

// Recursos operacionais (sem os de administração: Role/User/Module).
const OPERATIONAL = [
  'Customer',
  'Product',
  'Stock',
  'Order',
  'Cash',
  'Wallet',
  'ServiceOrder',
  'Budget',
  'Student',
  'EnrollmentPlan',
  'Enrollment',
  'Tuition',
  'Theme',
];

type RoleSeed = { name: string; description: string; rules: Rule[] };

const ROLES: RoleSeed[] = [
  {
    name: 'Administrador',
    description: 'Acesso total ao sistema.',
    rules: [{ action: 'manage', subject: 'all' }],
  },
  {
    name: 'Gerente',
    description: 'Gerencia operações; vê o controle de acesso.',
    rules: [
      { action: 'manage', subject: OPERATIONAL },
      { action: 'read', subject: ['Role', 'User', 'Module'] },
    ],
  },
  {
    name: 'Operador',
    description: 'Vendas e atendimento do dia a dia.',
    rules: [
      {
        action: ['create', 'read', 'update'],
        subject: [
          'Customer',
          'Product',
          'Order',
          'Cash',
          'ServiceOrder',
          'Budget',
          'Student',
          'Enrollment',
          'Tuition',
        ],
      },
      { action: 'read', subject: ['Stock', 'Wallet', 'EnrollmentPlan'] },
    ],
  },
  {
    name: 'Somente leitura',
    description: 'Apenas visualiza, sem alterar nada.',
    rules: [{ action: 'read', subject: 'all' }],
  },
];

const ROLE_BY_ENUM: Record<UserRole, string> = {
  OWNER: 'Administrador',
  ADMIN: 'Administrador',
  MANAGER: 'Gerente',
  CASHIER: 'Operador',
  SELLER: 'Operador',
  STOCK: 'Operador',
  MECHANIC: 'Operador',
  READONLY: 'Somente leitura',
};

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
