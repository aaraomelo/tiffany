import { Controller, Post, Body } from '@nestjs/common';
import { ApiTags, ApiOperation, ApiSecurity } from '@nestjs/swagger';
import { MessagingService } from './messaging.service';
import { MemoryService } from './memory.service';
import { SendMessageDto } from './dto/send-message.dto';

@ApiTags('Messaging')
@ApiSecurity('api-key')
@Controller('api/messaging')
export class MessagingSenderController {
  constructor(
    private messaging: MessagingService,
    private memory: MemoryService,
  ) {}

  @Post('send')
  @ApiOperation({ summary: 'Enviar mensagem via canal (WhatsApp/Telegram)' })
  async send(@Body() dto: SendMessageDto) {
    const result = await this.messaging.send(dto.channel, dto.target, dto.message);
    return { ok: true, ...result };
  }

  @Post('memories/backfill')
  @ApiOperation({ summary: 'Gerar embeddings para memórias sem embedding' })
  async backfillMemories() {
    const count = await this.memory.backfillEmbeddings();
    return { ok: true, updated: count };
  }

  @Post('memories/stats')
  @ApiOperation({ summary: 'Estatísticas da memória (acessos, estados, promoções)' })
  async memoryStats() {
    const stats = await this.memory.getStats();
    return { ok: true, stats };
  }
}
