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

    const pack = await this.prisma.modulePack.findUnique({ where: { slug: dto.packSlug } });
    if (!pack) throw new BadRequestException('segmento inválido');

    const passwordHash = await this.auth.hashPassword(dto.adminPassword);
    const landing = landingTemplateFor(pack.segment, dto.name);

    // tenant + owner + depósito padrão
    const { tenantId, ownerId } = await this.prisma.$transaction(async (tx) => {
      const tenant = await tx.tenant.create({
        data: {
          alias,
          name: dto.name,
          status: 'ACTIVE',
          plan: 'FREE',
          packSlug: pack.slug,
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

    // aplica o segmento (módulos + TenantPack) e provisiona perfis de acesso
    await this.modules.applyPack(tenantId, pack.slug, 'replace');
    await this.access.provisionDefaults(tenantId, ownerId);

    return { alias, name: dto.name, packSlug: pack.slug };
  }
}
