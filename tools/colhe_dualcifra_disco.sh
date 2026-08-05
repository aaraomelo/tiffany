#!/bin/bash
# ── O FORWARD FOI APAGADO, E ISTO DEPENDIA DELE ────────────────────────────────────
#
# O Aarao: "tira transplante e forward e tn, apaga — eles sao ruido. isso nao e'
# perfeccionismo: e' que outro agente entra e isso pode e vai se propagar."
#
# Este script compilava banco/forward.c para colher do gguf, e o forward saiu. Fica aqui
# porque a razao dele continua boa — colher do FICHEIRO e nao de um servidor — mas NAO
# CORRE ate' alguem escrever o leitor de embeddings sem os buffers de rascunho que
# fizeram o forward ser ruido.
#
# O que ele fazia, e que vale a pena refazer sem o resto:
#   COLHE=<saida> N=<n> GGUF=<blob>          le' n linhas de token_embd.weight
#   COLHE_FRASES=<entrada> SAIDA=<saida>     um vector por linha, byte a byte, O(1)
#   BANCO=<nome>                             le' do cristal onde ele tem, do gguf onde nao
#
# colhe_dualcifra_disco.sh — as duas metades, SEM SERVIDOR. Só o ficheiro.
#
# O Aarão: "não precisa estar acordado nada, é um arquivo como qualquer outro, navegável;
# a única era o forward que já tem — é o espelho, a involução dá."
#
# O colhe_dualcifra.sh original pede um embedding por HTTP a cada frase e a cada palavra.
# Aqui não se pede nada a ninguém: o mesmo `forward` que corre o modelo do disco tem o
# modo COLHE_FRASES, que lê byte a byte e devolve o vetor — leitura, involução, escrita,
# com memória O(1). O ollama pode estar desligado.
#
# As três saídas são as mesmas, no mesmo formato (o padrão de bits em hex):
#   /tmp/frases.txt    um vetor por frase
#   /tmp/palavras.txt  um vetor por palavra, na ordem em que aparecem
#   /tmp/mapa.txt      quantas palavras tem cada frase
#
#   sh tools/colhe_dualcifra_disco.sh
set -u
RAIZ=$(cd "$(dirname "$0")/.." && pwd)
cd "$RAIZ" || exit 1
BLOBS=${BLOBS:-/usr/share/ollama/.ollama/models/blobs}

# as MESMAS oito frases do colhe original — o que muda é de onde vêm os vetores
cat > /tmp/dc_frases_in.txt <<'FIM'
o corpo é finito
a cifra é o endereço
ler e escrever
o gato estica
crescer não é cair
a soma mede
o trie é o índice
guardar é grátis
FIM

# uma palavra por linha, na ordem, e o mapa a dizer quantas por frase
: > /tmp/dc_palavras_in.txt
: > /tmp/mapa.txt
while IFS= read -r fr; do
    [ -z "$fr" ] && continue
    n=0
    for w in $fr; do printf '%s\n' "$w" >> /tmp/dc_palavras_in.txt; n=$((n+1)); done
    echo "$n" >> /tmp/mapa.txt
done < /tmp/dc_frases_in.txt

# o gguf de embeddings, achado pelo MÁGICO e não pelo nome do sha
EMB=${EMB:-}
if [ -z "$EMB" ]; then
    for b in "$BLOBS"/sha256-*; do
        [ -f "$b" ] || continue
        s=$(stat -c%s "$b" 2>/dev/null) || continue
        [ "$s" -lt 50000000 ] && continue
        [ "$s" -gt 600000000 ] && continue
        [ "$(head -c 4 "$b" 2>/dev/null)" = "GGUF" ] && { EMB="$b"; break; }
    done
fi
[ -z "$EMB" ] && { echo "  não achei um gguf de embeddings em $BLOBS (use EMB=<caminho>)"; exit 1; }

# recompila se o fonte for MAIS NOVO que o binario — sem isto fica-se com um
# forward antigo, sem os modos novos, e o colhedor devolve zero em silencio
[ -x /tmp/forward_colhe ] && [ /tmp/forward_colhe -nt banco/forward.c ] || \
    cc -O2 -std=c99 -w -Ilib -I. -o /tmp/forward_colhe banco/forward.c -lm 2>/dev/null
[ -x /tmp/forward_colhe ] || { echo "  o forward não compilou"; exit 1; }

GGUF="$EMB" COLHE_FRASES=/tmp/dc_frases_in.txt   SAIDA=/tmp/frases.txt   /tmp/forward_colhe >/dev/null 2>&1
GGUF="$EMB" COLHE_FRASES=/tmp/dc_palavras_in.txt SAIDA=/tmp/palavras.txt /tmp/forward_colhe >/dev/null 2>&1

nf=$(wc -l < /tmp/frases.txt 2>/dev/null || echo 0)
np=$(wc -l < /tmp/palavras.txt 2>/dev/null || echo 0)
nm=$(wc -l < /tmp/mapa.txt 2>/dev/null || echo 0)
echo "  $nf frases, $np palavras, $nm no mapa — do FICHEIRO, sem servidor"
[ "$nf" -gt 0 ] && [ "$np" -gt 0 ] && [ "$nf" -eq "$nm" ] || exit 1
exit 0
