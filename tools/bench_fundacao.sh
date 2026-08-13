#!/usr/bin/env bash
# bench_fundacao.sh — smoke da escada no corpus
#   cd banco && ../tools/bench_fundacao.sh .fala/<hex>
set -euo pipefail
B="${1:?uso: ./bench_fundacao.sh <base>}"
D=$(cd "$(dirname "$0")" && pwd)
CV="$(cd "$D/.." && pwd)/banco/bin/conversa"
"$D/fundacao.sh" "$B" >/tmp/fund_seed.log

responde(){ "$CV" "$B" responde "$1" 2>/dev/null | head -1; }
ok=0; fail=0
pass(){ echo "  PASS $1"; ok=$((ok+1)); }
fail_(){ echo "  FAIL $1"; fail=$((fail+1)); }

check(){
  local q="$1" pat="$2"
  local r; r=$(responde "$q")
  echo "$r" | grep -qiE "$pat" && pass "$q" || fail_ "$q → $r"
}

echo "=== escada fundação ==="
check "mostra a fundação" "torre_fundacao|fundação|Corpos"
check "o que é um corpo" "corpo|invers|teoria|torre_fundacao"
check "o que é o corpo dual" "dual|V\*|teoria"
check "o que é hurwitz" "Hurwitz|hurwitz|R,C|álgebra|algebra|tests/hurwitz"
check "o que é dinamica de sistemas" "dinâmica|dinamica|órbita|orbita|F\(|torre_fundacao"
check "o que é a topologia dos corpos" "topologia|Δ|regua|régua|tests/topologia"
check "o que é clifford" "Clifford|clifford|orient|torre_fundacao"
check "o que é a curva de hilbert aqui" "Hilbert|hilbert|Lei|tests/hilbert"
check "o que é a lei 1" "dual|involu|Lei|lei|período|periodo|†|1"
check "o que é a lei 2" "rotor|T|periodo|período|Lei|lei|4"
check "porque hurwitz para em 8" "16|divisor|Hurwitz|hurwitz|cristal|zero"
check "dois corpos sao isomorfos quando" "Δ|isomorf|zero|assinatura|topologia|delta"
check "o que é o bidual de hilbert" "π|nu|ν|bidual|hilbert|Hilbert|estica|contrai"
check "o que faz o maestro" "projecta|π|tick|batuta|corpo_peano|Maestro|maestro"
check "o que faz o metronomo" "volta|λ|atesta|Metr|metr|resíduo|residuo|corpo_peano"
check "o que e histerese aqui" "borda|batuta|I|histerese|Lei|memória|memoria"
check "o que é o corpo de peano" "torre|π|peano|Peano|corpo_peano|retra"
check "o que e a rede dual" "rede|Hopfield|estaca|conjug|P|banda|corpo_peano|W="
check "o que e a conjugacao reversivel" "F_H|conjug|D|Hopfield|invers|corpo_peano"
check "o que e o estado hibrido" "X=|hibr|híbr|\\(x|estado|F:|corpo_peano"
check "hopfield e a inversa?" "Não|nao|memória|memoria|conjug|invers|Hopfield"
check "porque hopfield nao inverte sozinho" "Hopfield|atrac|conjug|invers|F_H|memória|memoria"
check "o que fecha lambda mais e menos" "conjug|lambda|λ|DF_H|rede_dual|soma|0"
check "onde a assistente usa a rede" "assistente|reten|Hopfield|Metr|lambda|λ|rede_dual"
check "o que e o teorema central no peano" "Hurwitz|Gentil|estrela|contar|integrar|central|corpo_peano"
check "como o teorema central sobe a torre" "π|pi_k|Maestro|Metr|controlo|rede|torre|Conserv|Batuta|corpo_peano"
check "o que e o conservatorio no peano" "Conserv|conserv|Gentil|cone|orquestra|corpo_peano|musica"
check "como a rede dual entra na musica" "Batuta|batuta|I|rede|Metr|lambda|λ|ℱ|F:|Π|Pi|partitura|corpo_peano"
check "o que e a partitura pi" "partitura|Π|Pi|assinatura|nota|Maestro|transporte|corpo_peano|partitura.tex"
check "como a partitura entra na assistente" "partituraFala|Π|Pi|batuta|I|ramo|Metr|maestro|assistente"

echo "PASS=$ok FAIL=$fail"
[ "$fail" -eq 0 ]
