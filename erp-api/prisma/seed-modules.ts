// Seed idempotente do catálogo de Módulos + Packs por segmento.
//
// Rodar:
//   npm run seed:modules
//
// O script é seguro: usa upsert em Module/ModulePack/ModulePackItem e
// faz backfill em tenants existentes sem packSlug → vira 'commerce'.

import { ModuleCategory, PrismaClient } from '@prisma/client';

const prisma = new PrismaClient();

type ModuleSeed = {
  slug: string;
  name: string;
  description?: string;
  category: ModuleCategory;
  routePath?: string;
  iconKey?: string;
  isCore?: boolean;
  sortOrder: number;
};

const MODULES: ModuleSeed[] = [
  // ---------- CORE (sempre ligado) ----------
  { slug: 'auth',              name: 'Autenticação',       category: 'CORE', isCore: true, sortOrder:  0 },
  { slug: 'tenant',            name: 'Tenant',             category: 'CORE', isCore: true, sortOrder:  1 },
  { slug: 'catalog',           name: 'Catálogo',           category: 'CORE', isCore: true, sortOrder:  2 },
  { slug: 'warehouse',         name: 'Depósitos',          category: 'CORE', isCore: true, sortOrder:  3 },
  { slug: 'customer-supplier', name: 'Clientes / Fornecedores', category: 'CORE', isCore: true, routePath: '/customers', iconKey: 'users',  sortOrder: 10 },
  { slug: 'product',           name: 'Produtos',           category: 'CORE', isCore: true, routePath: '/products', iconKey: 'box',    sortOrder: 11 },
  { slug: 'stock',             name: 'Estoque',            category: 'CORE', isCore: true, routePath: '/stock',    iconKey: 'layers', sortOrder: 12 },
  { slug: 'theme',             name: 'Aparência',          category: 'CORE', isCore: true, routePath: '/theme',    iconKey: 'palette', sortOrder: 90 },
  { slug: 'assistant',         name: 'Assistente Patrícia',category: 'CORE', isCore: true, routePath: '/assistant',iconKey: 'message', sortOrder: 91 },

  // ---------- SALES (PDV / Vendas) ----------
  { slug: 'pos',      name: 'PDV',        category: 'SALES', routePath: '/pos',     iconKey: 'shopping-cart', sortOrder: 20 },
  { slug: 'order',    name: 'Pedidos',    category: 'SALES', routePath: '/orders',  iconKey: 'list',          sortOrder: 21 },
  { slug: 'payment',  name: 'Pagamentos', category: 'SALES', sortOrder: 22 },
  { slug: 'checkout', name: 'Checkout',   category: 'SALES', sortOrder: 23 },
  { slug: 'cash',     name: 'Caixa',      category: 'SALES', routePath: '/cash',    iconKey: 'wallet',        sortOrder: 24 },
  { slug: 'wallet',   name: 'Carteira',   category: 'SALES', routePath: '/wallet',  iconKey: 'credit-card',   sortOrder: 25 },

  // ---------- SERVICE (Peças & Serviços) ----------
  { slug: 'service-order',  name: 'Ordem de Serviço', category: 'SERVICE', routePath: '/service-orders', iconKey: 'wrench',   sortOrder: 30 },
  { slug: 'budget',         name: 'Orçamentos',       category: 'SERVICE', routePath: '/budgets',        iconKey: 'file-text', sortOrder: 31 },
  { slug: 'vehicle',        name: 'Veículos',         category: 'SERVICE', sortOrder: 32 },
  { slug: 'warranty-term',  name: 'Termos de Garantia', category: 'SERVICE', sortOrder: 33 },

  // ---------- SCHOOL (Escola de Futebol) ----------
  { slug: 'student',         name: 'Alunos',       category: 'SCHOOL', routePath: '/students',         iconKey: 'users',     sortOrder: 40 },
  { slug: 'enrollment-plan', name: 'Planos',       category: 'SCHOOL', routePath: '/enrollment-plans', iconKey: 'file-text', sortOrder: 41 },
  { slug: 'enrollment',      name: 'Matrículas',   category: 'SCHOOL', routePath: '/enrollments',      iconKey: 'list',      sortOrder: 42 },
  { slug: 'tuition',         name: 'Mensalidades', category: 'SCHOOL', routePath: '/tuitions',         iconKey: 'wallet',    sortOrder: 43 },
];

type PackSeed = {
  slug: string;
  name: string;
  segment: string;
  description: string;
  isDefault?: boolean;
  sortOrder: number;
  // núcleo é incluído automaticamente; aqui só listamos os módulos extras
  modules: string[];
};

// Slugs CORE incluídos em todos os packs
const CORE_SLUGS = MODULES.filter(m => m.isCore).map(m => m.slug);

const SALES_BASE = ['pos', 'order', 'payment', 'checkout', 'cash', 'wallet'];
const SERVICE_BASE = ['service-order', 'budget', 'vehicle', 'warranty-term'];
const SCHOOL_BASE = ['student', 'enrollment-plan', 'enrollment', 'tuition'];

const PACKS: PackSeed[] = [
  {
    slug: 'commerce',
    name: 'Comércio',
    segment: 'commerce',
    description: 'Loja com PDV, vendas, caixa e estoque. Ideal pro varejo geral.',
    isDefault: true,
    sortOrder: 1,
    modules: SALES_BASE,
  },
  {
    slug: 'parts-service',
    name: 'Peças & Serviços',
    segment: 'parts_service',
    description: 'Autopeças, oficina, eletrônica, marcenaria. Combina produto físico com ordem de serviço.',
    sortOrder: 2,
    modules: [...SALES_BASE, ...SERVICE_BASE],
  },
  {
    slug: 'construction',
    name: 'Materiais de Construção',
    segment: 'construction',
    description: 'Loja de materiais de construção. Mesma base do comércio, com foco em ticket alto.',
    sortOrder: 3,
    modules: SALES_BASE,
  },
  {
    slug: 'food',
    name: 'Food Service',
    segment: 'food',
    description: 'Restaurantes, bares, lanchonetes. Núcleo de vendas; mesa/comanda/cardápio chegam em fase futura.',
    sortOrder: 4,
    modules: SALES_BASE,
  },
  {
    slug: 'event',
    name: 'Eventos',
    segment: 'event',
    description: 'Casas noturnas, festas, festivais. Núcleo de vendas; ingresso e comanda eletrônica chegam em fase futura.',
    sortOrder: 5,
    modules: SALES_BASE,
  },
  {
    slug: 'football-school',
    name: 'Escola de Futebol',
    segment: 'football_school',
    description: 'Escolinhas e centros de treinamento. Cadastro de alunos com anamnese, planos, matrículas e mensalidades.',
    sortOrder: 6,
    modules: SCHOOL_BASE,
  },
];

async function upsertModules() {
  for (const m of MODULES) {
    await prisma.module.upsert({
      where: { slug: m.slug },
      create: {
        slug: m.slug,
        name: m.name,
        description: m.description ?? null,
        category: m.category,
        routePath: m.routePath ?? null,
        iconKey: m.iconKey ?? null,
        isCore: m.isCore ?? false,
        sortOrder: m.sortOrder,
      },
      update: {
        name: m.name,
        description: m.description ?? null,
        category: m.category,
        routePath: m.routePath ?? null,
        iconKey: m.iconKey ?? null,
        isCore: m.isCore ?? false,
        sortOrder: m.sortOrder,
      },
    });
  }
  console.log(`✓ ${MODULES.length} módulos`);
}

async function upsertPacks() {
  for (const p of PACKS) {
    await prisma.modulePack.upsert({
      where: { slug: p.slug },
      create: {
        slug: p.slug,
        name: p.name,
        segment: p.segment,
        description: p.description,
        isDefault: p.isDefault ?? false,
        sortOrder: p.sortOrder,
      },
      update: {
        name: p.name,
        segment: p.segment,
        description: p.description,
        isDefault: p.isDefault ?? false,
        sortOrder: p.sortOrder,
      },
    });

    const allSlugs = Array.from(new Set([...CORE_SLUGS, ...p.modules]));

    // recria os items do pack pra refletir a definição atual
    await prisma.modulePackItem.deleteMany({ where: { packSlug: p.slug } });
    await prisma.modulePackItem.createMany({
      data: allSlugs.map(moduleSlug => ({ packSlug: p.slug, moduleSlug })),
      skipDuplicates: true,
    });
  }
  console.log(`✓ ${PACKS.length} packs`);
}

async function backfillTenants() {
  const tenants = await prisma.tenant.findMany({ select: { id: true, alias: true, packSlug: true } });
  if (tenants.length === 0) {
    console.log('· sem tenants para backfill');
    return;
  }

  const defaultPack = await prisma.modulePack.findFirst({
    where: { isDefault: true },
    include: { items: true },
  });
  if (!defaultPack) throw new Error('pack default não encontrado');

  for (const t of tenants) {
    const packSlug = t.packSlug ?? defaultPack.slug;
    const items = packSlug === defaultPack.slug
      ? defaultPack.items
      : await prisma.modulePackItem.findMany({ where: { packSlug } });

    if (!t.packSlug) {
      await prisma.tenant.update({ where: { id: t.id }, data: { packSlug } });
    }

    await prisma.tenantModule.createMany({
      data: items.map(it => ({
        tenantId: t.id,
        moduleSlug: it.moduleSlug,
        enabled: true,
      })),
      skipDuplicates: true,
    });

    console.log(`· tenant ${t.alias} → pack '${packSlug}' (${items.length} módulos)`);
  }
}

async function main() {
  await upsertModules();
  await upsertPacks();
  await backfillTenants();
}

main()
  .catch((e) => {
    console.error(e);
    process.exit(1);
  })
  .finally(async () => {
    await prisma.$disconnect();
  });
