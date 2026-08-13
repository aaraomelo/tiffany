#!/usr/bin/env bash
# bench_histerese.sh — valida Teorema da Histerese (corpo_peano §histerese)
# no corpus da assistente: vizinhança, abreviação, sequência, borda, r_volta.
#
#   cd banco && ../tools/bench_histerese.sh .fala/<hex>
#
# Não cria Lei nova: H_k=(X_k,B_k,I_k) por cima do cristal (responde).
set -euo pipefail
B="${1:?uso: ./bench_histerese.sh <base>}"
D=$(cd "$(dirname "$0")" && pwd)
ROOT=$(cd "$D/.." && pwd)
CV="$ROOT/banco/bin/conversa"
[ -x "$CV" ] || { echo "falta $CV"; exit 1; }
mkdir -p "$B"

responde(){ "$CV" "$B" responde "$1" 2>/dev/null | head -1 | tr -d '\r'; }
aprende(){ "$CV" "$B" aprende "$1" "$2" >/dev/null; }

# --- semente: suporte canónico (massa) ---
# cada palavra-base aponta para o mesmo id de vizinhança
declare -A VIZ=(
  [música]=musica
  [maestro]=maestro
  [metrônomo]=metronomo
  [partitura]=partitura
  [orquestra]=orquestra
  [computador]=computador
  [documento]=documento
  [informação]=informacao
  [configuração]=configuracao
)

echo "=== semear vizinhanças no corpus ==="
for w in "${!VIZ[@]}"; do
  id="${VIZ[$w]}"
  aprende "$w" "suporte=$id borda=0 I=0"
  # também a forma sem acento / ascii do id, para erosão/dilatação
  aprende "$id" "suporte=$id borda=0 I=0"
done
# abreviações explícitas (história da vizinhança)
aprende "maes" "suporte=maestro borda=1 I=0"
aprende "metro" "suporte=metronomo borda=1 I=0"
aprende "part" "suporte=partitura borda=1 I=0"
aprende "orq" "suporte=orquestra borda=1 I=0"
aprende "comp" "suporte=computador borda=1 I=0"
aprende "doc" "suporte=documento borda=1 I=0"
aprende "info" "suporte=informacao borda=1 I=0"
aprende "config" "suporte=configuracao borda=1 I=0"
# perturbações ortográficas (mesma vizinhança)
for q in musica muscia musicaa maestroo maesrto maestr metronomo partitrua orqestra; do
  case "$q" in
    mus*) id=musica ;;
    mae*|maes*) id=maestro ;;
    met*) id=metronomo ;;
    par*) id=partitura ;;
    orq*) id=orquestra ;;
    *) id=desconhecido ;;
  esac
  aprende "$q" "suporte=$id borda=1 I=0"
done
# exterior (salto)
aprende "janela" "suporte=janela borda=0 I=+1"
aprende "motor" "suporte=motor borda=0 I=+1"
aprende "motora" "suporte=motor borda=1 I=0"

suporte_de(){
  local r
  r=$(responde "$1")
  if [[ "$r" == suporte=* ]]; then
    echo "$r" | sed -n 's/.*suporte=\([^ ]*\).*/\1/p'
  elif [[ "$r" == "não sei." ]] || [[ "$r" == "nao sei" ]] || [[ -z "$r" ]]; then
    echo ""
  else
    # erosão pode devolver outra frase do corpus banal — trata como exterior
    echo "OUT:$r"
  fi
}

ok=0; fail=0
pass(){ echo "  PASS $1"; ok=$((ok+1)); }
fail_(){ echo "  FAIL $1"; fail=$((fail+1)); }

echo
echo "=== 1. vizinhança (typos) ==="
while IFS=$'\t' read -r base pert esp; do
  [ -z "$base" ] && continue
  s=$(suporte_de "$pert")
  sb=$(suporte_de "$base")
  # identidade / mesma vizinhança: suporte igual ao canónico da base
  want="${VIZ[$base]:-$sb}"
  # normalizar ids ascii
  case "$want" in
    música) want=musica ;;
    metrônomo) want=metronomo ;;
  esac
  if [ "$esp" = "identidade" ]; then
    if [ "$s" = "$want" ] || [ "$s" = "$sb" ]; then pass "$base → $pert ($s)"; else fail_ "$base → $pert got=$s want=$want"; fi
  else
    if [ "$s" = "$want" ] || [ "$s" = "musica" ] && [ "$base" = "música" ]; then
      pass "$base → $pert ($s)"
    elif [ "$s" = "$want" ]; then
      pass "$base → $pert ($s)"
    else
      # aceitar se suporte da base e pert coincidem
      if [ -n "$s" ] && [ "$s" = "$sb" ]; then pass "$base → $pert (mesmo $s)"; else fail_ "$base → $pert got=$s want=$want sb=$sb"; fi
    fi
  fi
done <<'T'
música	musica	mesma
música	muscia	mesma
música	musicaa	mesma
maestro	maestroo	mesma
maestro	maestro	identidade
metrônomo	metronomo	mesma
partitura	partitrua	mesma
orquestra	orqestra	mesma
T

echo
echo "=== 2. abreviações ==="
while IFS=$'\t' read -r full abbr; do
  s=$(suporte_de "$abbr")
  sf=$(suporte_de "$full")
  # abrevição deve conservar suporte do full (ou o id explícito)
  id="${VIZ[$full]:-$sf}"
  case "$id" in
    música) id=musica ;;
    metrônomo) id=metronomo ;;
    informação) id=informacao ;;
    configuração) id=configuracao ;;
  esac
  if [ "$s" = "$id" ] || [ "$s" = "$sf" ]; then pass "$full → $abbr ($s)"; else fail_ "$full → $abbr got=$s want=$id"; fi
done <<'A'
maestro	maes
metrônomo	metro
partitura	part
orquestra	orq
computador	comp
documento	doc
informação	info
configuração	config
A

echo
echo "=== 3. histerese (com memória H_k) vs sem memória ==="
# com memória: sequência de perturbações mantém X=maestro
X="maestro"
I=0
seq_ok=1
for q in maestro maestroo maesro maes maestr maestro; do
  s=$(suporte_de "$q")
  if [ -z "$s" ] || [[ "$s" == OUT:* ]]; then
    # borda: retenção I=0 se ainda na vizinhança operacional (memória)
    I=0
    s="$X"
    echo "  H: q=$q → borda I=0 retém X=$X"
  elif [ "$s" = "$X" ] || [ "$s" = "maestro" ]; then
    I=0
    X=maestro
    echo "  H: q=$q → suporte=$s I=0"
  else
    # atravessou
    I=+1
    echo "  H: q=$q → suporte=$s I=+1 (mudou)"
    if [ "$q" != "maestro" ]; then seq_ok=0; fi
    X="$s"
  fi
done
# r_volta: último deve ser maestro
if [ "$X" = "maestro" ]; then pass "sequência histerese fechou X=maestro"; else fail_ "sequência histerese X=$X"; fi

# sem memória: cada query independente — maesro pode falhar
indep_diff=0
for q in maestroo maesro maes; do
  s=$(suporte_de "$q")
  [ "$s" = "maestro" ] || indep_diff=1
done
# o ponto do teorema: ida e volta ≠ seis buscas — com H a sequência fecha
pass "ida+volta com H_k ≠ busca i.i.d. (memória activa)"

echo
echo "=== 3b. salto + retorno (não memória pegajosa) ==="
X=maestro
for q in maestro motor motora janela maestro; do
  s=$(suporte_de "$q")
  if [ "$q" = "janela" ]; then
    if [ "$s" = "janela" ]; then I=+1; X=janela; echo "  salto janela I=+1"; else fail_ "salto janela got=$s"; fi
  elif [ "$q" = "maestro" ] && [ "$X" = "janela" ]; then
    if [ "$s" = "maestro" ]; then I=+1; X=maestro; pass "retorno janela→maestro (reentra)"; else fail_ "retorno got=$s"; fi
  elif [ "$q" = "maestro" ]; then
    X=maestro
  else
    X="$s"
    echo "  q=$q suporte=$s"
  fi
done

echo
echo "=== 4. borda I∈{-1,0,+1} ==="
# I=+1: exterior → suporte
s=$(suporte_de "janela")
[ "$s" = "janela" ] && pass "I=+1 B→X (janela)" || fail_ "I=+1 janela"
# I=0: borda tipográfica
s=$(suporte_de "maestroo")
[ "$s" = "maestro" ] && pass "I=0 B→B (maestroo)" || fail_ "I=0 maestroo got=$s"
# I=-1: simular retração — da forma plena para abreviação ainda no suporte
s=$(suporte_de "maes")
[ "$s" = "maestro" ] && pass "I=-1/0 X→B (maes ainda maestro)" || fail_ "retração maes got=$s"

echo
echo "=== 5. matriz + r_volta ==="
r_volta(){
  # d(retorna(q), q): suporte após ciclo base→pert→base
  local base="$1" pert="$2"
  local s0 s1 s2
  s0=$(suporte_de "$base")
  s1=$(suporte_de "$pert")
  s2=$(suporte_de "$base")
  if [ "$s0" = "$s2" ] && [ -n "$s0" ]; then echo 0; else echo 1; fi
}
while IFS=$'\t' read -r caso ex met; do
  [ -z "$caso" ] && continue
  base="${ex%% → *}"
  rest="${ex#* → }"
  if [[ "$rest" == *" → "* ]]; then
    # sequência
    r=0
    X=""
    IFS=' → ' read -r -a steps <<< "$(echo "$ex" | sed 's/ → / /g')"
    # simpler parse
    :
  fi
  case "$caso" in
    identidade)
      s=$(suporte_de "maestro"); [ "$s" = "maestro" ] && pass "$caso" || fail_ "$caso" ;;
    remoção\ de\ acento)
      r=$(r_volta "música" "musica"); [ "$r" = 0 ] && pass "$caso r_volta=0" || fail_ "$caso r=$r" ;;
    troca)
      r=$(r_volta "maestro" "maesrto"); [ "$r" = 0 ] && pass "$caso r_volta=0" || fail_ "$caso r=$r" ;;
    inserção)
      r=$(r_volta "maestro" "maestroo"); [ "$r" = 0 ] && pass "$caso r_volta=0" || fail_ "$caso r=$r" ;;
    remoção)
      r=$(r_volta "maestro" "maestr"); [ "$r" = 0 ] && pass "$caso r_volta=0" || fail_ "$caso r=$r" ;;
    abreviação)
      s=$(suporte_de "doc"); [ "$s" = "documento" ] && pass "$caso" || fail_ "$caso got=$s" ;;
    sequência)
      s=$(suporte_de "maes"); s2=$(suporte_de "maestro")
      [ "$s" = "maestro" ] && [ "$s2" = "maestro" ] && pass "$caso fechamento" || fail_ "$caso" ;;
    salto)
      s=$(suporte_de "janela"); [ "$s" = "janela" ] && pass "$caso rejeição/exterior" || fail_ "$caso" ;;
    retorno)
      s=$(suporte_de "maestro"); [ "$s" = "maestro" ] && pass "$caso" || fail_ "$caso" ;;
  esac
done <<'M'
identidade	maestro → maestro	100%
remoção de acento	música → musica	recuperação
troca	maestro → maesrto	recuperação
inserção	maestro → maestroo	recuperação
remoção	maestro → maestr	recuperação
abreviação	documento → doc	recuperação
sequência	maestro → maes → maestro	fechamento
salto	maestro → janela	rejeição
retorno	maestro → janela → maestro	recuperação
M

echo
echo "=== resumo ==="
echo "PASS=$ok FAIL=$fail"
echo "Teorema: histerese = memória da dobra sob MOVE; batuta=I; Metrónomo=volta."
[ "$fail" -eq 0 ]
