import {
  Controller,
  Get,
  Post,
  Body,
  Param,
  Query,
  Headers,
  HttpException,
  HttpStatus,
} from '@nestjs/common';
import { PrismaService } from './prisma.service';
import { EmailService } from './email.service';

// Outbound: cold outreach pra prospects Enterprise.
// Workflow:
//   1. POST /draft  → insere status='draft'
//   2. GET  /       → lista todos (filtros opcionais)
//   3. POST /:id/send → envia via SMTP, marca como 'sent'
//
// Auth: X-Admin-Token (mesmo NCO_ADMIN_TOKEN).

@Controller('api/outbound')
export class OutboundController {
  constructor(
    private readonly prisma: PrismaService,
    private readonly email: EmailService,
  ) {}

  private requireAdmin(headers: Record<string, string>) {
    const adminToken = process.env.NCO_ADMIN_TOKEN || '';
    if (!adminToken) {
      throw new HttpException('Admin não configurado', HttpStatus.SERVICE_UNAVAILABLE);
    }
    const sent = (headers['x-admin-token'] as string) || '';
    if (sent !== adminToken) {
      throw new HttpException('Token admin inválido', HttpStatus.UNAUTHORIZED);
    }
  }

  // POST /api/outbound/draft
  @Post('draft')
  async createDraft(
    @Headers() headers: Record<string, string>,
    @Body()
    body: {
      to_email: string;
      to_name?: string;
      to_company?: string;
      subject: string;
      body: string;
      campaign_tag?: string;
      metadata?: any;
    },
  ) {
    this.requireAdmin(headers);
    if (!body?.to_email || !body?.subject || !body?.body) {
      throw new HttpException(
        'to_email, subject e body são obrigatórios',
        HttpStatus.BAD_REQUEST,
      );
    }
    const created = await this.prisma.outboundEmail.create({
      data: {
        toEmail: body.to_email,
        toName: body.to_name ?? null,
        toCompany: body.to_company ?? null,
        subject: body.subject,
        body: body.body,
        campaignTag: body.campaign_tag ?? null,
        metadata: body.metadata ?? null,
      },
    });
    return { ok: true, id: created.id, status: created.status };
  }

  // GET /api/outbound?status=draft&campaign=xyz
  @Get()
  async list(
    @Headers() headers: Record<string, string>,
    @Query('status') status?: string,
    @Query('campaign') campaign?: string,
    @Query('limit') limitRaw?: string,
  ) {
    this.requireAdmin(headers);
    const limit = Math.min(Math.max(parseInt(limitRaw || '50', 10), 1), 200);
    const where: any = {};
    if (status) where.status = status;
    if (campaign) where.campaignTag = campaign;
    const items = await this.prisma.outboundEmail.findMany({
      where,
      orderBy: { createdAt: 'desc' },
      take: limit,
    });
    return { items, count: items.length };
  }

  // POST /api/outbound/:id/send
  @Post(':id/send')
  async send(
    @Headers() headers: Record<string, string>,
    @Param('id') idRaw: string,
  ) {
    this.requireAdmin(headers);
    const id = parseInt(idRaw, 10);
    if (!Number.isInteger(id)) throw new HttpException('id inválido', HttpStatus.BAD_REQUEST);
    const e = await this.prisma.outboundEmail.findUnique({ where: { id } });
    if (!e) throw new HttpException('email não encontrado', HttpStatus.NOT_FOUND);
    if (e.status === 'sent') {
      return { ok: false, message: `já enviado em ${e.sentAt?.toISOString()}` };
    }

    try {
      await this.email.send(
        e.toEmail,
        e.subject,
        e.body.replace(/\n/g, '<br>'),
        e.body,
      );
      const updated = await this.prisma.outboundEmail.update({
        where: { id },
        data: { status: 'sent', sentAt: new Date(), errorMsg: null },
      });
      return { ok: true, id: updated.id, status: updated.status, sent_at: updated.sentAt };
    } catch (err: any) {
      await this.prisma.outboundEmail.update({
        where: { id },
        data: { status: 'failed', errorMsg: String(err?.message || err) },
      });
      throw new HttpException(
        `Falha no envio: ${err?.message || err}`,
        HttpStatus.INTERNAL_SERVER_ERROR,
      );
    }
  }

  // POST /api/outbound/:id/cancel
  @Post(':id/cancel')
  async cancel(
    @Headers() headers: Record<string, string>,
    @Param('id') idRaw: string,
  ) {
    this.requireAdmin(headers);
    const id = parseInt(idRaw, 10);
    const e = await this.prisma.outboundEmail.findUnique({ where: { id } });
    if (!e) throw new HttpException('não encontrado', HttpStatus.NOT_FOUND);
    if (e.status === 'sent') throw new HttpException('já enviado, não dá pra cancelar', HttpStatus.CONFLICT);
    const updated = await this.prisma.outboundEmail.update({
      where: { id },
      data: { status: 'cancelled' },
    });
    return { ok: true, status: updated.status };
  }
}
