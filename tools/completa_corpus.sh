#!/usr/bin/env bash
# completa_corpus.sh — Re-gera pares truncados (frases cortadas a meio).
#
#   cd banco && ../tools/completa_corpus.sh .fala/<hex> papers/conversa_arvore_n2_fundacao.tex
#   ../tools/completa_corpus.sh .fala/<hex>   # todos conversa_arvore_*.tex
#
set -euo pipefail
B="${1:?uso: ./completa_corpus.sh <base> [tex…]}"
shift || true
D=$(cd "$(dirname "$0")" && pwd)
ROOT=$(cd "$D/.." && pwd)
CV="$ROOT/banco/bin/conversa"
MODELO="${MODELO:-qwen2.5:1.5b}"
SISTEMA="$ROOT/tools/.gabarito_fundacao.txt"
[ -f "$SISTEMA" ] || SISTEMA="$ROOT/tools/.gabarito_sistema.txt"
[ -x "$CV" ] || { echo "falta $CV"; exit 1; }

if [ "$#" -eq 0 ]; then
  set -- "$ROOT"/papers/conversa_arvore_*.tex
fi

TMP=$(mktemp)
trap 'rm -f "$TMP"' EXIT

for tex in "$@"; do
  [ -f "$tex" ] || continue
  python3 - "$tex" >> "$TMP" <<'PY'
import sys, re
path = sys.argv[1]
txt = open(path, encoding="utf-8", errors="replace").read()
parts = re.split(r'\\(?:sub)*section\*?\{', txt)
for corpo in parts[1:]:
    fecha = corpo.find('}')
    if fecha < 0:
        continue
    tit = corpo[:fecha].strip()
    resto = corpo[fecha+1:]
    resto = re.sub(r'\\[a-zA-Z]+\*?\{?', ' ', resto)
    resto = resto.replace('}', ' ').replace('{', ' ')
    resto = re.sub(r'\s+', ' ', resto).strip()
    if len(tit) < 4 or len(resto) < 20:
        continue
    fim = resto.rstrip()
    trunc = bool(fim) and fim[-1] not in '.!?…'
    if re.search(r'\b(prát|opç|estrutu|fundaç|sistem|dinam|topolog|continu|selec|recomen|aplicad)\s*$', fim, re.I):
        trunc = True
    if trunc:
        # tab-separated; escape newlines already flattened
        print(tit.replace('\t',' ') + '\t' + resto.replace('\t',' '))
PY
done

n=0
while IFS=$'\t' read -r fala resp; do
  [ -z "${fala:-}" ] && continue
  echo "── corta: $fala"
  nova=$(python3 - "$MODELO" "$SISTEMA" "$fala" "$resp" <<'PY'
import json, sys, urllib.request, re
modelo, sistema_path, fala, pedaco = sys.argv[1:5]
sistema = open(sistema_path, encoding="utf-8").read().strip()
sistema += "\nCompleta frases cortadas. Devolve a resposta INTEIRA (1–4 frases), com pontuação final. Sem aspas."
user = f"Pergunta: {fala}\nResposta incompleta: {pedaco}\nEscreve a resposta completa e terminada."
payload = {
  "model": modelo, "stream": False,
  "options": {"temperature": 0.35, "num_predict": 220},
  "messages": [
    {"role": "system", "content": sistema},
    {"role": "user", "content": user},
  ],
}
req = urllib.request.Request(
  "http://127.0.0.1:11434/api/chat",
  data=json.dumps(payload).encode("utf-8"),
  headers={"Content-Type": "application/json"}, method="POST",
)
with urllib.request.urlopen(req, timeout=180) as r:
  d = json.loads(r.read().decode("utf-8"))
txt = re.sub(r"\s+", " ", d.get("message", {}).get("content", "").strip())
txt = txt.replace('"', "").replace("'", "")[:1200]
if txt and txt[-1] not in ".!?…":
  txt += "."
print(txt)
PY
)
  if [ -z "$nova" ]; then echo "   (vazio)"; continue; fi
  echo "   [completa] ${nova:0:120}…"
  "$CV" "$B" aprende "$fala" "$nova" >/dev/null
  n=$((n+1))
done < "$TMP"

echo "completadas: $n pares → $B"
