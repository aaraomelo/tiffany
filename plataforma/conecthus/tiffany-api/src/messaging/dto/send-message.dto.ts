import { IsString, IsOptional } from 'class-validator';
import { ApiProperty, ApiPropertyOptional } from '@nestjs/swagger';

export class SendMessageDto {
  @ApiProperty({ description: 'Canal de envio', example: 'whatsapp' })
  @IsString()
  channel: string;

  @ApiProperty({ description: 'Destinatário (phone ou group JID)', example: '+5511977808883' })
  @IsString()
  target: string;

  @ApiProperty({ description: 'Mensagem de texto' })
  @IsString()
  message: string;

  @ApiPropertyOptional({ description: 'URL de mídia (imagem, documento)' })
  @IsOptional()
  @IsString()
  mediaUrl?: string;
}
