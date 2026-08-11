#!/bin/bash
# sobe_tex_wasm.sh — sobe o tradutor .tex→PDF para wasm, e deixa-o onde o app o serve.
#
# O Aarão: PDF no cliente via WASM, sem servidor, sem TeX Live. A receita é a do
# tests/tex_wasm.js: unir libc+spline+núcleo+wrapper e traduzir com tools/traduz.c.
set -euo pipefail
CD="$(cd "$(dirname "$0")" && pwd)"
RAIZ="$(cd "$CD/.." && pwd)"
OUT="${1:-$RAIZ/assets/figuras/wasm/tex.wasm}"
TMP="${TMPDIR:-/tmp}/sobe_tex_wasm_$$"
mkdir -p "$(dirname "$OUT")" "$TMP"
trap 'rm -rf "$TMP"' EXIT

echo "sobe_tex_wasm: a construir o traduz…"
# o binário vive no TMP desta corrida — tools/bin/traduz às vezes fica root-owned
# e o `cc -o` falha em silêncio noutros UIDs, deixando um módulo com o tecto antigo (32 MB).
TRADUZ="$TMP/traduz"
cc -O2 -std=c99 -w "$CD/traduz.c" -o "$TRADUZ"
mkdir -p "$CD/bin"
cp -f "$TRADUZ" "$CD/bin/traduz" 2>/dev/null || true

sem_inc(){ grep -v '^\s*#\s*include' "$1"; }

PRE='#define SEEK_SET 0
#define SEEK_CUR 1
#define SEEK_END 2
#define NULL 0
#define EOF (-1)
#define stderr 0
#define stdout 1
#define stdin 3
'
{
  printf '%s' "$PRE"
  sem_inc "$CD/libc.c"
  echo
  sem_inc "$RAIZ/lib/le_num.h"
  echo
  sem_inc "$RAIZ/lib/spline.h"
  echo
  sem_inc "$RAIZ/tests/tex_core.c"
  echo
  sem_inc "$RAIZ/tests/tex.c"
} > "$TMP/unido.c"

echo "sobe_tex_wasm: a traduzir → $OUT"
"$TRADUZ" "$TMP/unido.c" -o "$OUT"
BYTES=$(wc -c < "$OUT" | tr -d ' ')
echo "sobe_tex_wasm: $BYTES bytes — o módulo está pronto para o navegador"
