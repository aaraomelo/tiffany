#!/bin/sh
# triagem_limiares.sh — CADA LIMIAR TEM UMA DE TRÊS CAUSAS, e só uma é honesta.
#
# A auditoria dos tipos deu 350 ocorrências na classe MEDIÇÃO — cada uma é uma régua minha
# dentro de uma conta. Mas nem todas são defeito, e reduzi-las às cegas seria o `sed` que
# esta casa já provou errado. As três causas, e como se distinguem NA FONTE:
#
#   (A) DECORAÇÃO       a conta à volta não tem função transcendente nenhuma.
#                       Os valores são inteiros ou racionais, e o limiar não é preciso:
#                       compara-se por IGUALDADE e o resíduo é ZERO.
#
#   (B) SABOR           há transcendentes (sin, cos, exp, log, sqrt, M_PI) mas a
#                       IDENTIDADE é algébrica — vale para quaisquer entradas. Os
#                       transcendentes foram trazidos por sabor e forçam o limiar.
#                       Troca-se por entradas inteiras e fica exacta.
#
#   (C) HONESTO         a quantidade é genuinamente transcendente e a pergunta é sobre
#                       ELA. Aqui o double é a representação certa — mas a asserção tem
#                       de ser sobre a FORMA FECHADA, e não sobre o decimal.
#
# A triagem automática separa (A) do resto com segurança: basta ver se há transcendentes
# na vizinhança. Separar (B) de (C) exige ler a identidade, e este programa NÃO o finge —
# marca-os como «a ler» e diz quantos são.
#
#   ./tools/triagem_limiares.sh [ficheiro]
#
set -u
D=$(cd "$(dirname "$0")" && pwd)
R=$(cd "$D/.." && pwd)
cd "$R" || exit 1

echo "A TRIAGEM DOS LIMIARES — (A) decoração · (B/C) a ler"
echo
printf '%-28s %6s %8s %8s\n' "FICHEIRO" "limiar" "(A) dec" "(B/C)"
printf '%s\n' "------------------------------------------------------"

TRANS='sin|cos|tan|exp|log|sqrt|M_PI|pow|atan|cbrt|hypot'
ta=0; tb=0; tn=0; nf=0
for f in ${1:-lib/*.h tests/*.c tools/*.c banco/*.c}; do
  [ -f "$f" ] || continue
  n=$(grep -cE "1e-[0-9]+" "$f" 2>/dev/null)
  [ "$n" -gt 0 ] || continue
  # (A): a LINHA do limiar e as duas à volta não têm transcendente
  a=$(grep -nE "1e-[0-9]+" "$f" | cut -d: -f1 | while read -r l; do
        s=$((l-2)); [ "$s" -lt 1 ] && s=1
        sed -n "${s},$((l+2))p" "$f" | grep -qE "$TRANS" || echo x
      done | wc -l)
  b=$((n - a))
  printf '%-28s %6d %8d %8d\n' "$f" "$n" "$a" "$b"
  ta=$((ta+a)); tb=$((tb+b)); tn=$((tn+n)); nf=$((nf+1))
done

echo
printf '%-28s %6d %8d %8d\n' "TOTAL ($nf ficheiros)" "$tn" "$ta" "$tb"
echo
echo "(A) DECORAÇÃO — sem transcendente à volta: o limiar não é preciso, e a comparação"
echo "    pode ser por IGUALDADE. Estes são defeito, e o conserto é mecânico e seguro."
echo
echo "(B/C) A LER — há transcendente por perto. Ou ele é SABOR (a identidade é algébrica"
echo "      e vale para quaisquer entradas: troca-se por inteiros e fica exacta) ou é"
echo "      HONESTO (a quantidade É transcendente, e então a asserção tem de ser sobre a"
echo "      FORMA FECHADA). Esta separação exige ler, e não se finge."
