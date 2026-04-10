import { Body, Controller, Post } from '@nestjs/common';
import { ApiOperation, ApiProperty, ApiPropertyOptional, ApiResponse, ApiSecurity, ApiTags } from '@nestjs/swagger';
import { IsBoolean, IsEmail, IsNotEmpty, IsOptional, IsString } from 'class-validator';
import { PrismaService } from './prisma.service';

export class CreateContactDto {
  @ApiProperty({ description: 'Nome do contato', example: 'João Silva' })
  @IsString()
  @IsNotEmpty()
  name: string;

  @ApiProperty({ description: 'Email do contato', example: 'joao@email.com' })
  @IsEmail()
  @IsNotEmpty()
  email: string;

  @ApiProperty({ description: 'Mensagem', example: 'Olá, gostaria de mais informações.' })
  @IsString()
  @IsNotEmpty()
  message: string;

  @ApiPropertyOptional({ description: 'Telefone do contato', example: '+5511999999999' })
  @IsOptional()
  @IsString()
  phone?: string;

  @ApiPropertyOptional({ description: 'Consentimento para contato via WhatsApp', example: false, default: false })
  @IsOptional()
  @IsBoolean()
  whatsappConsent?: boolean;
}

@ApiTags('Contacts')
@ApiSecurity('api-key')
@Controller('api/contacts')
export class ContactsController {
  constructor(private readonly prisma: PrismaService) {}

  @Post()
  @ApiOperation({ summary: 'Criar contato' })
  @ApiResponse({ status: 201, description: 'Contato criado com sucesso' })
  @ApiResponse({ status: 400, description: 'Dados inválidos' })
  create(@Body() dto: CreateContactDto) {
    return this.prisma.contact.create({
      data: {
        name: dto.name,
        email: dto.email,
        message: dto.message,
        phone: dto.phone,
        whatsappConsent: dto.whatsappConsent ?? false,
      },
    });
  }
}
