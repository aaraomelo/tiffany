import { Controller, Get, Query } from '@nestjs/common';
import { RequiresModule } from '../common/decorators/requires-module.decorator';
import { WalletService } from './wallet.service';

@RequiresModule('wallet')
@Controller('wallet')
export class WalletController {
  constructor(private readonly service: WalletService) {}

  @Get()
  async summary() {
    return this.service.getOrCreate();
  }

  @Get('transactions')
  transactions(@Query('limit') limit?: string) {
    return this.service.listTransactions(limit ? Number(limit) : 50);
  }
}
