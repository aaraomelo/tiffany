#!/bin/sh
# patria.sh — portão local da publicação (o live é a outra coluna).
#
#   bash tools/patria.sh           # só disco
#   bash tools/patria.sh --live    # + GET goldenkingdom
#
set -e
ROOT=$(cd "$(dirname "$0")/.." && pwd)
falha=0
tem(){ [ -s "$1" ] || { echo "  falta $1"; falha=1; }; }

echo "=== PATRIA portão (local) ==="
tem "$ROOT/conecthus/pipeline.tex"
tem "$ROOT/conecthus/claims/deploy.claim"
tem "$ROOT/assets/figuras/wasm/claim.wasm"
grep -q 'conecthus/pipeline.tex' "$ROOT/app/src/corpo.json" \
  || { echo "  corpo.json sem pipeline.tex"; falha=1; }
grep -q 'corpo/conecthus/pipeline.tex' "$ROOT/.github/workflows/publica.yml" \
  || { echo "  publica.yml sem health pipeline"; falha=1; }

if [ "${1:-}" = "--live" ]; then
  echo "=== PATRIA live ==="
  U=https://goldenkingdom.patriatechnology.com/corpo/conecthus/pipeline.tex
  C=$(curl -sf -o /dev/null -w '%{http_code}' "$U" || echo 000)
  echo "  GET $U → $C"
  [ "$C" = "200" ] || falha=1
fi

[ "$falha" = 0 ] && echo "portão local OK" || echo "portão REOPEN"
exit $falha
