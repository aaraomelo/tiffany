import {
  BadRequestException,
  ConflictException,
  Injectable,
} from '@nestjs/common';
import { Prisma, UserRole } from '@prisma/client';
import { AccessService } from '../access/access.service';
import { AuthService } from '../auth/auth.service';
import { ModulesService } from '../modules/modules.service';
import { PrismaService } from '../prisma/prisma.service';
import { SignupDto } from './dto/signup.dto';
import { landingTemplateFor } from './landing-templates';

@Injectable()
export class SignupService {
  constructor(
    private readonly prisma: PrismaService,
    private readonly auth: AuthService,
    private readonly modules: ModulesService,
    private readonly access: AccessService,
  ) {}

  /** Segmentos disponíveis para o cadastro (público). */
  async segments() {
    const packs = await this.prisma.modulePack.findMany({
      orderBy: { sortOrder: 'asc' },
      select: { slug: true, name: true, segment: true, description: true, isDefault: true },
    });
    return packs;
  }

  async aliasAvailable(alias: string) {
    const a = alias.trim().toLowerCase();
    const ok = /^[a-z0-9](?:[a-z0-9-]{0,30}[a-z0-9])?$/.test(a);
    if (!ok) return { available: false, reason: 'invalid' as const };
    const taken = await this.prisma.tenant.findUnique({ where: { alias: a }, select: { id: true } });
    return { available: !taken, reason: taken ? ('taken' as const) : null };
  }

  /**
   * Cadastro self-service: cria tenant + OWNER + depósito, aplica o segmento
   * (módulos), provisiona os perfis de acesso e gera uma landing inicial pelo
   * segmento. Fricção zero: o front loga em seguida com as mesmas credenciais.
   */
  async signup(dto: SignupDto) {
    const alias = dto.alias.trim().toLowerCase();

    const existing = await this.prisma.tenant.findUnique({ where: { alias }, select: { id: true } });
    if (existing) throw new ConflictException(`O endereço '${alias}' já está em uso`);

    // valida e ordena os segmentos (1º = principal)
    const slugs = [...new Set(dto.packSlugs)];
    const packs = await this.prisma.modulePack.findMany({ where: { slug: { in: slugs } } });
    if (packs.length !== slugs.length) throw new BadRequestException('segmento inválido');
    const ordered = slugs.map((s) => packs.find((p) => p.slug === s)!);
    const primary = ordered[0];

    const passwordHash = await this.auth.hashPassword(dto.adminPassword);
    const landing = landingTemplateFor(primary.segment, dto.name);

    // tenant + owner + depósito padrão
    const { tenantId, ownerId } = await this.prisma.$transaction(async (tx) => {
      const tenant = await tx.tenant.create({
        data: {
          alias,
          name: dto.name,
          status: 'ACTIVE',
          plan: 'FREE',
          packSlug: primary.slug,
          landingConfig: landing as unknown as Prisma.InputJsonValue,
        },
      });
      const owner = await tx.tenantUser.create({
        data: {
          tenantId: tenant.id,
          email: dto.adminEmail,
          passwordHash,
          name: dto.adminName,
          role: UserRole.OWNER,
        },
      });
      await tx.warehouse.create({
        data: { tenantId: tenant.id, code: 'MAIN', name: 'Depósito principal', isDefault: true },
      });
      return { tenantId: tenant.id, ownerId: owner.id };
    });

    // 1º segmento define a base (replace); os demais são somados (merge)
    await this.modules.applyPack(tenantId, primary.slug, 'replace');
    for (const p of ordered.slice(1)) {
      await this.modules.applyPack(tenantId, p.slug, 'merge');
    }
    await this.access.provisionDefaults(tenantId, ownerId);

    return { alias, name: dto.name, packSlugs: ordered.map((p) => p.slug) };
  }
}
