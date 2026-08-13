#!/usr/bin/env bash
# sobe_backends_wasm.sh — sobe as linguagens-backend (C subset) para wasm via traduz.
# Nenhuma é privilegiada: mesma porta, mesma régua. Claim e ISA entram na mesma fila.
set -euo pipefail
RAIZ="$(cd "$(dirname "$0")/.." && pwd)"
OUT="$RAIZ/assets/figuras/wasm"
TRADUZ="$RAIZ/tools/bin/traduz"
MAN="$RAIZ/conecthus/backends/manifesto.json"

mkdir -p "$RAIZ/tools/bin" "$OUT"
echo "sobe_backends_wasm: a construir o traduz…"
cc -O2 -std=c99 -w "$RAIZ/tools/traduz.c" -o "$TRADUZ"

sobe() {
  local fonte="$1" wasm="$2"
  echo "  → $(basename "$wasm")  ($(realpath --relative-to="$RAIZ" "$fonte" 2>/dev/null || echo "$fonte"))"
  "$TRADUZ" "$fonte" -o "$wasm"
}

# manifesto: cada entrada com fonte → wasm
python3 - <<PY
import json, os, subprocess, sys
raiz = "$RAIZ"
man = json.load(open("$MAN"))
traduz = "$TRADUZ"
out = "$OUT"
falhas = 0
for L in man["linguagens"]:
    fonte = os.path.join(raiz, L["fonte"])
    wasm = os.path.join(out, L["wasm"])
    if not os.path.isfile(fonte):
        print(f"  FALTA fonte {L['nome']}: {fonte}", file=sys.stderr)
        falhas += 1
        continue
    print(f"  → {L['wasm']}  ({L['fonte']})")
    r = subprocess.run([traduz, fonte, "-o", wasm], capture_output=True, text=True)
    sys.stdout.write(r.stdout)
    if r.returncode != 0:
        sys.stderr.write(r.stderr or "traduz falhou\n")
        falhas += 1
        continue
    print(f"     {os.path.getsize(wasm)} bytes · exports {L.get('exports')}")
sys.exit(falhas)
PY

echo "sobe_backends_wasm: pronto em $OUT"
