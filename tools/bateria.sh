#!/bin/bash
# bateria.sh — roda TODOS os medidores citados nos três papers, e diz o que cada um devolveu.
#
# A regra do projeto é "resíduo 0 ou falha", e por isso a bateria tem de distinguir três coisas:
#   VERDE     o medidor fechou (exit 0)
#   NEGATIVO  o medidor devolveu 1 POR PROJETO — é um teorema negativo, documentado no paper
#             (tatoeba/ancora.c e tatoeba/homogeneo.c: provam que NÃO existe atribuição que feche)
#   FALHA     qualquer outra coisa: não compilou, estourou o tempo, ou quebrou de verdade
#
# Alguns medidores exigem argumento e devolvem 1 em silêncio sem ele (neuronio pede um caminho,
# linear pede um .pgm) — os argumentos vão abaixo, para que a bateria não acuse falha onde é uso.
#
# Memória: cada medidor roda sob ulimit -v 2 GB e timeout, para nunca comer a swap da máquina.
#
#   ./tools/bateria.sh            (da raiz do projeto)

set -u
cd "$(dirname "$0")/.." || exit 1
RAIZ=$PWD

# a lista sai dos próprios papers: nada de lista mantida à mão
LISTA=$(mktemp)
{ grep -ohE '(tools|tatoeba)/[a-z_0-9]+\.c' teoria.tex tiffany.tex microprocessador.tex
  grep -ohE '(tools|tatoeba)/[a-z]+\\_[a-z]+\.c' teoria.tex tiffany.tex microprocessador.tex | sed 's/\\_/_/'
} | sort -u > "$LISTA"

# um .pgm de teste para o linear
printf 'P5\n32 32\n255\n' > /tmp/bat.pgm
python3 -c "open('/tmp/bat.pgm','ab').write(bytes(((x*7+y*13)%256) for y in range(32) for x in range(32)))" 2>/dev/null

args() { case "$1" in
  neuronio|neuronio_analog) echo "../teoria.tex" ;;
  linear)                   echo "/tmp/bat.pgm" ;;
  ancora)                   echo "pares.tsv 20000" ;;
  homogeneo|embedding)      echo "pares.tsv" ;;
  operador)                 echo "pares.tsv 6 0 0 1" ;;
  *)                        echo "" ;;
esac }

# os que devolvem 1 por projeto (teoremas negativos, e o paper diz o número)
negativo_esperado() { case "$1" in ancora|homogeneo) return 0 ;; *) return 1 ;; esac }

verde=0; negativo=0; falha=0; total=0
printf '%-26s %-8s %s\n' "MEDIDOR" "SAÍDA" "VEREDITO"
printf '%s\n' "-------------------------------------------------------------------------"

for f in $(cat "$LISTA"); do
  total=$((total+1))
  dir=$(dirname "$f"); base=$(basename "$f" .c)
  cd "$RAIZ/$dir" || continue
  if ! cc -O2 -std=c99 -I. -I../tools "$base.c" -lm -o "/tmp/bat_$base" 2>/dev/null; then
    printf '%-26s %-8s %s\n' "$f" "—" "NÃO COMPILOU"; falha=$((falha+1)); continue
  fi
  if [ "$base" = dente ]; then          # dente roda em duas etapas, com sort externo no meio
    (ulimit -v 2000000; timeout 200 "/tmp/bat_dente" emite pares.tsv > /tmp/bat_sig.txt 2>/dev/null) &&
      LC_ALL=C sort -S 64M /tmp/bat_sig.txt | (ulimit -v 2000000; timeout 120 "/tmp/bat_dente" agrupa > "/tmp/bat_out_$base.txt" 2>&1)
    r=$?
  else
    (ulimit -v 2000000; timeout 560 "/tmp/bat_$base" $(args "$base") </dev/null > "/tmp/bat_out_$base.txt" 2>&1); r=$?
  fi
  ver=$(grep -ohE 'RESIDUO 0|RESÍDUO 0|resíduo 0|resíduo total = 0|residuo=0|resíduo=0|viol=0|O DENTE|FALHOU|FALHA' \
        "/tmp/bat_out_$base.txt" 2>/dev/null | tail -1)
  if [ "$r" -eq 0 ]; then
    printf '%-26s %-8s %s\n' "$f" "VERDE" "${ver:-ok}"; verde=$((verde+1))
  elif [ "$r" -eq 1 ] && negativo_esperado "$base"; then
    printf '%-26s %-8s %s\n' "$f" "NEGATIVO" "teorema negativo por projeto — ${ver:-ver paper}"; negativo=$((negativo+1))
  else
    printf '%-26s %-8s %s\n' "$f" "FALHA" "exit $r — ${ver:-sem veredito}"; falha=$((falha+1))
  fi
done

printf '%s\n' "-------------------------------------------------------------------------"
printf 'total %d : %d verdes, %d negativos por projeto, %d falhas\n' "$total" "$verde" "$negativo" "$falha"
rm -f "$LISTA"
[ "$falha" -eq 0 ] || exit 1
