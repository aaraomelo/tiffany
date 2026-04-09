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
- Não criar arquivos fora de `src/`
- Não modificar `prisma/schema.prisma` (gerenciado externamente)
- Não modificar `.github/workflows/`
- Usar os services existentes: `PrismaService`, `TasksService`, `TaskStateMachine`, `DailySummaryService`

## Endpoints existentes

- `GET /api` — root
- `GET /api/health` — health check simples
- `GET /api/status` — resumo diário
- `GET/POST /api/tasks` — CRUD de tarefas
- `GET/POST /api/projects` — CRUD de projetos
- `POST /api/webhooks/github` — webhook GitHub

## Testes

Não há testes unitários configurados. Testar via deploy em DEV.
