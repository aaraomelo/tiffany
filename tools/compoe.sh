#!/bin/sh
# compoe.sh — A RESPOSTA COMPOSTA PELO TRADUTOR. O PDF é o backend.
#
# O `tex.c` já dizia o desenho: «o shell ABRE um .tex, e o STORE escreve no backend —
# os backends do banco (martelo, canal, pool) são destinos de LOAD/STORE que o banco não
# precisa de conhecer; o PDF é mais um». Aqui fecha-se a cadeia da assistente com esse
# desenho, sem lugar novo:
#
#   fala -> assistente resolve -> a resposta escreve-se na MEMBRANA (.tex)
#        -> o TRADUTOR compõe (o backend) -> e a VOLTA lê o PDF de volta
#
# As três roupas são a mesma coisa: a prosa e o ASCII são o que a assistente imprime, o
# LaTeX é a membrana, e o PDF é o backend a desenhá-la. Uma resposta, três leituras.
#
#   cd banco && ../tools/compoe.sh .fala/<hex> "a fala"  [saida.pdf]
#
set -u
FALA_B="${1:?uso: ./compoe.sh <base> \"a fala\" [saida.pdf]}"
FALA="${2:?falta a fala}"
D=$(cd "$(dirname "$0")" && pwd)
ROOT=$(cd "$D/.." && pwd)
CV="$ROOT/banco/bin/conversa"
TEX="$ROOT/tests/tex"
SAIDA="${3:-/tmp/resposta.pdf}"
FONTE="${SAIDA%.pdf}.tex"
VOLTA="${SAIDA%.pdf}.volta.tex"

[ -x "$CV" ]  || { echo "compoe: falta $CV";  exit 1; }
[ -x "$TEX" ] || { echo "compoe: falta o tradutor $TEX (cd tests && cc -O2 -std=c99 -I../lib tex.c -lm -o tex)"; exit 1; }

# 1. a assistente resolve — a prosa e o ASCII saem daqui
RESP=$("$CV" "$FALA_B" responde "$FALA" 2>/dev/null)
[ -n "$RESP" ] || { echo "compoe: a assistente não respondeu"; exit 1; }

# 2. a resposta veste-se de membrana. O `_`, o `&` e o `#` são roupa do compositor e
#    escapam-se; o resto vai como veio — a assistente já fala o dialecto.
esc(){ sed -e 's/\\/\\textbackslash /g' -e 's/_/\\_/g' -e 's/&/\\&/g' -e 's/#/\\#/g' \
           -e 's/%/\\%/g' -e 's/\$/\\$/g'; }
{
  echo '\documentclass[11pt,a4paper]{article}'
  echo '\usepackage[utf8]{inputenc}\usepackage[T1]{fontenc}\usepackage[portuguese]{babel}'
  echo '\usepackage[margin=2.5cm]{geometry}'
  echo '\newcommand{\code}[1]{\texttt{\small #1}}'
  echo '\begin{document}'
  printf '\\section{%s}\n' "$(printf '%s' "$FALA" | esc)"
  echo '\begin{verbatim}'
  printf '%s\n' "$RESP"
  echo '\end{verbatim}'
  echo '\end{document}'
} > "$FONTE"

# 3. O TRADUTOR COMPÕE — é ele o backend, não o pdflatex
"$TEX" "$FONTE" "$SAIDA" >/dev/null 2>&1 || { echo "compoe: o tradutor recusou $FONTE"; exit 1; }
[ -s "$SAIDA" ] || { echo "compoe: o PDF saiu vazio"; exit 1; }

# 4. A VOLTA: o mesmo tradutor lê o PDF de volta, e mede-se o que atravessou.
#    Um objecto que só emite é o buraco branco — o próprio tex.c o diz.
"$TEX" -volta "$SAIDA" "$VOLTA" >/dev/null 2>&1
ok=0; fail=0
[ -s "$VOLTA" ] && { echo "  PASS a volta existe: o PDF lê-se de volta em $VOLTA"; ok=$((ok+1)); } \
                || { echo "  FAIL a volta não produziu texto"; fail=$((fail+1)); }

# O QUE A RESPOSTA AFIRMA TEM DE ATRAVESSAR — e a chave é o VALOR, não um dígito
# qualquer. A primeira versão pegava no primeiro número que via («0», «2») e um dígito
# solto atravessa sempre: era asserção a passar sem poder falhar. A chave é o que a
# linha «dá …» declara; sem ela, o número mais longo da resposta.
CHAVE=$(printf '%s\n' "$RESP" | sed -n 's/^dá \([-0-9][-0-9/]*\).*/\1/p' | head -1)
[ -n "$CHAVE" ] || CHAVE=$(printf '%s' "$RESP" | grep -oE '[0-9]+(/[0-9]+)?' \
                            | awk '{ print length($0), $0 }' | sort -rn | head -1 | cut -d' ' -f2)
if [ -n "$CHAVE" ]; then
  if grep -qF "$CHAVE" "$VOLTA" 2>/dev/null; then
    echo "  PASS o valor «$CHAVE» atravessou o backend e voltou"; ok=$((ok+1))
  else
    echo "  FAIL o valor «$CHAVE» não voltou do PDF"; fail=$((fail+1))
  fi
fi

echo
printf '%s\n' "$RESP"
echo
echo "fonte:  $FONTE"
echo "backend: $SAIDA  ($(wc -c < "$SAIDA") bytes)"
echo "volta:  $VOLTA"
echo "PASS=$ok FAIL=$fail (a resposta composta pelo tradutor, e a volta medida)"
[ "$fail" -eq 0 ] || exit 1
