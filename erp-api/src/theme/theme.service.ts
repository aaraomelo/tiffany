import { ForbiddenException, Injectable } from '@nestjs/common';
import {
  getTenantContext,
  requireTenantId,
} from '../common/tenant-context/tenant-context';
import { PrismaService } from '../prisma/prisma.service';
import { ThemeConfigDto } from './dto/theme.dto';

const PRIVILEGED_ROLES = ['OWNER', 'ADMIN', 'MANAGER'];

@Injectable()
export class ThemeService {
  constructor(private readonly prisma: PrismaService) {}

  async get() {
    const tenantId = requireTenantId();
    const tenant = await this.prisma.tenant.findUnique({
      where: { id: tenantId },
      select: { themeConfig: true },
    });
    return { config: tenant?.themeConfig ?? null };
  }

  async update(dto: ThemeConfigDto) {
    const { role } = getTenantContext();
    if (!role || !PRIVILEGED_ROLES.includes(role)) {
      throw new ForbiddenException(
        'Apenas OWNER/ADMIN/MANAGER podem alterar o tema',
      );
    }
    const tenantId = requireTenantId();
    const tenant = await this.prisma.tenant.update({
      where: { id: tenantId },
      data: { themeConfig: dto as never },
      select: { themeConfig: true },
    });
    return { config: tenant.themeConfig };
  }

  async reset() {
    const { role } = getTenantContext();
    if (!role || !PRIVILEGED_ROLES.includes(role)) {
      throw new ForbiddenException(
        'Apenas OWNER/ADMIN/MANAGER podem alterar o tema',
      );
    }
    const tenantId = requireTenantId();
    const tenant = await this.prisma.tenant.update({
      where: { id: tenantId },
      data: { themeConfig: null as never },
      select: { themeConfig: true },
    });
    return { config: tenant.themeConfig };
  }
}
