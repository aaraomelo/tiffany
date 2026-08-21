import {
  Controller,
  Get,
  Post,
  Body,
  HttpException,
  HttpStatus,
} from '@nestjs/common';

const LLM_BASE = process.env.PATRIA_LLM_URL || 'http://host.docker.internal:8002';

@Controller('api/llm')
export class LLMController {
  @Get('health')
  async health() {
    try {
      const r = await fetch(`${LLM_BASE}/api/llm/health`, {
        signal: AbortSignal.timeout(3000),
      });
      return await r.json();
    } catch (e: any) {
      return { ok: false, error: e.message };
    }
  }

  @Get('info')
  async info() {
    try {
      const r = await fetch(`${LLM_BASE}/api/llm/info`, {
        signal: AbortSignal.timeout(5000),
      });
      if (!r.ok) throw new HttpException(`upstream ${r.status}`, HttpStatus.SERVICE_UNAVAILABLE);
      return await r.json();
    } catch (e: any) {
      throw new HttpException(`LLM server indisponível: ${e.message}`, HttpStatus.SERVICE_UNAVAILABLE);
    }
  }

  @Post('generate')
  async generate(@Body() body: any) {
    if (!body?.prompt) throw new HttpException('prompt obrigatório', HttpStatus.BAD_REQUEST);
    // clamp pra evitar abuso
    const safeBody = {
      prompt: String(body.prompt).slice(0, 500),
      max_new: Math.max(1, Math.min(120, body.max_new ?? 80)),
      temperature: Math.max(0.1, Math.min(1.5, body.temperature ?? 0.85)),
      top_k: Math.max(1, Math.min(100, body.top_k ?? 50)),
    };
    try {
      const r = await fetch(`${LLM_BASE}/api/llm/generate`, {
        method: 'POST',
        headers: { 'Content-Type': 'application/json' },
        body: JSON.stringify(safeBody),
        signal: AbortSignal.timeout(60000),
      });
      if (!r.ok) {
        const t = await r.text();
        throw new HttpException(t || `upstream ${r.status}`, r.status);
      }
      return await r.json();
    } catch (e: any) {
      if (e instanceof HttpException) throw e;
      throw new HttpException(`Falha na geração: ${e.message}`, HttpStatus.INTERNAL_SERVER_ERROR);
    }
  }
}
