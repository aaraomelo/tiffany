#!/bin/sh
# audita_tipos.sh — A MATRIZ DA MIGRAÇÃO: classificar ANTES de reduzir.
#
# O eval, sobre a migração binária: «não deveria simplesmente substituir todos os
# `double`, `int` e `uint8_t` mecanicamente. Primeiro precisamos classificar cada
# ocorrência:
#
#       ESTRUTURA | REALIZAÇÃO | MEDIÇÃO | APRESENTAÇÃO
#
# Só então reduzir.»
#
# E é a mesma regra que esta casa já aprendeu com os `if`: não se elimina por estética,
# elimina-se quando a condição é absorvida por algo com nome. Um tipo largo num printf é
# APRESENTAÇÃO e não custa nada; um tipo largo dentro de uma comparação com tolerância é
# MEDIÇÃO e é um defeito; um tipo largo num campo de struct é ESTRUTURA e decide tudo o
# resto.
#
# As quatro classes, e como se reconhecem na fonte:
#
#   ESTRUTURA     em typedef/struct/campo — decide a representação de todos os outros
#   REALIZAÇÃO    em variável local de cálculo — é onde a conta acontece
#   MEDIÇÃO       junto de uma tolerância (1e-, fabs, <, >) — é onde a régua entra
#   APRESENTAÇÃO  em printf/fprintf/sprintf — não entra em conta nenhuma
#
#   ./tools/audita_tipos.sh [double|long|int]
#
set -u
D=$(cd "$(dirname "$0")" && pwd)
R=$(cd "$D/.." && pwd)
cd "$R" || exit 1
T=${1:-double}

echo "A MATRIZ DA MIGRAÇÃO — o tipo '$T', classificado antes de reduzir"
echo
printf '%-26s %6s %6s %6s %6s %6s\n' "FICHEIRO" "total" "estrt" "reali" "medic" "apres"
printf '%s\n' "-------------------------------------------------------------------------"

# E CONTA-SE O CÓDIGO, E NÃO O TEXTO — o mesmo defeito que a `triagem_limiares.sh`
# tinha. Depois de fechar o `matricial.c` em ZERO doubles, esta auditoria continuava a
# contar-lhe 26: eram os comentários que EXPLICAM os que saíram. Uma métrica que conta a
# palavra em vez do tipo mede o texto, e quem escreve mais sobre a migração parece migrar
# menos. O filtro é o mesmo scanner de um percurso: apaga comentários e literais de
# texto, preservando as quebras de linha, para que os números de linha não se movam.
DESPIR=$(mktemp -d)
trap 'rm -rf "$DESPIR"' EXIT INT TERM
despe() {
  python3 -c '
import sys
s = open(sys.argv[1], encoding="utf-8", errors="replace").read()
out, i, n = [], 0, len(s)
while i < n:
    c = s[i]
    if c == "/" and i+1 < n and s[i+1] == "*":
        j = s.find("*/", i+2); j = n if j < 0 else j+2
    elif c == "/" and i+1 < n and s[i+1] == "/":
        j = s.find(chr(10), i); j = n if j < 0 else j
    elif c in "\"\x27":
        j, q = i+1, c
        while j < n and s[j] != q:
            j += 2 if s[j] == "\\" else 1
        j = min(j+1, n)
    else:
        out.append(c); i += 1; continue
    out.append("".join(ch if ch == chr(10) else " " for ch in s[i:j]))
    i = j
sys.stdout.write("".join(out))
' "$1"
}

tot=0; est=0; rea=0; med=0; apr=0; nf=0
for f0 in lib/*.h tests/*.c tools/*.c banco/*.c; do
  [ -f "$f0" ] || continue
  f="$DESPIR/$(echo "$f0" | tr '/' '_')"
  despe "$f0" > "$f" 2>/dev/null || continue
  n=$(grep -c "\b$T\b" "$f" 2>/dev/null)
  [ "$n" -gt 0 ] || continue
  # ESTRUTURA: aparece em typedef, struct ou declaração de campo
  e=$(grep "\b$T\b" "$f" | grep -cE "typedef|struct|\}[[:space:]]*[A-Za-z_]+;")
  # APRESENTAÇÃO: dentro de um printf/fprintf/sprintf, ou num %f/%g
  a=$(grep "\b$T\b" "$f" | grep -cE "printf|%[0-9.]*[fge]")
  # MEDIÇÃO: junto de uma tolerância ou de fabs.
  # E as fronteiras de palavra NÃO são decoração: sem elas o padrão apanhava `eps_r`
  # (a permissividade relativa) e `EPS0` (a do vácuo), que são CONSTANTES FÍSICAS e não
  # réguas minhas. Eram 25 dos 347, e 21 num único ficheiro — o `colheita.c` aparecia no
  # topo da lista de migração por causa de física que nunca foi limiar. Medir a migração
  # com esta régua é escolher o alvo errado.
  m=$(grep "\b$T\b" "$f" | grep -cE "1e-|fabs|\bEPS\b|\beps\b|\btol|TOLER")
  # REALIZAÇÃO: o resto
  r=$((n - e - a - m)); [ "$r" -lt 0 ] && r=0
  printf '%-26s %6d %6d %6d %6d %6d\n' "$f0" "$n" "$e" "$r" "$m" "$a"
  tot=$((tot+n)); est=$((est+e)); rea=$((rea+r)); med=$((med+m)); apr=$((apr+a)); nf=$((nf+1))
done

echo
printf '%-26s %6d %6d %6d %6d %6d\n' "TOTAL ($nf ficheiros)" "$tot" "$est" "$rea" "$med" "$apr"
echo
echo "A LEITURA:"
echo "  ESTRUTURA    decide a representação de tudo o resto — migrar aqui é migrar o sistema"
echo "  REALIZAÇÃO   é onde a conta acontece — migrável, e o par de 32 segura os intermédios"
echo "  MEDIÇÃO      é onde a RÉGUA entra — cada um destes é um limiar meu dentro da conta"
echo "  APRESENTAÇÃO não entra em conta nenhuma — migrar aqui não compra nada"
echo
echo "E a ordem certa é: MEDIÇÃO primeiro (é defeito), depois ESTRUTURA (é alavanca),"
echo "depois REALIZAÇÃO. A APRESENTAÇÃO fica — reduzir um printf é estética."
