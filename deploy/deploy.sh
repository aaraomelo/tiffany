#!/bin/sh
# Roda no servidor (após rsync do código). Build da imagem + restart do container.
set -e
cd /root/patria-erp
docker build -t patria-erp:latest .
DBPASS=$(cat /root/.erp_db_pass)
JWT=$(cat /root/.erp_jwt)
# Conexões: DIRECT sempre no superusuário (migrate/DDL). Runtime no erp_app só
# quando o arquivo de ativação da RLS nativa existir (senão fica no superusuário).
DB_SUPER="postgresql://erp:${DBPASS}@erp-postgres-prod:5432/erp?schema=public"
DB_RUNTIME="$DB_SUPER"
NATIVE_FLAG=""
# Se o app vai conectar como erp_app (não-super), a RLS nativa PRECISA estar
# ligada (senão a policy bloqueia tudo). Os dois andam acoplados.
if [ -f /root/.erp_app_dburl ]; then
  DB_RUNTIME="$(cat /root/.erp_app_dburl)"
  NATIVE_FLAG="-e RLS_NATIVE=on"
fi
# SMTP (recuperação de senha) — opcional; só monta a flag se o arquivo existir
SMTP_FLAG=""
if [ -f /root/.erp_smtp ]; then
  SMTP_FLAG="--env-file /root/.erp_smtp"
fi
# RLS (controle de acesso a nível de linha) — knob controlável por arquivo.
# Conteúdo: "shadow" (observa/loga, no-op) ou "enforce" (aplica). Ausente = off.
RLS_FLAG=""
if [ -f /root/.erp_rls_mode ]; then
  RLS_FLAG="-e RLS_MODE=$(cat /root/.erp_rls_mode)"
fi
# Operadores de plataforma (admin geral, ⊤) — emails separados por vírgula.
OP_FLAG=""
if [ -f /root/.erp_operator_emails ]; then
  OP_FLAG="-e PLATFORM_OPERATOR_EMAILS=$(cat /root/.erp_operator_emails)"
fi
docker rm -f patria-erp 2>/dev/null || true
docker run -d --name patria-erp --network erp-net --restart unless-stopped \
  -p 127.0.0.1:8090:8080 \
  -e PORT=8080 -e NODE_ENV=production \
  -e DATABASE_URL="$DB_RUNTIME" \
  -e DIRECT_DATABASE_URL="$DB_SUPER" \
  -e JWT_SECRET="$JWT" -e JWT_EXPIRES_IN=7d \
  $SMTP_FLAG \
  $RLS_FLAG \
  $OP_FLAG \
  $NATIVE_FLAG \
  patria-erp:latest
echo "deploy ok"
