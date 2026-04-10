import { IsString, IsNotEmpty, IsOptional } from 'class-validator';
import { ApiProperty, ApiPropertyOptional } from '@nestjs/swagger';

export class PatriciaActionDto {
  @ApiProperty({ description: 'Action name', example: 'create_task' })
  @IsString()
  @IsNotEmpty()
  action: string;

  @ApiProperty({ description: 'Channel', example: 'whatsapp' })
  @IsString()
  channel: string;

  @ApiProperty({ description: 'Target (phone/group)', example: '+5511977808883' })
  @IsString()
  target: string;

  @ApiPropertyOptional({ description: 'Action parameters' })
  @IsOptional()
  params?: Record<string, any>;
}
