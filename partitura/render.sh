#!/usr/bin/env bash
# partitura/render.sh --- .ly -> PDF (gravação tipográfica)
# Usa tools/lilypond/ se existir; senão lilypond do PATH.
set -euo pipefail
ROOT="$(cd "$(dirname "$0")/.." && pwd)"
HERE="$ROOT/partitura"
OUT="$HERE/render"
GAB="$HERE/gabarito"

LP=""
if [[ -x "$ROOT/tools/lilypond/bin/lilypond" ]]; then
  LP="$ROOT/tools/lilypond/bin/lilypond"
elif command -v lilypond >/dev/null 2>&1; then
  LP="$(command -v lilypond)"
else
  echo "lilypond ausente. Extraia o binário oficial em tools/lilypond/ ou instale no PATH." >&2
  exit 1
fi

mkdir -p "$OUT"
cd "$GAB"
for ly in tradutor-completo.ly assinatura-quatro-naipes.ly assinatura-pqr.ly lei8-ciclo.ly; do
  base="${ly%.ly}"
  echo "==> $ly"
  "$LP" -o "$OUT/$base" "$ly"
done

# PNG da partitura do sistema (preview / papers sem PDF multi-página)
if command -v pdftoppm >/dev/null 2>&1; then
  pdftoppm -png -r 150 -singlefile "$OUT/tradutor-completo.pdf" "$OUT/tradutor-completo"
fi

# lixo midi
rm -f "$OUT"/*.midi
echo "OK: $OUT"
ls -lh "$OUT"/*.pdf
