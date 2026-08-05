#!/bin/sh
# involucoes.sh — QUEM AFIRMA INVOLUCAO, E QUEM PODE ESTAR A APLICAR O MESMO LADO DUAS VEZES.
#
# O relogio_opera.c mediu que aplicar A MESMA involucao duas vezes num objecto de DOIS
# LADOS devolve a identidade e portanto NAO TESTA o segundo lado. Foi assim que se
# diagnosticou o tresp.c. A pergunta seguinte e' se ele e' o unico.
#
# ESTE FICHEIRO NAO DA' VEREDITO — DA' INVENTARIO. O criterio decisivo (o objecto tem um
# lado ou dois?) nao se decide por grep: decide-se lendo. O que aqui se faz e' estreitar a
# lista para quem vale a pena ler, e deixar o numero escrito para que ninguem o suponha.
#
#   sh tools/involucoes.sh
RAIZ=$(cd "$(dirname "$0")/.." && pwd); cd "$RAIZ" || exit 1
afirma=0; suspeitos=0
for f in tests/*.c banco/*.c; do
    [ -f "$f" ] || continue
    grep -qiE 'ok\(.*(involu|duas vezes|volta ao|devolve o original)' "$f" || continue
    afirma=$((afirma+1))
    # tem DOIS lados no texto? entao dois passos podem nao chegar
    if grep -qiE 'bidual|dois lados|duas involu|periodo 4|período 4|G\^4|F\^4' "$f"; then
        suspeitos=$((suspeitos+1)); echo "  a ler: $f"
    fi
done
echo "  ── $afirma ficheiros afirmam involucao; $suspeitos falam de DOIS LADOS e pedem leitura"
echo "  ── lidos e resolvidos ate' agora: tresp.c (diagnosticado, por corrigir)"
