import {
  BadRequestException,
  Injectable,
  NotFoundException,
} from '@nestjs/common';
import {
  CashOperationType,
  CashSessionStatus,
} from '@prisma/client';
import {
  getTenantContext,
  requireTenantId,
} from '../common/tenant-context/tenant-context';
import { PrismaService } from '../prisma/prisma.service';
import {
  CashOperationDto,
  CloseSessionDto,
  CreateCashDto,
  OpenSessionDto,
  OperatorChangeDto,
  PhysicalCloseDto,
} from './dto/cash.dto';

function requireUserId(): string {
  const { userId } = getTenantContext();
  if (!userId) throw new BadRequestException('Usuário não autenticado');
  return userId;
}

@Injectable()
export class CashService {
  constructor(private readonly prisma: PrismaService) {}

  create(dto: CreateCashDto) {
    return this.prisma.cash.create({
      data: { tenantId: requireTenantId(), name: dto.name },
    });
  }

  list() {
    return this.prisma.cash.findMany({
      where: { tenantId: requireTenantId() },
      orderBy: { name: 'asc' },
    });
  }

  async currentSession(cashId: string) {
    const tenantId = requireTenantId();
    return this.prisma.cashSession.findFirst({
      where: {
        tenantId,
        cashId,
        status: { in: [CashSessionStatus.OPEN, CashSessionStatus.PARTIALLY_CLOSED] },
      },
      include: { openedBy: { select: { id: true, name: true, email: true } } },
      orderBy: { openedAt: 'desc' },
    });
  }

  async openSession(cashId: string, dto: OpenSessionDto) {
    const tenantId = requireTenantId();
    const userId = requireUserId();

    const cash = await this.prisma.cash.findFirst({ where: { id: cashId, tenantId } });
    if (!cash) throw new NotFoundException('Caixa não encontrado');

    const existing = await this.currentSession(cashId);
    if (existing) {
      throw new BadRequestException('Já existe sessão aberta neste caixa');
    }

    return this.prisma.cashSession.create({
      data: {
        tenantId,
        cashId,
        openedById: userId,
        openingAmount: dto.openingAmount,
        status: CashSessionStatus.OPEN,
      },
    });
  }

  async closeSession(cashId: string, sessionId: string, dto: CloseSessionDto) {
    const tenantId = requireTenantId();
    const userId = requireUserId();

    const session = await this.prisma.cashSession.findFirst({
      where: { id: sessionId, cashId, tenantId },
    });
    if (!session) throw new NotFoundException('Sessão não encontrada');
    if (session.status === CashSessionStatus.CLOSED) {
      throw new BadRequestException('Sessão já fechada');
    }

    return this.prisma.cashSession.update({
      where: { id: sessionId },
      data: {
        status: CashSessionStatus.CLOSED,
        closingAmount: dto.closingAmount,
        closedAt: new Date(),
        closedById: userId,
      },
    });
  }

  async addOperation(
    cashId: string,
    type: CashOperationType,
    dto: CashOperationDto,
  ) {
    const session = await this.currentSession(cashId);
    if (!session) throw new BadRequestException('Sem sessão aberta');
    const operatorId = requireUserId();

    return this.prisma.cashOperation.create({
      data: {
        sessionId: session.id,
        type,
        amount: dto.amount,
        operatorId,
        notes: dto.notes,
      },
    });
  }

  /// Troca de operador atômica: fecha sessão atual e abre nova com o novo operador.
  async operatorChange(cashId: string, dto: OperatorChangeDto) {
    const tenantId = requireTenantId();
    const closerId = requireUserId();

    const newOperator = await this.prisma.tenantUser.findFirst({
      where: { id: dto.newOperatorId, tenantId, active: true },
    });
    if (!newOperator) throw new NotFoundException('Operador entrante inválido');

    return this.prisma.$transaction(async (tx) => {
      const session = await tx.cashSession.findFirst({
        where: {
          tenantId,
          cashId,
          status: { in: [CashSessionStatus.OPEN, CashSessionStatus.PARTIALLY_CLOSED] },
        },
        orderBy: { openedAt: 'desc' },
      });
      if (!session) throw new BadRequestException('Sem sessão aberta');

      await tx.cashOperation.create({
        data: {
          sessionId: session.id,
          type: CashOperationType.OPERATOR_CHANGE,
          amount: dto.closingAmount,
          operatorId: closerId,
          notes: dto.notes ?? 'troca de operador',
        },
      });

      await tx.cashSession.update({
        where: { id: session.id },
        data: {
          status: CashSessionStatus.CLOSED,
          closingAmount: dto.closingAmount,
          closedAt: new Date(),
          closedById: closerId,
        },
      });

      return tx.cashSession.create({
        data: {
          tenantId,
          cashId,
          openedById: dto.newOperatorId,
          openingAmount: dto.openingAmount,
          status: CashSessionStatus.OPEN,
        },
      });
    });
  }

  async physicalClose(cashId: string, dto: PhysicalCloseDto) {
    const session = await this.currentSession(cashId);
    if (!session) throw new BadRequestException('Sem sessão aberta');

    return this.prisma.cashPhysicalClose.upsert({
      where: { sessionId_method: { sessionId: session.id, method: dto.method } },
      create: {
        sessionId: session.id,
        method: dto.method,
        amount: dto.amount,
      },
      update: { amount: dto.amount },
    });
  }

  async sessionDetail(cashId: string, sessionId: string) {
    const tenantId = requireTenantId();
    const session = await this.prisma.cashSession.findFirst({
      where: { id: sessionId, cashId, tenantId },
      include: {
        operations: { orderBy: { createdAt: 'desc' } },
        physicalCloses: true,
        openedBy: { select: { id: true, name: true } },
        closedBy: { select: { id: true, name: true } },
      },
    });
    if (!session) throw new NotFoundException('Sessão não encontrada');
    return session;
  }
}
