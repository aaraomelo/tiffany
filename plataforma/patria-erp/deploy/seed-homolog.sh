#!/bin/sh
# Seed de módulos/packs do HOMOLOG (roda no host contra o postgres de homolog).
set -e
cd /root/patria-erp-homolog/erp-api
npm ci --silent
DBPASS=$(cat /root/.erp_homolog_db_pass)
URL="postgresql://erp:${DBPASS}@127.0.0.1:5437/erp?schema=public"
# Gera o Prisma Client no host (senão o seed em ts-node não enxerga enums como
# ModuleCategory). DIRECT precisa existir só p/ a validação do schema.
DATABASE_URL="$URL" DIRECT_DATABASE_URL="$URL" npx prisma generate
DATABASE_URL="$URL" DIRECT_DATABASE_URL="$URL" npm run seed:modules
