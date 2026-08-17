#!/usr/bin/env bash
# cobertura.sh — quanto de cada medidor a bateria CORRE de facto.
#
# Uma asserção só vê o que é executado. O crash do shell do tex.c — SIGSEGV em todas as
# invocações do comando ABRE — viveu no repo sem que nenhum medidor o pudesse ver, porque
# nenhum percorria aquele caminho. Este script diz onde é que isso pode voltar a acontecer.
#
# E TEM UMA ARMADILHA, que está aqui escrita porque já caí nela: correr os medidores SEM
# os argumentos que a bateria lhes dá faz quatro deles parecerem ter 4% de cobertura,
# quando têm 100%. O número seria da consulta, não do repo. Por isso a tabela `args()`
# abaixo é a MESMA da bateria, e se ela mudar lá tem de mudar aqui.
#
#     bash tools/cobertura.sh          o resumo
#     bash tools/cobertura.sh -v       com a lista dos piores
#     bash tools/cobertura.sh <nome>   as linhas não executadas de um medidor
set -u
RAIZ="$(cd "$(dirname "$0")/.." && pwd)"
COV="${TMPDIR:-/tmp}/cobertura.$$"
mkdir -p "$COV"
trap 'rm -rf "$COV"' EXIT

# a MESMA tabela da bateria — ver tools/bateria.sh, função args()
args() { case "$1" in
  neuronio|neuronio_analog)                echo "../teoria.tex" ;;
  banco|sql)                               echo "teste" ;;
  fala)                                    echo "-teste" ;;
  linear|venom)                            echo "/tmp/bat.pgm" ;;
  ancora)                                  echo "pares.tsv 20000" ;;
  homogeneo|embedding|regua|centro|bairro) echo "pares.tsv" ;;
  operador)                                echo "pares.tsv 6 0 0 1" ;;
  *)                                       echo "" ;;
esac }

# o .pgm que os medidores de imagem esperam, igual ao da bateria
printf 'P5\n32 32\n255\n' > /tmp/bat.pgm
python3 -c "open('/tmp/bat.pgm','ab').write(bytes(((x*7+y*13)%256) for y in range(32) for x in range(32)))" 2>/dev/null

UM="${1:-}"
echo "a compilar com --coverage..."
n=0
for f in "$RAIZ"/tests/*.c; do
    b=$(basename "$f" .c)
    [ -n "$UM" ] && [ "$UM" != "-v" ] && [ "$b" != "$UM" ] && continue
    cc -O0 --coverage -I "$RAIZ/lib" -I "$RAIZ/tests" -o "$COV/$b" "$f" -lm 2>/dev/null && n=$((n+1))
done
echo "  $n compilados; a correr..."
cd "$RAIZ/tests" || exit 1
for b in $(ls "$COV" 2>/dev/null | grep -v '\.'); do
    # shellcheck disable=SC2046
    timeout 120 "$COV/$b" $(args "$b") > /dev/null 2>&1 < /dev/null
done

cd "$COV" || exit 1
: > cob.txt
for g in *.gcda; do
    b=${g%.gcda}
    gcov -n "$g" 2>/dev/null | grep -A1 "File '.*$b\.c'" | grep "Lines executed" | head -1 \
        | sed "s/Lines executed://; s/% of /|/" | awk -v n="$b" -F'|' '{print $1"|"$2"|"n}' >> cob.txt
done

if [ -n "$UM" ] && [ "$UM" != "-v" ]; then
    gcov "$UM.gcda" > /dev/null 2>&1
    echo; echo "linhas NÃO executadas de $UM.c:"
    grep -n '#####' "$UM.c.gcov" 2>/dev/null | head -40 || echo "  (nenhuma)"
    exit 0
fi

echo
echo "COBERTURA DE LINHAS — quanto a bateria corre de cada medidor"
echo
awk -F'|' '{if($1==100)a++; else if($1>=95)b++; else if($1>=90)c++; else if($1>=80)d++; else e++}
  END{printf "   a 100%%: %d\n   95-99%%: %d\n   90-94%%: %d\n   80-89%%: %d\n   abaixo de 80%%: %d\n",a,b,c,d,e}' cob.txt
tot=$(awk -F'|' '{s+=$2} END{print s}' cob.txt)
exe=$(awk -F'|' '{s+=$1*$2/100} END{printf "%d",s}' cob.txt)
echo
echo "   TOTAL: $exe de $tot linhas = $(awk -v a="$exe" -v b="$tot" 'BEGIN{printf "%.2f", 100*a/b}')%"
[ "$UM" = "-v" ] && { echo; echo "   os dez com menor cobertura:";
  sort -t'|' -k1 -n cob.txt | head -10 | awk -F'|' '{printf "   %6.2f%%  de %5d linhas   %s\n", $1, $2, $3}'; }
echo
echo "Uma cobertura baixa não é por si um defeito: pode ser um modo de linha de comando que"
echo "a bateria não corre (o nomeia tem cinco, e são a ferramenta — o autoteste é o medidor)."
echo "É uma pergunta: o que vive ali, e quem o mede?"
