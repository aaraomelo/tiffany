import type { MongoAbility } from '@casl/ability';

// Ações no estilo CASL. `manage` = curinga (qualquer ação).
export const ACTIONS = [
  'manage',
  'create',
  'read',
  'update',
  'delete',
] as const;
export type Action = (typeof ACTIONS)[number];

// Recursos protegíveis. `all` = curinga (qualquer recurso).
export type Subject =
  | 'all'
  | 'Customer'
  | 'Product'
  | 'Stock'
  | 'Order'
  | 'Cash'
  | 'Wallet'
  | 'ServiceOrder'
  | 'Budget'
  | 'Student'
  | 'EnrollmentPlan'
  | 'Enrollment'
  | 'Tuition'
  | 'Role'
  | 'User'
  | 'Module'
  | 'Theme';

export type AppAbility = MongoAbility<[Action, Subject]>;

// Catálogo de recursos para a tela montar a matriz de permissões, agrupado
// pelo módulo a que pertence (mesmos slugs do catálogo de módulos).
// `model` = nome do model Prisma correspondente (quando há tabela isolada por
// tenant). Subject CASL ≠ nome do model: a RLS injeta a condição efetiva na
// query pelo model, e precisa do mapa inverso. Subjects sem `model` (ex.:
// Theme, que é JSON no Tenant) não recebem escopo de linha.
export interface SubjectMeta {
  key: Exclude<Subject, 'all'>;
  module: string; // slug do módulo (categoria visual na tela)
  model?: string; // model Prisma isolado por tenant (para a RLS)
}

export const SUBJECTS: SubjectMeta[] = [
  { key: 'Customer', module: 'customer-supplier', model: 'CustomerSupplier' },
  { key: 'Product', module: 'product', model: 'Product' },
  { key: 'Stock', module: 'stock', model: 'Stock' },
  { key: 'Order', module: 'order', model: 'Order' },
  { key: 'Cash', module: 'cash', model: 'Cash' },
  { key: 'Wallet', module: 'wallet', model: 'MerchantWallet' },
  { key: 'ServiceOrder', module: 'service-order', model: 'ServiceOrder' },
  { key: 'Budget', module: 'budget', model: 'Budget' },
  { key: 'Student', module: 'student', model: 'Student' },
  { key: 'EnrollmentPlan', module: 'enrollment-plan', model: 'EnrollmentPlan' },
  { key: 'Enrollment', module: 'enrollment', model: 'Enrollment' },
  { key: 'Tuition', module: 'tuition', model: 'Tuition' },
  { key: 'Role', module: 'access-control', model: 'AccessRole' },
  { key: 'User', module: 'access-control', model: 'TenantUser' },
  { key: 'Module', module: 'access-control', model: 'TenantModule' },
  { key: 'Theme', module: 'theme' },
];

// model Prisma → subject CASL (mapa inverso usado pela extensão RLS).
export const SUBJECT_BY_MODEL: Record<string, Exclude<Subject, 'all'>> =
  SUBJECTS.reduce(
    (acc, s) => {
      if (s.model) acc[s.model] = s.key;
      return acc;
    },
    {} as Record<string, Exclude<Subject, 'all'>>,
  );

// Forma serializada de uma regra/capacidade (como guardada no banco / enviada
// pela API). `propagationDepth` é a intensidade δ (orçamento de re-delegação):
// null = ∞ (irrestrito, legado), 0 = não re-delegável. Ver delegation.ts e
// doc/casl-propagation.tex §4. Verificada no ato de delegar, não na leitura.
export interface RuleInput {
  action: Action | Action[];
  subject: Subject | Subject[];
  fields?: string | string[] | null;
  conditions?: Record<string, unknown> | null;
  inverted?: boolean;
  reason?: string | null;
  propagationDepth?: number | null;
}
