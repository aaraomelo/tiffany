import { Controller, Post, Get, Body, Query, BadRequestException } from '@nestjs/common';
import { ApiTags, ApiOperation, ApiSecurity } from '@nestjs/swagger';
import { PatriciaGatewayService } from './patricia-gateway.service';
import { PatriciaActionDto } from './patricia-gateway.dto';

@ApiTags('Patricia Gateway')
@ApiSecurity('api-key')
@Controller('api/patricia')
export class PatriciaGatewayController {
  constructor(private readonly gateway: PatriciaGatewayService) {}

  @Post('action')
  @ApiOperation({ summary: 'Execute a validated action through Patricia gateway' })
  async executeAction(@Body() dto: PatriciaActionDto) {
    if (!dto.action) throw new BadRequestException('action is required');
    return this.gateway.executeAction(dto.action, dto.channel, dto.target, dto.params);
  }

  @Get('context')
  @ApiOperation({ summary: 'Get dynamic context for Patricia session' })
  async getContext(@Query('channel') channel: string, @Query('target') target: string) {
    if (!channel || !target) throw new BadRequestException('channel and target required');
    const context = await this.gateway.getContext(channel, target);
    return { context };
  }

  @Get('session')
  @ApiOperation({ summary: 'Get current session state' })
  async getSession(@Query('channel') channel: string, @Query('target') target: string) {
    if (!channel || !target) throw new BadRequestException('channel and target required');
    const session = await this.gateway.getOrCreateSession(channel, target);
    const refreshed = await this.gateway.refreshPhase(session);
    return this.gateway.buildSessionState(refreshed);
  }
}
