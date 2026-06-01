import {
  BadRequestException,
  Injectable,
  UnauthorizedException,
} from '@nestjs/common';
import { JwtService } from '@nestjs/jwt';
import * as bcrypt from 'bcrypt';
import * as crypto from 'crypto';
import { PrismaService } from '../prisma/prisma.service';
import { LoginDto } from './dto/login.dto';
import { MailService } from './mail.service';

@Injectable()
export class AuthService {
  constructor(
    private readonly prisma: PrismaService,
    private readonly jwt: JwtService,
    private readonly mail: MailService,
  ) {}

  async login(dto: LoginDto) {
    const users = await this.prisma.tenantUser.findMany({
      where: {
        email: dto.email,
        active: true,
        ...(dto.tenantAlias
          ? { tenant: { alias: dto.tenantAlias } }
          : {}),
      },
      include: { tenant: true },
    });

    if (users.length === 0) {
      throw new UnauthorizedException('Credenciais inválidas');
    }
    if (users.length > 1) {
      throw new UnauthorizedException(
        'Email pertence a múltiplos tenants — informe tenantAlias',
      );
    }

    const user = users[0];
    const ok = await bcrypt.compare(dto.password, user.passwordHash);
    if (!ok) {
      throw new UnauthorizedException('Credenciais inválidas');
    }

    await this.prisma.tenantUser.update({
      where: { id: user.id },
      data: { lastLoginAt: new Date() },
    });

    const payload = {
      sub: user.id,
      email: user.email,
      tenantId: user.tenantId,
      role: user.role,
    };

    return {
      accessToken: this.jwt.sign(payload),
      user: {
        id: user.id,
        email: user.email,
        name: user.name,
        role: user.role,
        tenantId: user.tenantId,
        tenantAlias: user.tenant.alias,
        tenantName: user.tenant.name,
      },
    };
  }

  async hashPassword(plain: string): Promise<string> {
    return bcrypt.hash(plain, 10);
  }

  private hashToken(raw: string): string {
    return crypto.createHash('sha256').update(raw).digest('hex');
  }

  /**
   * Inicia a recuperação de senha. Resolve o tenant pelo alias (header X-Tenant
   * / subdomínio) e envia um email com link. Resposta é sempre genérica para não
   * revelar se o email existe.
   */
  async forgotPassword(email: string, alias: string | null, baseUrl: string) {
    const user = await this.prisma.tenantUser.findFirst({
      where: {
        email,
        active: true,
        ...(alias ? { tenant: { alias } } : {}),
      },
      include: { tenant: true },
    });

    if (user) {
      // invalida pedidos anteriores ainda válidos
      await this.prisma.passwordReset.updateMany({
        where: { userId: user.id, usedAt: null },
        data: { usedAt: new Date() },
      });

      const raw = crypto.randomBytes(32).toString('hex');
      const expiresAt = new Date(Date.now() + 60 * 60 * 1000); // 1h
      await this.prisma.passwordReset.create({
        data: { userId: user.id, tokenHash: this.hashToken(raw), expiresAt },
      });

      const link = `${baseUrl.replace(/\/$/, '')}/reset-password?token=${raw}`;
      await this.mail.sendPasswordReset(user.email, user.name, link);
    }

    return { ok: true };
  }

  async resetPassword(token: string, newPassword: string) {
    if (!newPassword || newPassword.length < 8) {
      throw new BadRequestException('A senha deve ter ao menos 8 caracteres');
    }

    const reset = await this.prisma.passwordReset.findUnique({
      where: { tokenHash: this.hashToken(token) },
    });

    if (!reset || reset.usedAt || reset.expiresAt < new Date()) {
      throw new BadRequestException('Link inválido ou expirado');
    }

    const passwordHash = await this.hashPassword(newPassword);
    await this.prisma.$transaction([
      this.prisma.tenantUser.update({
        where: { id: reset.userId },
        data: { passwordHash },
      }),
      this.prisma.passwordReset.update({
        where: { id: reset.id },
        data: { usedAt: new Date() },
      }),
      // invalida quaisquer outros tokens pendentes do usuário
      this.prisma.passwordReset.updateMany({
        where: { userId: reset.userId, usedAt: null },
        data: { usedAt: new Date() },
      }),
    ]);

    return { ok: true };
  }
}
