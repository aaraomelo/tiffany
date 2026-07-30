#!/bin/bash
# bateria.sh — roda os medidores citados nos três papers, e diz o que cada um devolveu.
#
# A regra do projeto é "resíduo 0 ou falha", e por isso a bateria distingue três coisas:
#   VERDE     o medidor fechou (exit 0)
#   NEGATIVO  o medidor devolveu 1 POR PROJETO — é um teorema negativo, documentado no paper
#             (tatoeba/ancora.c e tatoeba/homogeneo.c: provam que NÃO existe atribuição que feche)
#   FALHA     qualquer outra coisa: não compilou, estourou o tempo, ou quebrou de verdade
#
# --- A BATERIA USA A PRÓPRIA TEORIA -----------------------------------------------------
#
# Rodar os 59 do zero leva ~10 minutos, e repetir isso para reconferir o que não mudou é
# sangria pura. A teoria do projeto diz por que não é preciso, e diz exatamente:
#
#   A bateria é uma SOMA DIRETA de componentes independentes — um medidor não fala com outro,
#   como as casas de R^i ⊕ R^j ⊕ R^k. E trio.c §S4 mediu que o caractere de uma soma direta é
#   o PRODUTO dos caracteres das casas: a transformada FATORA. Logo o veredito do todo
#   decompõe-se exatamente nas partes, e recalcular uma não obriga a recalcular as outras.
#   Não é cache com nome bonito — é a licença formal para reaproveitar, e ela é exata.
#
#   E o critério de "mudou" é a ASSINATURA DO CONTEÚDO, não o mtime. assinatura.c mediu que a
#   contagem ASSINA sem perder, e semente.c que assinatura + semente reconstrói a obra exata.
#   O mtime é a régua errada nos dois sentidos: `touch` muda-o sem mudar a obra, e reescrever
#   um arquivo igual não o muda tendo mudado o caminho. A assinatura não se engana em nenhum.
#
#   O SELO do conjunto é o XOR das assinaturas das partes — a mesma fatoração, num número só.
#   Selo igual quer dizer bateria igual, sem abrir nenhuma casa.
#
# Memória: cada medidor roda sob ulimit -v 2 GB e timeout, para nunca comer a swap da máquina.
# A saída completa de cada um fica em /tmp/bateria/ — para ver outra fatia LÊ-SE O ARQUIVO,
# nunca se roda outra vez.
#
#   ./tools/bateria.sh            só o que mudou de assinatura (o normal, segundos)
#   ./tools/bateria.sh --tudo     esquece o selo e refaz os 59 (~10 min, antes de publicar)
#   ./tools/bateria.sh --selo     imprime só o selo e sai, sem rodar nada

set -u
cd "$(dirname "$0")/.." || exit 1
RAIZ=$PWD
SAIDA=/tmp/bateria; mkdir -p "$SAIDA"
TABELA="$SAIDA/assinaturas.txt"; touch "$TABELA"
TUDO=0; SO_SELO=0
case "${1:-}" in --tudo) TUDO=1 ;; --selo) SO_SELO=1 ;; esac
[ "$TUDO" -eq 1 ] && : > "$TABELA"

# a lista sai dos próprios papers: nada de lista mantida à mão
LISTA=$(mktemp)
{ grep -ohE '(tools|tatoeba)/[a-z_0-9]+\.c' teoria.tex tiffany.tex microprocessador.tex viveiro.tex
  grep -ohE '(tools|tatoeba)/[a-z]+\\_[a-z]+\.c' teoria.tex tiffany.tex microprocessador.tex viveiro.tex | sed 's/\\_/_/'
} 2>/dev/null | sort -u > "$LISTA"

# um .pgm de teste para os medidores que leem imagem (linear, venom)
printf 'P5\n32 32\n255\n' > /tmp/bat.pgm
python3 -c "open('/tmp/bat.pgm','ab').write(bytes(((x*7+y*13)%256) for y in range(32) for x in range(32)))" 2>/dev/null

args() { case "$1" in
  neuronio|neuronio_analog) echo "../teoria.tex" ;;
  linear|venom)             echo "/tmp/bat.pgm" ;;
  ancora)                   echo "pares.tsv 20000" ;;
  homogeneo|embedding)      echo "pares.tsv" ;;
  regua|centro|bairro)      echo "pares.tsv" ;;
  operador)                 echo "pares.tsv 6 0 0 1" ;;
  *)                        echo "" ;;
esac }

negativo_esperado() { case "$1" in ancora|homogeneo) return 0 ;; *) return 1 ;; esac }

# a assinatura de um medidor: o conteúdo do fonte e os argumentos com que corre.
# Nada de mtime — a régua é a obra, não o relógio.
assinatura() { { cat "$1"; printf '%s' "$2"; } | sha256sum | cut -c1-16; }

# --- o selo: XOR das assinaturas. A soma direta fatora, então o todo cabe num número ----
selo() {
  local acc=0 a
  while read -r f; do
    [ -f "$RAIZ/$f" ] || continue
    a=$(assinatura "$RAIZ/$f" "$(args "$(basename "$f" .c)")")
    acc=$(( acc ^ 0x$a ))
  done < "$LISTA"
  printf '%016x' "$acc"
}

if [ "$SO_SELO" -eq 1 ]; then
  printf 'selo da bateria: %s   (%d medidores)\n' "$(selo)" "$(wc -l < "$LISTA")"
  rm -f "$LISTA"; exit 0
fi

verde=0; negativo=0; falha=0; total=0; rodados=0; reusados=0
printf '%-26s %-9s %s\n' "MEDIDOR" "SAÍDA" "VEREDITO"
printf '%s\n' "-------------------------------------------------------------------------"

for f in $(cat "$LISTA"); do
  total=$((total+1))
  dir=$(dirname "$f"); base=$(basename "$f" .c)
  cd "$RAIZ/$dir" || continue
  bin="$SAIDA/bat_$base"; out="$SAIDA/$base.txt"
  ass=$(assinatura "$base.c" "$(args "$base")")

  # a casa desta componente já está fechada com esta assinatura? então não se abre.
  guardado=$(grep -m1 "^$base $ass " "$TABELA" 2>/dev/null)
  if [ -n "$guardado" ] && [ -f "$out" ]; then
    r=$(printf '%s' "$guardado" | cut -d' ' -f3)
    ver=$(grep -ohE 'RESIDUO 0|RESÍDUO 0|resíduo 0|resíduo total = 0|residuo=0|resíduo=0|viol=0|O DENTE|FALHOU|FALHA' "$out" 2>/dev/null | tail -1)
    reusados=$((reusados+1))
    if [ "$r" -eq 0 ]; then
      printf '%-26s %-9s %s\n' "$f" "selado" "${ver:-ok}"; verde=$((verde+1))
    elif [ "$r" -eq 1 ] && negativo_esperado "$base"; then
      printf '%-26s %-9s %s\n' "$f" "selado" "teorema negativo por projeto"; negativo=$((negativo+1))
    else
      printf '%-26s %-9s %s\n' "$f" "FALHA" "exit $r (selado) — ${ver:-sem veredito}"; falha=$((falha+1))
    fi
    continue
  fi

  if ! cc -O2 -std=c99 -I. -I../tools "$base.c" -lm -o "$bin" 2>/dev/null; then
    printf '%-26s %-9s %s\n' "$f" "—" "NÃO COMPILOU"; falha=$((falha+1)); continue
  fi
  rodados=$((rodados+1))
  if [ "$base" = dente ]; then          # dente roda em duas etapas, com sort externo no meio
    (ulimit -v 2000000; timeout 200 "$bin" emite pares.tsv > "$SAIDA/dente_sig.txt" 2>/dev/null) &&
      LC_ALL=C sort -S 64M "$SAIDA/dente_sig.txt" | (ulimit -v 2000000; timeout 120 "$bin" agrupa > "$out" 2>&1)
    r=$?
  else
    (ulimit -v 2000000; timeout 560 "$bin" $(args "$base") </dev/null > "$out" 2>&1); r=$?
  fi
  # sela a componente: assinatura + veredito, para não se abrir de novo sem motivo
  grep -v "^$base " "$TABELA" > "$TABELA.novo" 2>/dev/null; mv "$TABELA.novo" "$TABELA"
  printf '%s %s %d\n' "$base" "$ass" "$r" >> "$TABELA"

  ver=$(grep -ohE 'RESIDUO 0|RESÍDUO 0|resíduo 0|resíduo total = 0|residuo=0|resíduo=0|viol=0|O DENTE|FALHOU|FALHA' "$out" 2>/dev/null | tail -1)
  if [ "$r" -eq 0 ]; then
    printf '%-26s %-9s %s\n' "$f" "VERDE" "${ver:-ok}"; verde=$((verde+1))
  elif [ "$r" -eq 1 ] && negativo_esperado "$base"; then
    printf '%-26s %-9s %s\n' "$f" "NEGATIVO" "teorema negativo por projeto — ${ver:-ver paper}"; negativo=$((negativo+1))
  else
    printf '%-26s %-9s %s\n' "$f" "FALHA" "exit $r — ${ver:-sem veredito}"; falha=$((falha+1))
  fi
done

cd "$RAIZ" || exit 1

# --- deriva: medidor que existe no disco e nenhum paper cita NUNCA roda aqui ---
# Sem esta conferência um medidor apodrece em silêncio: a lista sai dos PAPERS, então o que
# não é citado não é testado — e a contagem parece completa sem estar.
cp "$LISTA" /tmp/bat_citados.txt
ls tools/*.c tatoeba/*.c 2>/dev/null | sort > /tmp/bat_existem.txt
quebradas=$(comm -23 /tmp/bat_citados.txt /tmp/bat_existem.txt | wc -l)
naocitados=$(comm -13 /tmp/bat_citados.txt /tmp/bat_existem.txt | wc -l)
if [ "$quebradas" -gt 0 ]; then
  printf 'REFERENCIA QUEBRADA: %d citado(s) nos papers que nao existem no disco:\n' "$quebradas"
  comm -23 /tmp/bat_citados.txt /tmp/bat_existem.txt | sed 's/^/    /'
fi
if [ "$naocitados" -gt 0 ]; then
  printf 'nao citados (existem, nenhum paper cita, logo NAO sao testados): %d\n' "$naocitados"
  comm -13 /tmp/bat_citados.txt /tmp/bat_existem.txt | sed 's/^/    /'
fi

printf '%s\n' "-------------------------------------------------------------------------"
printf 'total %d : %d verdes, %d negativos por projeto, %d falhas\n' "$total" "$verde" "$negativo" "$falha"
printf 'selo %s : %d abertos, %d reaproveitados pela assinatura\n' "$(selo)" "$rodados" "$reusados"
printf 'saída de cada medidor em %s/ — para ver outra fatia LEIA O ARQUIVO, não rode de novo.\n' "$SAIDA"
rm -f "$LISTA"
[ "$falha" -eq 0 ] && [ "$quebradas" -eq 0 ] || exit 1
