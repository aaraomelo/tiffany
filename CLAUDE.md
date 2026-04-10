# Instruções para Claude Code — patria-api

## IMPORTANTE: Leia PRODUCT.md

Antes de planejar ou implementar qualquer feature, leia o arquivo `PRODUCT.md` na raiz deste repo. Ele contém a visão do produto, modelo de dados, regras de negócio e convenções. Toda nova entidade deve ter `tenantId`, todo endpoint deve filtrar por tenant.

## Estrutura Docker

O código roda dentro de um container Docker com esta estrutura:
```
/app/
├── package.json
├── package-lock.json
├── prisma/
├── dist/          ← código compilado (nest build)
│   ├── main.js
│   ├── app.module.js
│   └── ...
├── public/        ← frontend (patria-app build)
└── data/          ← volume persistente
```

**IMPORTANTE:** Ao referenciar `package.json` no código, use `path.join(__dirname, '..', 'package.json')` (um nível acima de dist/). NÃO use `../../package.json`.

## Stack

- NestJS 11 + TypeScript
- Prisma ORM + PostgreSQL
- Swagger (@nestjs/swagger) — documentação de API em /api/docs
- class-validator + class-transformer — validação de entrada
- Node 20

## Convenções

- Arquivos em `src/` (compilados para `dist/`)
- Não criar arquivos fora de `src/` (exceto prisma/)
- Não modificar `.github/workflows/`
- Usar os services existentes: `PrismaService`, `TasksService`, `TaskStateMachine`, `DailySummaryService`

## OBRIGATÓRIO: Swagger + Validação

Todo endpoint DEVE ter:

### 1. DTO com validação (class-validator)
```typescript
import { IsString, IsEmail, IsInt, Min, Max, IsOptional } from 'class-validator';
import { ApiProperty, ApiPropertyOptional } from '@nestjs/swagger';

export class CreateFeedbackDto {
  @ApiProperty({ description: 'Nome do remetente', example: 'João Silva' })
  @IsString()
  nome: string;

  @ApiProperty({ description: 'Email do remetente', example: 'joao@email.com' })
  @IsEmail()
  email: string;

  @ApiProperty({ description: 'Nota de 1 a 5', minimum: 1, maximum: 5 })
  @IsInt()
  @Min(1)
  @Max(5)
  nota: number;

  @ApiPropertyOptional({ description: 'Mensagem opcional' })
  @IsOptional()
  @IsString()
  mensagem?: string;
}
```

### 2. Controller com decorators Swagger
```typescript
import { ApiTags, ApiOperation, ApiResponse, ApiSecurity } from '@nestjs/swagger';

@ApiTags('Feedbacks')
@ApiSecurity('api-key')
@Controller('api/feedbacks')
export class FeedbacksController {

  @Post()
  @ApiOperation({ summary: 'Criar feedback' })
  @ApiResponse({ status: 201, description: 'Feedback criado' })
  @ApiResponse({ status: 400, description: 'Dados inválidos' })
  create(@Body() dto: CreateFeedbackDto) { ... }

  @Get()
  @ApiOperation({ summary: 'Listar feedbacks' })
  findAll() { ... }
}
```

### 3. Regras
- SEMPRE criar DTOs para POST/PATCH (nunca usar `@Body() body: any`)
- SEMPRE adicionar `@ApiTags` no controller
- SEMPRE adicionar `@ApiOperation` e `@ApiResponse` em cada endpoint
- SEMPRE usar `@ApiProperty` nos DTOs
- DTOs ficam no mesmo arquivo do controller ou em arquivo separado `*.dto.ts`
- Validação: `@IsString`, `@IsEmail`, `@IsInt`, `@Min`, `@Max`, `@IsOptional`, `@IsNotEmpty`

## Migrations (Prisma)

Quando precisar alterar o banco de dados (adicionar tabela, campo, relação, etc.):

1. Edite `prisma/schema.prisma` com as alterações
2. Execute via Bash: `npx prisma migrate dev --create-only --name descricao_curta`
3. Isso gera o arquivo SQL em `prisma/migrations/TIMESTAMP_descricao_curta/migration.sql`
4. Inclua TUDO no commit: `schema.prisma` + pasta `prisma/migrations/`
5. NÃO execute `prisma migrate deploy` — o pipeline faz isso automaticamente

**Regras:**
- Use `--create-only` (não aplica a migration, só cria o SQL)
- Nomes de migration em snake_case sem acentos (ex: `add_users_table`)
- Se adicionar um novo model, registre o service correspondente em `app.module.ts`
- Após criar migration, rode `npx prisma generate` para atualizar os tipos TypeScript

## Endpoints existentes

- `GET /api` — root
- `GET /api/health` — health check simples
- `GET /api/docs` — Swagger UI (documentação interativa)
- `GET /api/status` — resumo diário
- `GET /api/pending-actions` — ações pendentes do diretor
- `GET/POST /api/tasks` — CRUD de tarefas
- `GET/POST /api/projects` — CRUD de projetos
- `POST /api/webhooks/github` — webhook GitHub

## Testes

Não há testes unitários configurados. Testar via deploy em DEV.
