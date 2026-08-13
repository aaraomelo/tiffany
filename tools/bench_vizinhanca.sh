#!/usr/bin/env bash
# bench_vizinhanca.sh — regressão linguística (bandas 1–7) + histerese de contexto.
#   cd banco && ../tools/bench_vizinhanca.sh .fala/<hex>
set -euo pipefail
B="${1:?uso: ./bench_vizinhanca.sh <base>}"
D=$(cd "$(dirname "$0")" && pwd)
ROOT=$(cd "$D/.." && pwd)
CV="$ROOT/banco/bin/conversa"
[ -x "$CV" ] || { echo "falta $CV"; exit 1; }

"$D/vizinhanca.sh" "$B" >/tmp/viz_seed.log

responde(){ "$CV" "$B" responde "$1" 2>/dev/null | head -1 | tr -d '\r'; }
tem(){ # $1 fala $2 substring esperada (casefold approx)
  local r; r=$(responde "$1")
  echo "$r" | grep -qi "$2"
}
ok=0; fail=0
pass(){ echo "  PASS $1"; ok=$((ok+1)); }
fail_(){ echo "  FAIL $1"; fail=$((fail+1)); }

echo "=== B1 erros leves ==="
tem "muzica" "música\|musica\|leve" && pass "muzica" || fail_ "muzica→$(responde muzica)"
tem "cafeh" "café\|Cafe\|quentinho" && pass "cafeh" || fail_ "cafeh"
tem "canssado" "descanso\|cansado\|Compreendo" && pass "canssado" || fail_ "canssado"
tem "bom diaa" "Bom dia\|bom dia" && pass "bom diaa" || fail_ "bom diaa"
tem "to cansado hj" "descans\|leve\|calma" && pass "to cansado hj" || fail_ "to cansado hj"

echo "=== B2 abreviações ==="
tem "vc tbm gosta de cafe?" "Gosto\|quente\|café\|Cafe" && pass "vc tbm…" || fail_ "vc tbm"
tem "blz" "Beleza\|ajudar" && pass "blz" || fail_ "blz"
tem "obg" "nada\|De nada" && pass "obg" || fail_ "obg"
tem "flw" "Falou\|já" && pass "flw" || fail_ "flw"

echo "=== B3 oral (sem corrigir) ==="
r=$(responde "to morrendo de fome")
echo "$r" | grep -qi "quis dizer\|você quis\|corrig" && fail_ "corrigiu utilizador" || pass "não corrige (fome)"
tem "ta frio ai?" "café\|Cafe\|quentinho" && pass "ta frio ai?" || fail_ "ta frio"
tem "nem dormi direito hj" "descans\|pesado" && pass "nem dormi…" || fail_ "nem dormi"

echo "=== B4 variação de expressão (café / fome) ==="
tem "bora um café?" "Bora\|quentinho\|Café" && pass "bora um café?" || fail_ "bora café"
tem "cafezim agora ia bem" "Ia\|vamos\|café\|Cafe" && pass "cafezim" || fail_ "cafezim"
tem "preciso comer" "almoço\|lanche\|bora\|comer" && pass "preciso comer" || fail_ "preciso comer"
tem "bora almoçar?" "Bora\|Arroz\|salada" && pass "bora almoçar?" || fail_ "almoçar"

echo "=== B5 ruído ==="
tem "vc gosta d cafe? kkk" "Gosto\|Café\|Cafe" && pass "cafe kkk" || fail_ "cafe kkk"
tem "to cansado pra caramba hj" "calma\|descans" && pass "cansado pra caramba" || fail_ "caramba"
tem "me indica alguma coisa simples pra comer" "Sanduíche\|omelete\|ovo\|simples" && pass "indica comer" || fail_ "indica"

echo "=== B6 continuidade (H_k) ==="
# sequência: fome → sim → sem queijo (perturbações)
r1=$(responde "to com fome")
r2=$(responde "sim")
r3=$(responde "sem queijo")
r4=$(responde "sem qjo")
r5=$(responde "n quero queijo")
echo "$r1" | grep -qi "sugestão\|comer\|Quer" && pass "turno1 fome" || fail_ "turno1:$r1"
echo "$r2" | grep -qi "sanduíche\|omelete\|opções\|opcoes" && pass "turno2 sim" || fail_ "turno2:$r2"
echo "$r3" | grep -qi "omelete\|tomate\|ervas\|queijo" && pass "turno3 sem queijo" || fail_ "turno3:$r3"
[ "$r3" = "$r4" ] || echo "$r4" | grep -qi "omelete\|tomate" && pass "sem qjo mesma vizinhança" || fail_ "sem qjo:$r4"
echo "$r5" | grep -qi "omelete\|tomate\|ervas" && pass "n quero queijo" || fail_ "n quero:$r5"

echo "=== B7 não-colagem ==="
# após fome, mudança clara de assunto
responde "to com fome" >/dev/null
r=$(responde "qual é a capital do Chile?")
echo "$r" | grep -qi "Santiago" && pass "salto Chile (não pegajoso)" || fail_ "Chile:$r"
echo "$r" | grep -qi "sanduíche\|omelete\|fome" && fail_ "colou na fome" || pass "não colou comida no Chile"

echo "=== B8 ambiguidade controlada ==="
tem "banco" "contexto\|Banco\|sentar\|dinheiro" && pass "banco pede contexto" || fail_ "banco:$(responde banco)"
tem "manga" "fruta\|camisa\|Manga" && pass "manga ambígua" || fail_ "manga"
tem "quero manga pra comer" "manga\|doce\|leve" && pass "manga+contexto comida" || fail_ "manga comer"
tem "rasgou a manga" "camisa\|agulha\|peça" && pass "manga+contexto roupa" || fail_ "manga roupa"
tem "jantar" "jantar\|comer\|ideia" && pass "jantar intenção" || fail_ "jantar"

echo "=== B9 reparação (adaptar, não insistir) ==="
responde "to com fome" >/dev/null
r=$(responde "nao, queria algo doce")
echo "$r" | grep -qi "doce\|fruta\|bolo\|chocolate" && pass "reparação → doce" || fail_ "reparação:$r"
echo "$r" | grep -qi "sanduíche quente ou uma omelete" && fail_ "insistiu omelete" || pass "não insistiu sugestão anterior"
tem "esquece o cafe" "Esquecido\|ajudar\|então" && pass "soltar café" || fail_ "esquece cafe"
tem "muda de assunto" "Claro\|falar\|Sobre" && pass "muda de assunto" || fail_ "muda"

echo
echo "PASS=$ok FAIL=$fail (bandas 1–9)"
echo "cadeia: reconhecer→reter→adaptar→reparar→soltar"
[ "$fail" -eq 0 ]
