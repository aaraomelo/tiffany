import { Injectable, NotFoundException } from '@nestjs/common';
import { Prisma } from '@prisma/client';
import { requireTenantId } from '../common/tenant-context/tenant-context';
import { PrismaService } from '../prisma/prisma.service';
import { UpdateLandingDto } from './dto/landing.dto';

@Injectable()
export class StorefrontService {
  constructor(private readonly prisma: PrismaService) {}

  /**
   * Dados públicos para a landing page do cliente, resolvidos pelo ALIAS
   * (vindo do subdomínio via header X-Tenant). Sem autenticação.
   */
  async publicSite(alias: string) {
    const tenant = await this.prisma.tenant.findUnique({
      where: { alias },
      select: {
        alias: true,
        name: true,
        companyName: true,
        status: true,
        themeConfig: true,
        landingConfig: true,
      },
    });
    if (!tenant) throw new NotFoundException('site não encontrado');
    return {
      alias: tenant.alias,
      name: tenant.name,
      companyName: tenant.companyName,
      theme: tenant.themeConfig ?? null,
      landing: tenant.landingConfig ?? null,
    };
  }

  /** Conteúdo da landing do tenant logado (para o editor). */
  async getLanding() {
    const tenantId = requireTenantId();
    const tenant = await this.prisma.tenant.findUnique({
      where: { id: tenantId },
      select: { name: true, companyName: true, alias: true, landingConfig: true },
    });
    if (!tenant) throw new NotFoundException('tenant não encontrado');
    return {
      alias: tenant.alias,
      name: tenant.name,
      companyName: tenant.companyName,
      landing: tenant.landingConfig ?? null,
    };
  }

  async updateLanding(dto: UpdateLandingDto) {
    const tenantId = requireTenantId();
    await this.prisma.tenant.update({
      where: { id: tenantId },
      data: { landingConfig: dto as unknown as Prisma.InputJsonValue },
    });
    return this.getLanding();
  }
}
