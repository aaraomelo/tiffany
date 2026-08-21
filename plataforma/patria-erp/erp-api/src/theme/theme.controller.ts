import { Body, Controller, Delete, Get, Put } from '@nestjs/common';
import { ThemeConfigDto } from './dto/theme.dto';
import { ThemeService } from './theme.service';

@Controller('theme')
export class ThemeController {
  constructor(private readonly service: ThemeService) {}

  @Get()
  get() {
    return this.service.get();
  }

  @Put()
  update(@Body() dto: ThemeConfigDto) {
    return this.service.update(dto);
  }

  @Delete()
  reset() {
    return this.service.reset();
  }
}
