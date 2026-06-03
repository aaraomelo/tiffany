import {
  BadRequestException,
  ConflictException,
  Injectable,
} from '@nestjs/common';
import { UserRole } from '@prisma/client';
import { AuthService } from '../auth/auth.service';
import { defaultUnitRows } from '../catalog/default-units';
import { PrismaService } from '../prisma/prisma.service';
import { BootstrapTenantDto } from './dto/bootstrap-tenant.dto';

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
}
