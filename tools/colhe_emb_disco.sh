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
# colhe_emb_disco.sh — os vetores dos termos, DO FICHEIRO. Sem servidor.
#
# O quarto medidor que nunca mediu: o encaixa.c pede /tmp/emb.txt e dizia-o na cara
# ("encaixa: sem /tmp/emb.txt — corre tools/colhe_emb.sh"), enquanto a atestação o dava
# por bom com exit 1.
#
# O colhe_emb.sh original pede um embedding por HTTP a cada termo. Aqui usa-se o mesmo
# COLHE_FRASES do forward — leitura byte a byte do gguf, memória O(1) — e converte-se
# para o formato que o encaixa lê: nome<TAB>v1 v2 v3 …, em decimal.
#
# A conversão do padrão de bits para decimal é aqui e não no forward de propósito: o
# forward escreve BITS porque é o que atravessa exato; quem quiser decimal que o peça.
#
#   sh tools/colhe_emb_disco.sh
set -u
RAIZ=$(cd "$(dirname "$0")/.." && pwd)
cd "$RAIZ" || exit 1
BLOBS=${BLOBS:-/usr/share/ollama/.ollama/models/blobs}
SAIDA=${SAIDA:-/tmp/emb.txt}

# os MESMOS termos do colhe original
TERMOS="rei rainha homem mulher torre castelo ouro prata zero um cifra numero corpo dual simetria antissimetria"
: > /tmp/emb_in.txt
for t in $TERMOS; do printf '%s\n' "$t" >> /tmp/emb_in.txt; done

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

[ -x /tmp/forward_colhe ] && [ /tmp/forward_colhe -nt banco/forward.c ] || \
    cc -O2 -std=c99 -w -Ilib -I. -o /tmp/forward_colhe banco/forward.c -lm 2>/dev/null
[ -x /tmp/forward_colhe ] || { echo "  o forward não compilou"; exit 1; }

GGUF="$EMB" COLHE_FRASES=/tmp/emb_in.txt SAIDA=/tmp/emb_hex.txt /tmp/forward_colhe >/dev/null 2>&1
[ -s /tmp/emb_hex.txt ] || { echo "  o forward não produziu vetores"; exit 1; }

# hex -> decimal, com o nome à frente. O encaixa lê "%.9g", e é isso que se escreve.
python3 - "$SAIDA" <<'PY'
import struct, sys
saida = sys.argv[1]
termos = [l.strip() for l in open("/tmp/emb_in.txt", encoding="utf-8") if l.strip()]
with open("/tmp/emb_hex.txt", encoding="utf-8") as fh, open(saida, "w", encoding="utf-8") as fo:
    for termo, linha in zip(termos, fh):
        vals = [struct.unpack("<f", struct.pack("<I", int(h, 16)))[0] for h in linha.split()]
        fo.write(termo + "\t" + " ".join("%.9g" % v for v in vals) + "\n")
print(f"  {len(termos)} termos -> {saida}, do FICHEIRO, sem servidor")
PY
exit 0
