import { Body, Controller, Headers, Post, Req, UseGuards } from '@nestjs/common';
import { ApiHeader, ApiOperation, ApiResponse, ApiSecurity, ApiTags } from '@nestjs/swagger';
import { Request } from 'express';
import { AuthService } from './auth.service';
import { LoginDto, LoginResponseDto, RegisterDto, TenantLoginDto, TenantLoginResponseDto } from './auth.dto';
import { JwtAuthGuard } from './jwt-auth.guard';
import { CurrentUser, AuthUser } from './current-user.decorator';

@ApiTags('Auth')
@Controller('api/auth')
export class AuthController {
  constructor(private readonly authService: AuthService) {}

  @Post('register')
  @ApiOperation({ summary: 'Registrar novo usuário e obter JWT' })
  @ApiResponse({ status: 201, description: 'Usuário criado', type: LoginResponseDto })
  @ApiResponse({ status: 400, description: 'Dados inválidos ou senhas não coincidem' })
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
}
