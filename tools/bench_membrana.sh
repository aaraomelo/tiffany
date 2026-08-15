#!/bin/sh
# bench_membrana.sh — O DIALECTO É DO TRADUTOR, NÃO DA ASSISTENTE.
#
# Dois caminhos que têm de concordar: cada comando LaTeX que a assistente DESDOBRA na
# entrada (banco/conversa.c, tabela LX) tem de ser um comando que o tradutor COMPÕE
# (tests/tex_core.c — a tabela de símbolos e os operadores nomeados). O tradutor
# desenha e a assistente resolve; falam a MESMA língua ou não falam nenhuma.
#
# Foi assim que se apanharam o `\ast` e o `\pmod`: o LaTeX de fora tem-nos, esta casa
# não — e eu tinha-os escrito à mão em vez de ler o dialecto que já existia.
#
#   ./tools/bench_membrana.sh
#
set -u
D=$(cd "$(dirname "$0")" && pwd)
ROOT=$(cd "$D/.." && pwd)
CONV="$ROOT/banco/conversa.c"
TEX="$ROOT/tests/tex_core.c"
[ -f "$CONV" ] && [ -f "$TEX" ] || { echo "faltam as fontes"; exit 1; }

# os nomes que a assistente diz saber desdobrar
NOMES=$(sed -n '/tabela LX/,/^};/p' "$CONV" | grep -o '"[a-zA-Z]*"' | tr -d '"' | sort -u)
[ -n "$NOMES" ] || { echo "FAIL: a tabela LX não foi encontrada em conversa.c"; exit 1; }

ok=0; fail=0
for c in $NOMES; do
  # o tradutor conhece-o? na tabela de símbolos ("nome",0x..) ou nos operadores
  # nomeados ("nome") ou tratado por nome (!strcmp(cmd, "nome"))
  if grep -q "\"$c\"" "$TEX"; then
    echo "  PASS \\$c — o tradutor compõe-no"
    ok=$((ok+1))
  else
    echo "  FAIL \\$c — a assistente desdobra, o tradutor NÃO conhece (dialecto inventado)"
    fail=$((fail+1))
  fi
done

# ── E O OUTRO LADO DO PAR ────────────────────────────────────────────────────────
# A tabela LX é o que a assistente DESDOBRA (a entrada). Mas ela também ESCREVE LaTeX
# nas respostas, e essa metade não era medida: um `\ast` emitido passava, porque a
# varredura só olhava para a tabela. A membrana tem dois sentidos (Dual com sinal), e
# mede-se nos dois — o que ela lê e o que ela escreve.
echo
EMITIDOS=$(grep -o '\\\\[a-zA-Z]\+' "$CONV" | sed 's/^\\\\//' | sort -u)
for c in $EMITIDOS; do
  case "$c" in n|t|r|0|textbackslash) continue;; esac   # escapes do C, não do TeX
  if grep -q "\"$c\"" "$TEX"; then
    echo "  PASS \\$c — a assistente escreve-o e o tradutor compõe-no"
    ok=$((ok+1))
  else
    echo "  FAIL \\$c — a assistente ESCREVE-o e o tradutor NÃO o conhece (inventado na saída)"
    fail=$((fail+1))
  fi
done

echo
echo "PASS=$ok FAIL=$fail (o dialecto medido nos DOIS sentidos: o que lê e o que escreve)"
echo "o tradutor desenha, a assistente resolve — e a língua é uma só."
[ "$fail" -eq 0 ] || exit 1
