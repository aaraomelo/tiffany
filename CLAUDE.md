# Instruções para Claude Code — patria-api

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
- Node 20

## Convenções

- Arquivos em `src/` (compilados para `dist/`)
- Não criar arquivos fora de `src/` (exceto prisma/)
- Não modificar `.github/workflows/`
- Usar os services existentes: `PrismaService`, `TasksService`, `TaskStateMachine`, `DailySummaryService`

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
- `GET /api/health/detailed` — health detalhado (pode não existir ainda)
- `GET /api/status` — resumo diário
- `GET /api/pending-actions` — ações pendentes do diretor
- `GET/POST /api/tasks` — CRUD de tarefas
- `GET/POST /api/projects` — CRUD de projetos
- `POST /api/webhooks/github` — webhook GitHub

## Testes

Não há testes unitários configurados. Testar via deploy em DEV.
