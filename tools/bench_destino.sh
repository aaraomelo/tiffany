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
# O `esc_qz` NÃO se conta, e por construção: ele imprime dentro de si próprio, portanto
# consome a fatia antes de devolver. É o padrão seguro, e quem escrever muitas frações
# numa linha deve usá-lo em vez de contar chamadas à mão.
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

# ── E A COLUNA QUE CONTA BYTES ────────────────────────────────────────────────────
# `printf("%-9s", "bisseção")` enche até NOVE BYTES, e «bisseção» tem 8 caracteres em 10
# bytes: a coluna sai curta e a tabela sai torta. Aconteceu duas vezes na mesma sessão —
# no medidor e outra vez na fala, logo a seguir a eu o ter corrigido. A cura é `esc_col`,
# que conta as cabeças de UTF-8; o que aqui se mede é que ninguém volta ao `%-Ns`.
#
# E A REGRA NÃO SE ESCREVE À MÃO: para cada `%-Ns`, tiram-se os ACESSORES chamados na
# linha (`nome(`) e vai-se ver se a definição deles devolve texto com acentos. Assim o
# `%-20s` sobre os nomes ASCII da tabela das provas NÃO é acusado — um medidor que grita
# sem defeito é um medidor que se passa a ignorar.
echo
FONTES="$CONV $ROOT/lib/identifica.h $ROOT/lib/reais.h $ROOT/lib/racionais.h"
acentuado(){   # 1 se a definição da função $1 devolve literais com bytes ≥ 0x80
  grep -h -A4 "\\*$1(" $FONTES 2>/dev/null | grep -q -P '[\x80-\xFF]'
}
for ln in $(grep -n '%-[0-9]*s' "$CONV" | cut -d: -f1); do
  linha=$(sed -n "${ln}p" "$CONV")
  risco=""
  for acc in $(printf '%s\n' "$linha" | grep -o '[a-z_][a-z_0-9]*(' | tr -d '(' | sort -u); do
    case "$acc" in printf|snprintf|sizeof|strlen|if|for|while) continue;; esac
    if acentuado "$acc"; then risco="$acc"; fi
  done
  if [ -n "$risco" ]; then
    echo "  FAIL conversa.c:$ln — %-Ns enche por BYTES e $risco() devolve texto acentuado:"
    echo "       a coluna sai torta. Usar esc_col(), que conta caracteres"
    fail=$((fail+1))
  else
    ok=$((ok+1))
  fi
done

echo
echo "PASS=$ok FAIL=$fail (chamadas rotativas por printf, contra o tamanho que a fonte declara)"
echo "o valor pode estar certo e o texto errado — e é o texto que o Aarão lê."
[ "$fail" -eq 0 ] || exit 1
