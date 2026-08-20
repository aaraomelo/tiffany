#!/bin/sh
# corpus.sh — O CORPUS INTEIRO, numa linha.
#
# O Aarão: "para isto não ficar solto precisa ir para o corpus. Já temos a formalização
# completa em teoria.tex — é ela. Ingere o corpus."
#
# São três fontes, e nenhuma é escrita à mão de propósito para a assistente:
#
#   ciencia.sh   o que se sabe, com a régua dita em cada entrada
#   semear.sh    como se opera este sistema — só o que a bateria mede
#   teoria.tex   a formalização, ingerida pela ESTRUTURA que o documento já declara
#                (uma \section, uma \subsection, um \item = uma fala)
#
# A terceira é o ponto: o corpus não se inventa, já existe. E por isso ele não pode
# divergir da teoria — se a teoria muda, o corpus muda com ela, porque é ela.
#
#   ./corpus.sh /caminho/da/base
set -e
B="${1:?uso: ./corpus.sh <base>}"
D=$(dirname "$0")
[ -x "$D/conversa" ] || cc -O2 -std=c99 -I"$D" "$D/conversa.c" -o "$D/conversa" -lm
"$D/ciencia.sh" "$B" >/dev/null
"$D/semear.sh"  "$B" >/dev/null
CV="$D/../banco/bin/conversa"
[ -x "$CV" ] || CV="$D/conversa"
# TeX: índice vivo (só @TEX), NÃO copiar corpos
export TIFFANY_ROOT="$(cd "$D/.." && pwd)"
python3 "$D/indexa_tex_vivo.py" | "$CV" "$B" - >/dev/null
# o cristal recuperado — se for TeX de conceitos, preferir vivo; senão cristal.sh
"$D/cristal.sh" "$B" >/dev/null
# casual: cumprimentos (não TeX)
"$D/casual.sh" "$B" >/dev/null
echo "corpus: $("$CV" "$B" conversa </dev/null 2>&1 | head -1)"
