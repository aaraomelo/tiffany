import { ConflictException, Injectable, UnauthorizedException } from '@nestjs/common';
import { JwtService } from '@nestjs/jwt';
import * as bcrypt from 'bcrypt';
import { PrismaService } from '../prisma.service';
import { LoginDto, LoginResponseDto, RegisterDto, TenantLoginDto, TenantLoginResponseDto } from './auth.dto';

@Injectable()
export class AuthService {
  constructor(
    private readonly prisma: PrismaService,
    private readonly jwtService: JwtService,
  ) {}

  async login(dto: LoginDto): Promise<LoginResponseDto> {
    const user = await this.prisma.user.findUnique({ where: { email: dto.email } });
    if (!user) {
      throw new UnauthorizedException('Credenciais inválidas');
    }

    const valid = await bcrypt.compare(dto.password, user.passwordHash);
    if (!valid) {
      throw new UnauthorizedException('Credenciais inválidas');
    }

    const payload = { sub: user.id, email: user.email, name: user.name };
    const token = this.jwtService.sign(payload);

    return {
      token,
      user: { id: user.id, email: user.email, name: user.name },
    };
  }

  async register(dto: RegisterDto): Promise<LoginResponseDto> {
    const existing = await this.prisma.user.findUnique({ where: { email: dto.email } });
    if (existing) {
      throw new ConflictException('Email já está em uso');
    }

    const passwordHash = await bcrypt.hash(dto.password, 10);
    const user = await this.prisma.user.create({
      data: { email: dto.email, passwordHash, name: dto.name ?? null, lastName: dto.lastName ?? null },
    });

    const payload = { sub: user.id, email: user.email, name: user.name };
    const token = this.jwtService.sign(payload);

    return {
      token,
      user: { id: user.id, email: user.email, name: user.name },
    };
  }

  async logout(token: string): Promise<void> {
    // Idempotent: ignore if already revoked
    await this.prisma.revokedToken.upsert({
      where: { token },
      update: {},
      create: { token },
    });
  }

  async isTokenRevoked(token: string): Promise<boolean> {
    const revoked = await this.prisma.revokedToken.findUnique({ where: { token } });
    return !!revoked;
  }

  async tenantLogin(tenantAlias: string, dto: TenantLoginDto): Promise<TenantLoginResponseDto> {
    const tenant = await this.prisma.tenant.findUnique({ where: { alias: tenantAlias } });
    if (!tenant) {
      throw new UnauthorizedException('Credenciais inválidas');
    }

    const tenantUser = await this.prisma.tenantUser.findFirst({
      where: { tenantId: tenant.id, email: dto.email },
    });
    if (!tenantUser) {
      throw new UnauthorizedException('Credenciais inválidas');
    }

    const valid = await bcrypt.compare(dto.password, tenantUser.passwordHash);
    if (!valid) {
      throw new UnauthorizedException('Credenciais inválidas');
    }

    const payload = {
      sub: tenantUser.id,
      email: tenantUser.email,
      name: tenantUser.name,
      tenantId: tenantUser.tenantId,
      role: tenantUser.role,
      type: 'tenant',
    };
    const token = this.jwtService.sign(payload);

    return {
      token,
      user: {
        id: tenantUser.id,
        email: tenantUser.email,
        name: tenantUser.name,
        tenantId: tenantUser.tenantId,
        role: tenantUser.role,
      },
    };
  }
}
