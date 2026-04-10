import { Body, Controller, Post, Req, UseGuards } from '@nestjs/common';
import { ApiOperation, ApiResponse, ApiSecurity, ApiTags } from '@nestjs/swagger';
import { Request } from 'express';
import { AuthService } from './auth.service';
import { LoginDto, LoginResponseDto } from './auth.dto';
import { JwtAuthGuard } from './jwt-auth.guard';
import { CurrentUser, AuthUser } from './current-user.decorator';

@ApiTags('Auth')
@Controller('api/auth')
export class AuthController {
  constructor(private readonly authService: AuthService) {}

  @Post('login')
  @ApiOperation({ summary: 'Autenticar usuário e obter JWT' })
  @ApiResponse({ status: 201, description: 'Login realizado', type: LoginResponseDto })
  @ApiResponse({ status: 401, description: 'Credenciais inválidas' })
  @ApiResponse({ status: 400, description: 'Dados inválidos' })
  login(@Body() dto: LoginDto): Promise<LoginResponseDto> {
    return this.authService.login(dto);
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
