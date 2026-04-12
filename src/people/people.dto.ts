import { IsString, IsNotEmpty, MinLength } from 'class-validator';
import { ApiProperty, ApiPropertyOptional } from '@nestjs/swagger';

export class SearchPeopleQueryDto {
  @ApiProperty({ description: 'Termo de busca (nome, email ou telefone)', example: 'João' })
  @IsString()
  @IsNotEmpty()
  @MinLength(1)
  q: string;
}

export class ContactResponseDto {
  @ApiProperty({ example: 'whatsapp' })
  channelType: string;

  @ApiProperty({ example: '5511999999999@s.whatsapp.net' })
  remoteId: string;

  @ApiPropertyOptional({ example: 'João Silva' })
  displayName: string | null;

  @ApiPropertyOptional({ example: '+5511999999999' })
  phone: string | null;
}

export class PersonResponseDto {
  @ApiProperty({ example: 'uuid' })
  id: string;

  @ApiProperty({ example: 'João Silva' })
  name: string;

  @ApiProperty({ example: 'member' })
  role: string;

  @ApiPropertyOptional({ example: 'joao@email.com' })
  email: string | null;

  @ApiPropertyOptional({ example: '+5511999999999' })
  phone: string | null;

  @ApiPropertyOptional({ example: 'uuid' })
  profileId: string | null;

  @ApiProperty({ example: 'patria' })
  tenantId: string;

  @ApiPropertyOptional()
  metadata: any;

  @ApiProperty()
  createdAt: Date;

  @ApiProperty()
  updatedAt: Date;

  @ApiProperty({ type: [ContactResponseDto] })
  contacts: ContactResponseDto[];
}
