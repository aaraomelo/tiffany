#!/bin/bash
# gabarito_arvore.sh — sobe a complexidade explorando a árvore do ollama.
#
# eval.txt: cotidiano → contexto → intenção → diálogo mais complexo → (teoria depois)
# Entra pelo assunto banal e ramifica: follow-ups, preferências, planos, empatia.
# Continua SEM Dual Sort / Maestro / papers.
#
#   cd banco && ../tools/gabarito_arvore.sh .fala/<hex>
#   NIVEL=2 PROF=2 ../tools/gabarito_arvore.sh .fala/<hex>
#
# NIVEL: 1=contexto  2=intenção/plano  3=diálogo mais denso (ainda sem teoria)
# PROF:  profundidade de follow-ups por semente (default 2)
#
set -euo pipefail
B="${1:?uso: ./gabarito_arvore.sh <base>}"
D=$(cd "$(dirname "$0")" && pwd)
ROOT=$(cd "$D/.." && pwd)
CV="$ROOT/banco/bin/conversa"
MODELO="${MODELO:-qwen2.5:1.5b}"
NIVEL="${NIVEL:-2}"
PROF="${PROF:-2}"
SUFIXO="${SUFIXO:-}"
TEX_OUT="$ROOT/papers/conversa_arvore_n${NIVEL}${SUFIXO}.tex"
SISTEMA="$ROOT/tools/.gabarito_sistema.txt"
mkdir -p "$B"
[ -x "$CV" ] || { echo "falta $CV"; exit 1; }

# sementes: override com SEMENTES="chá frio livro" … (onda nova)
if [ -n "${SEMENTES:-}" ]; then
  # shellcheck disable=SC2206
  sementes=($SEMENTES)
else
  sementes=(
    "café"
    "chuva"
    "sono"
    "almoço"
    "caminhada"
    "música"
    "filme"
    "gato"
    "domingo"
    "amizade"
    "cozinhar"
    "passatempo"
  )
fi

ollama_json() {
  # $1 = system extra, $2 = user
  python3 - "$MODELO" "$SISTEMA" "$1" "$2" <<'PY'
import json, sys, urllib.request
modelo, sistema_path, extra, fala = sys.argv[1], sys.argv[2], sys.argv[3], sys.argv[4]
base = open(sistema_path, encoding="utf-8").read().strip()
sistema = base + "\n" + extra
payload = {
  "model": modelo,
  "stream": False,
  "options": {"temperature": 0.55, "num_predict": 180},
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

limpa() {
  python3 -c 'import sys,re; t=sys.stdin.read(); t=re.sub(r"\s+"," ",t).strip(); t=t.replace(chr(34),"").replace(chr(39),""); print(t[:1200])'
}

aprende_par() {
  local fala="$1" resp="$2"
  [ -z "$resp" ] && return 1
  # nunca gravar títulos com ") " / "1) "
  fala=$(printf '%s' "$fala" | python3 -c 'import sys,re
t=sys.stdin.read().strip()
for _ in range(6):
 n=re.sub(r"^[\d\*\#•\-–—\.\)\(]+\s*","",t).strip(); n=re.sub(r"^\)\s*","",n).strip()
 if n==t: break
 t=n
print(t)')
  [ -z "$fala" ] && return 1
  "$CV" "$B" aprende "$fala" "$resp" >/dev/null
  python3 - "$fala" "$resp" >> "$TEX_OUT" <<'PY'
import sys
def esc(s):
  return (s.replace("\\","\\textbackslash{}")
           .replace("&","\\&").replace("%","\\%")
           .replace("$","\\$").replace("#","\\#")
           .replace("_","\\_").replace("{","\\{").replace("}","\\}"))
print(f"\\section{{{esc(sys.argv[1])}}}\n{esc(sys.argv[2])}\n")
PY
}

# perguntas por nível a partir da semente
perguntas_nivel() {
  local s="$1" n="$2"
  case "$n" in
    1) # contexto
      printf '%s\n' \
        "falemos de $s" \
        "o que achas de $s" \
        "conta uma coisa simples sobre $s"
      ;;
    2) # intenção / plano / preferência
      printf '%s\n' \
        "como encaixas $s na tua rotina" \
        "se eu estiver indeciso sobre $s o que sugeres" \
        "ajuda-me a escolher algo ligado a $s"
      ;;
    3) # diálogo mais denso (ainda banal)
      printf '%s\n' \
        "estou a ter um dia difícil e $s poderia ajudar como" \
        "explica com calma a alguém novo o que é bom em $s" \
        "faz um mini plano de 3 passos sobre $s para hoje"
      ;;
    *)
      printf '%s\n' "falemos de $s"
      ;;
  esac
}

followups_de() {
  # pede ao ollama 3 follow-ups curtos que um utilizador faria
  local tema="$1" resposta="$2"
  local extra='Responde SÓ com 3 perguntas curtas que um amigo faria a seguir, uma por linha. SEM numeração, SEM aspas, SEM prefixo ") " nem "1)" nem "- ".'
  local out
  out=$(ollama_json "$extra" "Tema: $tema. Acabei de dizer: $resposta. Que 3 perguntas naturais viriam a seguir?" || true)
  printf '%s\n' "$out" | python3 -c '
import sys,re
def limpa(l):
  l=l.strip()
  for _ in range(6):
    n=re.sub(r"^[\d\*\#•\-–—\.\)\(]+\s*","",l).strip()
    n=re.sub(r"^\)\s*","",n).strip()
    if n==l: break
    l=n
  return l
for raw in sys.stdin:
  l=limpa(raw)
  if 8 < len(l) < 160:
    print(l[:150])
'
}

SYS_EXTRA='Continua a regra: 1 ou 2 frases, educada, sem teoria técnica. Agora podes dar um pouco mais de contexto ou intenção, mas continua leve.'

{
  echo "% !TeX program = pdflatex"
  echo "% Árvore ollama — nível $NIVEL profundidade $PROF — sem teoria"
  echo "% Modelo: $MODELO"
  echo "\\documentclass[11pt,a4paper]{article}"
  echo "\\usepackage[utf8]{inputenc}\\usepackage[T1]{fontenc}\\usepackage[brazil]{babel}"
  echo "\\usepackage[margin=2.4cm]{geometry}\\usepackage[hidelinks]{hyperref}"
  echo "\\newcommand{\\code}[1]{\\texttt{\\small #1}}"
  echo "\\input{gkcapa}"
  echo "\\begin{document}"
  echo "\\gkcapa{Gabarito Árvore}{sobe a complexidade}{cotidiano → contexto → intenção → diálogo}"
  echo "\\maketitle"
  echo "\\begin{abstract}Exploração do ollama a partir de sementes banais (nível $NIVEL). Teoria continua de fora.\\end{abstract}"
} > "$TEX_OUT"

n=0
for s in "${sementes[@]}"; do
  echo "══ semente: $s (nível $NIVEL)"
  while IFS= read -r fala; do
    [ -z "$fala" ] && continue
    echo "── $fala"
    bruta=$(ollama_json "$SYS_EXTRA" "$fala" || true)
    resp=$(printf '%s' "$bruta" | limpa)
    if [ -z "$resp" ]; then echo "   (vazio)"; continue; fi
    echo "   [llama] $resp"
    aprende_par "$fala" "$resp"
    n=$((n+1))
    # ramifica: follow-ups do utilizador → novas respostas
    prof=1
    while [ "$prof" -le "$PROF" ]; do
      mapfile -t fus < <(followups_de "$s" "$resp" || true)
      [ "${#fus[@]}" -eq 0 ] && break
      for fu in "${fus[@]}"; do
        [ -z "$fu" ] && continue
        echo "   └─ $fu"
        bruta2=$(ollama_json "$SYS_EXTRA" "$fu" || true)
        resp2=$(printf '%s' "$bruta2" | limpa)
        [ -z "$resp2" ] && continue
        echo "      [llama] $resp2"
        aprende_par "$fu" "$resp2"
        n=$((n+1))
        resp="$resp2"
      done
      prof=$((prof+1))
    done
  done < <(perguntas_nivel "$s" "$NIVEL")
done

echo "\\end{document}" >> "$TEX_OUT"
echo "árvore: $n pares → $B + $TEX_OUT (nível $NIVEL)"
echo "smoke:"
for q in "falemos de café" "como encaixas sono na tua rotina" "ajuda-me a escolher algo ligado a música" "faz um mini plano de 3 passos sobre caminhada para hoje"; do
  echo "Q: $q"
  "$CV" "$B" responde "$q" 2>/dev/null | head -1 || echo "(não sei)"
done
