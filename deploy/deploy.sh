#!/bin/sh
# Roda no servidor (após rsync do código). Build da imagem + restart do container.
set -e
cd /root/patria-erp
docker build -t patria-erp:latest .
DBPASS=$(cat /root/.erp_db_pass)
JWT=$(cat /root/.erp_jwt)
docker rm -f patria-erp 2>/dev/null || true
docker run -d --name patria-erp --network erp-net --restart unless-stopped \
  -p 127.0.0.1:8090:8080 \
  -e PORT=8080 -e NODE_ENV=production \
  -e DATABASE_URL="postgresql://erp:${DBPASS}@erp-postgres-prod:5432/erp?schema=public" \
  -e JWT_SECRET="$JWT" -e JWT_EXPIRES_IN=7d \
  patria-erp:latest
echo "deploy ok"
