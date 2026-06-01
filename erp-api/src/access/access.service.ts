import {
  BadRequestException,
  ForbiddenException,
  Injectable,
  NotFoundException,
} from '@nestjs/common';
import { packRules } from '@casl/ability/extra';
import { Prisma } from '@prisma/client';
import { requireTenantId } from '../common/tenant-context/tenant-context';
import { PrismaService } from '../prisma/prisma.service';
import { buildAbility } from './ability.factory';
import {
  ACTIONS,
  SUBJECTS,
  type Action,
  type Subject,
} from './access.types';
import {
  AssignRolesDto,
  CreateRoleDto,
  RuleDto,
  UpdateRoleDto,
} from './dto/access.dto';

const VALID_ACTIONS = new Set<string>(ACTIONS);
const VALID_SUBJECTS = new Set<string>(['all', ...SUBJECTS.map((s) => s.key)]);

@Injectable()
export class AccessService {
  constructor(private readonly prisma: PrismaService) {}

  // -------- catálogo (para a tela montar a matriz) --------
  catalog() {
    return { actions: ACTIONS, subjects: SUBJECTS };
  }

  // -------- perfis --------
  async listRoles() {
    const tenantId = requireTenantId();
    const roles = await this.prisma.accessRole.findMany({
      where: { tenantId },
      orderBy: [{ isSystem: 'desc' }, { name: 'asc' }],
      include: {
        rules: true,
        _count: { select: { users: true } },
      },
    });
    return roles.map((r) => ({
      id: r.id,
      name: r.name,
      description: r.description,
      isSystem: r.isSystem,
      userCount: r._count.users,
      rules: r.rules.map(this.toRuleOut),
    }));
  }

  async getRole(id: string) {
    const tenantId = requireTenantId();
    const role = await this.prisma.accessRole.findFirst({
      where: { id, tenantId },
      include: { rules: true, _count: { select: { users: true } } },
    });
    if (!role) throw new NotFoundException('Perfil não encontrado');
    return {
      id: role.id,
      name: role.name,
      description: role.description,
      isSystem: role.isSystem,
      userCount: role._count.users,
      rules: role.rules.map(this.toRuleOut),
    };
  }

  async createRole(dto: CreateRoleDto) {
    const tenantId = requireTenantId();
    const rules = (dto.rules ?? []).map((r) => this.sanitizeRule(r));
    const role = await this.prisma.accessRole.create({
      data: {
        tenantId,
        name: dto.name,
        description: dto.description,
        rules: { create: rules },
      },
    });
    return this.getRole(role.id);
  }

  async updateRole(id: string, dto: UpdateRoleDto) {
    const tenantId = requireTenantId();
    const existing = await this.prisma.accessRole.findFirst({
      where: { id, tenantId },
      select: { id: true },
    });
    if (!existing) throw new NotFoundException('Perfil não encontrado');

    await this.prisma.$transaction(async (tx) => {
      await tx.accessRole.update({
        where: { id },
        data: {
          ...(dto.name !== undefined ? { name: dto.name } : {}),
          ...(dto.description !== undefined
            ? { description: dto.description }
            : {}),
        },
      });
      // replace total das regras quando vier `rules`
      if (dto.rules !== undefined) {
        await tx.accessRule.deleteMany({ where: { roleId: id } });
        const rules = dto.rules.map((r) => this.sanitizeRule(r));
        if (rules.length > 0) {
          await tx.accessRule.createMany({
            data: rules.map((r) => ({ ...r, roleId: id })),
          });
        }
      }
    });
    return this.getRole(id);
  }

  async deleteRole(id: string) {
    const tenantId = requireTenantId();
    const role = await this.prisma.accessRole.findFirst({
      where: { id, tenantId },
      select: { id: true, isSystem: true },
    });
    if (!role) throw new NotFoundException('Perfil não encontrado');
    if (role.isSystem) {
      throw new ForbiddenException('perfil de sistema não pode ser excluído');
    }
    await this.prisma.accessRole.delete({ where: { id } });
    return { ok: true };
  }

  // -------- usuários e atribuição --------
  async listUsers() {
    const tenantId = requireTenantId();
    const users = await this.prisma.tenantUser.findMany({
      where: { tenantId },
      orderBy: { name: 'asc' },
      select: {
        id: true,
        name: true,
        email: true,
        role: true,
        active: true,
        accessRoles: { select: { id: true, name: true } },
      },
    });
    return users;
  }

  async setUserRoles(userId: string, dto: AssignRolesDto) {
    const tenantId = requireTenantId();
    const user = await this.prisma.tenantUser.findFirst({
      where: { id: userId, tenantId },
      select: { id: true },
    });
    if (!user) throw new NotFoundException('Usuário não encontrado');

    // garante que todos os perfis pertencem ao tenant
    const roles = await this.prisma.accessRole.findMany({
      where: { id: { in: dto.roleIds }, tenantId },
      select: { id: true },
    });
    if (roles.length !== dto.roleIds.length) {
      throw new BadRequestException('um ou mais perfis são inválidos');
    }

    await this.prisma.tenantUser.update({
      where: { id: userId },
      data: { accessRoles: { set: roles.map((r) => ({ id: r.id })) } },
    });
    return this.listUsers();
  }

  // -------- abilities do usuário logado (para o front) --------
  async me(userId: string) {
    const user = await this.prisma.tenantUser.findUnique({
      where: { id: userId },
      include: { accessRoles: { include: { rules: true } } },
    });
    const roles = user?.accessRoles ?? [];
    const ability = buildAbility(roles);
    return {
      roles: roles.map((r) => ({ id: r.id, name: r.name })),
      rules: packRules(ability.rules),
    };
  }

  // -------- helpers --------
  private toRuleOut = (r: {
    id: string;
    action: Prisma.JsonValue;
    subject: Prisma.JsonValue;
    fields: Prisma.JsonValue;
    conditions: Prisma.JsonValue;
    inverted: boolean;
    reason: string | null;
  }) => ({
    id: r.id,
    action: r.action,
    subject: r.subject,
    fields: r.fields ?? null,
    conditions: r.conditions ?? null,
    inverted: r.inverted,
    reason: r.reason,
  });

  private sanitizeRule(r: RuleDto) {
    const actions = this.asArray(r.action).filter((a) =>
      VALID_ACTIONS.has(a),
    ) as Action[];
    const subjects = this.asArray(r.subject).filter((s) =>
      VALID_SUBJECTS.has(s),
    ) as Subject[];
    if (actions.length === 0) {
      throw new BadRequestException('regra sem ação válida');
    }
    if (subjects.length === 0) {
      throw new BadRequestException('regra sem recurso válido');
    }
    return {
      action: (actions.length === 1 ? actions[0] : actions) as Prisma.InputJsonValue,
      subject: (subjects.length === 1
        ? subjects[0]
        : subjects) as Prisma.InputJsonValue,
      fields: (r.fields ?? undefined) as Prisma.InputJsonValue | undefined,
      conditions: (r.conditions ?? undefined) as
        | Prisma.InputJsonValue
        | undefined,
      inverted: r.inverted ?? false,
      reason: r.reason ?? null,
    };
  }

  private asArray(v: unknown): string[] {
    if (Array.isArray(v)) return v.map(String);
    if (typeof v === 'string') return [v];
    return [];
  }

  /**
   * Provisiona os perfis padrão de um tenant novo (Administrador/Gerente/
   * Operador/Somente leitura) e atribui o OWNER ao Administrador. Usado no
   * cadastro self-service. Idempotente.
   */
  async provisionDefaults(tenantId: string, ownerUserId: string) {
    const OPERATIONAL = [
      'Customer', 'Product', 'Stock', 'Order', 'Cash', 'Wallet',
      'ServiceOrder', 'Budget', 'Student', 'EnrollmentPlan', 'Enrollment',
      'Tuition', 'Theme',
    ];
    const ROLES = [
      { name: 'Administrador', description: 'Acesso total ao sistema.', rules: [{ action: 'manage', subject: 'all' }] },
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
          { action: ['create', 'read', 'update'], subject: ['Customer', 'Product', 'Order', 'Cash', 'ServiceOrder', 'Budget', 'Student', 'Enrollment', 'Tuition'] },
          { action: 'read', subject: ['Stock', 'Wallet', 'EnrollmentPlan'] },
        ],
      },
      { name: 'Somente leitura', description: 'Apenas visualiza, sem alterar nada.', rules: [{ action: 'read', subject: 'all' }] },
    ];

    let adminRoleId = '';
    for (const r of ROLES) {
      const role = await this.prisma.accessRole.upsert({
        where: { tenantId_name: { tenantId, name: r.name } },
        create: { tenantId, name: r.name, description: r.description, isSystem: true },
        update: { description: r.description, isSystem: true },
      });
      await this.prisma.accessRule.deleteMany({ where: { roleId: role.id } });
      await this.prisma.accessRule.createMany({
        data: r.rules.map((rule) => ({
          roleId: role.id,
          action: rule.action as Prisma.InputJsonValue,
          subject: rule.subject as Prisma.InputJsonValue,
        })),
      });
      if (r.name === 'Administrador') adminRoleId = role.id;
    }

    if (adminRoleId) {
      await this.prisma.tenantUser.update({
        where: { id: ownerUserId },
        data: { accessRoles: { connect: { id: adminRoleId } } },
      });
    }
  }
}
