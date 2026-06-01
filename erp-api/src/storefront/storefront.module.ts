import { Module } from '@nestjs/common';
import {
  LandingController,
  PublicSiteController,
} from './storefront.controller';
import { StorefrontService } from './storefront.service';

@Module({
  controllers: [PublicSiteController, LandingController],
  providers: [StorefrontService],
})
export class StorefrontModule {}
