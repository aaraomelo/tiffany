import { Controller, Get, Param, Res, NotFoundException, BadRequestException } from '@nestjs/common';
import { ApiTags, ApiOperation } from '@nestjs/swagger';
import type { Response } from 'express';
import { existsSync, createReadStream, statSync } from 'fs';
import { join } from 'path';

const IMAGE_DIR = process.env.IMAGE_DIR || '/app/data/images';

@ApiTags('Images (público)')
@Controller('api/images')
export class ImagesController {
  @Get(':filename')
  @ApiOperation({ summary: 'Serve imagem gerada (público — link mandado em chat)' })
  serve(@Param('filename') filename: string, @Res() res: Response) {
    if (!/^[\w.-]+\.(png|jpg|jpeg|webp)$/i.test(filename)) {
      throw new BadRequestException('invalid filename');
    }
    const fullPath = join(IMAGE_DIR, filename);
    if (!existsSync(fullPath)) {
      throw new NotFoundException('image not found');
    }
    const stat = statSync(fullPath);
    const ext = filename.split('.').pop()?.toLowerCase();
    const mime: Record<string, string> = { png: 'image/png', jpg: 'image/jpeg', jpeg: 'image/jpeg', webp: 'image/webp' };
    res.setHeader('Content-Type', mime[ext || 'png'] || 'application/octet-stream');
    res.setHeader('Content-Length', String(stat.size));
    res.setHeader('Cache-Control', 'public, max-age=86400');
    createReadStream(fullPath).pipe(res);
  }
}
