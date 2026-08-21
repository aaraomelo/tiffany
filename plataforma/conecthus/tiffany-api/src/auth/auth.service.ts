import { ConflictException, HttpException, HttpStatus, Injectable, InternalServerErrorException, UnauthorizedException, BadRequestException, Logger } from '@nestjs/common';
import { JwtService } from '@nestjs/jwt';
import * as bcrypt from 'bcrypt';
import { randomBytes } from 'crypto';
import { PrismaService } from '../prisma.service';
import { EmailService } from '../email.service';
import { LoginDto, LoginResponseDto, RegisterDto, SignupDto, TenantLoginDto, TenantLoginResponseDto } from './auth.dto';

const LANDPAGE_URL = process.env.LANDPAGE_URL || 'https://patriatechnology.com';

function aliasFromCompany(name: string): string {
  const base = name
    .toLowerCase()
    .normalize('NFD')
    .replace(/[̀-ͯ]/g, '')
    .replace(/[^a-z0-9]+/g, '-')
    .replace(/^-+|-+$/g, '')
    .slice(0, 32) || 'tenant';
  return base;
}

function generateApiKey(): string {
  return 'pk_' + randomBytes(24).toString('hex');
}

export interface GithubRepo {
  id: number;
  name: string;
  fullName: string;
  private: boolean;
  url: string;
  defaultBranch: string;
}

@Injectable()
export class AuthService {
  private readonly logger = new Logger(AuthService.name);

  constructor(
    private readonly prisma: PrismaService,
    private readonly jwtService: JwtService,
    private readonly emailService: EmailService,
  ) {}

  async login(dto: LoginDto): Promise<LoginResponseDto> {
    const user = await this.prisma.user.findUnique({
      where: { email: dto.email },
      include: { tenant: true },
    });
    if (!user) {
      throw new UnauthorizedException('Credenciais inválidas');
    }

    const valid = await bcrypt.compare(dto.password, user.passwordHash);
    if (!valid) {
      throw new UnauthorizedException('Credenciais inválidas');
    }

    const payload: any = { sub: user.id, email: user.email, name: user.name };
    if (user.tenantId) payload.tenantId = user.tenantId;
    const token = this.jwtService.sign(payload);

    const res: LoginResponseDto = {
      token,
      user: { id: user.id, email: user.email, name: user.name },
    };
    if (user.tenant && user.tenant.apiKey) {
      res.tenant = {
        id: user.tenant.id,
        alias: user.tenant.alias,
        companyName: user.tenant.companyName,
        apiKey: user.tenant.apiKey,
      };
    }
    return res;
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

  async signup(dto: SignupDto): Promise<LoginResponseDto> {
    const existing = await this.prisma.user.findUnique({ where: { email: dto.email } });
    if (existing) {
      throw new ConflictException('Email já está em uso');
    }

    const passwordHash = await bcrypt.hash(dto.password, 10);
    const baseAlias = aliasFromCompany(dto.companyName);

    let alias = baseAlias;
    for (let i = 0; i < 10; i++) {
      const taken = await this.prisma.tenant.findUnique({ where: { alias } });
      if (!taken) break;
      alias = `${baseAlias}-${randomBytes(2).toString('hex')}`;
    }

    const apiKey = generateApiKey();
    const trialEndsAt = new Date(Date.now() + 30 * 24 * 3600 * 1000);

    const result = await this.prisma.$transaction(async (tx) => {
      const tenant = await tx.tenant.create({
        data: {
          alias,
          name: dto.companyName,
          companyName: dto.companyName,
          apiKey,
          trialEndsAt,
        },
      });
      const user = await tx.user.create({
        data: {
          email: dto.email,
          passwordHash,
          name: dto.name,
          tenantId: tenant.id,
        },
      });
      return { user, tenant };
    });

    // Email de verificação (assíncrono, não bloqueia o signup)
    this.sendVerificationEmail(result.user.id, result.user.email, result.user.name).catch((e) =>
      this.logger.error(`Falha ao enviar verificação: ${e.message}`),
    );

    const payload = {
      sub: result.user.id,
      email: result.user.email,
      name: result.user.name,
      tenantId: result.tenant.id,
    };
    const token = this.jwtService.sign(payload);

    return {
      token,
      user: { id: result.user.id, email: result.user.email, name: result.user.name },
      tenant: {
        id: result.tenant.id,
        alias: result.tenant.alias,
        companyName: result.tenant.companyName,
        apiKey: result.tenant.apiKey!,
      },
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

  getGithubAuthUrl(tenantAlias: string): string {
    const clientId = process.env.GITHUB_CLIENT_ID;
    const callbackUrl = process.env.GITHUB_CALLBACK_URL;
    const params = new URLSearchParams({
      client_id: clientId,
      redirect_uri: callbackUrl,
      scope: 'repo',
      state: tenantAlias,
    });
    return `https://github.com/login/oauth/authorize?${params.toString()}`;
  }

  async handleGithubCallback(code: string, state: string): Promise<string> {
    const clientId = process.env.GITHUB_CLIENT_ID;
    const clientSecret = process.env.GITHUB_CLIENT_SECRET;
    const callbackUrl = process.env.GITHUB_CALLBACK_URL;
    const redirectUrl = process.env.GITHUB_REDIRECT_URL;

    const tokenRes = await fetch('https://github.com/login/oauth/access_token', {
      method: 'POST',
      headers: { 'Accept': 'application/json', 'Content-Type': 'application/json' },
      body: JSON.stringify({ client_id: clientId, client_secret: clientSecret, code, redirect_uri: callbackUrl }),
    });

    const tokenData = await tokenRes.json() as { access_token?: string; error?: string };
    if (!tokenData.access_token) {
      throw new InternalServerErrorException('Falha ao obter token do GitHub');
    }

    const tenant = await this.prisma.tenant.findUnique({ where: { alias: state } });
    if (!tenant) {
      throw new UnauthorizedException('Tenant não encontrado');
    }

    await this.prisma.tenant.update({
      where: { id: tenant.id },
      data: { githubAccessToken: tokenData.access_token },
    });

    return redirectUrl;
  }

  async listGithubRepos(tenantId: string): Promise<GithubRepo[]> {
    const tenant = await this.prisma.tenant.findUnique({ where: { id: tenantId } });
    if (!tenant?.githubAccessToken) {
      throw new UnauthorizedException('GitHub não conectado');
    }

    const res = await fetch('https://api.github.com/user/repos?per_page=100', {
      headers: {
        'Authorization': `Bearer ${tenant.githubAccessToken}`,
        'Accept': 'application/vnd.github+json',
        'X-GitHub-Api-Version': '2022-11-28',
      },
    });

    if (!res.ok) {
      throw new InternalServerErrorException('Falha ao listar repos do GitHub');
    }

    const repos = await res.json() as any[];
    return repos.map((r) => ({
      id: r.id,
      name: r.name,
      fullName: r.full_name,
      private: r.private,
      url: r.html_url,
      defaultBranch: r.default_branch,
    }));
  }

  async disconnectGithub(tenantId: string): Promise<void> {
    const tenant = await this.prisma.tenant.findUnique({ where: { id: tenantId } });
    if (!tenant) {
      throw new UnauthorizedException('Tenant não encontrado');
    }
    await this.prisma.tenant.update({
      where: { id: tenantId },
      data: { githubAccessToken: null },
    });
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

  // ---------- Email verification ----------
  async sendVerificationEmail(userId: string, email: string, name: string | null): Promise<void> {
    const token = randomBytes(24).toString('hex');
    const expiresAt = new Date(Date.now() + 7 * 24 * 3600 * 1000); // 7 dias
    await this.prisma.emailVerification.create({
      data: { userId, token, expiresAt },
    });
    const verifyUrl = `${LANDPAGE_URL}/verify?token=${token}`;
    await this.emailService.sendVerification(email, name, verifyUrl);
  }

  async resendVerification(userId: string): Promise<{ ok: true; cooldown_seconds: number }> {
    const user = await this.prisma.user.findUnique({ where: { id: userId } });
    if (!user) throw new UnauthorizedException('Usuário não encontrado');
    if (user.emailVerified) {
      throw new BadRequestException('Email já verificado');
    }

    // Rate limit: max 3 envios em 1h, e mínimo 60s entre envios consecutivos
    const oneHourAgo = new Date(Date.now() - 3600 * 1000);
    const recent = await this.prisma.emailVerification.findMany({
      where: { userId, createdAt: { gte: oneHourAgo } },
      orderBy: { createdAt: 'desc' },
      take: 5,
    });
    if (recent.length >= 3) {
      const oldest = recent[recent.length - 1];
      const retrySeconds = Math.ceil((oldest.createdAt.getTime() + 3600 * 1000 - Date.now()) / 1000);
      throw new HttpException(
        {
          message: `Limite atingido — 3 envios por hora. Tenta de novo em ${Math.ceil(retrySeconds / 60)} min.`,
          retry_after_seconds: Math.max(retrySeconds, 60),
        },
        HttpStatus.TOO_MANY_REQUESTS,
      );
    }
    if (recent.length > 0) {
      const last = recent[0];
      const elapsed = (Date.now() - last.createdAt.getTime()) / 1000;
      if (elapsed < 60) {
        const wait = Math.ceil(60 - elapsed);
        throw new HttpException(
          {
            message: `Aguarde ${wait}s antes de reenviar.`,
            retry_after_seconds: wait,
          },
          HttpStatus.TOO_MANY_REQUESTS,
        );
      }
    }

    // Marca tokens antigos como usados pra invalidar
    await this.prisma.emailVerification.updateMany({
      where: { userId, used: false },
      data: { used: true },
    });
    await this.sendVerificationEmail(user.id, user.email, user.name);
    return { ok: true, cooldown_seconds: 60 };
  }

  async verifyEmail(token: string): Promise<{ ok: true; email: string }> {
    if (!token) throw new BadRequestException('Token ausente');
    const v = await this.prisma.emailVerification.findUnique({ where: { token } });
    if (!v || v.used) throw new BadRequestException('Token inválido ou já usado');
    if (v.expiresAt < new Date()) throw new BadRequestException('Token expirado — peça um novo');

    const user = await this.prisma.user.update({
      where: { id: v.userId },
      data: { emailVerified: true, emailVerifiedAt: new Date() },
    });
    await this.prisma.emailVerification.update({
      where: { id: v.id },
      data: { used: true },
    });
    return { ok: true, email: user.email };
  }
}
