#!/bin/bash
# assistente.sh — A ASSISTENTE LIGADA AO FORWARD: corpus primeiro, llama depois, e aprende.
#
# O Aarão: "pode ligar, e também cabe realizar o llama aí, porque agora ele roda num ambiente
# reversível via o corpo da cifra."
#
# As duas peças já existiam e nunca se tinham tocado:
#
#   conversa.c   o corpus — cifra a fala, desce a árvore em pread, e a resposta mora na folha.
#                Nada em RAM. E tem o DECRETO: quando nada alcança a fala, diz "não sei" em vez
#                de inventar — é o único dos três métodos sem dual, e é ele que a torna honesta.
#   forward.c    o qwen inteiro, do disco, em CPU. Sem ollama, sem servidor, sem GPU.
#
# O encaixe sai do decreto e não de uma regra minha: o corpus responde o que sabe, e é o "não
# sei" que passa a palavra ao llama. Depois a resposta VOLTA para o corpus — a assistente
# aprende do que o modelo disse, e da segunda vez responde sem o acordar.
#
#   ./assistente.sh <base> "a fala"        pergunta
#   ./assistente.sh <base> conversa        modo interativo
set -u
CD="$(cd "$(dirname "$0")" && pwd)"
BASE=${1:-$CD/../.torre/assistente}
CONVERSA=${CONVERSA:-$CD/conversa}
FORWARD=${FORWARD:-$CD/forward}
N=${N:-24}

[ -x "$CONVERSA" ] || { echo "assistente: falta o corpus — cc -O2 -std=c99 -I$CD $CD/conversa.c -lm -o $CONVERSA" >&2; exit 1; }
[ -x "$FORWARD" ]  || { echo "assistente: falta o forward — cc -O2 -std=c99 -I$CD $CD/forward.c -lm -o $FORWARD" >&2; exit 1; }
mkdir -p "$(dirname "$BASE")"

responde(){
  local fala="$1"
  local r
  r=$("$CONVERSA" "$BASE" responde "$fala" 2>/dev/null | head -1)
  if [ -n "$r" ] && [ "$r" != "não sei." ]; then
    echo "[corpus] $r"
    return 0
  fi
  # O DECRETO passou a palavra. Agora o llama — do disco, em CPU, sem servidor nenhum.
  echo "[corpus] não sei — a perguntar ao llama (do disco, CPU)" >&2
  local g
  g=$("$FORWARD" "$fala" "$N" 2>/dev/null | sed -n 's/^      gerado   "\(.*\)"$/\1/p')
  if [ -z "$g" ]; then echo "[llama] (não respondeu)"; return 1; fi
  echo "[llama] $g"
  # e VOLTA para o corpus: da próxima vez o modelo não é preciso
  "$CONVERSA" "$BASE" aprende "$fala" "$g" >/dev/null 2>&1
  echo "[corpus] aprendido — da próxima respondo sem acordar o modelo" >&2
}

if [ "${2:-}" = "conversa" ] || [ "${1:-}" = "conversa" ]; then
  echo "assistente: corpus em $BASE, llama em $FORWARD (disco+CPU). Ctrl-D para sair."
  while IFS= read -r -p "> " linha; do
    [ -z "$linha" ] && continue
    responde "$linha"
  done
else
  shift 2>/dev/null || true
  fala="${*:-}"
  [ -z "$fala" ] && { echo "uso: $0 <base> \"a fala\"   |   $0 <base> conversa" >&2; exit 2; }
  responde "$fala"
fi
