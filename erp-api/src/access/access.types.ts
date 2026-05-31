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
export interface SubjectMeta {
  key: Exclude<Subject, 'all'>;
  module: string; // slug do módulo (categoria visual na tela)
}

export const SUBJECTS: SubjectMeta[] = [
  { key: 'Customer', module: 'customer-supplier' },
  { key: 'Product', module: 'product' },
  { key: 'Stock', module: 'stock' },
  { key: 'Order', module: 'order' },
  { key: 'Cash', module: 'cash' },
  { key: 'Wallet', module: 'wallet' },
  { key: 'ServiceOrder', module: 'service-order' },
  { key: 'Budget', module: 'budget' },
  { key: 'Student', module: 'student' },
  { key: 'EnrollmentPlan', module: 'enrollment-plan' },
  { key: 'Enrollment', module: 'enrollment' },
  { key: 'Tuition', module: 'tuition' },
  { key: 'Role', module: 'access-control' },
  { key: 'User', module: 'access-control' },
  { key: 'Module', module: 'access-control' },
  { key: 'Theme', module: 'theme' },
];

// Forma serializada de uma regra (como guardada no banco / enviada pela API).
export interface RuleInput {
  action: Action | Action[];
  subject: Subject | Subject[];
  fields?: string | string[] | null;
  conditions?: Record<string, unknown> | null;
  inverted?: boolean;
  reason?: string | null;
}
