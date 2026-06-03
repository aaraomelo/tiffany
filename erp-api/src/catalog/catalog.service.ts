import { Injectable } from '@nestjs/common';
import { requireTenantId } from '../common/tenant-context/tenant-context';
import { PrismaService } from '../prisma/prisma.service';
import { defaultUnitRows } from './default-units';
import {
  CreateBrandDto,
  CreateDivisionDto,
  CreateGroupDto,
  CreateSectionDto,
  CreateSubgroupDto,
  CreateUnitDto,
} from './dto/catalog.dto';

@Injectable()
export class CatalogService {
  constructor(private readonly prisma: PrismaService) {}

  // ----- Brand -----
  createBrand(dto: CreateBrandDto) {
    return this.prisma.brand.create({
      data: { tenantId: requireTenantId(), name: dto.name },
    });
  }
  listBrands() {
    return this.prisma.brand.findMany({
      where: { tenantId: requireTenantId() },
      orderBy: { name: 'asc' },
    });
  }

  // ----- Unit -----
  createUnit(dto: CreateUnitDto) {
    return this.prisma.unit.create({
      data: {
        tenantId: requireTenantId(),
        code: dto.code,
        description: dto.description,
        fractional: dto.fractional ?? false,
        conversionTo: dto.conversionTo,
      },
    });
  }
  async listUnits() {
    const tenantId = requireTenantId();
    const units = await this.prisma.unit.findMany({
      where: { tenantId },
      orderBy: { code: 'asc' },
    });
    if (units.length > 0) return units;

    // Tenant sem nenhuma unidade: semeia o conjunto padrão sob demanda.
    await this.prisma.unit.createMany({
      data: defaultUnitRows(tenantId),
      skipDuplicates: true,
    });
    return this.prisma.unit.findMany({
      where: { tenantId },
      orderBy: { code: 'asc' },
    });
  }

  // ----- Division -----
  createDivision(dto: CreateDivisionDto) {
    return this.prisma.division.create({
      data: { tenantId: requireTenantId(), code: dto.code, name: dto.name },
    });
  }
  listDivisions() {
    return this.prisma.division.findMany({
      where: { tenantId: requireTenantId() },
      orderBy: { code: 'asc' },
    });
  }

  // ----- Section -----
  createSection(dto: CreateSectionDto) {
    return this.prisma.section.create({
      data: {
        tenantId: requireTenantId(),
        divisionId: dto.divisionId,
        code: dto.code,
        name: dto.name,
      },
    });
  }
  listSections(divisionId?: string) {
    return this.prisma.section.findMany({
      where: {
        tenantId: requireTenantId(),
        ...(divisionId ? { divisionId } : {}),
      },
      orderBy: { code: 'asc' },
    });
  }

  // ----- Group -----
  createGroup(dto: CreateGroupDto) {
    return this.prisma.group.create({
      data: {
        tenantId: requireTenantId(),
        sectionId: dto.sectionId,
        code: dto.code,
        name: dto.name,
      },
    });
  }
  listGroups(sectionId?: string) {
    return this.prisma.group.findMany({
      where: {
        tenantId: requireTenantId(),
        ...(sectionId ? { sectionId } : {}),
      },
      orderBy: { code: 'asc' },
    });
  }

  // ----- Subgroup -----
  createSubgroup(dto: CreateSubgroupDto) {
    return this.prisma.subgroup.create({
      data: {
        tenantId: requireTenantId(),
        groupId: dto.groupId,
        code: dto.code,
        name: dto.name,
      },
    });
  }
  listSubgroups(groupId?: string) {
    return this.prisma.subgroup.findMany({
      where: {
        tenantId: requireTenantId(),
        ...(groupId ? { groupId } : {}),
      },
      orderBy: { code: 'asc' },
    });
  }
}
