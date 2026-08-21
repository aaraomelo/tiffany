import { Body, Controller, Get, Headers, Post, Query, Req, Res, UseGuards } from '@nestjs/common';
import { ApiHeader, ApiOperation, ApiQuery, ApiResponse, ApiSecurity, ApiTags } from '@nestjs/swagger';
import { Request, Response } from 'express';
import { AuthService } from './auth.service';
import { LoginDto, LoginResponseDto, RegisterDto, SignupDto, TenantLoginDto, TenantLoginResponseDto } from './auth.dto';
import { JwtAuthGuard } from './jwt-auth.guard';
import { CurrentUser, AuthUser } from './current-user.decorator';

@ApiTags('Auth')
@Controller('api/auth')
export class AuthController {
  constructor(private readonly authService: AuthService) {}

  @Post('signup')
  @ApiOperation({ summary: 'Cadastro unificado: cria usuário + tenant + apiKey + JWT' })
  @ApiResponse({ status: 201, description: 'Usuário e tenant criados', type: LoginResponseDto })
  @ApiResponse({ status: 409, description: 'Email já está em uso' })
  signup(@Body() dto: SignupDto): Promise<LoginResponseDto> {
    return this.authService.signup(dto);
  }

  @Post('register')
  @ApiOperation({ summary: '[deprecated] Use /signup' })
  @ApiResponse({ status: 201, description: 'Usuário criado', type: LoginResponseDto })
  @ApiResponse({ status: 409, description: 'Email já está em uso' })
  register(@Body() dto: RegisterDto): Promise<LoginResponseDto> {
    return this.authService.register(dto);
  }

  @Post('login')
  @ApiOperation({ summary: 'Autenticar usuário e obter JWT' })
  @ApiResponse({ status: 201, description: 'Login realizado', type: LoginResponseDto })
  @ApiResponse({ status: 401, description: 'Credenciais inválidas' })
  @ApiResponse({ status: 400, description: 'Dados inválidos' })
  login(@Body() dto: LoginDto): Promise<LoginResponseDto> {
    return this.authService.login(dto);
  }

  @Post('tenant/login')
  @ApiOperation({ summary: 'Autenticar TenantUser e obter JWT com tenantId e role' })
  @ApiHeader({ name: 'X-Tenant', description: 'Alias do tenant', required: true })
  @ApiResponse({ status: 201, description: 'Login realizado', type: TenantLoginResponseDto })
  @ApiResponse({ status: 400, description: 'Dados inválidos' })
  @ApiResponse({ status: 401, description: 'Credenciais inválidas ou tenant não encontrado' })
  tenantLogin(
    @Headers('x-tenant') tenantAlias: string,
    @Body() dto: TenantLoginDto,
  ): Promise<TenantLoginResponseDto> {
    return this.authService.tenantLogin(tenantAlias, dto);
  }

  @Get('github')
  @UseGuards(JwtAuthGuard)
  @ApiSecurity('bearer')
  @ApiHeader({ name: 'X-Tenant', description: 'Alias do tenant', required: true })
  @ApiOperation({ summary: 'Iniciar fluxo OAuth do GitHub' })
  @ApiResponse({ status: 200, description: 'URL de autorização GitHub', schema: { properties: { authUrl: { type: 'string' } } } })
  @ApiResponse({ status: 401, description: 'Token inválido' })
  githubInitiate(@Headers('x-tenant') tenantAlias: string): { authUrl: string } {
    return { authUrl: this.authService.getGithubAuthUrl(tenantAlias) };
  }

  @Get('github/callback')
  @ApiOperation({ summary: 'Callback OAuth do GitHub (chamado pelo GitHub)' })
  @ApiQuery({ name: 'code', description: 'Código de autorização' })
  @ApiQuery({ name: 'state', description: 'Alias do tenant' })
  @ApiResponse({ status: 302, description: 'Redireciona para o frontend' })
  async githubCallback(
    @Query('code') code: string,
    @Query('state') state: string,
    @Res() res: Response,
  ): Promise<void> {
    try {
      const redirectUrl = await this.authService.handleGithubCallback(code, state);
      res.redirect(redirectUrl);
    } catch {
      const redirectUrl = process.env.GITHUB_REDIRECT_URL || '/';
      res.redirect(`${redirectUrl}?error=github_oauth_failed`);
    }
  }

  @Post('logout')
  @UseGuards(JwtAuthGuard)
  @ApiSecurity('bearer')
  @ApiOperation({ summary: 'Invalidar token JWT (logout)' })
  @ApiResponse({ status: 201, description: 'Logout realizado com sucesso' })
  @ApiResponse({ status: 401, description: 'Token inválido ou ausente' })
  async logout(
    @Req() req: Request,
    @CurrentUser() _user: AuthUser,
  ): Promise<{ message: string }> {
    const token = (req.headers['authorization'] as string)?.replace('Bearer ', '');
    await this.authService.logout(token);
    return { message: 'Logout realizado com sucesso' };
  }

  // Verifica email via link no email — público, recebe ?token=xxx
  @Get('verify')
  @ApiOperation({ summary: 'Confirma email via token enviado no signup' })
  async verify(@Query('token') token: string) {
    return this.authService.verifyEmail(token);
  }

  // Reenvia email — autenticado, pra usuário logado pedir novo link
  @Post('resend-verification')
  @UseGuards(JwtAuthGuard)
  @ApiSecurity('bearer')
  @ApiOperation({ summary: 'Reenvia email de verificação' })
  async resendVerification(@CurrentUser() user: AuthUser) {
    return this.authService.resendVerification(user.id);
  }
}
