#!/usr/bin/env bash
# sobe_backends_wasm.sh — sobe as linguagens-backend (C subset) para wasm via traduz.
# Nenhuma é privilegiada: mesma porta, mesma régua. Claim e ISA entram na mesma fila.
set -euo pipefail
RAIZ="$(cd "$(dirname "$0")/.." && pwd)"
OUT="$RAIZ/assets/figuras/wasm"
TRADUZ="$RAIZ/tools/bin/traduz"
if [[ -x "$RAIZ/tools/bin/traduz.exe" ]]; then TRADUZ="$RAIZ/tools/bin/traduz.exe"; fi

mkdir -p "$RAIZ/tools/bin" "$OUT"
CC="${CC:-cc}"
command -v "$CC" >/dev/null 2>&1 || CC=gcc
echo "sobe_backends_wasm: a construir o traduz com $CC…"
"$CC" -O2 -std=c99 -w "$RAIZ/tools/traduz.c" -o "$TRADUZ"

echo "sobe_backends_wasm: a subir linguagens[]…"
node "$RAIZ/tools/sobe_backends_wasm.mjs"

echo "sobe_backends_wasm: pronto em $OUT"
