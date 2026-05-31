import {
  Body,
  Controller,
  Delete,
  Get,
  Param,
  ParseUUIDPipe,
  Patch,
  Post,
  Query,
} from '@nestjs/common';
import { CheckPolicies } from '../access/check-policies.decorator';
import { CreateProductDto } from './dto/create-product.dto';
import { ListProductDto } from './dto/list-product.dto';
import { UpdateProductDto } from './dto/update-product.dto';
import { ProductService } from './product.service';

@Controller('products')
export class ProductController {
  constructor(private readonly service: ProductService) {}

  @Post()
  @CheckPolicies({ action: 'create', subject: 'Product' })
  create(@Body() dto: CreateProductDto) {
    return this.service.create(dto);
  }

  @Get()
  @CheckPolicies({ action: 'read', subject: 'Product' })
  list(@Query() dto: ListProductDto) {
    return this.service.list(dto);
  }

  @Get('search/semantic')
  @CheckPolicies({ action: 'read', subject: 'Product' })
  semanticSearch(
    @Query('q') q: string,
    @Query('limit') limit?: string,
  ) {
    return this.service.semanticSearch(q ?? '', limit ? Number(limit) : 20);
  }

  @Post('backfill-embeddings')
  @CheckPolicies({ action: 'update', subject: 'Product' })
  backfillEmbeddings() {
    return this.service.backfillEmbeddings();
  }

  @Get(':id')
  @CheckPolicies({ action: 'read', subject: 'Product' })
  findOne(@Param('id', ParseUUIDPipe) id: string) {
    return this.service.findOne(id);
  }

  @Patch(':id')
  @CheckPolicies({ action: 'update', subject: 'Product' })
  update(
    @Param('id', ParseUUIDPipe) id: string,
    @Body() dto: UpdateProductDto,
  ) {
    return this.service.update(id, dto);
  }

  @Delete(':id')
  @CheckPolicies({ action: 'delete', subject: 'Product' })
  remove(@Param('id', ParseUUIDPipe) id: string) {
    return this.service.remove(id);
  }
}
