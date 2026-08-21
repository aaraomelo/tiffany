import { ForbiddenException, Injectable, NotFoundException } from '@nestjs/common';
import { UserRole } from '@prisma/client';
import {
  getTenantContext,
  requireTenantId,
} from '../common/tenant-context/tenant-context';
import { PrismaService } from '../prisma/prisma.service';
import {
  DEFAULT_ALLOWED_TOOLS,
  DEFAULT_PROFILE_PROMPTS,
} from './prompts/soul';

const PRIVILEGED_ROLES = ['OWNER', 'ADMIN', 'MANAGER'];

@Injectable()
export class AssistantProfileService {
  constructor(private readonly prisma: PrismaService) {}

  private assertPrivileged() {
    const { role } = getTenantContext();
    if (!role || !PRIVILEGED_ROLES.includes(role)) {
      throw new ForbiddenException(
        'Apenas OWNER/ADMIN/MANAGER podem alterar perfis do assistente',
      );
    }
  }

  /// Cria perfis default para todos os papéis se ainda não existem.
  async bootstrap() {
    const tenantId = requireTenantId();
    const roles = Object.values(UserRole);
    const existing = await this.prisma.assistantProfile.findMany({
      where: { tenantId },
      select: { role: true },
    });
    const existingRoles = new Set(existing.map((e) => e.role));
    const toCreate = roles.filter((r) => !existingRoles.has(r));
    if (toCreate.length === 0) {
      return { created: 0 };
    }
    await this.prisma.assistantProfile.createMany({
      data: toCreate.map((role) => ({
        tenantId,
        role,
        name: `Assistente — ${role.toLowerCase()}`,
        systemPrompt: DEFAULT_PROFILE_PROMPTS[role] ?? '',
        allowedTools: DEFAULT_ALLOWED_TOOLS[role] ?? [],
        memoryAccess: role === 'OWNER' || role === 'ADMIN' ? 'all' : 'own',
      })),
    });
    return { created: toCreate.length };
  }

  async list() {
    return this.prisma.assistantProfile.findMany({
      where: { tenantId: requireTenantId() },
      orderBy: { role: 'asc' },
    });
  }

  async findByRole(role: UserRole) {
    const profile = await this.prisma.assistantProfile.findUnique({
      where: { tenantId_role: { tenantId: requireTenantId(), role } },
    });
    if (!profile) {
      // se não foi bootstrap ainda, retorna default sem persistir
      return {
        id: null,
        tenantId: requireTenantId(),
        role,
        name: `Assistente — ${role.toLowerCase()}`,
        systemPrompt: DEFAULT_PROFILE_PROMPTS[role] ?? '',
        allowedTools: DEFAULT_ALLOWED_TOOLS[role] ?? [],
        memoryAccess: role === 'OWNER' || role === 'ADMIN' ? 'all' : 'own',
        active: true,
      };
    }
    return profile;
  }

  async update(role: UserRole, dto: {
    name?: string;
    systemPrompt?: string;
    allowedTools?: string[];
    memoryAccess?: string;
    active?: boolean;
  }) {
    this.assertPrivileged();
    const tenantId = requireTenantId();
    await this.prisma.assistantProfile.upsert({
      where: { tenantId_role: { tenantId, role } },
      create: {
        tenantId,
        role,
        name: dto.name ?? `Assistente — ${role.toLowerCase()}`,
        systemPrompt: dto.systemPrompt ?? (DEFAULT_PROFILE_PROMPTS[role] ?? ''),
        allowedTools: dto.allowedTools ?? (DEFAULT_ALLOWED_TOOLS[role] ?? []),
        memoryAccess: dto.memoryAccess ?? 'own',
        active: dto.active ?? true,
      },
      update: {
        ...(dto.name !== undefined ? { name: dto.name } : {}),
        ...(dto.systemPrompt !== undefined ? { systemPrompt: dto.systemPrompt } : {}),
        ...(dto.allowedTools !== undefined ? { allowedTools: dto.allowedTools } : {}),
        ...(dto.memoryAccess !== undefined ? { memoryAccess: dto.memoryAccess } : {}),
        ...(dto.active !== undefined ? { active: dto.active } : {}),
      },
    });
    return this.findByRole(role);
  }

  async resetToDefault(role: UserRole) {
    this.assertPrivileged();
    const tenantId = requireTenantId();
    const exists = await this.prisma.assistantProfile.findUnique({
      where: { tenantId_role: { tenantId, role } },
    });
    if (!exists) throw new NotFoundException('Perfil não existe');
    await this.prisma.assistantProfile.update({
      where: { tenantId_role: { tenantId, role } },
      data: {
        systemPrompt: DEFAULT_PROFILE_PROMPTS[role] ?? '',
        allowedTools: DEFAULT_ALLOWED_TOOLS[role] ?? [],
      },
    });
    return this.findByRole(role);
  }
}
