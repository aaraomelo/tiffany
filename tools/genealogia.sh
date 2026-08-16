#!/bin/sh
# genealogia.sh — CADA CONSTANTE APRESENTA A SUA GENEALOGIA OU SAI FORA.
#
# A regra vem do Corpo Universal e o eval pô-la em palavras: «cada constante tem de
# apresentar a sua genealogia no Universal ou sair fora». Um literal decimal na fonte é
# uma afirmação sem proveniência — e esta casa já sabe o que isso custa: a referência
# escrita à mão é o defeito que ela persegue há mais tempo.
#
# As genealogias que EXISTEM neste quadro, e o que cada uma obriga:
#
#   LIMITE      π — o membro PARABÓLICO da família metálica (thm:pi-familia). Não é uma
#               constante operacional: é a FRONTEIRA. π_n é exacto por andar; π_∞ não
#               fecha, por teorema. Um `#define M_PI` é aceitável como recurso do
#               compilador, mas usá-lo numa AFIRMAÇÃO é pedir o limite.
#
#   RECORRÊNCIA φ, σ_m, √2, √3 — algébricos, com passo inteiro por trás. O valor
#               DERIVA-SE da recorrência; escrevê-lo é copiar o que se pode gerar.
#
#   DEFINIÇÃO   c = 299792458 m/s é EXACTO por definição do SI (o metro define-se a
#               partir dele). É um inteiro disfarçado de decimal.
#
#   EXPERIMENTO 2.725 K, 376.73 Ω — medidas do mundo. É genealogia honesta, mas tem de
#               ser DECLARADA: um número medido não é um número derivado.
#
#   NENHUMA     o resto. Ou apresenta proveniência, ou sai.
#
#   ./tools/genealogia.sh
#
set -u
D=$(cd "$(dirname "$0")" && pwd)
R=$(cd "$D/.." && pwd)
cd "$R" || exit 1

echo "A GENEALOGIA DAS CONSTANTES — cada uma apresenta a sua, ou sai fora"
echo
printf '%-26s %6s  %s\n' "o literal" "quantos" "a genealogia"
printf '%s\n' "--------------------------------------------------------------------"

grep -ohE "\b[0-9]+\.[0-9]{3,}\b" lib/*.h tests/*.c tools/*.c banco/*.c 2>/dev/null \
| sort | uniq -c | sort -rn | while read -r n v; do
  case "$v" in
    3.14159*)      g="LIMITE — o parabólico. Fronteira, não constante" ;;
    1.61803*)      g="RECORRÊNCIA — φ: p_{k+1}=p_k+p_{k-1}. DERIVA-SE" ;;
    1.41421*)      g="RECORRÊNCIA — √2, e 11 em F17" ;;
    3.236*)        g="RECORRÊNCIA — 2φ: deriva-se de φ" ;;
    0.742742944625) g="RECORRÊNCIA — φ^(1−φ): deriva-se. Ver aurea.c" ;;
    1.73205*)      g="RECORRÊNCIA — √3, e 9 em F13" ;;
    2.71828*)      g="LIMITE — e, transcendente. Fronteira" ;;
    299.792458)    g="DEFINIÇÃO — c, exacto no SI: inteiro disfarçado" ;;
    376.730313668) g="DEFINIÇÃO — Z0 = mu0*c, derivada de c" ;;
    2.725)         g="EXPERIMENTO — a temperatura do fundo. Declarar" ;;
    0.00*|0.0[0-9]*) g="tolerância ou escala — ver a triagem dos limiares" ;;
    *)             g="?  sem genealogia declarada" ;;
  esac
  printf '%-26s %6s  %s\n' "$v" "$n" "$g"
done

echo
echo "A REGRA: um literal decimal é uma afirmação SEM PROVENIÊNCIA. As genealogias que"
echo "este quadro reconhece são quatro — LIMITE, RECORRÊNCIA, DEFINIÇÃO e EXPERIMENTO —"
echo "e as três primeiras obrigam: o valor DERIVA-SE em vez de se escrever. O EXPERIMENTO"
echo "é honesto, mas tem de ser declarado: um número medido não é um número derivado."
echo
echo "E o caso que não tem desculpa: um literal que seja o RESULTADO de uma conta que o"
echo "programa podia fazer. Esse é a referência escrita à mão, e ela reintroduz o defeito"
echo "dentro da própria correcção."
