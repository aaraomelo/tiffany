// Definição canônica dos perfis de sistema (RBAC/CASL), sem efeitos colaterais.
// Fonte da verdade compartilhada entre o seed (`prisma/seed-access.ts`) e o
// teste de contrato (`ability.contract.spec.ts`). Mexeu aqui → o contrato
// trava a regressão. CASL-free de propósito (importável em qualquer lugar).

export interface SystemRoleRule {
  action: string | string[];
  subject: string | string[];
  inverted?: boolean;
}

export interface SystemRole {
  name: string;
  description: string;
  rules: SystemRoleRule[];
}

// Recursos operacionais (sem os de administração: Role/User/Module).
export const OPERATIONAL = [
  'Customer',
  'Supplier',
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
] as const;

export const SYSTEM_ROLES: SystemRole[] = [
  {
    name: 'Administrador',
    description: 'Acesso total ao sistema.',
    rules: [{ action: 'manage', subject: 'all' }],
  },
  {
    name: 'Gerente',
    description: 'Gerencia operações; vê o controle de acesso.',
    rules: [
      { action: 'manage', subject: [...OPERATIONAL] },
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

// enum UserRole (string) → nome do perfil de sistema.
export const ROLE_BY_ENUM: Record<string, string> = {
  OWNER: 'Administrador',
  ADMIN: 'Administrador',
  MANAGER: 'Gerente',
  CASHIER: 'Operador',
  SELLER: 'Operador',
  STOCK: 'Operador',
  MECHANIC: 'Operador',
  READONLY: 'Somente leitura',
};
