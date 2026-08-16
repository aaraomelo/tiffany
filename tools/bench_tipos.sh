#!/bin/sh
# bench_tipos.sh — CABE NO TIPO? A pergunta do Teorema do Gato feita ao próprio código.
#
# O Gato tem uma cláusula que é sobre a máquina e não sobre a matemática:
#
#   «falha de representação NÃO é contra-exemplo matemático»
#
# e a regra que dela sai é operacional — sempre que uma realização atinge o seu limite,
# verifica-se noutra. Mas para saber que se atingiu o limite é preciso MEDIR o limite, e
# esta casa nunca o tinha feito ao seu tipo de base.
#
# `Qz` é `{long p, q}`. A ordem compara p·q' com p'·q, e a soma faz a.q·b.q — logo o que
# estoura primeiro é o PRODUTO CRUZADO, não o racional. Com -DQZ_MEDE o `racionais.h`
# regista a maior magnitude e o maior produto que passaram, e imprime-os no rodapé.
#
# O que este medidor decide:
#
#   · quanto é que cada medidor REALMENTE usa do tipo;
#   · quais caberiam em `int` (2,147e9) e quais não;
#   · e se algum passou do que o `long` aguenta — que é o único caso vermelho, porque aí
#     houve enrolamento SILENCIOSO e o resultado não vale nada.
#
# E há uma leitura que só aparece aqui: um número grande costuma ser sintoma de uma prova
# feita por CRESCIMENTO — bissectar 40 vezes, iterar Newton até saturar, varrer andares —
# em vez de pelo PASSO. Foi assim que se achou o `fn_bissec` a correr 40 passos com o
# denominador a duplicar: pedia-se profundidade 40 e a aritmética morria no passo 32, com
# os últimos nove a correr sobre inteiros enrolados e a função a devolver 1 na mesma.
#
#   ./tools/bench_tipos.sh
#
set -u
D=$(cd "$(dirname "$0")" && pwd)
ROOT=$(cd "$D/.." && pwd)
cd "$ROOT" || exit 1
TMP=$(mktemp -d)
trap 'rm -rf "$TMP"' EXIT

INT_MAX=2147483647
LONG_SEGURO=9000000000000000000

echo "CABE NO TIPO? — o Teorema do Gato aplicado ao código"
echo
printf '%-30s %14s %14s  %s\n' "MEDIDOR" "maior |p|,|q|" "maior p·q'" "cabe em"
printf '%s\n' "---------------------------------------------------------------------------"

usam=0; so_int=0; precisa_long=0; enrolou=0
for f in tests/*.c banco/conversa.c; do
  b=$(basename "$f" .c)
  cc -O2 -std=c99 -DQZ_MEDE -Ilib -I. -Itests -Itools -o "$TMP/$b" "$f" -lm 2>/dev/null || continue
  ( cd "$(dirname "$f")" && timeout 300 "$TMP/$b" >/dev/null 2>"$TMP/e" )
  linha=$(grep '^#QZMAX' "$TMP/e" | tail -1) || true
  [ -n "$linha" ] || continue                    # nao usa Qz: nada a decidir
  mx=$(echo "$linha" | awk '{print $2}')
  pr=$(echo "$linha" | awk '{print $3}')
  usam=$((usam+1))
  if [ "$pr" -ge "$LONG_SEGURO" ]; then
    cabe="ENROLOU — nem long"; enrolou=$((enrolou+1))
  elif [ "$mx" -le "$INT_MAX" ] && [ "$pr" -le "$INT_MAX" ]; then
    cabe="int"; so_int=$((so_int+1))
  else
    cabe="long"; precisa_long=$((precisa_long+1))
  fi
  printf '%-30s %14s %14s  %s\n' "$b" "$mx" "$pr" "$cabe"
done

echo
echo "usam Qz: $usam    cabem em int: $so_int    precisam de long: $precisa_long"
echo "enrolaram (produto acima do que o long aguenta): $enrolou"
echo
echo "A leitura: um numero grande e' quase sempre sintoma de prova por CRESCIMENTO —"
echo "bissectar fundo, iterar ate' saturar, varrer andares — em vez de prova do PASSO."
echo "Trocar long por int nao conserta isso: torna-o RUIDOSO mais cedo, que ja' e' muito."
[ "$enrolou" -eq 0 ] || exit 1
