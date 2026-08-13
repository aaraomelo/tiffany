#!/bin/bash
# assistente.sh — corpus primeiro; ollama como gabarito; aprende de volta.
#
#   ./assistente.sh <base> "a fala"
#   ./assistente.sh <base> conversa
#   MODELO=qwen2.5:1.5b ./assistente.sh <base> "oi"
#
# O llama NÃO é o Maestro — só gabarito temporário quando o corpus diz «não sei».
set -u
CD="$(cd "$(dirname "$0")" && pwd)"
ROOT=$(cd "$CD/.." && pwd)
BASE=${1:-$ROOT/banco/.fala/assistente}
CV=${CONVERSA:-$ROOT/banco/bin/conversa}
MODELO="${MODELO:-qwen2.5:1.5b}"
SISTEMA="$CD/.gabarito_sistema.txt"

[ -x "$CV" ] || { echo "assistente: falta $CV" >&2; exit 1; }
mkdir -p "$(dirname "$BASE")" "$BASE"

if [ ! -f "$SISTEMA" ]; then
  cat > "$SISTEMA" <<'SYS'
És uma assistente educada e simpática. Português do Brasil, 1–3 frases.
Assuntos do dia a dia. NÃO fales de teoria, papers ou algoritmos.
SYS
fi

ollama_ask() {
  local fala="$1"
  python3 - "$MODELO" "$SISTEMA" "$fala" <<'PY'
import json, sys, urllib.request
modelo, sistema_path, fala = sys.argv[1], sys.argv[2], sys.argv[3]
sistema = open(sistema_path, encoding="utf-8").read().strip()
payload = {
  "model": modelo,
  "stream": False,
  "options": {"temperature": 0.35, "num_predict": 200},
  "messages": [
    {"role": "system", "content": sistema},
    {"role": "user", "content": fala},
  ],
}
req = urllib.request.Request(
  "http://127.0.0.1:11434/api/chat",
  data=json.dumps(payload).encode("utf-8"),
  headers={"Content-Type": "application/json"},
  method="POST",
)
with urllib.request.urlopen(req, timeout=120) as r:
  d = json.loads(r.read().decode("utf-8"))
print(d.get("message", {}).get("content", "").strip())
PY
}

responde() {
  local fala="$1"
  local r
  r=$("$CV" "$BASE" responde "$fala" 2>/dev/null | head -1)
  if [ -n "$r" ] && [ "$r" != "não sei." ] && [ "$r" != "nao sei" ]; then
    echo "[corpus] $r"
    return 0
  fi
  echo "[corpus] não sei — gabarito ollama ($MODELO)" >&2
  local g
  g=$(ollama_ask "$fala" 2>/dev/null | tr '\n' ' ' | sed 's/  */ /g; s/^[[:space:]]*//; s/[[:space:]]*$//')
  if [ -z "$g" ]; then
    echo "[llama] (não respondeu — ollama a correr?)"
    return 1
  fi
  echo "[llama] $g"
  "$CV" "$BASE" aprende "$fala" "$g" >/dev/null 2>&1
  echo "[corpus] aprendido — da próxima sem acordar o modelo" >&2
}

if [ "${2:-}" = "conversa" ] || [ "${1:-}" = "conversa" ]; then
  echo "assistente: corpus=$BASE gabarito=$MODELO  (Ctrl-D sai)"
  while IFS= read -r -p "> " linha; do
    [ -z "$linha" ] && continue
    responde "$linha"
  done
else
  shift 2>/dev/null || true
  fala="${*:-}"
  [ -z "$fala" ] && { echo "uso: $0 <base> \"fala\" | $0 <base> conversa" >&2; exit 2; }
  responde "$fala"
fi
