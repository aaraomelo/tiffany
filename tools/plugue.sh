#!/bin/bash
# plugue.sh — O PLUGUE DE DENTRO, EM BASH: os verbos da máquina, um por linha de comando.
#
# O Aarão: "você tem os plugues do lado de dentro com bash e assembly."
#
# O `erg.c` dá o assembly; este dá o BASH. É a mesma máquina — não há segunda implementação
# aqui, e isso é deliberado: todo verbo abaixo chama o `erg`, e se o `erg` mudar este muda com
# ele. Um plugue que reimplementasse a ISA seria um terceiro caminho para divergir em silêncio.
#
# E O PLUGUE É INVERSÍVEL, que era a condição do Aarão: "ler e escrever é a mesma operação
# dual". Aqui isso é literal —
#
#     ve   slot          ler       o banco → o piloto
#     poe  slot t e      escrever  o piloto → o banco
#
# — e o `mede` confirma que a volta fecha: escrever e depois ler devolve o que se escreveu, em
# todos os slots testados. Não é uma promessa; é um verbo que se corre.
#
#   ./plugue.sh liga                 compila o erg e prepara o banco
#   ./plugue.sh ve 3                 lê o slot 3          (o lado de LER)
#   ./plugue.sh poe 3 7 8            escreve {7,8}        (o lado de ESCREVER)
#   ./plugue.sh corre soma           monta e corre banco/apps/soma.erg
#   ./plugue.sh olha                 os slots todos que não são zero
#   ./plugue.sh mede                 mede que ler∘escrever = id
#   ./plugue.sh gato 13 8            o gato ida e volta, pelo app
set -u
CD="$(cd "$(dirname "$0")" && pwd)"
ERG=${ERG:-/tmp/erg}
MEM=${MEM:-/tmp/plugue_mem.dat}
NSLOTS=${NSLOTS:-64}

garante(){
  [ -x "$ERG" ] && [ "$ERG" -nt "$CD/erg.c" ] && return 0
  cc -O2 -std=c99 -Wall "$CD/erg.c" -o "$ERG" 2>/dev/null || { echo "  o erg.c não compilou"; exit 1; }
}

case "${1:-ajuda}" in
liga)
  garante
  "$ERG" zera "$MEM" "$NSLOTS"
  echo "  o plugue está ligado: $NSLOTS slots em $MEM, 16 bytes cada ($((NSLOTS*16)) bytes)"
  echo "  a máquina: $ERG — a ISA do sql.c, sem RAM"
  ;;

ve)   garante; "$ERG" ve "$MEM" "${2:?diga o slot}" ;;
poe)  garante; "$ERG" poe "$MEM" "${2:?slot}" "${3:?total}" "${4:-0}"
      printf '  slot %s ← (%s, %s)\n' "$2" "$3" "${4:-0}" ;;

olha)
  garante
  printf '  %6s   %12s %12s\n' slot total e
  i=0
  while [ $i -lt $NSLOTS ]; do
    v=$("$ERG" ve "$MEM" $i)
    [ "$v" != "0 0" ] && printf '  %6d   %12s %12s\n' $i ${v% *} ${v#* }
    i=$((i+1))
  done
  ;;

corre)
  garante
  app=${2:?diga o app (sem .erg)}
  f="$CD/apps/$app.erg"; [ -f "$f" ] || f="$app"
  [ -f "$f" ] || { echo "  não há $app"; exit 1; }
  "$ERG" monta "$f" /tmp/plugue_app.bin || exit 1
  "$ERG" corre /tmp/plugue_app.bin "$MEM" "${3:-100000}"
  ;;

gato)
  # o gato ida e volta, pelo app — e a volta é INTEIRA porque det = −1
  garante
  t=${2:-13}; e=${3:-8}
  "$ERG" zera "$MEM" "$NSLOTS"
  "$ERG" poe "$MEM" 1 "$t" "$e"
  "$ERG" monta "$CD/apps/gato.erg" /tmp/plugue_app.bin >/dev/null
  "$ERG" corre /tmp/plugue_app.bin "$MEM" 1000 >/dev/null
  printf '  (%s, %s)  --GOLD-->  (%s)  --NEGRO_OURO-->  (%s)\n' \
         "$t" "$e" "$("$ERG" ve "$MEM" 2)" "$("$ERG" ve "$MEM" 3)"
  [ "$("$ERG" ve "$MEM" 3)" = "$t $e" ] && echo "  a volta fechou, exata" \
                                        || echo "  A VOLTA NÃO FECHOU"
  ;;

mede)
  # LER E ESCREVER SÃO ADJUNTOS, e isto mede-o: escrever e ler devolve o mesmo, em N slots.
  garante
  "$ERG" zera "$MEM" "$NSLOTS"
  falhas=0; n=0
  for s in 1 2 3 7 15 31 63; do
    for par in "0 0" "1 0" "-5 9" "1000000 -1000000" "7 7"; do
      set -- $par
      "$ERG" poe "$MEM" $s "$1" "$2"
      [ "$("$ERG" ve "$MEM" $s)" = "$1 $2" ] || falhas=$((falhas+1))
      n=$((n+1))
    done
  done
  printf '  ler∘escrever em %d pares sobre 7 slots: %d falhas\n' "$n" "$falhas"
  if [ "$falhas" -eq 0 ]; then
    echo "  RESIDUO 0 — o plugue é inversível, e é por isso que ele é túnica e não leitor."
  else
    echo "  FALHOU"; exit 1
  fi
  ;;

*)
  sed -n '2,30p' "$0" | sed 's/^# \?//'
  ;;
esac
