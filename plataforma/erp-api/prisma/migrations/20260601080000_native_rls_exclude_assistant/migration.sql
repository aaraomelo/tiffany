-- Exclui as tabelas da assistente (Patrícia) da RLS nativa, para a primeira
-- ativação não exigir reescrever os múltiplos raws/updates por id do módulo.
-- Elas continuam escopadas em app-level (requireTenantId + WHERE "tenantId").
-- Reabilitar + envolver os raws no GUC fica como hardening posterior.
DO $rls$
DECLARE
  t text;
BEGIN
  FOR t IN
    SELECT table_name FROM information_schema.tables
     WHERE table_schema = 'public' AND table_name LIKE 'assistant_%'
  LOOP
    EXECUTE format('ALTER TABLE %I NO FORCE ROW LEVEL SECURITY', t);
    EXECUTE format('ALTER TABLE %I DISABLE ROW LEVEL SECURITY', t);
    EXECUTE format('DROP POLICY IF EXISTS tenant_isolation ON %I', t);
  END LOOP;
END
$rls$;
