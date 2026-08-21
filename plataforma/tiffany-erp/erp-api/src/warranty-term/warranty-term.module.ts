import {
  Body,
  Controller,
  Get,
  Module,
  Param,
  ParseUUIDPipe,
  Patch,
  Post,
} from '@nestjs/common';
import { PartialType } from '@nestjs/mapped-types';
import { Type } from 'class-transformer';
import {
  IsBoolean,
  IsInt,
  IsNotEmpty,
  IsOptional,
  IsString,
  Min,
} from 'class-validator';
import { Injectable, NotFoundException } from '@nestjs/common';
import { requireTenantId } from '../common/tenant-context/tenant-context';
import { PrismaService } from '../prisma/prisma.service';

class CreateWarrantyTermDto {
  @IsString()
  @IsNotEmpty()
  name!: string;

  @IsOptional()
  @Type(() => Number)
  @IsInt()
  @Min(1)
  days?: number;

  @IsString()
  @IsNotEmpty()
  textBody!: string;

  @IsOptional()
  @IsBoolean()
  active?: boolean;
}

class UpdateWarrantyTermDto extends PartialType(CreateWarrantyTermDto) {}

@Injectable()
class WarrantyTermService {
  constructor(private readonly prisma: PrismaService) {}

  create(dto: CreateWarrantyTermDto) {
    return this.prisma.warrantyTerm.create({
      data: {
        tenantId: requireTenantId(),
        name: dto.name,
        days: dto.days ?? 30,
        textBody: dto.textBody,
        active: dto.active ?? true,
      },
    });
  }

  list() {
    return this.prisma.warrantyTerm.findMany({
      where: { tenantId: requireTenantId() },
      orderBy: { name: 'asc' },
    });
  }

  async findOne(id: string) {
    const w = await this.prisma.warrantyTerm.findFirst({
      where: { id, tenantId: requireTenantId() },
    });
    if (!w) throw new NotFoundException('Termo não encontrado');
    return w;
  }

  async update(id: string, dto: UpdateWarrantyTermDto) {
    await this.findOne(id);
    return this.prisma.warrantyTerm.update({ where: { id }, data: dto });
  }
}

@Controller('warranty-terms')
class WarrantyTermController {
  constructor(private readonly service: WarrantyTermService) {}

  @Post() create(@Body() dto: CreateWarrantyTermDto) { return this.service.create(dto); }
  @Get() list() { return this.service.list(); }
  @Get(':id') findOne(@Param('id', ParseUUIDPipe) id: string) { return this.service.findOne(id); }
  @Patch(':id') update(@Param('id', ParseUUIDPipe) id: string, @Body() dto: UpdateWarrantyTermDto) { return this.service.update(id, dto); }
}

@Module({
  controllers: [WarrantyTermController],
  providers: [WarrantyTermService],
  exports: [WarrantyTermService],
})
export class WarrantyTermModule {}
