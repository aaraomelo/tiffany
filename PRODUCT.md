# Patria Technology — Visão do Produto

## O que é

Plataforma de automação de código via IA. O cliente conecta seu repositório GitHub, pede alterações em linguagem natural (chat), e a IA planeja, codifica, testa e deploya automaticamente.

## Para quem

Startups e PMEs que não têm equipe técnica dedicada mas precisam construir e manter software.

## Proposta de valor

"Diga o que quer em português. A IA escreve o código, deploya e mantém."

O cliente não precisa saber programar. Ele descreve o que quer — a Patria Technology transforma em código funcionando em produção.

## Monetização

Assinatura mensal por tenant com planos escalonados:

| Plano | Tarefas/mês | Projetos/mês | Repos | Ambientes |
|-------|-------------|--------------|-------|-----------|
| Starter | 10 | 2 | 1 | DEV |
| Pro | 50 | 10 | 3 | DEV + HOMOLOG |
| Enterprise | Ilimitado | Ilimitado | Ilimitado | DEV + HOMOLOG + PROD |

## Módulos (ordem de prioridade)

### 1. Tenant & Onboarding (Fase 1)
- Cadastro do cliente (nome, empresa, email)
- Configuração do subdomínio (`{alias}.patriatechnology.com`)
- Conexão com repositório GitHub (OAuth ou deploy key)
- Setup inicial: clonar repo, configurar worker, primeiro deploy

**Modelo de dados:**
```
Tenant {
  id, alias (subdomínio), name, companyName,
  plan (starter/pro/enterprise), status (active/suspended/trial),
  githubOrg, githubRepos[], 
  createdAt, trialEndsAt
}

TenantUser {
  id, tenantId, email, passwordHash, name, role (admin/member),
  createdAt
}
```

### 2. Dashboard do Cliente (Fase 1)
- Visão geral: projetos ativos, tarefas recentes, deploys
- Detalhe de projeto: subtarefas, status, timeline
- Detalhe de tarefa: plano, resultado, commits
- Filtros por status e ambiente

### 3. Interface de Pedidos — Chat Web (Fase 2)
- Chat integrado no app (não depender só de WhatsApp)
- Cliente digita pedido → sistema cria tarefa/projeto
- Histórico de conversas por projeto
- Notificações in-app

### 4. Billing & Planos (Fase 2)
- Integração Stripe para cobranças
- Controle de limites (tarefas/mês, repos)
- Upgrade/downgrade de plano
- Período trial (14 dias)
- Dashboard de uso (quantas tarefas usou, quanto falta)

### 5. Gestão de Repos (Fase 2)
- Cliente conecta repositório GitHub via OAuth
- Sistema clona, configura branches (main/develop)
- CI/CD configurado automaticamente
- Suporte a múltiplos repos por tenant

### 6. Isolamento Multi-tenant (Fase 3)
- Worker dedicado por tenant (ou pool compartilhado com fila)
- Banco isolado ou schema por tenant
- Secrets/env vars por tenant
- Rate limiting por plano

## Arquitetura Multi-tenant

### Roteamento
```
{alias}.patriatechnology.com → nginx → X-Tenant: {alias} → API
```
O header `X-Tenant` identifica o cliente em toda requisição. Já funciona hoje.

### Isolamento de dados
Toda query filtra por `tenantId`. Nenhum dado de um tenant é visível para outro.

### Repos do cliente
- Cliente conecta via GitHub OAuth
- Sistema clona no servidor em `/tenants/{alias}/repos/{repo}`
- Worker executa Claude Code no diretório do tenant
- CI/CD configurado por tenant (GitHub Actions no repo do cliente)

## Fluxo do Usuário

```
1. Cliente acessa patriatechnology.com → Landing page
2. Clica em "Começar" → Cadastro (nome, empresa, email, senha)
3. Configura subdomínio (ex: minhaempresa.patriatechnology.com)
4. Conecta repositório GitHub
5. Acessa dashboard → Vê que está tudo pronto
6. Faz pedido via chat: "Cria uma página de contato"
7. Sistema planeja → Cliente aprova → IA executa → Deploy automático
8. Cliente vê resultado no dashboard
```

## Stack Técnica

- **Landing page:** React 19 + Vite 5 (landpage)
- **App do cliente:** React 19 + Vite 5 + Redux + Tailwind (patria-app)
- **API:** NestJS 11 + TypeScript + Prisma + PostgreSQL (patria-api)
- **Worker:** Node.js + Claude Code CLI + Prisma (patria-worker)
- **IA:** Claude Code (execução), Gemini Flash (inferência), Gemini Embeddings (busca vetorial)
- **Deploy:** Docker + GitHub Actions + nginx reverse proxy
- **Comunicação:** WhatsApp (OpenClaw), chat web (futuro)

## Regras de Negócio

1. **Um tenant = um subdomínio.** Alias único, imutável após criação.
2. **Plano define limites.** Tarefas, projetos, repos, ambientes.
3. **Trial de 14 dias.** Sem cartão. Após trial, suspende se não assinar.
4. **Dados isolados.** Tenant A nunca vê dados do Tenant B.
5. **Repo do cliente.** A Patria não é dona do código — o cliente é.
6. **Deploy no repo do cliente.** CI/CD roda no GitHub do cliente.
7. **Sem lock-in.** Cliente pode sair e levar todo o código a qualquer momento.

## Convenções para Desenvolvimento

Ao implementar features do SaaS:
- Toda entidade nova deve ter `tenantId` como campo obrigatório
- Todo endpoint deve filtrar por tenant (extraído do `X-Tenant` header)
- Validar limites do plano antes de criar tarefas/projetos
- Logs e auditoria devem incluir `tenantId`
- Testes devem cobrir isolamento (tenant A não acessa tenant B)
