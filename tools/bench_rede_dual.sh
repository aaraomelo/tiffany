#!/usr/bin/env bash
# bench_rede_dual.sh — medidor da rede neural dual (UI + conjugação)
#   cd repo && ./tools/bench_rede_dual.sh
set -euo pipefail
D=$(cd "$(dirname "$0")" && pwd)
ROOT=$(cd "$D/.." && pwd)
cd "$ROOT"
echo "=== rede dual (tests/rede_dual.js) ==="
node tests/rede_dual.js
