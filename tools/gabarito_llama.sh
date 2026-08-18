#!/bin/bash
# gabarito_llama.sh — enche o corpus com conversa BANAL (baixa complexidade).
#
# Teoria fica de fora: o modelo erra; a assistente primeiro aprende a falar
# educadamente de assuntos do dia a dia. Depois sobe a complexidade.
#
#   cd banco && ../tools/gabarito_llama.sh .fala/<hex16>
#   FORCA=1 MODELO=qwen2.5:1.5b ../tools/gabarito_llama.sh .fala/<hex>
#
# FORCA=1 (default): pergunta sempre ao ollama e sobrescreve o par no corpus.
# FORCA=0: só pergunta se o corpus disser «não sei».
#
set -euo pipefail
B="${1:?uso: ./gabarito_llama.sh <base>}"
D=$(cd "$(dirname "$0")" && pwd)
ROOT=$(cd "$D/.." && pwd)
CV="$ROOT/banco/bin/conversa"
MODELO="${MODELO:-qwen2.5:1.5b}"
FORCA="${FORCA:-1}"
TEX_OUT="$ROOT/corpus/fala/conversa_gabarito.tex"
SISTEMA="$ROOT/tools/.gabarito_sistema.txt"
mkdir -p "$B"
[ -x "$CV" ] || { echo "falta $CV — compila banco/conversa"; exit 1; }

# lista banal — baixa complexidade; sem teoria
perguntas=(
  "bom dia"
  "boa tarde"
  "boa noite"
  "oi"
  "olá"
  "oi tudo bem"
  "tudo bem"
  "como estás"
  "como vai"
  "e aí"
  "obrigado"
  "obrigada"
  "por favor"
  "desculpa"
  "com licença"
  "tchau"
  "até logo"
  "até amanhã"
  "bom fim de semana"
  "como foi o teu dia"
  "está a chover"
  "está sol hoje"
  "está frio"
  "está calor"
  "que tempo faz"
  "gostas de café"
  "preferes chá ou café"
  "o que comes ao pequeno almoço"
  "recomenda um almoço simples"
  "gostas de chocolate"
  "água com ou sem gás"
  "tens fome"
  "tens sede"
  "dormiste bem"
  "estou cansado"
  "estou feliz"
  "estou triste"
  "conta uma piada curta"
  "diz algo engraçado"
  "qual a tua cor preferida"
  "gostas de música"
  "que música ouves"
  "gostas de filmes"
  "recomenda um filme leve"
  "gostas de livros"
  "tens um animal de estimação"
  "gostas de cães"
  "gostas de gatos"
  "praticas desporto"
  "gostas de caminhar"
  "o que fazes no domingo"
  "o que gostas de fazer"
  "contas até dez"
  "qual é o teu nome"
  "quem és tu"
  "como te chamas"
  "podes ajudar"
  "ajuda"
  "não percebi"
  "repete por favor"
  "fala mais devagar"
  "está tudo bem"
  "cuida-te"
  "força"
  "parabéns"
  "feliz aniversário"
  "bom trabalho"
  "que horas são"
  "que dia é hoje"
  "é sexta-feira"
  "vamos conversar"
  "conta-me uma história curta"
  "diz um provérbio"
  "como se diz obrigado em inglês"
  "como se diz bom dia em inglês"
  "qual a capital do brasil"
  "quantos dias tem uma semana"
  "quantos meses tem um ano"
  "o que é a amizade"
  "o que é a educação"
  "porque é importante ser educado"
  "como se pede desculpa"
  "posso fazer uma pergunta"
  "tens um minuto"
  "estou aborrecido"
  "sugere um passatempo"
  "gostas de cozinhar"
  "receita fácil de ovo"
  "como vai o tempo aí"
  "boa viagem"
  "boas férias"
  "bem-vindo"
  "prazer em conhecer-te"
)

ollama_ask() {
  local fala="$1"
  python3 - "$MODELO" "$SISTEMA" "$fala" <<'PY'
import json, sys, urllib.request
modelo, sistema_path, fala = sys.argv[1], sys.argv[2], sys.argv[3]
sistema = open(sistema_path, encoding="utf-8").read().strip()
payload = {
  "model": modelo,
  "stream": False,
  "options": {"temperature": 0.55, "num_predict": 140},
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

{
  echo "% !TeX program = pdflatex"
  echo "% Gerado por tools/gabarito_llama.sh — conversa BANAL (sem teoria)"
  echo "% Modelo: $MODELO"
  echo "\\documentclass[11pt,a4paper]{article}"
  echo "\\usepackage[utf8]{inputenc}\\usepackage[T1]{fontenc}\\usepackage[brazil]{babel}"
  echo "\\usepackage[margin=2.4cm]{geometry}\\usepackage[hidelinks]{hyperref}"
  echo "\\newcommand{\\code}[1]{\\texttt{\\small #1}}"
  echo "\\input{gkcapa}"
  echo "\\begin{document}"
  echo "\\gkcapa{Gabarito Casual}{assuntos banais}{baixa complexidade; teoria depois}"
  echo "\\maketitle"
  echo "\\begin{abstract}Pares banais do \\texttt{$MODELO} no corpus. Sem teoria: a assistente aprende a falar primeiro.\\end{abstract}"
} > "$TEX_OUT"

n=0
ok=0
for fala in "${perguntas[@]}"; do
  echo "── $fala"
  ja=$("$CV" "$B" responde "$fala" 2>/dev/null | head -1 || true)
  if [ "$FORCA" != "1" ] && [ -n "$ja" ] && [ "$ja" != "não sei." ] && [ "$ja" != "nao sei" ]; then
    echo "   [corpus] $ja"
    resp=$(printf '%s' "$ja" | limpa)
  else
    echo "   [llama] ($MODELO)…"
    bruta=$(ollama_ask "$fala" || true)
    resp=$(printf '%s' "$bruta" | limpa)
    if [ -z "$resp" ]; then
      echo "   [llama] (vazio — salto)"
      continue
    fi
    echo "   [llama] $resp"
    "$CV" "$B" aprende "$fala" "$resp" >/dev/null
    echo "   [corpus] aprendido"
    ok=$((ok+1))
  fi
  python3 - "$fala" "$resp" >> "$TEX_OUT" <<'PY'
import sys
def esc(s):
  return (s.replace("\\","\\textbackslash{}")
           .replace("&","\\&").replace("%","\\%")
           .replace("$","\\$").replace("#","\\#")
           .replace("_","\\_").replace("{","\\{").replace("}","\\}"))
fala, resp = sys.argv[1], sys.argv[2]
print(f"\\section{{{esc(fala)}}}\n{esc(resp)}\n")
PY
  n=$((n+1))
done

echo "\\end{document}" >> "$TEX_OUT"
echo "gabarito banal: $n secções, $ok novos do llama → $B + $TEX_OUT"
echo "smoke:"
for q in "bom dia" "gostas de café" "estou cansado" "conta uma piada curta" "prazer em conhecer-te"; do
  echo "Q: $q"
  "$CV" "$B" responde "$q" | head -1
done
