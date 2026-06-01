import { Module } from '@nestjs/common';
import { AccessModule } from '../access/access.module';
import { AuthModule } from '../auth/auth.module';
import {
  LandingController,
  PublicSiteController,
} from './storefront.controller';
import { SignupService } from './signup.service';
import { StorefrontService } from './storefront.service';

@Module({
  imports: [AuthModule, AccessModule],
  controllers: [PublicSiteController, LandingController],
  providers: [StorefrontService, SignupService],
})
export class StorefrontModule {}
