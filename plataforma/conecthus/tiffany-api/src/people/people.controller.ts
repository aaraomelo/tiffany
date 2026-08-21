import { Controller, Get, Param, Query } from '@nestjs/common';
import { ApiTags, ApiOperation, ApiResponse, ApiSecurity, ApiParam, ApiQuery } from '@nestjs/swagger';
import { PeopleService } from './people.service';
import { SearchPeopleQueryDto, PersonResponseDto } from './people.dto';
import { RequestContext } from '../request-context';

@ApiTags('People')
@ApiSecurity('api-key')
@Controller('api/people')
export class PeopleController {
  constructor(private readonly peopleService: PeopleService) {}

  @Get('search')
  @ApiOperation({ summary: 'Buscar pessoas por nome, email ou telefone' })
  @ApiQuery({ name: 'q', description: 'Termo de busca', required: true })
  @ApiResponse({ status: 200, description: 'Lista de pessoas encontradas', type: [PersonResponseDto] })
  @ApiResponse({ status: 400, description: 'Parâmetro q ausente ou inválido' })
  search(@Query() query: SearchPeopleQueryDto): Promise<PersonResponseDto[]> {
    return this.peopleService.search(query.q, RequestContext.alias);
  }

  @Get(':id')
  @ApiOperation({ summary: 'Buscar pessoa por ID' })
  @ApiParam({ name: 'id', description: 'UUID da pessoa' })
  @ApiResponse({ status: 200, description: 'Pessoa encontrada', type: PersonResponseDto })
  @ApiResponse({ status: 404, description: 'Pessoa não encontrada' })
  findOne(@Param('id') id: string): Promise<PersonResponseDto> {
    return this.peopleService.findById(id, RequestContext.alias);
  }
}
