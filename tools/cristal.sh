#!/bin/sh
# cristal.sh — o cristal recuperado entra no banco: as falas E os 4234 conceitos
# (4286 recuperados − 52 fusões de curadoria; tools/cristal_cura.py).
# Fontes: cristal/cristal.jsonl (fonte), cristal/cristal_*.tex (projeções),
# tests/cristal_volta.js (a volta, R=0). Ordem do gerente (eval 13/08), passo 3:
# «registrar o resultado no banco/proveniência».
#
#   cd banco && ../tools/cristal.sh .fala/<hex>
set -e
B="${1:?uso: ./cristal.sh <base>}"
D=$(cd "$(dirname "$0")" && pwd)
ROOT=$(cd "$D/.." && pwd)
CV="$ROOT/banco/bin/conversa"
[ -x "$CV" ] || { echo "falta $CV — compila o banco primeiro"; exit 1; }
mkdir -p "$B"
aprende(){ "$CV" "$B" aprende "$1" "$2" >/dev/null; }

# —— o cristal, e onde estava ——
aprende "mostra o cristal" "cristal/cristal.jsonl — 4234 conceitos (4286 recuperados do broca-so, 52 fusões de curadoria); projeções em cristal/cristal_*.tex; volta em tests/cristal_volta.js"
aprende "onde estava o cristal" "broca-so/conversa/dados/conhecimento.graph.jsonl — jornal de 75165 registos, 4286 conceitos. O jornal ficou lá, intocado."
aprende "quantos conceitos tem o cristal" "4234 — 4286 recuperados (última versão de cada id) menos 52 fusões de curadoria; a história fica em cristal/historia.tsv (o floxina_investigacao tem 2908 versões)."
aprende "o que e a volta do cristal" "Reconstrução byte a byte das 10 projeções LaTeX contra a fonte: R=0. tests/cristal_volta.js; claim CristalVolta (id 15)."
aprende "o que e cristalvolta" "conecthus/claims/cristal.claim — step project_latex, back reconstruct, measure byte_compare; 0 e 0 não fecha; réu não fecha."
aprende "a inducao do cristal" "Apagar conceito, trocar resposta, alterar categoria, corromper projeção: R diferente de 0, REOPEN. Reordenar sobrevive por desenho (ordem canónica é derivada do id)."
aprende "a energia do cristal" "E = soma dos quadrados — a massa da cruz; E(fonte)=E(reconstrução)=38771546660 exato (a âncora pré-curadoria era 38731623179; a diferença é a soma dos contornos das 52 fusões); Parseval dourado 1D/2D/4D fator 256. tests/cristal_energia.js"
aprende "o que e meta-inducao" "O passo sobre os passos: a leitura dual que mede a indução e valida a conservação de energia. corpo_topologico.tex def:inducao"
aprende "o estresse do cristal" "e = R - k, lido pelo endereço: zero exato com o endereço vivo; o par faltante+excedente quando morre. tests/lyapunov_torre.js; Teorema da Absorção, papers/corpo_topologico.tex"
aprende "a historia do cristal" "cristal/historia.tsv: id, versões no jornal. A normalização não amassa a linha do tempo — a versão canónica sabe que tem passado."
aprende "de onde vem cada conceito" "Cada registo carrega origem, meta.fonte, meta.dominio, epistemico, confianca e as arestas do grafo. A proveniência viaja no %CRISTAL de cada secção."
aprende "quais sao os grupos do cristal" "ciencias 899, matematica 679, manual 516, computacao 515, engenharia 441, papers 427, fisica 332, floxina 187, xadrez 150, diversos 88."
aprende "o que e a curadoria do cristal" "52 fusões (44 por texto idêntico + 8 julgadas com leitura) e 13 pares mantidos com motivo, tudo em cristal/curadoria.tsv; nada se apaga — cada fusão guarda as duas partes e desfazer devolve os 4286. tools/cristal_cura.py; tests/cristal_curadoria.js"

# —— os 4234: uma fala por conceito, pela estrutura que o documento declara ——
for T in "$ROOT"/cristal/cristal_*.tex; do
  python3 "$D/ingere.py" "$T" | "$CV" "$B" - >/dev/null
done

echo "cristal.sh: falas + 10 projeções ingeridas em $B"
