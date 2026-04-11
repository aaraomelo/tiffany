import { Controller, Post, Body } from '@nestjs/common';
import { ApiTags, ApiOperation, ApiSecurity } from '@nestjs/swagger';
import { MessagingService } from './messaging.service';
import { SendMessageDto } from './dto/send-message.dto';

@ApiTags('Messaging')
@ApiSecurity('api-key')
@Controller('api/messaging')
export class MessagingSenderController {
  constructor(private messaging: MessagingService) {}

  @Post('send')
  @ApiOperation({ summary: 'Enviar mensagem via canal (WhatsApp/Telegram)' })
  async send(@Body() dto: SendMessageDto) {
    const result = await this.messaging.send(dto.channel, dto.target, dto.message);
    return { ok: true, ...result };
  }
}
