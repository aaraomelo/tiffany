import {
  Injectable,
  UnauthorizedException,
} from '@nestjs/common';
import { JwtService } from '@nestjs/jwt';
import * as bcrypt from 'bcrypt';
import { PrismaService } from '../prisma/prisma.service';
import { LoginDto } from './dto/login.dto';

@Injectable()
export class AuthService {
  constructor(
    private readonly prisma: PrismaService,
    private readonly jwt: JwtService,
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
}
