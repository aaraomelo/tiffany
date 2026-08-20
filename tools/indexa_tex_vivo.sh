#!/usr/bin/env bash
# indexa_tex_vivo.sh — põe na base SÓ títulos → @TEX (lê .tex ao vivo no responde).
#
#   bash tools/indexa_tex_vivo.sh [base]
#   default: banco/.fala/reino
#
set -euo pipefail
D=$(cd "$(dirname "$0")" && pwd)
ROOT=$(cd "$D/.." && pwd)
CV="$ROOT/banco/bin/conversa"
B="${1:-$ROOT/banco/.fala/reino}"

[ -x "$CV" ] || cc -O2 -std=c99 -w -I"$ROOT/lib" -o "$CV" "$ROOT/banco/conversa.c" -lm
mkdir -p "$B"
export TIFFANY_ROOT="$ROOT"

echo "=== indexa tex vivo → $B (sem duplicar corpo) ==="
python3 "$D/indexa_tex_vivo.py" | "$CV" "$B" -
# Ponteiros explícitos (ganham sobre títulos homónimos em fala/)
aprende(){ "$CV" "$B" aprende "$1" "$2" >/dev/null; }
aprende "mostra a fundação" "@TEX corpus/docs/torre_fundacao.tex"
aprende "mostra a teoria" "@TEX teoria.tex"
aprende "mostra o corpo de peano" "@TEX papers/corpo_topologico.tex"
aprende "mostra a partitura" "@TEX corpus/docs/partitura.tex"
aprende "mostra o catálogo" "@TEX catalogo.tex"
aprende "mostra o catalogo" "@TEX catalogo.tex"
aprende "ensina a cadeia" "@TEX corpus/docs/ciencia_dragao.tex"
aprende "mostra a ciencia do dragao" "@TEX corpus/docs/ciencia_dragao.tex"
aprende "membrana latex ascii" "@TEX corpus/docs/ciencia_dragao.tex"
echo "pronto. responde lê o .tex na hora (TIFFANY_ROOT=$ROOT)."
echo "uso: TIFFANY_ROOT=$ROOT $CV $B responde \"…\""
