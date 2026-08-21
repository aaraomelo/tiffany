#!/bin/bash
# deploy.sh — corre NO SERVIDOR, depois de o artefato chegar por scp.
#
# Substitui o script que o workflow do Patria-Labs/patria-api escrevia inline e mandava por
# ssh. Está em ficheiro por duas razões: um heredoc dentro de YAML parte-se ao primeiro
# descuido de indentação (partiu-se, aliás), e um script versionado pode ser lido e corrigido
# sem se abrir o workflow.
#
#   deploy.sh <dir> <compose> <container> <porta> <migrar 0|1> <app> <ambiente> <caminho de saúde>
#
# O DATABASE_URL não vem por argumento nem por ambiente do workflow: procura-se aqui, em
# /root/plataforma/conecthus/deploy-vars.txt (chmod 600), que guarda as environment variables
# dos repositórios antes de eles serem apagados. O mesmo princípio dos /root/.erp_* .
set -e
DIR="$1"; COMPOSE="$2"; CONTAINER="$3"; PORTA="$4"; MIGRAR="$5"; APP="$6"; AMB="$7"; SAUDE="$8"
VARS=/root/plataforma/conecthus/deploy-vars.txt
ART=/tmp/conecthus-artefato.tar.gz

[ -f "$ART" ] || { echo "o artefato não chegou a $ART"; exit 1; }
mkdir -p "$DIR"
tar -xzf "$ART" -C "$DIR"
rm -f "$ART"
echo "extraído em $DIR"

if [ "$MIGRAR" != "0" ] && [ "$APP" = "patria-api" ]; then
  E=$(echo "$AMB" | tr a-z A-Z); [ "$E" = "PRODUCAO" ] && E=PROD
  URL=$(grep "^patria-api $E DATABASE_URL=" "$VARS" | cut -d= -f2-)
  [ -n "$URL" ] || { echo "sem DATABASE_URL para $E em $VARS"; exit 1; }
  cd "$DIR"
  npm install prisma@5.22.0 --no-save >/dev/null 2>&1
  DATABASE_URL="$URL" npx prisma migrate deploy
  echo "migrations aplicadas em $E"
fi

docker compose -f "$COMPOSE" build "$CONTAINER"
docker compose -f "$COMPOSE" up -d --force-recreate "$CONTAINER"

for i in $(seq 1 20); do
  S=$(curl -sf -o /dev/null -w '%{http_code}' "http://127.0.0.1:$PORTA$SAUDE" 2>/dev/null || echo 000)
  [ "$S" = "200" ] && { echo "no ar: $CONTAINER responde 200 em $PORTA$SAUDE"; exit 0; }
  echo "  tentativa $i: $S"
  sleep 5
done
echo "não ficou 200 a tempo; os últimos registos de $CONTAINER:"
docker logs "$CONTAINER" --tail 25
exit 1
