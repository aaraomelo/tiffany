#!/bin/sh
# Seed de módulos/packs do HOMOLOG (roda no host contra o postgres de homolog).
set -e
cd /root/patria-erp-homolog/erp-api
npm ci --silent
DBPASS=$(cat /root/.erp_homolog_db_pass)
DATABASE_URL="postgresql://erp:${DBPASS}@127.0.0.1:5437/erp?schema=public" npm run seed:modules
