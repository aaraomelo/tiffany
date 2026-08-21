-- Row-Level Security NATIVA do PostgreSQL (Fase B): piso duro que confina por
-- tenant até em raw query. Ver doc/casl-propagation.tex §7 (piso nativo).
--
-- INERTE até a ativação: o app hoje conecta como SUPERUSUÁRIO (erp), que IGNORA
-- RLS. Só passa a valer quando o app conectar como um role dedicado não-super
-- (erp_app) — ver deploy/rls-native-activate.sql. Migrations/seeds seguem no
-- superusuário (bypass).
--
-- Policy: a linha é visível sse pertence ao tenant ativo (GUC app.tenant_id) ou
-- o operador pediu bypass (GUC app.bypass_rls=on). GUC vazio/ausente → NULL →
-- fail-closed (nada visível).

DO $rls$
DECLARE
  t text;
BEGIN
  FOR t IN
    SELECT c.table_name
      FROM information_schema.columns c
      JOIN information_schema.tables tb
        ON tb.table_schema = 'public'
       AND tb.table_name = c.table_name
       AND tb.table_type = 'BASE TABLE'
     WHERE c.table_schema = 'public'
       AND c.column_name = 'tenantId'
  LOOP
    EXECUTE format('ALTER TABLE %I ENABLE ROW LEVEL SECURITY', t);
    EXECUTE format('ALTER TABLE %I FORCE ROW LEVEL SECURITY', t);
    EXECUTE format('DROP POLICY IF EXISTS tenant_isolation ON %I', t);
    EXECUTE format(
      'CREATE POLICY tenant_isolation ON %I'
      || ' USING ("tenantId" = NULLIF(current_setting(''app.tenant_id'', true), '''')::uuid'
      || '        OR current_setting(''app.bypass_rls'', true) = ''on'')'
      || ' WITH CHECK ("tenantId" = NULLIF(current_setting(''app.tenant_id'', true), '''')::uuid'
      || '        OR current_setting(''app.bypass_rls'', true) = ''on'')',
      t);
  END LOOP;
END
$rls$;
