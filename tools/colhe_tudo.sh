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
# colhe_tudo.sh — OS VETORES SAEM DO FICHEIRO. Nao ha' servidor nenhum a acordar.
#
# O Aarao: "nao precisa estar acordado nada, e' um arquivo como qualquer outro,
# navegavel; a unica era o forward que ja' tem — e' o espelho, a involucao da'."
#
# E' literal, e o proprio forward.c ja' o dizia no cabecalho: NAO HA' output.weight — o
# lm_head SAO OS PROPRIOS EMBEDDINGS (tied). A mesma matriz entra e sai, E na ida e E^T
# na volta. E' a involucao — e por isso ler token_embd.weight CHEGA: nao ha' segunda
# matriz a que ir buscar coisa nenhuma.
#
# O modelo e' um ficheiro no disco. Colher e' navega-lo:
#
#     COLHE=/tmp/vetores.txt N=20 GGUF=<blob> ./forward
#
# Nada de ollama serve, nada de HTTP, nada de esperar. E os medidores que passavam meses
# a dizer "NAO MEDIU" passam a medir sempre — inclusive depois de um reboot, que era o
# buraco que ficava.
#
#   sh tools/colhe_tudo.sh          colhe o que falta
#   sh tools/colhe_tudo.sh --forca  colhe outra vez
#
set -u
RAIZ=$(cd "$(dirname "$0")/.." && pwd)
FORCA=0; [ "${1:-}" = "--forca" ] && FORCA=1
BLOBS=${BLOBS:-/usr/share/ollama/.ollama/models/blobs}

echo
echo "  ─── COLHER DO FICHEIRO ───────────────────────────────────────────────"

# o gguf com embeddings de 768: procura-se pelo magico, nao pelo nome do sha
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
if [ -z "$EMB" ]; then
    echo "  NAO ACHEI um gguf de embeddings em $BLOBS"
    echo "  aponte-o com  EMB=<caminho>  ou  BLOBS=<pasta>"
    echo
    exit 1
fi
echo "  ficheiro: $(basename "$EMB" | cut -c1-24)...  ($(( $(stat -c%s "$EMB") / 1048576 )) MB)"

cd "$RAIZ" || exit 1
# recompila se o fonte for MAIS NOVO que o binario — sem isto fica-se com um
# forward antigo, sem os modos novos, e o colhedor devolve zero em silencio
[ -x /tmp/forward_colhe ] && [ /tmp/forward_colhe -nt banco/forward.c ] || \
    cc -O2 -std=c99 -w -Ilib -I. -o /tmp/forward_colhe banco/forward.c -lm 2>/dev/null
if [ ! -x /tmp/forward_colhe ]; then echo "  o forward nao compilou"; exit 1; fi

feitos=0; saltados=0
if [ "$FORCA" -eq 0 ] && [ -s /tmp/vetores.txt ]; then
    echo "  [ja la esta]  vetores (/tmp/vetores.txt)"; saltados=$((saltados+1))
else
    GGUF="$EMB" COLHE=/tmp/vetores.txt N=${N:-20} /tmp/forward_colhe >/dev/null 2>&1
    if [ -s /tmp/vetores.txt ]; then
        echo "  [colhido]     vetores -> /tmp/vetores.txt  ($(wc -l < /tmp/vetores.txt) linhas)"
        feitos=$((feitos+1))
    else echo "  FALHOU a colher vetores"; fi
fi

# o dualcifra tambem ja' le' do ficheiro
if [ "$FORCA" -eq 0 ] && [ -s /tmp/frases.txt ] && [ -s /tmp/palavras.txt ]; then
    echo "  [ja la esta]  frases e palavras (dualcifra)"; saltados=$((saltados+1))
else
    bash tools/colhe_dualcifra_disco.sh >/dev/null 2>&1
    if [ -s /tmp/frases.txt ] && [ -s /tmp/palavras.txt ]; then
        echo "  [colhido]     frases e palavras -> /tmp/frases.txt, /tmp/palavras.txt"
        feitos=$((feitos+1))
    else echo "  FALHOU a colher frases/palavras"; fi
fi

# /tmp/emb.txt e' lido por TRES medidores, e eles NAO pedem a mesma coisa:
#
#   encaixa    mede ESTRUTURA de vizinhanca  -> os vetores do ficheiro chegam
#   semantico  mede SIGNIFICADO              -> NAO chegam, e ele falha com razao
#
# a media dos embeddings dos BYTES de "rei" nao e' o embedding semantico de "rei" — o
# nomic poe doze camadas de atencao entre uma coisa e outra. ler token_embd.weight da' a
# estrutura e nao da' o significado, porque o significado esta' nas camadas.
#
# logo: PREFERE-SE O MODELO A RESPONDER quando ele existe, e cai-se para o ficheiro quando nao.
# e diz-se QUAL foi usado, porque a diferenca e' mensuravel e nao e' de gosto.
if [ "$FORCA" -eq 0 ] && [ -s /tmp/emb.txt ]; then
    echo "  [ja la esta]  termos (encaixa, semantico)"; saltados=$((saltados+1))
elif curl -s -m 5 http://localhost:11434/api/tags >/dev/null 2>&1; then
    bash tools/colhe_emb.sh >/dev/null 2>&1
    if [ -s /tmp/emb.txt ]; then
        echo "  [colhido]     termos -> /tmp/emb.txt  (do MODELO VIVO: tem semantica)"
        feitos=$((feitos+1))
    else echo "  FALHOU a colher termos do modelo"; fi
else
    bash tools/colhe_emb_disco.sh >/dev/null 2>&1
    if [ -s /tmp/emb.txt ]; then
        echo "  [colhido]     termos -> /tmp/emb.txt  (do FICHEIRO: estrutura sim,"
        echo "                SEMANTICA NAO — o semantico.c vai falhar, e com razao)"
        feitos=$((feitos+1))
    else echo "  FALHOU a colher termos"; fi
fi

# so' este ainda pede o modelo a responder — a base do protocolo nao e' embeddings, e' uma
# conversa com refinamento, e isso nao sai de uma matriz.
for par in "/tmp/protocolo_base.tsv:tools/protocolo.sh:a base do protocolo (protocolo)"; do
    alvo=${par%%:*}; resto=${par#*:}; cmd=${resto%%:*}; desc=${resto#*:}
    if [ "$FORCA" -eq 0 ] && [ -s "$alvo" ]; then
        echo "  [ja la esta]  $desc"; saltados=$((saltados+1)); continue
    fi
    if curl -s -m 5 http://localhost:11434/api/tags >/dev/null 2>&1; then
        echo "  [a colher]    $desc  (ainda pede o modelo a responder)"
        ( bash "$cmd" ) >/dev/null 2>&1 && [ -s "$alvo" ] && feitos=$((feitos+1))
    else
        echo "  [em falta]    $desc — precisa do modelo a responder, e ele nao esta'"
    fi
done

echo
echo "  $feitos colhidos, $saltados ja existiam."
echo "  agora:  bash tools/bateria.sh"
echo
exit 0
