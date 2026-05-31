import {
  BadRequestException,
  ForbiddenException,
  Injectable,
  NotFoundException,
} from '@nestjs/common';
import { PrismaService } from '../prisma/prisma.service';

@Injectable()
export class ModulesService {
  constructor(private readonly prisma: PrismaService) {}

  listModules() {
    return this.prisma.module.findMany({
      orderBy: [{ category: 'asc' }, { sortOrder: 'asc' }],
    });
  }

  listPacks() {
    return this.prisma.modulePack.findMany({
      orderBy: { sortOrder: 'asc' },
      include: {
        items: { select: { moduleSlug: true } },
      },
    });
  }

  async listTenantModules(tenantId: string) {
    const [tenant, modules] = await Promise.all([
      this.prisma.tenant.findUnique({
        where: { id: tenantId },
        select: { packSlug: true },
      }),
      this.prisma.tenantModule.findMany({
        where: { tenantId },
        include: { module: true },
      }),
    ]);

    if (!tenant) throw new NotFoundException('tenant não encontrado');

    return {
      packSlug: tenant.packSlug,
      modules: modules.map((tm) => ({
        slug: tm.moduleSlug,
        enabled: tm.enabled,
        enabledAt: tm.enabledAt,
        disabledAt: tm.disabledAt,
        name: tm.module.name,
        description: tm.module.description,
        category: tm.module.category,
        routePath: tm.module.routePath,
        iconKey: tm.module.iconKey,
        isCore: tm.module.isCore,
        sortOrder: tm.module.sortOrder,
      })),
    };
  }

  /**
   * Aplica um pack: sobrescreve TenantModule do tenant com os módulos do pack.
   * Núcleo permanece sempre ativo (independente do pack escolhido — o seed
   * já inclui CORE em todos os packs, mas garantimos aqui também).
   * Módulos fora do pack são desativados (não deletados, pra preservar histórico).
   */
  async applyPack(
    tenantId: string,
    packSlug: string,
    mode: 'replace' | 'merge' = 'replace',
  ) {
    const pack = await this.prisma.modulePack.findUnique({
      where: { slug: packSlug },
      include: { items: true },
    });
    if (!pack) throw new NotFoundException(`pack '${packSlug}' não encontrado`);

    const allModules = await this.prisma.module.findMany({
      select: { slug: true, isCore: true },
    });
    const packSlugs = new Set(pack.items.map((i) => i.moduleSlug));

    await this.prisma.$transaction(async (tx) => {
      await tx.tenant.update({
        where: { id: tenantId },
        data: { packSlug },
      });

      for (const m of allModules) {
        const inPack = m.isCore || packSlugs.has(m.slug);

        // merge (aditivo): só liga os módulos do segmento; nunca desliga nada.
        // replace: liga os do segmento e desliga os de fora.
        if (mode === 'merge' && !inPack) continue;
        const shouldEnable = mode === 'merge' ? true : inPack;

        await tx.tenantModule.upsert({
          where: {
            tenantId_moduleSlug: { tenantId, moduleSlug: m.slug },
          },
          create: {
            tenantId,
            moduleSlug: m.slug,
            enabled: shouldEnable,
            disabledAt: shouldEnable ? null : new Date(),
          },
          update: {
            enabled: shouldEnable,
            enabledAt: shouldEnable ? new Date() : undefined,
            disabledAt: shouldEnable ? null : new Date(),
          },
        });
      }
    });

    return this.listTenantModules(tenantId);
  }

  async toggleModule(tenantId: string, moduleSlug: string, enabled: boolean) {
    const mod = await this.prisma.module.findUnique({
      where: { slug: moduleSlug },
    });
    if (!mod) throw new NotFoundException(`módulo '${moduleSlug}' não encontrado`);
    if (mod.isCore && !enabled) {
      throw new ForbiddenException('módulo core não pode ser desativado');
    }

    const existing = await this.prisma.tenantModule.findUnique({
      where: { tenantId_moduleSlug: { tenantId, moduleSlug } },
    });
    if (!existing) {
      throw new BadRequestException(
        'módulo não está no catálogo do tenant; aplique um pack antes',
      );
    }

    return this.prisma.tenantModule.update({
      where: { tenantId_moduleSlug: { tenantId, moduleSlug } },
      data: {
        enabled,
        enabledAt: enabled ? new Date() : existing.enabledAt,
        disabledAt: enabled ? null : new Date(),
      },
    });
  }

  /** Usado pelo guard: retorna true se o módulo está ativo pro tenant. */
  async isEnabled(tenantId: string, moduleSlug: string): Promise<boolean> {
    const tm = await this.prisma.tenantModule.findUnique({
      where: { tenantId_moduleSlug: { tenantId, moduleSlug } },
      select: { enabled: true },
    });
    return tm?.enabled ?? false;
  }
}
