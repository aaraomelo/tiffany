import { Controller, Get, Param, Res, NotFoundException, BadRequestException } from '@nestjs/common';
import { ApiTags, ApiOperation } from '@nestjs/swagger';
import type { Response } from 'express';
import { existsSync, createReadStream, statSync } from 'fs';
import { join } from 'path';

const AUDIO_DIR = process.env.AUDIO_DIR || '/app/data/audio';

@ApiTags('Audio (público)')
@Controller('api/audio')
export class AudioController {
  @Get(':filename')
  @ApiOperation({ summary: 'Serve áudio gerado (TTS) — público' })
  serve(@Param('filename') filename: string, @Res() res: Response) {
    if (!/^[\w.-]+\.(ogg|mp3|opus|wav|m4a)$/i.test(filename)) {
      throw new BadRequestException('invalid filename');
    }
    const fullPath = join(AUDIO_DIR, filename);
    if (!existsSync(fullPath)) {
      throw new NotFoundException('audio not found');
    }
    const stat = statSync(fullPath);
    const ext = filename.split('.').pop()?.toLowerCase();
    const mime: Record<string, string> = {
      ogg: 'audio/ogg',
      opus: 'audio/ogg',
      mp3: 'audio/mpeg',
      wav: 'audio/wav',
      m4a: 'audio/mp4',
    };
    res.setHeader('Content-Type', mime[ext || 'ogg'] || 'application/octet-stream');
    res.setHeader('Content-Length', String(stat.size));
    res.setHeader('Cache-Control', 'public, max-age=86400');
    createReadStream(fullPath).pipe(res);
  }
}
