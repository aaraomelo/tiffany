#!/bin/sh
# pergunta.sh — PERGUNTAR AO REPOSITÓRIO ANTES DE PERGUNTAR AO AARÃO.
#
# O Aarão: «lê o repositório, tira as tuas dúvidas e segue — podes até automatizar essa
# resposta para toda vez que parares.»
#
# E ele tem razão porque isto já me custou caro, e sempre da mesma forma: o que já está
# medido e eu não sei, escrevo pior. A reversão pdf→estrela→latex estava escrita no
# `tests/estrela_emite.c` §E3 — eu tinha-a escrito — e mesmo assim construí o
# `tools/compara.js` contra o pdflatex, que é exactamente o que o `corpo-estelar.tex`
# proíbe na última linha da especificação.
#
# A ferramenta responde em quatro lados, e os quatro importam:
#
#   1. A ESPECIFICAÇÃO  o que os papers dizem — é a fonte, e ganha às outras
#   2. OS DOCUMENTOS    teoria, catálogo, enredo
#   3. OS MEDIDORES     o que já é medido, com nome de ficheiro
#   4. O QUE FALTA      \medido sem medidor nomeado, que é afirmação sem prova
#
#   ./pergunta.sh <tema> [<tema2> ...]
#
# Sem argumento, faz a pergunta que interessa quando não se sabe o que perguntar: mostra o
# fecho da especificação — a tabela «a camada / o que a define / o que ela obriga».
cd "$(dirname "$0")/.." || exit 1

if [ $# -eq 0 ]; then
  echo
  echo "  ═══ A ESPECIFICAÇÃO, NUMA PÁGINA ═══════════════════════════════════════════"
  echo
  sed -n '/A especificação, numa página/,/bottomrule/p' papers/corpo-estelar.tex \
    | grep ' & ' | sed 's/\\\\$//; s/\\textbf{\([^}]*\)}/\1/g; s/\\code{\([^}]*\)}/\1/g' \
    | sed 's/\\emph{\([^}]*\)}/\1/g; s/[\\$]//g; s/^/    /'
  echo
  echo "  Uma dúvida concreta:  ./tools/pergunta.sh <tema>"
  echo
  exit 0
fi

for T in "$@"; do
  echo
  echo "════════════════════════════════════════════════════════════  $T"

  # ─── 1. a especificação: os papers ganham, porque são a fonte ──────────────────────
  echo
  echo "  ── A ESPECIFICAÇÃO (papers/) ────────────────────────────────────────────"
  N=$(grep -rin -- "$T" papers/*.tex 2>/dev/null | wc -l)
  if [ "$N" -eq 0 ]; then
    echo "     nada — e se o tema é de projecto, isso já é uma resposta:"
    echo "     o que a especificação não diz, não se inventa a partir dela."
  else
    grep -rin -- "$T" papers/*.tex 2>/dev/null | head -6 \
      | sed 's/\\textbf{\([^}]*\)}/\1/g; s/\\emph{\([^}]*\)}/\1/g; s/\\code{\([^}]*\)}/\1/g' \
      | cut -c1-118 | sed 's/^/     /'
    [ "$N" -gt 6 ] && echo "     ... e mais $((N - 6)) linhas"
  fi

  # ─── 2. os três documentos ─────────────────────────────────────────────────────────
  echo
  echo "  ── OS DOCUMENTOS ────────────────────────────────────────────────────────"
  for D in teoria.tex catalogo.tex enredo.tex; do
    C=$(grep -ci -- "$T" "$D" 2>/dev/null)
    [ "$C" -gt 0 ] && printf "     %-14s %s ocorrências\n" "$D" "$C"
  done | sort -k2 -rn
  grep -in -- "$T" teoria.tex catalogo.tex 2>/dev/null | grep -i 'textbf\|section' | head -3 \
    | sed 's/\\textbf{\([^}]*\)}/\1/g; s/\\label{[^}]*}//g' | cut -c1-118 | sed 's/^/     /'

  # ─── 3. os medidores: o que JÁ é medido ────────────────────────────────────────────
  echo
  echo "  ── JÁ É MEDIDO POR ──────────────────────────────────────────────────────"
  # POR RELEVÂNCIA, não por ordem alfabética: um ficheiro que menciona o tema uma vez
  # aparecia antes do que o mede, e a lista dizia o contrário do que devia.
  M=$(grep -rcin -- "$T" tests/*.c tests/*.js tools/*.c tools/*.js 2>/dev/null \
      | awk -F: '$2>0' | sort -t: -k2 -rn | head -8 | cut -d: -f1)
  if [ -z "$M" ]; then
    echo "     nenhum medidor menciona isto"
  else
    for F in $M; do
      C=$(grep -cin -- "$T" "$F")
      # o cabeçalho é a linha que tem o nome do ficheiro e o travessão — nem todos abrem
      # com ele, e `sed -n 1p` devolvia `#include <stdio.h>`, que não diz o que se mede
      H=$(grep -m1 -- "$(basename "$F") —" "$F" 2>/dev/null \
          || grep -m1 -- "$(basename "$F") --" "$F" 2>/dev/null)
      [ -z "$H" ] && H=$(grep -m1 -E '^ \* [A-ZÀ-Ú]' "$F" 2>/dev/null)
      printf "     %-26s %2sx  %s\n" "$(basename "$F")" "$C" \
        "$(echo "$H" | sed 's|^/\* *||; s|^ \* *||; s|^// *||' | cut -c1-64)"
    done
  fi

  # ─── 4. o que falta: afirmação sem prova ───────────────────────────────────────────
  echo
  echo "  ── E O QUE ESTÁ AFIRMADO SEM MEDIDOR ────────────────────────────────────"
  # um \medido que não nomeia programa é número sem quem o produza. Contam-se os que
  # falam do tema: é aí que uma dúvida costuma ter resposta escrita e não provada.
  ORF=$(awk -v t="$T" 'BEGIN{IGNORECASE=1}
        /\\medido\{/{b=""; d=1}
        d{b=b" "$0; if(/\}/ && b ~ /\\medido/){ if(b ~ t && b !~ /tests\/|tools\//) n++; d=0 }}
        END{print n+0}' teoria.tex catalogo.tex 2>/dev/null)
  if [ "$ORF" -gt 0 ]; then
    echo "     $ORF blocos \\medido falam disto e NÃO nomeiam programa"
    echo "     (número afirmado sem quem o produza — a bateria não os vê)"
  else
    echo "     nenhum: o que se afirma sobre isto nomeia quem o mediu"
  fi
done

echo
echo "  ─────────────────────────────────────────────────────────────────────────────"
echo "  A ordem não é de conveniência: a especificação ganha aos documentos, e o que"
echo "  já é medido ganha ao que eu ia escrever. O que ela não responde, mede-se —"
echo "  e medir aqui é reverter e ler o resíduo, não comparar contra um valor posto."
echo
