#!/usr/bin/env bash
# indexa_orbitas.sh — gera corpus_orbitas_pt.txt e IMPORT IDIOMA no banco.
#
#   bash tools/indexa_orbitas.sh [base_sql]
#   default: /tmp/tiffany_orbita_idx  (ou .torre/idioma se existir)
#
set -euo pipefail
D=$(cd "$(dirname "$0")" && pwd)
ROOT=$(cd "$D/.." && pwd)
BASE="${1:-/tmp/tiffany_orbita_idx}"
IDX="$ROOT/lib/classe/corpus_orbitas_pt.txt"

cd "$ROOT"
cc -O2 -std=c99 -w -Ilib -o /tmp/indexa_orbitas tests/indexa_orbitas.c -lm
/tmp/indexa_orbitas

[ -x /tmp/sqlb ] || cc -O2 -std=c99 -w -Ilib -Ibanco -o /tmp/sqlb banco/sql.c -lm
# também o léxico + órbitas
/tmp/sqlb "$BASE" "IMPORT IDIOMA 'pt'" >/tmp/ix_lex.out 2>&1 || true
/tmp/sqlb "$BASE" "IMPORT IDIOMA 'pt' '$IDX'" | tee /tmp/ix_orb.out
echo "índice: $IDX"
echo "base sql: $BASE"
echo "uso: /tmp/sqlb $BASE \"ACHA TEXTO 'idioma/pt/orbita/…'\""
