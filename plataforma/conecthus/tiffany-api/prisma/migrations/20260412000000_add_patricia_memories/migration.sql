-- CreateTable
CREATE TABLE "patricia_memories" (
    "id" TEXT NOT NULL,
    "category" TEXT NOT NULL,
    "title" TEXT NOT NULL,
    "content" TEXT NOT NULL,
    "created_at" TIMESTAMP(3) NOT NULL DEFAULT CURRENT_TIMESTAMP,
    "updated_at" TIMESTAMP(3) NOT NULL,

    CONSTRAINT "patricia_memories_pkey" PRIMARY KEY ("id")
);

-- Full-text search column
ALTER TABLE "patricia_memories" ADD COLUMN "search_text" tsvector
    GENERATED ALWAYS AS (
        setweight(to_tsvector('portuguese', coalesce(title, '')), 'A') ||
        setweight(to_tsvector('portuguese', coalesce(content, '')), 'B') ||
        setweight(to_tsvector('portuguese', coalesce(category, '')), 'C')
    ) STORED;

-- Indexes
CREATE INDEX "patricia_memories_category_idx" ON "patricia_memories"("category");
CREATE INDEX "patricia_memories_search_idx" ON "patricia_memories" USING GIN ("search_text");

-- Seed: product knowledge from PRODUCT.md
INSERT INTO "patricia_memories" (id, category, title, content, updated_at) VALUES
    (gen_random_uuid(), 'product', 'O que é o produto',
     'Plataforma de automação de código via IA. O cliente conecta seu repositório GitHub, pede alterações em linguagem natural (chat), e a IA planeja, codifica, testa e deploya automaticamente.',
     NOW()),
    (gen_random_uuid(), 'product', 'Para quem é o produto',
     'Startups e PMEs que não têm equipe técnica dedicada mas precisam construir e manter software.',
     NOW()),
    (gen_random_uuid(), 'product', 'Monetização',
     'SaaS com assinatura mensal por tenant. Planos: Starter (1 repo, 10 tarefas/mês), Pro (5 repos, 50 tarefas/mês), Enterprise (ilimitado + suporte prioritário). Trial de 14 dias.',
     NOW()),
    (gen_random_uuid(), 'product', 'Módulos do produto',
     'Fase 1 (implementado): Tenant e Onboarding, Dashboard do cliente. Fase 2 (planejado): Chat Web (cliente conversa com IA pelo browser), Billing/Stripe (cobrança automática).',
     NOW()),
    (gen_random_uuid(), 'product', 'Stack tecnológica',
     'Backend: NestJS + Prisma + PostgreSQL. Frontend: React + Vite. Worker: Node.js + Claude Code CLI. CI/CD: GitHub Actions. Deploy: Docker containers. WhatsApp/Telegram: Evolution API + Claude API.',
     NOW()),
    (gen_random_uuid(), 'product', 'Arquitetura multi-tenant',
     'Cada cliente tem um subdomínio: alias.patriatechnology.com. Dados isolados por tenantId. Nginx extrai o alias do subdomínio e passa como header X-Tenant.',
     NOW()),
    (gen_random_uuid(), 'person', 'Aarão Melo',
     'Diretor e operador principal da Patria Technology. Perfil técnico, prefere automação. Gosta de respostas diretas e soluções práticas. Contato WhatsApp: +5511977808883.',
     NOW()),
    (gen_random_uuid(), 'person', 'Patrícia Cunha',
     'Diretora da Patria Technology.',
     NOW()),
    (gen_random_uuid(), 'person', 'Carlos Daniel',
     'Diretor da Patria Technology.',
     NOW()),
    (gen_random_uuid(), 'decision', 'Migração do OpenClaw',
     'Decisão de migrar do OpenClaw para solução própria com Evolution API (WhatsApp) + Claude API (tool calling). Motivo: OpenClaw é caixa preta, não expõe contexto de conversa (grupo vs DM). Telegram já migrado e funcionando.',
     NOW()),
    (gen_random_uuid(), 'technical', 'Ambientes de deploy',
     'Três ambientes: DEV (branch develop), HOMOLOG (branch homolog), PROD (branch main). Push na branch dispara deploy automático via GitHub Actions. Validação pre-deploy testa a aplicação antes de deployar.',
     NOW()),
    (gen_random_uuid(), 'technical', 'Sistema de tarefas',
     'Patrícia cria tarefas/projetos via gateway. Worker polls o banco, invoca Claude Code headless para planejar e executar. 13 estados na máquina de estado. Fix tasks automáticas quando deploy falha (máx 3 na cadeia).',
     NOW());
