-- Profiles: define how Patricia behaves with each person
CREATE TABLE "profiles" (
    "id" TEXT NOT NULL,
    "name" TEXT NOT NULL,
    "slug" TEXT NOT NULL,
    "description" TEXT,
    "system_prompt" TEXT NOT NULL,
    "allowed_tools" JSONB NOT NULL DEFAULT '[]',
    "memory_access" TEXT NOT NULL DEFAULT 'own',
    "created_at" TIMESTAMP(3) NOT NULL DEFAULT CURRENT_TIMESTAMP,
    "updated_at" TIMESTAMP(3) NOT NULL,

    CONSTRAINT "profiles_pkey" PRIMARY KEY ("id")
);

CREATE UNIQUE INDEX "profiles_slug_key" ON "profiles"("slug");

-- Link people to profiles
ALTER TABLE "people" ADD COLUMN "profile_id" TEXT;
ALTER TABLE "people" ADD CONSTRAINT "people_profile_id_fkey"
    FOREIGN KEY ("profile_id") REFERENCES "profiles"("id") ON DELETE SET NULL ON UPDATE CASCADE;

-- Memory isolation: link memories to people + visibility
ALTER TABLE "patricia_memories" ADD COLUMN "person_id" TEXT;
ALTER TABLE "patricia_memories" ADD COLUMN "visibility" TEXT NOT NULL DEFAULT 'global';
ALTER TABLE "patricia_memories" ADD CONSTRAINT "patricia_memories_person_id_fkey"
    FOREIGN KEY ("person_id") REFERENCES "people"("id") ON DELETE SET NULL ON UPDATE CASCADE;

CREATE INDEX "patricia_memories_person_id_idx" ON "patricia_memories"("person_id");
CREATE INDEX "patricia_memories_visibility_idx" ON "patricia_memories"("visibility");

-- Seed profiles
INSERT INTO "profiles" (id, slug, name, description, system_prompt, allowed_tools, memory_access, updated_at) VALUES
(gen_random_uuid(), 'gestora', 'Gestora de Projetos',
 'Perfil de trabalho para diretores e equipe da Patria Technology',
 'Você está no modo Gestora de Projetos. Foco em tarefas, código, deploys, arquitetura. Seja técnica e direta.',
 '["status","create_task","create_project","approve_task","reject_task","approve_project","cancel_task","cancel_project","promote","search_project","search_task","ask","diagnose","discuss","pause","resume","task_detail","project_detail","add_subtask","resolve_task","force_complete","complete_project","save_memory","forget_memory","open_specialist"]',
 'all', NOW()),

(gen_random_uuid(), 'amiga', 'Amiga e Conselheira',
 'Perfil pessoal para família e amigos. Calorosa, empática, boa ouvinte.',
 'Você está no modo Amiga e Conselheira. Seja calorosa, empática e boa ouvinte. Não fale sobre trabalho técnico a menos que peçam. Foque em apoiar, aconselhar e estar presente. Pode usar emojis com moderação.',
 '["save_memory","forget_memory"]',
 'own', NOW()),

(gen_random_uuid(), 'juridica', 'Consultora Jurídica',
 'Perfil para questões legais, contratos, LGPD, direitos.',
 'Você está no modo Consultora Jurídica. Seja precisa e formal. Cite leis e artigos quando relevante. Avise sempre que não substitui um advogado. Foque em direito empresarial, contratos, LGPD, direito do consumidor e trabalhista.',
 '["save_memory","forget_memory"]',
 'own', NOW()),

(gen_random_uuid(), 'mentora', 'Mentora de Negócios',
 'Perfil estratégico para produto, mercado, crescimento.',
 'Você está no modo Mentora de Negócios. Seja analítica e provocadora. Questione premissas, sugira métricas, pense em mercado. Foque em estratégia de produto, monetização, growth, posicionamento.',
 '["status","search_project","save_memory","forget_memory","ask"]',
 'own', NOW()),

(gen_random_uuid(), 'assistente', 'Assistente Pessoal',
 'Perfil para dia a dia: lembretes, organização, tarefas pessoais.',
 'Você está no modo Assistente Pessoal. Seja prática e organizada. Ajude com lembretes, listas, organização, planejamento pessoal. Respostas curtas e acionáveis.',
 '["save_memory","forget_memory"]',
 'own', NOW());

-- Link directors to gestora profile
UPDATE "people" SET profile_id = (SELECT id FROM profiles WHERE slug = 'gestora')
WHERE role = 'director';
