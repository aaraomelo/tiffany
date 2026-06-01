-- Ativação da RLS NATIVA (Fase B) — rodar como SUPERUSUÁRIO (erp) UMA vez.
-- Cria o role do app (não-super, SUJEITO a RLS) e concede privilégios. Depois
-- da execução: repontar o DATABASE_URL do app para erp_app e ligar RLS_NATIVE=on.
-- Migrations/seeds CONTINUAM no superusuário erp (bypass de RLS).
--
-- Reversível: para desativar, repontar o DATABASE_URL de volta para erp e
-- remover RLS_NATIVE. As policies podem ficar (inertes p/ superusuário).
--
-- A senha NÃO vai versionada: definir manualmente após criar o role.

DO $$
BEGIN
  IF NOT EXISTS (SELECT 1 FROM pg_roles WHERE rolname = 'erp_app') THEN
    CREATE ROLE erp_app LOGIN NOSUPERUSER NOBYPASSRLS;
  END IF;
END $$;

-- Definir a senha manualmente (não commitar):
--   ALTER ROLE erp_app PASSWORD '<senha-forte>';

GRANT CONNECT ON DATABASE erp TO erp_app;
GRANT USAGE ON SCHEMA public TO erp_app;
GRANT SELECT, INSERT, UPDATE, DELETE ON ALL TABLES IN SCHEMA public TO erp_app;
GRANT USAGE, SELECT ON ALL SEQUENCES IN SCHEMA public TO erp_app;

-- Privilégios padrão para objetos FUTUROS criados pelo owner (erp) — ex.: novas
-- tabelas de migrations. Sem isso, o app perderia acesso a tabelas novas.
ALTER DEFAULT PRIVILEGES FOR ROLE erp IN SCHEMA public
  GRANT SELECT, INSERT, UPDATE, DELETE ON TABLES TO erp_app;
ALTER DEFAULT PRIVILEGES FOR ROLE erp IN SCHEMA public
  GRANT USAGE, SELECT ON SEQUENCES TO erp_app;

-- PRÉ-REQUISITO antes de ligar RLS_NATIVE: as ~4 raw queries (product pgvector,
-- dashboard estoque, assistant memory) precisam setar o GUC do tenant na mesma
-- transação, senão a RLS as bloqueia (fail-closed). Ver TODO no rollout.
