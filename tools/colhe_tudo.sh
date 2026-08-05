#!/bin/bash
# colhe_tudo.sh — ACORDA O DOADOR UMA VEZ e colhe tudo o que os medidores pedem.
#
# TRÊS medidores desta bateria não medem sem dados do doador — e os três passaram meses
# a dizê-lo na cara ("NÃO MEDIU") enquanto a tabela de atestações os dava por verdes:
#
#   transfusao_real   pede /tmp/vetores.txt      -> tools/colhe_transfusao.sh
#   dualcifra         pede /tmp/frases.txt       -> tools/colhe_dualcifra.sh
#   protocolo         pede a base do protocolo   -> tools/protocolo.sh
#
# Os dados ficam em /tmp e NÃO estão versionados, de propósito: o doador tem de estar
# ACORDADO para a transfusão ser transfusão. Congelar os vetores no git faria os
# medidores medirem um cadáver — passariam sempre, e não provariam nada.
#
# O preço disso é que depois de um reboot eles voltam a não medir. Este script é o que
# torna isso um comando em vez de uma arqueologia.
#
#   sh tools/colhe_tudo.sh          colhe o que falta
#   sh tools/colhe_tudo.sh --forca  colhe tudo outra vez, mesmo que já exista
#
# Saída 0 se tudo ficou pronto, 1 se o doador está a dormir.
set -u
RAIZ=$(cd "$(dirname "$0")/.." && pwd)
FORCA=0; [ "${1:-}" = "--forca" ] && FORCA=1

echo
echo "  ─── COLHER DO DOADOR ─────────────────────────────────────────────────"

if ! curl -s -m 5 http://localhost:11434/api/tags >/dev/null 2>&1; then
    echo "  O DOADOR ESTÁ A DORMIR — nenhum ollama em localhost:11434."
    echo "  Acorde-o com  ollama serve  e corra outra vez."
    echo "  (sem ele, três medidores dizem NÃO MEDIU e a bateria conta-os como falha —"
    echo "   o que é honesto: não medir não é passar.)"
    echo
    exit 1
fi
echo "  doador acordado."
echo

feitos=0; saltados=0
colhe(){ # $1 = ficheiro-testemunha  $2 = descrição  $3... = comando
    alvo="$1"; desc="$2"; shift 2
    if [ "$FORCA" -eq 0 ] && [ -s "$alvo" ]; then
        echo "  [já lá está]  $desc  ($alvo)"
        saltados=$((saltados+1)); return
    fi
    echo "  [a colher]    $desc"
    if ( cd "$RAIZ" && "$@" ) >/dev/null 2>&1 && [ -s "$alvo" ]; then
        echo "                pronto -> $alvo"
        feitos=$((feitos+1))
    else
        echo "                FALHOU — $desc não produziu $alvo"
    fi
}

colhe /tmp/vetores.txt  "vetores do doador (transfusao_real)" bash tools/colhe_transfusao.sh
colhe /tmp/frases.txt   "frases e palavras (dualcifra)"       bash tools/colhe_dualcifra.sh
colhe /tmp/protocolo_base.tsv "a base do protocolo (protocolo)"     bash tools/protocolo.sh

echo
echo "  $feitos colhidos, $saltados já existiam."
echo "  agora:  bash tools/bateria.sh"
echo
exit 0
