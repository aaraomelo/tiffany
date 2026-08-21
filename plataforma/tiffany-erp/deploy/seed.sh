#!/bin/sh
# Seed de módulos/packs (roda no host contra o postgres local do erp).
set -e
cd /root/patria-erp/erp-api
npm ci --silent
DBPASS=$(cat /root/.erp_db_pass)
DATABASE_URL="postgresql://erp:${DBPASS}@127.0.0.1:5436/erp?schema=public" npm run seed:modules
