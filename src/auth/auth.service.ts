import { ConflictException, Injectable, InternalServerErrorException, UnauthorizedException } from '@nestjs/common';
import { JwtService } from '@nestjs/jwt';
import * as bcrypt from 'bcrypt';
import { PrismaService } from '../prisma.service';
import { LoginDto, LoginResponseDto, RegisterDto, TenantLoginDto, TenantLoginResponseDto } from './auth.dto';

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
}
