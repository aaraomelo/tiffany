import {
  BadRequestException,
  ConflictException,
  Injectable,
} from '@nestjs/common';
import { UserRole } from '@prisma/client';
import { AuthService } from '../auth/auth.service';
import { defaultUnitRows } from '../catalog/default-units';
import { PrismaService } from '../prisma/prisma.service';
import { requireTenantId } from '../common/tenant-context/tenant-context';
import { BootstrapTenantDto } from './dto/bootstrap-tenant.dto';
import { UpdateCompanyDto } from './dto/update-company.dto';

@Injectable()
export class TenantService {
  constructor(
    private readonly prisma: PrismaService,
    private readonly auth: AuthService,
  ) {}

  /// Cria tenant + primeiro usuário OWNER em uma transação.
  /// Endpoint público (idempotente em prod via guard adicional).
  async bootstrap(dto: BootstrapTenantDto) {
    const existing = await this.prisma.tenant.findUnique({
      where: { alias: dto.alias },
    });
    if (existing) {
      throw new ConflictException(`Tenant '${dto.alias}' já existe`);
    }

    const passwordHash = await this.auth.hashPassword(dto.adminPassword);

    return this.prisma.$transaction(async (tx) => {
      const tenant = await tx.tenant.create({
        data: {
          alias: dto.alias,
          name: dto.name,
          companyName: dto.companyName,
          document: dto.document,
          status: 'ACTIVE',
          plan: 'FREE',
        },
      });

      const user = await tx.tenantUser.create({
        data: {
          tenantId: tenant.id,
          email: dto.adminEmail,
          passwordHash,
          name: dto.adminName,
          role: UserRole.OWNER,
        },
      });

      const warehouse = await tx.warehouse.create({
        data: {
          tenantId: tenant.id,
          code: 'MAIN',
          name: 'Depósito principal',
          isDefault: true,
        },
      });

      // Unidades de medida padrão (UN, PC, KG, L, H, ...) — sem elas o
      // cadastro de produto fica travado (Product.unitId é obrigatório).
      await tx.unit.createMany({
        data: defaultUnitRows(tenant.id),
        skipDuplicates: true,
      });

      return {
        tenant: {
          id: tenant.id,
          alias: tenant.alias,
          name: tenant.name,
        },
        owner: {
          id: user.id,
          email: user.email,
          name: user.name,
          role: user.role,
        },
        warehouse: {
          id: warehouse.id,
          code: warehouse.code,
        },
      };
    });
  }

  async findByAlias(alias: string) {
    const tenant = await this.prisma.tenant.findUnique({ where: { alias } });
    if (!tenant) {
      throw new BadRequestException(`Tenant '${alias}' não encontrado`);
    }
    return tenant;
  }

  /// Dados de cabeçalho da empresa (cabeçalho de Ordem de Serviço etc.).
  /// Retorna campos crus (pro formulário) + shape `CompanyInfo` (pro PDF).
  async getCompany() {
    const tenantId = requireTenantId();
    const t = await this.prisma.tenant.findUnique({
      where: { id: tenantId },
      include: { address: { include: { city: true } } },
    });
    if (!t) throw new BadRequestException('Tenant não encontrado');

    const addr = t.address;
    const city = addr?.city ?? null;
    const phones = [t.phone, t.phone2].filter((p): p is string => !!p);
    const addressLine = addr
      ? [addr.street, addr.number, addr.complement].filter(Boolean).join(', ')
      : undefined;
    const cityLine = addr
      ? [addr.neighborhood, city ? `${city.name}-${city.state}` : null]
          .filter(Boolean)
          .join(', ')
      : undefined;

    return {
      // crus (formulário)
      companyName: t.companyName ?? t.name,
      document: t.document ?? undefined,
      responsible: t.responsible ?? undefined,
      email: t.email ?? undefined,
      phone: t.phone ?? undefined,
      phone2: t.phone2 ?? undefined,
      instagram: t.instagram ?? undefined,
      logoUrl: t.logoUrl ?? undefined,
      paymentMethods: t.paymentMethods ?? undefined,
      paymentTerms: t.paymentTerms ?? undefined,
      street: addr?.street ?? undefined,
      number: addr?.number ?? undefined,
      complement: addr?.complement ?? undefined,
      neighborhood: addr?.neighborhood ?? undefined,
      zipCode: addr?.zipCode ?? undefined,
      cityName: city?.name ?? undefined,
      state: city?.state ?? undefined,
      // shape CompanyInfo (PDF)
      name: t.companyName ?? t.name,
      cnpj: t.document ?? undefined,
      address: addressLine,
      cityLine,
      cep: addr?.zipCode ?? undefined,
      city: city?.name ?? undefined,
      logo: t.logoUrl ?? undefined,
      phones,
    };
  }

  async updateCompany(dto: UpdateCompanyDto) {
    const tenantId = requireTenantId();
    const t = await this.prisma.tenant.findUnique({ where: { id: tenantId } });
    if (!t) throw new BadRequestException('Tenant não encontrado');

    const addressFields = [
      dto.street,
      dto.number,
      dto.complement,
      dto.neighborhood,
      dto.zipCode,
      dto.cityName,
      dto.state,
    ];
    const hasAddress = addressFields.some((v) => v != null);

    await this.prisma.$transaction(async (tx) => {
      let addressId = t.addressId;

      if (hasAddress) {
        let cityId: string | null = null;
        if (dto.cityName && dto.state) {
          const existing = await tx.city.findFirst({
            where: { name: dto.cityName, state: dto.state },
          });
          const city =
            existing ??
            (await tx.city.create({
              data: { name: dto.cityName, state: dto.state },
            }));
          cityId = city.id;
        }
        const addrData = {
          type: 'COMMERCIAL' as const,
          street: dto.street ?? '',
          number: dto.number ?? null,
          complement: dto.complement ?? null,
          neighborhood: dto.neighborhood ?? null,
          zipCode: dto.zipCode ?? null,
          cityId,
        };
        if (addressId) {
          await tx.address.update({ where: { id: addressId }, data: addrData });
        } else {
          const created = await tx.address.create({ data: addrData });
          addressId = created.id;
        }
      }

      await tx.tenant.update({
        where: { id: tenantId },
        data: {
          companyName: dto.companyName ?? undefined,
          document: dto.document ?? undefined,
          responsible: dto.responsible ?? undefined,
          email: dto.email ?? undefined,
          phone: dto.phone ?? undefined,
          phone2: dto.phone2 ?? undefined,
          instagram: dto.instagram ?? undefined,
          logoUrl: dto.logoUrl ?? undefined,
          paymentMethods: dto.paymentMethods ?? undefined,
          paymentTerms: dto.paymentTerms ?? undefined,
          addressId: addressId ?? undefined,
        },
      });
    });

    return this.getCompany();
  }
}
