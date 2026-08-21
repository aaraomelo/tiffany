import { Controller, Get, Query } from '@nestjs/common';
import { CheckPolicies } from '../access/check-policies.decorator';
import { RequiresModule } from '../common/decorators/requires-module.decorator';
import { WalletService } from './wallet.service';

@RequiresModule('wallet')
@Controller('wallet')
export class WalletController {
  constructor(private readonly service: WalletService) {}

  @Get()
  @CheckPolicies({ action: 'read', subject: 'Wallet' })
  async summary() {
    return this.service.getOrCreate();
  }

  @Get('transactions')
  @CheckPolicies({ action: 'read', subject: 'Wallet' })
  transactions(@Query('limit') limit?: string) {
    return this.service.listTransactions(limit ? Number(limit) : 50);
  }
}
