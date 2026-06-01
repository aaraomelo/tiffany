import {
  BadRequestException,
  Body,
  Controller,
  Get,
  Headers,
  Put,
  Query,
} from '@nestjs/common';
import { Public } from '../common/decorators/public.decorator';
import { UpdateLandingDto } from './dto/landing.dto';
import { StorefrontService } from './storefront.service';

// Landing page pública do cliente — sem autenticação. O tenant é resolvido
// pelo alias do subdomínio (header X-Tenant injetado pelo nginx) ou, em dev,
// por ?tenant=alias.
@Controller('public')
export class PublicSiteController {
  constructor(private readonly service: StorefrontService) {}

  @Public()
  @Get('site')
  site(
    @Headers('x-tenant') headerAlias?: string,
    @Query('tenant') queryAlias?: string,
  ) {
    const alias = (headerAlias || queryAlias || '').trim().toLowerCase();
    if (!alias) throw new BadRequestException('tenant não informado');
    return this.service.publicSite(alias);
  }
}

// Editor da landing — autenticado (tenant logado).
@Controller('tenant/landing')
export class LandingController {
  constructor(private readonly service: StorefrontService) {}

  @Get()
  get() {
    return this.service.getLanding();
  }

  @Put()
  update(@Body() dto: UpdateLandingDto) {
    return this.service.updateLanding(dto);
  }
}
