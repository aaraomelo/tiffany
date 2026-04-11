import { Injectable, NotFoundException, ConflictException } from '@nestjs/common';
import * as bcrypt from 'bcrypt';
import { PrismaService } from '../prisma.service';
import { CreateTenantDto, UpdateTenantDto, RegisterTenantDto } from './tenants.dto';

const TENANT_SELECT = {
  id: true,
  alias: true,
  name: true,
  companyName: true,
  plan: true,
  status: true,
  githubOrg: true,
  githubRepos: true,
  trialEndsAt: true,
  createdAt: true,
  updatedAt: true,
};

const USER_SELECT = {
  id: true,
  tenantId: true,
  email: true,
  name: true,
  role: true,
  createdAt: true,
};

@Injectable()
export class TenantsService {
  constructor(private readonly prisma: PrismaService) {}

  async create(dto: CreateTenantDto) {
    const existing = await this.prisma.tenant.findUnique({ where: { alias: dto.alias } });
    if (existing) {
      throw new ConflictException(`Alias '${dto.alias}' já está em uso`);
    }

    const passwordHash = await bcrypt.hash(dto.adminPassword, 10);
    const trialEndsAt = new Date(Date.now() + 14 * 24 * 60 * 60 * 1000);

    const tenant = await this.prisma.$transaction(async (tx) => {
      const created = await tx.tenant.create({
        data: {
          alias: dto.alias,
          name: dto.name,
          companyName: dto.companyName,
          plan: dto.plan ?? 'starter',
          githubOrg: dto.githubOrg,
          githubRepos: dto.githubRepos ?? [],
          trialEndsAt,
        },
        select: TENANT_SELECT,
      });

      await tx.tenantUser.create({
        data: {
          tenantId: created.id,
          email: dto.adminEmail,
          passwordHash,
          name: dto.adminName,
          role: 'admin',
        },
      });

      return created;
    });

    return tenant;
  }

  async findAll() {
    return this.prisma.tenant.findMany({
      select: TENANT_SELECT,
      orderBy: { createdAt: 'desc' },
    });
  }

  async findOne(id: string) {
    const tenant = await this.prisma.tenant.findUnique({
      where: { id },
      select: {
        ...TENANT_SELECT,
        users: { select: USER_SELECT },
      },
    });

    if (!tenant) {
      throw new NotFoundException(`Tenant '${id}' não encontrado`);
    }

    return tenant;
  }

  async register(dto: RegisterTenantDto) {
    const existing = await this.prisma.tenant.findUnique({ where: { alias: dto.alias } });
    if (existing) {
      throw new ConflictException(`Alias '${dto.alias}' já está em uso`);
    }

    const passwordHash = await bcrypt.hash(dto.senha, 10);
    const trialEndsAt = new Date(Date.now() + 14 * 24 * 60 * 60 * 1000);

    const result = await this.prisma.$transaction(async (tx) => {
      const tenant = await tx.tenant.create({
        data: {
          alias: dto.alias,
          name: dto.nome,
          companyName: dto.empresa,
          plan: 'starter',
          status: 'trial',
          trialEndsAt,
        },
        select: TENANT_SELECT,
      });

      const user = await tx.tenantUser.create({
        data: {
          tenantId: tenant.id,
          email: dto.email,
          passwordHash,
          name: dto.nome,
          role: 'admin',
        },
        select: USER_SELECT,
      });

      return { tenant, user };
    });

    return result;
  }

  async checkAlias(alias: string) {
    const existing = await this.prisma.tenant.findUnique({ where: { alias } });
    return { available: !existing, alias };
  }

  async update(id: string, dto: UpdateTenantDto) {
    await this.findOne(id);

    return this.prisma.tenant.update({
      where: { id },
      data: {
        ...(dto.name !== undefined && { name: dto.name }),
        ...(dto.companyName !== undefined && { companyName: dto.companyName }),
        ...(dto.plan !== undefined && { plan: dto.plan }),
        ...(dto.status !== undefined && { status: dto.status }),
        ...(dto.githubOrg !== undefined && { githubOrg: dto.githubOrg }),
        ...(dto.githubRepos !== undefined && { githubRepos: dto.githubRepos }),
        ...(dto.trialEndsAt !== undefined && { trialEndsAt: new Date(dto.trialEndsAt) }),
      },
      select: TENANT_SELECT,
    });
  }
}
