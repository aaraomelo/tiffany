#!/bin/sh
# bench_destino.sh — O DESTINO ROTATIVO TEM TAMANHO, E A FONTE TEM DE O RESPEITAR.
#
# `frac2` e `fc_da_borda` devolvem um ponteiro para um buffer estático que RODA. Escrever
# mais chamadas do que fatias num único `printf` faz a última sobrescrever a primeira: a
# linha sai com o número errado, e NENHUMA asserção o apanha — as asserções leem os
# valores, não o texto impresso. Aconteceu duas vezes:
#
#   ∫₀³x² : «F(3) − F(0) = 0 − 3 = 3» com sete chamadas e quatro fatias
#   a sucessão de Cauchy : «1, 4/3, 7/5, 24/17, 1» — o quinto termo era 41/29
#
# Não há medida do lado do valor, porque o valor está certo: o defeito é do TEXTO. Então
# a medida é do lado da FONTE — contar as chamadas por `printf` e recusar acima do
# tamanho da rotação, que a própria fonte declara.
#
#   ./tools/bench_destino.sh
#
set -u
D=$(cd "$(dirname "$0")" && pwd)
ROOT=$(cd "$D/.." && pwd)
CONV="$ROOT/banco/conversa.c"
[ -f "$CONV" ] || { echo "falta $CONV"; exit 1; }

# o tamanho não se escreve aqui: LÊ-SE da fonte, senão era um número meu
N=$(sed -n 's/^#define FRAC2_N \([0-9]*\).*/\1/p' "$CONV" | head -1)
[ -n "$N" ] || { echo "FAIL: FRAC2_N não está declarado em conversa.c"; exit 1; }
NB=$(sed -n 's/.*static char buf\[FRAC2_N\]\[\([0-9]*\)\].*/\1/p' "$CONV" | head -1)
[ -n "$NB" ] || { echo "FAIL: o buffer de frac2 não usa FRAC2_N — a rotação e o teto divergiram"; exit 1; }
echo "  a fonte declara FRAC2_N = $N fatias de $NB bytes"

ok=0; fail=0
# junta cada `printf(` até ao `);` da mesma instrução e conta as chamadas rotativas
awk -v N="$N" '
  /printf *\(/ { junta = 1; linha = NR; acc = "" }
  junta { acc = acc $0
          if (/\); *$/ || /\);[^"]*$/) {
            n = gsub(/frac2 *\(/, "&", acc) + gsub(/fc_da_borda *\(/, "&", acc)
            if (n > N) printf "FAIL %d %d\n", linha, n
            else if (n > 0) printf "OK %d %d\n", linha, n
            junta = 0
          }
        }
' "$CONV" > /tmp/bench_destino.$$

while read -r v ln n; do
  if [ "$v" = FAIL ]; then
    echo "  FAIL conversa.c:$ln — $n chamadas rotativas num só printf (a rotação é $N):"
    echo "       a última sobrescreve a primeira e a linha imprime o número ERRADO"
    fail=$((fail+1))
  else
    ok=$((ok+1))
  fi
done < /tmp/bench_destino.$$
rm -f /tmp/bench_destino.$$

echo
echo "PASS=$ok FAIL=$fail (chamadas rotativas por printf, contra o tamanho que a fonte declara)"
echo "o valor pode estar certo e o texto errado — e é o texto que o Aarão lê."
[ "$fail" -eq 0 ] || exit 1
