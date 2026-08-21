import { Body, Controller, Get, Post, Query } from '@nestjs/common';
import { CatalogService } from './catalog.service';
import {
  CreateBrandDto,
  CreateDivisionDto,
  CreateGroupDto,
  CreateSectionDto,
  CreateSubgroupDto,
  CreateUnitDto,
} from './dto/catalog.dto';

@Controller('catalog')
export class CatalogController {
  constructor(private readonly catalog: CatalogService) {}

  @Post('brands')
  createBrand(@Body() dto: CreateBrandDto) {
    return this.catalog.createBrand(dto);
  }
  @Get('brands')
  listBrands() {
    return this.catalog.listBrands();
  }

  @Post('units')
  createUnit(@Body() dto: CreateUnitDto) {
    return this.catalog.createUnit(dto);
  }
  @Get('units')
  listUnits() {
    return this.catalog.listUnits();
  }

  @Post('divisions')
  createDivision(@Body() dto: CreateDivisionDto) {
    return this.catalog.createDivision(dto);
  }
  @Get('divisions')
  listDivisions() {
    return this.catalog.listDivisions();
  }

  @Post('sections')
  createSection(@Body() dto: CreateSectionDto) {
    return this.catalog.createSection(dto);
  }
  @Get('sections')
  listSections(@Query('divisionId') divisionId?: string) {
    return this.catalog.listSections(divisionId);
  }

  @Post('groups')
  createGroup(@Body() dto: CreateGroupDto) {
    return this.catalog.createGroup(dto);
  }
  @Get('groups')
  listGroups(@Query('sectionId') sectionId?: string) {
    return this.catalog.listGroups(sectionId);
  }

  @Post('subgroups')
  createSubgroup(@Body() dto: CreateSubgroupDto) {
    return this.catalog.createSubgroup(dto);
  }
  @Get('subgroups')
  listSubgroups(@Query('groupId') groupId?: string) {
    return this.catalog.listSubgroups(groupId);
  }
}
