#!/bin/sh
# HOMOLOG do patria-erp. Espelha deploy/deploy.sh, mas em container/porta/DB/secrets
# próprios — não toca nada do PROD. Roda no servidor após o rsync do código.
set -e
cd /root/patria-erp-homolog
docker build -t patria-erp-homolog:latest .
DBPASS=$(cat /root/.erp_homolog_db_pass)
JWT=$(cat /root/.erp_homolog_jwt)
# Conexões: DIRECT sempre no superusuário (migrate/DDL). Runtime no erp_app só
# quando o arquivo de ativação da RLS nativa existir (senão fica no superusuário).
# 21/08/2026: o postgres saiu do docker — cluster `erphomolog` do sistema, porta 5437.
# E ISTO TEM DE ESTAR AQUI, no repositório: o deploy extrai o artefato POR CIMA de
# deploy/, portanto uma correcção feita só no servidor é apagada no deploy seguinte.
# Foi o que aconteceu, e deixou o homolog em ciclo de P1001.
DB_SUPER="postgresql://erp:${DBPASS}@host.docker.internal:5437/erp?schema=public"
DB_RUNTIME="$DB_SUPER"
NATIVE_FLAG=""
if [ -f /root/.erp_homolog_app_dburl ]; then
  DB_RUNTIME="$(cat /root/.erp_homolog_app_dburl)"
  NATIVE_FLAG="-e RLS_NATIVE=on"
fi
SMTP_FLAG=""
if [ -f /root/.erp_homolog_smtp ]; then
  SMTP_FLAG="--env-file /root/.erp_homolog_smtp"
fi
RLS_FLAG=""
if [ -f /root/.erp_homolog_rls_mode ]; then
  RLS_FLAG="-e RLS_MODE=$(cat /root/.erp_homolog_rls_mode)"
fi
OP_FLAG=""
if [ -f /root/.erp_homolog_operator_emails ]; then
  OP_FLAG="-e PLATFORM_OPERATOR_EMAILS=$(cat /root/.erp_homolog_operator_emails)"
fi
SUP_FLAG=""
if [ -f /root/.erp_homolog_supplier_secret ]; then
  SUP_FLAG="-e SUPPLIER_SECRET=$(cat /root/.erp_homolog_supplier_secret)"
fi
LIVE_FLAG=""
if [ -f /root/.erp_homolog_supplier_live ]; then
  LIVE_FLAG="-e SUPPLIER_INTEGRATION_LIVE=on"
fi
docker rm -f patria-erp-homolog 2>/dev/null || true
docker run -d --name patria-erp-homolog --add-host=host.docker.internal:host-gateway --network erp-net --restart unless-stopped \
  -p 127.0.0.1:8091:8080 \
  -e PORT=8080 -e NODE_ENV=production \
  -e DATABASE_URL="$DB_RUNTIME" \
  -e DIRECT_DATABASE_URL="$DB_SUPER" \
  -e JWT_SECRET="$JWT" -e JWT_EXPIRES_IN=7d \
  -e ERP_FLARESOLVERR_URL=http://flaresolverr:8191 \
  $SMTP_FLAG \
  $RLS_FLAG \
  $OP_FLAG \
  $SUP_FLAG \
  $LIVE_FLAG \
  $NATIVE_FLAG \
  patria-erp-homolog:latest
echo "deploy homolog ok"
