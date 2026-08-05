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
cat <<'NOTA'
  ── LIDOS ATE' AGORA, com veredito e razao:

     tresp.c        DEFEITO  aplica a MESMA operacao duas vezes num objecto de dois
                             lados; residuo 0,402 contra controlo 0,421 e' meia
                             orbita, nao ruido. Diagnosticado, POR CORRIGIR.

     bidual.c       LIMPO    k -> n-k -> k e' UMA involucao de periodo 2, e dois
                             passos sao os certos. Bidualidade de Poincare, nao os
                             dois lados — o nome engana, a medida esta' bem.

     quatro.c       LIMPO    e melhor que limpo: testa cada regua consigo (periodo 2)
                             E o FECHO do produto de duas quaisquer. E' o grupo de
                             Klein verificado — faz exactamente o que o tresp nao faz.

     pontofixo.c    LIMPO    nu e' bijeccao E involucao, medidas em separado, com os
                             pontos fixos contados. Periodo 2 provado, nao suposto.

     furos.c        LIMPO    sigma.sigma' = -1, involucao COM CONTROLO explicito
                             (somar 1 nao e' involucao, e mede-se que nao e'). E' a
                             involucao de referencia — o §F4 que os outros citam.

     hurwitz.c      LIMPO    a conjugacao em todo andar, e diz ser a MESMA do furos
                             §F4: objecto testado uma vez e reutilizado, nao repetido.

     fusao.c        LIMPO    dualizar duas vezes devolve na fusao — outra vez o §F4.

     dual_cadeia.c  LIMPO    e testa DOIS NIVEIS: reverter tem "ordem exactamente 2",
                             e det(A.A) = +1 — verifica o PAR, nao so' um lado.

     espelho.c      N/A      FALSO POSITIVO do filtro: "nada conta duas vezes" e' sobre
                             soma e uniao, nao e' afirmacao de involucao nenhuma.

     matricial.c    LIMPO    N^2 = I para toda regua — o periodo e' PROVADO, nao suposto.
     metalica.c     LIMPO    J.J = id em toda a janela, residuo exactamente zero.
     cantor.c       LIMPO    nu.nu = id e reverte exacto, contado em todas as voltas.
     gauss.c        LIMPO    e o exemplar: mede a ORDEM do operador (ordem_A == 2) em
                             vez de supor que ela e' 2. E' o que faltava ao tresp.
     numerica.c     LIMPO    e do tipo bom: mede que subir e descer a mesma
                             percentagem NAO volta (da' 75) — um CONTRA-exemplo
                             deliberado, que e' o controlo que o tresp nao tem.
     natural.c      N/A      os escalares que a regua tolera sao +-1: e' o lambda^2=1
                             do thm:transporte, nao um teste de involucao composta.

     fecha.c        LIMPO    o mais completo: involucao, norma multiplicativa E
                             N = x.nu(x), as tres juntas nos 6 corpos.
     metrica.c      LIMPO    nu e' involucao E isometria — duas propriedades, 169 reguas.
     escada.c       LIMPO    f = a/x e' involucao, 108 pares, zero discordancias.
     palavra.c      LIMPO    A_0.A_0 = I verificado ENTRADA A ENTRADA, nao pelo traco.
     xx.c           LIMPO    nu.nu = id com o PAR: as duas raizes tem o mesmo x.ln x.
     base.c         LIMPO    o conjugado e' uma dobra — duas vezes desdobra, sem residuo.
     teletransporte.c LIMPO  sair da fita devolve o original, residuo 0.

  ── O PADRAO, ao fim de 22: os limpos PROVAM a ordem (ou reutilizam uma involucao
     ja' provada); o tresp SUPOE-A. Nenhum outro defeito encontrado ate' agora.

  ── falta ler: 5    (conversa, sql, catalogo, corpo_fisico, encanamento) — mais os
     cinco desta sessao (dissipa, hipercubo32, spinor32, trialidade, relogio_opera),
     escritos ja' com o criterio e por isso nao contados como divida.
  ── e o filtro tem falsos positivos: conta-os antes de os crer.
NOTA
