import { Injectable, UnauthorizedException } from '@nestjs/common';
import { PassportStrategy } from '@nestjs/passport';
import { ExtractJwt, Strategy } from 'passport-jwt';
import { AuthService } from './auth.service';

export interface JwtPayload {
  sub: string;
  email: string;
  name: string;
}

@Injectable()
export class JwtStrategy extends PassportStrategy(Strategy) {
  constructor(private readonly authService: AuthService) {
    super({
      jwtFromRequest: ExtractJwt.fromAuthHeaderAsBearerToken(),
      ignoreExpiration: false,
      secretOrKey: process.env.JWT_SECRET || 'jwt_secret_patria_2026',
      passReqToCallback: true,
    });
  }

  async validate(req: Request, payload: JwtPayload) {
    const authHeader = (req.headers as any)['authorization'] as string;
    const token = authHeader?.replace('Bearer ', '');

    if (token && await this.authService.isTokenRevoked(token)) {
      throw new UnauthorizedException('Token revogado');
    }

    return { id: payload.sub, email: payload.email, name: payload.name };
  }
}
