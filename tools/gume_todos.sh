#!/usr/bin/env bash
# gume_todos.sh — O GUME AUTOMÁTICO SOBRE A BATERIA INTEIRA.
#
# O `gume.py` responde por um medidor. Isto responde pela casa: corre-o em todos, e o
# relatório é uma lista só — as asserções que sobreviveram a TODAS as mutações que lhes
# foram atiradas.
#
# NÃO É UM VEREDICTO. Uma unidade sobrevivente pode estar perfeitamente certa: se
# nenhuma mutação toca no que ela mede, ela não tinha como cair. O que a lista diz é
# ONDE OLHAR, e a pergunta a fazer em cada linha é a de sempre — que entrada faria isto
# falhar?
#
# O tecto de mutações por ficheiro é DITO no relatório, e não escondido: com amostragem
# a lista fica mais longa do que a verdade, porque uma mutação que mataria a asserção
# pode simplesmente não ter sido corrida. Um tecto calado leria-se como cobertura, que é
# a lição da saturação.
#
#   uso:  bash tools/gume_todos.sh [max_mutacoes] [paralelo]

set -u
RAIZ="$(cd "$(dirname "$0")/.." && pwd)"
MAX="${1:-25}"
PAR="${2:-6}"
SAIDA="${GUME_SAIDA:-$RAIZ/tools/gume_relatorio.txt}"
TMPD="$(mktemp -d)"
trap 'rm -rf "$TMPD"' EXIT

cd "$RAIZ"
lista=$(ls tests/*.c | sed 's|tests/||; s|\.c$||' | sort)
n=$(echo "$lista" | wc -l)
echo "  a correr o gume em $n medidores, até $MAX mutações cada, $PAR em paralelo…"

i=0
for b in $lista; do
  (
    # o gume.py limpa o seu proprio directorio no `finally`; o `rm` que aqui estava lia
    # uma variavel que so existia no ambiente do comando, e com `set -u` matava o ramo
    d="$(mktemp -d)"
    GUME_TMP="$d" timeout 900 python3 tools/gume.py "tests/$b.c" --max "$MAX" \
      > "$TMPD/$b.txt" 2>&1
    rm -rf "$d" 2>/dev/null
  ) &
  i=$((i+1))
  if [ $((i % PAR)) -eq 0 ]; then wait; printf '.'; fi
done
wait; echo

{
  echo "GUME AUTOMÁTICO — as asserções que sobreviveram a todas as mutações"
  echo "tecto: $MAX mutações por medidor (amostragem uniforme; o que ficou de fora é dito"
  echo "em cada linha do gume.py, e com tecto a lista fica MAIOR que a verdade)"
  echo
  tot_v=0; tot_s=0; nfich=0
  for f in "$TMPD"/*.txt; do
    b=$(basename "$f" .txt)
    v=$(grep -oE '^  tests/[^:]+: [0-9]+ unidades' "$f" 2>/dev/null | grep -oE '[0-9]+ unidades' | grep -oE '^[0-9]+')
    s=$(grep -oE 'SOBREVIVERAM a todas  : [0-9]+' "$f" 2>/dev/null | grep -oE '[0-9]+$')
    [ -z "${v:-}" ] && continue
    nfich=$((nfich+1)); tot_v=$((tot_v + v)); tot_s=$((tot_s + ${s:-0}))
    if [ "${s:-0}" -gt 0 ]; then
      echo "── $b: $s de $v sobreviveram"
      sed -n '/SOBREVIVERAM a todas/,$p' "$f" | grep '^        ·' | sed 's/^      /  /'
    fi
  done
  echo
  echo "TOTAL: $nfich medidores lidos, $tot_v unidades verdes, $tot_s sobreviventes"
  echo "  (medidores que não compilaram ou não correram não entram — e por isso o número"
  echo "   de medidores lidos deve bater com o da bateria; se não bater, faltou algum)"
} | tee "$SAIDA"
echo "  relatório em $SAIDA"
