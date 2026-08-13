#!/bin/sh
# pipeline.sh — Cresce o corpus com o inventário Claim + fronteira + Controlo.
# Fontes: conecthus/pipeline.tex, claims/*.claim, papers/conversa_pipeline.tex
#
#   cd banco && ../tools/pipeline.sh .fala/<hex>
#
set -e
B="${1:?uso: ./pipeline.sh <base>}"
D=$(cd "$(dirname "$0")" && pwd)
ROOT=$(cd "$D/.." && pwd)
CV="$ROOT/banco/bin/conversa"
[ -x "$CV" ] || { echo "falta $CV — compila o banco primeiro"; exit 1; }
mkdir -p "$B"
aprende(){ "$CV" "$B" aprende "$1" "$2" >/dev/null; }

# —— porta ——
aprende "mostra o pipeline" "conecthus/pipeline.tex — IR Claim; papers/conversa_pipeline.tex"
aprende "o que e o pipeline" "IR do desenvolvimento: Claim especifica; Result observa. conecthus/pipeline.tex"
aprende "falemos do pipeline" "terceiro eixo da arquitetura (porta, torre, pipeline). papers/arquitetura.tex"

# —— inventário LMS ——
aprende "quantas claims lean ha" "Dez no disco: 9 ferramentas LMS + PipelineClosure. conecthus/claims/"
aprende "o que e um claim" "Especificação: lei, objecto, passo, volta, medidor, invariante, mutação. Sem residual=. conecthus/claims/"
aprende "claim e resultado sao a mesma coisa" "Não. Claim=especificação; Result=observação após STEP/BACK/MEASURE. tests/claim_ir.c"

aprende "mostra o pareto" "conecthus/claims/pareto.claim — soma=100; tests/claim_ir.c"
aprende "mostra o gut" "conecthus/claims/gut.claim — produto G×U×T; par do Pareto"
aprende "mostra o pdca" "conecthus/claims/pdca.claim — Check precisa de alvo numérico"
aprende "mostra o kanban" "conecthus/claims/kanban.claim — 3 estados; fundir colide"
aprende "mostra o why" "conecthus/claims/why.claim — ponto fixo da erosão"
aprende "mostra o 5w2h" "conecthus/claims/fivew2h.claim — dilatação; raiz no plano"
aprende "mostra o ishikawa" "conecthus/claims/ishikawa.claim — k livre (convenção)"
aprende "mostra o vsm" "conecthus/claims/vsm.claim — dual actual↔futuro"
aprende "mostra o fluxograma" "conecthus/claims/fluxograma.claim — dual nó a nó"

# —— fronteira ——
aprende "o que e a fronteira no pipeline" "Resíduo entre Claims (Lei 7): ligar sem fundir. conecthus/core/fronteira.c"
aprende "mostra a fronteira" "fronteira_gut / fronteira_why / fronteira_pdca — tests/claim_fronteira.c · na UI: chip F (app/src/fronteira.js)"
aprende "compara gut e 5w2h" "Inversões de ranking: F=0 se concordam. UI: gut:60,48,30,12 ordem:0,1,2,3"
aprende "pdca e vsm discordam como" "Alvo do Plan ≠ futuro do VSM. fronteira_pdca.claim"

# —— controlo / acaso ——
aprende "o que e retain" "R=0 e D⪯Θ — estado preservado. app/src/controlo.js"
aprende "o que e retract" "R≠0 — REOPEN. tests/claim_control.c"
aprende "o que e o acaso no controlo" "Quarto cenário: se o acaso empata, a métrica é degenerada. fundamento.tex; control_acaso"
aprende "mostra o controlo" "RETAIN/MOVE/RETRACT acima da IR. conecthus/core/control.c; chip C na assistente"

# —— meta ——
aprende "o que e pipelineclosure" "emit∘parse=id; mutação alter_ast denuncia. pipeline.claim"
aprende "mostra a estacao" "projectado contra medido na estação — conecthus/claims/estacao.claim · chip E"
aprende "o que e o residuo na estacao" "R=|P−M|; fecha só com leitura MES, não com referência do réu. fundamento.tex ob:residuo"
aprende "mostra o banco" "R da implantação é documento, não peso — conecthus/claims/banco.claim · chip B"
aprende "o que e a volta no banco" "write_document / retrieve; emit∘parse=id; fundamento.tex ob:banco"
aprende "mostra o deploy" "portão Pátria: corpo local contra HTTP. conecthus/claims/deploy.claim · chip D · tools/patria.sh"
aprende "o que e o deploy na patria" "Fecha só com fetch 200 de /corpo/conecthus/pipeline.tex. 0∧0 não fecha. O push é do dono."

echo "pipeline.sh: corpus Claim/fronteira/controlo em $B"
