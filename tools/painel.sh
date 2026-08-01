#!/bin/bash
# painel.sh — O PAINEL DO PILOTO: o estado numa tela, e o corpo que se fecha sozinho.
#
# O Aarão: "faz o painel também, paralelo ao manual. O painel conecta nos hooks e as operações
# são via contrato." E logo a seguir, a simplificação: "não há necessidade de contrato — fecha
# quando o corpo completa."
#
# PARALELO AO MANUAL, e a palavra é exata: o PILOTO.md diz o que as coisas são, o painel diz em
# que estado elas estão AGORA. Um não substitui o outro, e nenhum dos dois inventa — o painel
# lê, não decide.
#
# E O CONTRATO NÃO SE ASSINA: LIQUIDA-SE. O Aarão corrigiu duas vezes — primeiro "não há
# necessidade de contrato, fecha quando o corpo completa", e depois "pronto, smart contracts: é um
# contrato inteligente, chama agentes". O que morre é a ASSINATURA, não o contrato. O painel não
# pede quatro cláusulas assinadas; pede QUATRO NÚMEROS, e o resto deriva-se e EXECUTA-SE:
#
#     o piloto dá        alguns termos — o lado branco da torre
#     sai a RÉGUA        (B, C), por Cramer, exata em inteiros
#     sai o lado NEGRO   ν(a,b) = (a + B·b, −b), forçado
#     FECHOU             quando a reversão volta com resíduo 0
#     e CHAMA O AGENTE   que o Δ determina — gira, estica ou o limite
#
# Um corpo não promete fechar: ou fecha, ou os termos não eram de um corpo. Não há o que assinar,
# e é por isso que este painel tem um verbo a menos do que tinha.
#
# E CONECTA NOS HOOKS: a secção 4 do `session-start.sh` injeta o índice cifrado da memória. O
# painel mostra o mesmo índice e o estado do hook, para o piloto ver o que a assistente vê.
#
# Sem rede, sem ollama, sem RAM: tudo sai de ficheiros.
#
#   ./painel.sh                    o estado inteiro
#   ./painel.sh fecha 0 1 1 2 3 5  dê os termos, e o corpo diz-se
#   ./painel.sh op SOMA 3 5        uma operação solta sobre o banco do painel
#   ./painel.sh hook               o que o hook de entrada injeta
#   ./painel.sh apps               os apps do piloto, montados e pesados
#   ./painel.sh bateria            o resíduo da bateria, se houver corrida recente
set -u
CD="$(cd "$(dirname "$0")" && pwd)"
RAIZ="$(cd "$CD/.." && pwd)"
BANCO=${BANCO:-/tmp/painel_banco.dat}
Q=${Q:-12}
HOOK="$HOME/.claude/hooks/session-start.sh"

az(){ printf '\033[1m%s\033[0m\n' "$*"; }
linha(){ printf '  %-30s %s\n' "$1" "$2"; }

# ---------------------------------------------------------------- as operações, derivadas
# Não são declaradas: saem da régua, que sai dos termos. O painel não guarda uma lista.
clausula(){
  local nome=$1 a=$2 b=$3
  case "$nome" in
    SOMA)     echo $(( (a + b) % Q )) ;;                      # ⊕ Clifford — Kirchhoff
    PRODUTO)  echo $(( (a * b) % Q )) ;;                      # ⊗ La Hire — o ganho
    OPERADOR) python3 -c "print(pow($a or 1, $b or 1, $Q))" ;;# ∏ Pontryagin — exp∘Σ∘log
    DUAL)     echo $(( ((-a) % Q + Q) % Q )) ;;               # ν — o espelho, ordem 2
    *)        echo "ERRO" ;;
  esac
}

modo=${1:-estado}

case "$modo" in
fecha)
  # O VERBO PRINCIPAL DO PAINEL. O piloto dá os termos; o corpo diz-se inteiro ou recusa.
  shift
  [ -x /tmp/fecha ] || cc -O2 -std=c99 "$CD/fecha.c" -lm -o /tmp/fecha 2>/dev/null
  if [ $# -lt 4 ]; then
    az "FECHAR UM CORPO — dê pelo menos 4 termos (n+2 para grau 2)"
    echo "    ./painel.sh fecha 0 1 1 2 3 5      → ouro,  Δ = 5,  hiperbólico"
    echo "    ./painel.sh fecha 1 0 -1 0 1 0     → i,     Δ = −4, elíptico"
    echo "    ./painel.sh fecha 0 1 2 5 12 29    → prata"
    echo
    echo "  Não se declara nada: régua, borda, soma, produto, dual e Δ saem dos termos,"
    echo "  e a reversão verifica-se sozinha. Cabe ao piloto apenas decidir que termos dar."
    exit 0
  fi
  /tmp/fecha "$@"
  ;;

polar|cartesiana|forma)
  # AS DUAS FORMAS, e o painel usa as duas porque o piloto usa as duas.
  [ -x /tmp/polar ] || cc -O2 -std=c99 "$CD/polar.c" -lm -o /tmp/polar 2>/dev/null
  shift
  if [ $# -lt 4 ]; then
    az "AS DUAS FORMAS — dê a régua (B C) e o ponto (a b)"
    echo "    ./painel.sh polar  1 -1  3 2      o ouro, no ponto 3 + 2σ"
    echo "    ./painel.sh polar  0  1  1 1      o i, no ponto 1 + i"
    echo
    echo "  ALGÉBRICA  z = a + b·σ      soma bem   — é o produto DIRETO (mede)"
    echo "  POLAR      z = ρ·E(θ)       multiplica bem — é o CRUZADO (ordena)"
    echo "  e o espelho troca só o cruzado: a peça que mede é a mesma dos dois lados."
    exit 0
  fi
  /tmp/polar "$@"
  ;;

op)
  nome=${2:?diga a operação: SOMA PRODUTO OPERADOR DUAL}
  a=${3:-0}; b=${4:-0}
  r=$(clausula "$nome" "$a" "$b")
  if [ "$r" = "ERRO" ]; then
    echo "  '$nome' não é uma das operações. São: SOMA PRODUTO OPERADOR DUAL —"
    echo "  e as quatro NÃO são declaradas: saem da régua. Para as ver derivadas de termos,"
    echo "  use  ./painel.sh fecha <termos>."
    exit 1
  fi
  echo "$r" >> "$BANCO"
  printf '  %s(%s, %s) = %s   sobre Z_%s   · o banco tem agora %s registos\n' \
         "$nome" "$a" "$b" "$r" "$Q" "$(wc -l < "$BANCO" 2>/dev/null || echo 0)"
  ;;

hook)
  az "O HOOK DE ENTRADA"
  if [ -f "$HOOK" ]; then
    linha "ficheiro" "$HOOK"
    linha "secções" "$(grep -c '^  echo "## ' "$HOOK" 2>/dev/null || echo '?')"
    if grep -q memoria_banco.sh "$HOOK" 2>/dev/null; then
      linha "a túnica" "LIGADA (secção 4 injeta o índice cifrado)"
    else
      linha "a túnica" "não ligada"
    fi
    if bash -n "$HOOK" 2>/dev/null; then linha "sintaxe" "ok"; else linha "sintaxe" "QUEBRADA"; fi
  else
    linha "ficheiro" "não existe — o painel corre na mesma"
  fi
  echo
  az "O QUE ELE INJETA (a memória cifrada)"
  if [ -s /tmp/memoria_banco.txt ]; then
    printf '  %s memórias, índice de %s bytes\n' \
           "$(wc -l < /tmp/memoria_banco.txt)" "$(wc -c < /tmp/memoria_banco.txt)"
    cut -f1,3 /tmp/memoria_banco.txt 2>/dev/null | head -6 | \
      awk -F'\t' '{printf "    %-38s %s\n", $1, $2}'
  else
    echo "  (nenhum índice em /tmp — corre  tools/memoria_banco.sh ingere)"
  fi
  ;;

apps)
  az "OS APPS DO PILOTO"
  if [ ! -d "$CD/apps" ]; then echo "  (ainda não há tools/apps/)"; exit 0; fi
  [ -x /tmp/erg ] || cc -O2 -std=c99 "$CD/erg.c" -o /tmp/erg 2>/dev/null
  printf '  %-24s %8s %8s   %s\n' "app" "linhas" "bytes" "monta?"
  for a in "$CD"/apps/*.erg; do
    [ -f "$a" ] || continue
    saida=$(/tmp/erg monta "$a" /tmp/painel_app.bin 2>&1)
    if echo "$saida" | grep -q bytes; then
      b=$(echo "$saida" | awk '{print $1}'); est="sim"
    else b="—"; est="NÃO: $saida"; fi
    printf '  %-24s %8s %8s   %s\n' "$(basename "$a")" "$(grep -cv '^\s*\(;.*\)\?$' "$a")" "$b" "$est"
  done
  ;;

bateria)
  az "A BATERIA"
  if [ -f /tmp/bateria_ultima.txt ]; then
    tail -4 /tmp/bateria_ultima.txt
  else
    echo "  (sem corrida registada — corre  tools/bateria.sh)"
    echo "  E LEIA O TOTAL, não a linha das unidades: um medidor que não compila não falha,"
    echo "  desaparece."
  fi
  ;;

*)
  az "PAINEL DO PILOTO — $(basename "$RAIZ")"
  echo
  az "1. A LIQUIDAÇÃO (o contrato não se assina: dê os termos, ele corre)"
  linha "o piloto dá" "alguns termos — 4 bastam"
  linha "sai a régua" "(B,C), por Cramer, exata em inteiros"
  linha "sai o dual" "ν(a,b) = (a + B·b, −b) — forçado"
  linha "e a soma e o produto" "também, da mesma régua"
  linha "FECHOU quando" "a reversão volta com resíduo 0"
  linha "e chama o agente" "que o Δ determina — gira, estica ou o limite"
  echo "  uso:  ./painel.sh fecha 0 1 1 2 3 5"
  echo
  az "1b. AS DUAS FORMAS (e o painel usa as duas)"
  linha "ALGÉBRICA  z = a + b·σ" "soma bem — é o produto DIRETO, e MEDE"
  linha "POLAR      z = ρ·E(θ)" "multiplica bem — é o CRUZADO, e ORDENA"
  linha "o regime" "Δ<0 gira · Δ>0 estica · Δ=0 o limite"
  linha "sob o espelho ν" "o direto FICA, o cruzado TROCA de sinal"
  echo "  uso:  ./painel.sh polar 1 -1 3 2"
  echo
  az "2. A MÁQUINA (a ISA ERG-64, sem RAM)"
  linha "opcodes expostos ao piloto" "$(grep -c '^    { "' "$CD/erg.c" 2>/dev/null || echo '?')"
  linha "registos" "A, B, R — e o pc"
  linha "memória" "um ficheiro, 16 bytes por slot, pread/pwrite"
  linha "as duas armadilhas" "STORE grava R · FL_ZERO é AMBOS zero"
  echo
  az "3. O BANCO DO PAINEL"
  if [ -f "$BANCO" ]; then
    linha "registos" "$(wc -l < "$BANCO")"
    linha "últimos" "$(tail -5 "$BANCO" | tr '\n' ' ')"
  else
    linha "registos" "0 (vazio — use  op  para escrever)"
  fi
  linha "o corpo" "Z_$Q — finito, logo todo percurso FECHA por gaiola"
  echo
  az "4. O HOOK DE ENTRADA"
  if [ -f "$HOOK" ] && grep -q memoria_banco.sh "$HOOK" 2>/dev/null; then
    linha "a túnica" "LIGADA — a assistente entra vestida"
  else
    linha "a túnica" "não ligada"
  fi
  if [ -s /tmp/memoria_banco.txt ]; then
    linha "índice cifrado" "$(wc -l < /tmp/memoria_banco.txt) memórias, $(wc -c < /tmp/memoria_banco.txt) bytes"
  else
    linha "índice cifrado" "ausente (tools/memoria_banco.sh ingere)"
  fi
  echo
  az "5. OS PLUGUES"
  for p in "erg.c:assembly ERG-64 (montador e executor)" \
           "plugue.sh:bash — os verbos do lado de dentro" \
           "dominios.c:PTX — a GPU escreve na janela" \
           "chessb.c:WASM e Node — a pilha do wasm na nossa ISA" \
           "fecha.c:o fecho — meia dualidade dá a outra metade" \
           "smartcontract.c:o contrato que se liquida e chama agentes" \
           "polar.c:as duas formas — algébrica (direto) e polar (cruzado)" \
           "prisma.c:o corpo prismático — o triângulo que enche" \
           "tex.c:LaTeX → PDF, sem dependência nenhuma" \
           "sql.c:SQL no metal — a mesma ISA, o disco é a memória"; do
    f=${p%%:*}; d=${p#*:}
    if [ -f "$CD/$f" ]; then printf '  %-16s %s\n' "$f" "$d"
    else printf '  %-16s %s  (ausente)\n' "$f" "$d"; fi
  done
  echo
  echo "  o manual:  PILOTO.md    modos:  fecha · polar · hook · apps · bateria · op"
  ;;
esac
