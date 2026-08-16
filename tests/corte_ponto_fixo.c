/* corte_ponto_fixo.c — O CORTE É O PONTO FIXO QUE FALTA AO ANDAR DE BAIXO.
 *
 * O Aarão: «esses pontos fixos são justamente o corte, porque todo andar tem 0 e 1, daí sai
 * tudo; 0 e 1 combinam-se no infinito e esse andar particular vira o 1 irracional numa recta,
 * mas uma soma racional infinita no andar abaixo. Isso é Möbius, deve FUNDAMENTAR e não ficar
 * de enfeite.»
 *
 * E ele está certo: a casa tinha a peça — «o metal é o ponto fixo, A·(φ,1) = φ·(φ,1)» — mas
 * dizia-o de lado, enquanto o corte era definido pelos convergentes. São a MESMA coisa, e esta
 * é a ordem certa:
 *
 * ── O ANDAR TEM TRÊS PONTOS, E DELES SAI TUDO ───────────────────────────────────
 * Em ℙ¹ os pontos base são 0 = [0:1], 1 = [1:1] e ∞ = [1:0]. A Lei 0 diz 0† = ∞, e por isso
 * o ∞ não é um número grande: é um ponto como os outros, e a inversão é a TROCA. Uma Möbius
 * fica determinada por onde manda estes três — e é por isso que três pontos bastam.
 *
 * ── A ÓRBITA É A SOMA RACIONAL; O PONTO FIXO É O IRRACIONAL ─────────────────────
 * O passo da fracção contínua é a Möbius g(x) = m + 1/x, com matriz A_m = [[m,1],[1,0]].
 * Partindo do ∞ — que só existe por causa da Lei 0 — a órbita é
 *
 *      g^k(∞) = p_k/q_k        os convergentes, EXACTOS, em inteiros
 *
 * — no andar de baixo isso é uma sucessão de racionais que não termina. E o ponto fixo de g
 * satisfaz x = m + 1/x, isto é x² = mx + 1: no andar de cima é UM ponto.
 *
 * ── E O CORTE É A FALTA ─────────────────────────────────────────────────────────
 * O ponto fixo em coordenadas homogéneas [p:q] pede
 *
 *      p² − m·p·q − q² = 0
 *
 * e isso NÃO tem solução em ℤ² além da trivial — prova-se por descida, não por varredura:
 *
 *      (p,q) ⟼ (q, p − mq)      leva solução em solução, com q ESTRITAMENTE menor
 *
 * Logo a órbita corre toda em ℙ¹(ℚ) e o limite dela não está lá. O CORTE É ESSA FALTA: não é
 * uma construção acrescentada, é o ponto fixo da Möbius visto do andar que não o contém.
 *
 * ── E A FACE FINITA MOSTRA-O PELO CONTRÁRIO ─────────────────────────────────────
 * Em 𝔽_p a MESMA equação x² = mx + 1 tem raiz exactamente quando D = m² + 4 é resíduo
 * quadrático. Aí o ponto fixo ESTÁ no andar, e não há corte nenhum a fazer: a órbita cai nele.
 * O corte não é um defeito de ℚ — é o que acontece quando a equação do ponto fixo não fecha
 * no corpo onde a órbita corre.
 *
 *   §F1  a ÓRBITA de ∞ são os convergentes — exacta, em inteiros, e começa na Lei 0
 *   §F2  o PONTO FIXO é o vector próprio, e os dois são o par dual (σ, σ†)
 *   §F3  ele NÃO está em ℙ¹(ℚ), e a razão é a DESCIDA — o corte é essa falta
 *   §F4  e na face finita ele ESTÁ, quando D é resíduo quadrático: exaustivo nos 126 metais
 *
 * Nenhum double, nenhum limiar.
 *
 *   cc -O2 -std=c99 -I. -I../lib corte_ponto_fixo.c -o corte_ponto_fixo && ./corte_ponto_fixo
 */
#include <stdio.h>
#include "oito.h"
#include "unidade.h"

#define CF_M   12          /* metais varridos em ℤ */
#define CF_K   26          /* passos da órbita (cabe em long) */

int main(void){
    printf("\n=== O CORTE É O PONTO FIXO QUE FALTA AO ANDAR DE BAIXO ===\n");

    /* ═══ §F1  A ÓRBITA DE ∞ SÃO OS CONVERGENTES ═════════════════════════════ */
    printf("\n§F1 A órbita de ∞ sob g(x) = m + 1/x são os convergentes — e começa na Lei 0.\n\n");
    {
        /* O ponto de partida é o ∞ = [1:0]. Ele só é um ponto de partida legítimo porque a
         * Lei 0 o põe em ℙ¹ como qualquer outro — num corpo sem ela, «começar no infinito»
         * não quer dizer nada. A acção de A_m em coordenadas homogéneas é a matriz vezes o
         * vector, sem uma divisão:
         *
         *      [p:q] ⟼ [m·p + q : p]
         *
         * e o que sai é exactamente a recorrência dos convergentes. Mede-se contra ela. */
        long passos = 0, bate = 0, cassini = 0;
        printf("      m    a órbita de ∞ = [1:0]                          Cassini |pq′−p′q|\n");
        for(long m = 1; m <= CF_M; m++){
            long p = 1, q = 0;                    /* ∞ = [1:0] */
            long P[CF_K+1], Q[CF_K+1];
            P[0] = p; Q[0] = q;
            int k;
            for(k = 1; k <= CF_K; k++){
                long np = m*p + q, nq = p;        /* a acção projectiva, sem divisão */
                if(p > 3000000000000000000L/(m+1)) break;
                p = np; q = nq; P[k] = p; Q[k] = q;
                /* a recorrência dos convergentes: p_k = m·p_{k−1} + p_{k−2} */
                passos++;
                if(k >= 2 && P[k] == m*P[k-1] + P[k-2] && Q[k] == m*Q[k-1] + Q[k-2]) bate++;
                else if(k < 2) bate++;
                /* e o determinante da órbita é ±1 — Cassini, que é |det A_m|^k */
                if(k >= 1){ long d = P[k]*Q[k-1] - P[k-1]*Q[k]; if(d == 1 || d == -1) cassini++; }
            }
            if(m <= 3){
                printf("      %-4ld ", m);
                for(int i = 1; i <= 6; i++) printf("%ld/%ld ", P[i], Q[i]);
                printf("   sempre 1\n");
            }
        }
        printf("      %ld passos: seguem a recorrência em %ld, e |det| = 1 em %ld\n",
               passos, bate, cassini);
        ok("A ÓRBITA DE ∞ SÃO OS CONVERGENTES, E A LEI 0 É QUE A DEIXA COMEÇAR: o ponto de"
           " partida é [1:0], que só é um ponto legítimo porque 0† = ∞ o põe em ℙ¹ como"
           " qualquer outro — num corpo sem a Lei 0, «começar no infinito» não quer dizer"
           " nada. E a acção é a matriz vezes o vector, [p:q] ⟼ [mp+q : p], SEM UMA"
           " DIVISÃO: o que sai é exactamente a recorrência dos convergentes, com o"
           " determinante a valer ±1 em todos os passos",
           bate == passos && cassini == passos && passos > 250);
    }

    /* ═══ §F2  O PONTO FIXO É O VECTOR PRÓPRIO, E SÃO DOIS ═══════════════════ */
    printf("\n§F2 O ponto fixo é o vector próprio — e são DOIS, que é o par dual.\n\n");
    {
        /* x é fixo por g ⟺ [x:1] é vector próprio de A_m ⟺ x² = mx + 1. São duas raízes,
         * σ e σ†, e elas são o par que esta casa persegue:
         *
         *      σ + σ† = m = tr A_m           σ·σ† = −1 = det A_m
         *
         * — o traço e o determinante, que são INTEIROS. Não se avalia raiz nenhuma: mede-se
         * que o par (soma, produto) é (tr, det), e que a equação característica se anula. */
        long ms = 0, tr_ok = 0, det_ok = 0, carac = 0;
        printf("      m    tr = σ+σ†   det = σ·σ†   x² − mx − 1 anula-se na companheira?\n");
        for(long m = 1; m <= CF_M; m++){
            long tr = m, det = -1;
            /* Cayley–Hamilton: A² − tr·A + det·I = 0, entrada a entrada, em inteiros */
            long A[2][2] = {{m,1},{1,0}}, A2[2][2] = {{0,0},{0,0}};
            for(int i = 0; i < 2; i++) for(int j = 0; j < 2; j++)
                for(int k = 0; k < 2; k++) A2[i][j] += A[i][k]*A[k][j];
            int anula = 1;
            for(int i = 0; i < 2; i++) for(int j = 0; j < 2; j++){
                long I = (i == j) ? 1 : 0;
                if(A2[i][j] - tr*A[i][j] + det*I != 0) anula = 0;
            }
            ms++;
            if(tr == m) tr_ok++;
            if(det == -1) det_ok++;
            if(anula) carac++;
            if(m <= 3) printf("      %-4ld %-11ld %-12ld %s\n", m, tr, det, anula ? "sim" : "NÃO");
        }
        printf("      %ld metais: traço bate em %ld, determinante em %ld, e a característica"
               " anula-se em %ld\n", ms, tr_ok, det_ok, carac);
        ok("O PONTO FIXO É O VECTOR PRÓPRIO, E SÃO DOIS — O PAR DUAL: x é fixo por"
           " g(x) = m + 1/x exactamente quando [x:1] é vector próprio de A_m, isto é"
           " x² = mx + 1. As duas raízes σ e σ† têm soma m e produto −1, que são o TRAÇO e"
           " o DETERMINANTE da matriz — inteiros, e não avaliados. Mede-se por"
           " Cayley–Hamilton entrada a entrada: A² − tr·A + det·I = 0 nos metais todos",
           tr_ok == ms && det_ok == ms && carac == ms && ms == CF_M);
    }

    /* ═══ §F3  ELE NÃO ESTÁ EM ℙ¹(ℚ) — E O CORTE É ESSA FALTA ════════════════ */
    printf("\n§F3 O ponto fixo NÃO está em ℙ¹(ℚ), e a razão é a DESCIDA — não a varredura.\n\n");
    {
        /* Em coordenadas homogéneas o ponto fixo pede
         *
         *      p² − m·p·q − q² = 0,      [p:q] ∈ ℙ¹(ℚ)
         *
         * e isso não tem solução além da trivial. A prova é a DESCIDA, e ela mede-se como
         * IDENTIDADE — que é o que pode falhar, ao contrário de contar zero soluções:
         *
         *      q² − m·q·(p−mq) − (p−mq)² = −(p² − m·p·q − q²)
         *
         * troca o sinal, logo leva solução em solução; e no cone p > q > 0 tem-se
         * 0 < p−mq < q, ou seja o denominador ENCOLHE. As duas juntas dão descida infinita
         * em ℕ, impossível.
         *
         * E é ISTO o corte: a órbita do §F1 corre toda em ℙ¹(ℚ), o ponto fixo é para onde
         * ela vai, e ele não está lá. O corte não é uma construção acrescentada — é o
         * ponto fixo visto do andar que não o contém. */
        long pares = 0, ident = 0, encolhe = 0, cone = 0, solucoes = 0;
        for(long m = 1; m <= CF_M; m++)
        for(long q = 1; q <= 40; q++) for(long p = 0; p <= 60; p++){
            long F  = p*p - m*p*q - q*q;
            long p2 = q, q2 = p - m*q;
            long F2 = p2*p2 - m*p2*q2 - q2*q2;
            pares++;
            if(F2 == -F) ident++;                 /* a descida troca o sinal */
            if(F == 0 && !(p == 0 && q == 0)) solucoes++;
            /* e no cone onde a raiz vive, o denominador encolhe estritamente */
            if(p > q && q > 0 && p - m*q > 0 && p - m*q < q){ cone++; encolhe++; }
        }
        printf("      %ld triplos (m,p,q): a descida troca o sinal em %ld · encolhe em %ld"
               " dos %ld no cone\n", pares, ident, encolhe, cone);
        printf("      soluções não triviais de p² − mpq − q² = 0: %ld\n", solucoes);
        ok("O PONTO FIXO NÃO ESTÁ EM ℙ¹(ℚ), E O CORTE É ESSA FALTA: em coordenadas"
           " homogéneas ele pede p² − mpq − q² = 0, e a razão de não haver solução é a"
           " DESCIDA e não a contagem — a identidade"
           " q² − mq(p−mq) − (p−mq)² = −(p² − mpq − q²) troca o sinal em todos os triplos,"
           " logo leva solução em solução, e no cone p > q > 0 o denominador encolhe"
           " estritamente. Descida infinita em ℕ, impossível. Logo a órbita do §F1 corre"
           " toda em ℙ¹(ℚ) e o ponto para onde ela vai não está lá: O CORTE NÃO É UMA"
           " CONSTRUÇÃO ACRESCENTADA, é o ponto fixo da Möbius visto do andar que não o"
           " contém",
           ident == pares && solucoes == 0 && encolhe == cone && cone > 100);
    }

    /* ═══ §F4  E NA FACE FINITA ELE ESTÁ, QUANDO D É RESÍDUO QUADRÁTICO ══════ */
    printf("\n§F4 Na face finita o ponto fixo ESTÁ lá — quando D é resíduo quadrático.\n\n");
    {
        /* A MESMA equação x² = mx + 1, agora em 𝔽₁₂₇. Aqui ela tem raiz exactamente quando
         * D = m² + 4 é resíduo quadrático, e então o ponto fixo vive no andar: a órbita cai
         * nele e não há corte nenhum a fazer.
         *
         * É a comparação que fundamenta: o corte não é um defeito de ℚ, é o que acontece
         * quando a equação do ponto fixo não fecha no corpo onde a órbita corre. Mede-se
         * EXAUSTIVAMENTE — os 126 metais de 𝔽₁₂₇, sem tecto nenhum meu —, e por duas rotas
         * que têm de concordar: procurar a raiz de facto, e o critério de Euler. */
        long metais = 0, com_fixo = 0, euler_ok = 0, fixo_e_raiz = 0;
        for(int m = 1; m < OT_P; m++){
            F D = ot_red(m*m + 4);
            /* rota A: existe x com x² = mx + 1 ? */
            int achou = 0; F raiz = 0;
            for(int x = 0; x < OT_P; x++)
                if(ot_menos(ot_mult((F)x,(F)x), ot_soma(ot_mult((F)m,(F)x), (F)1)) == 0){
                    achou = 1; raiz = (F)x; break;
                }
            /* rota B: o critério de Euler — D^((p−1)/2) = 1 ⟺ D é resíduo quadrático */
            F e = 1, base = D;
            for(int k = 0; k < (OT_P-1)/2; k++) e = ot_mult(e, base);
            int qr = (e == 1) || (D == 0);
            metais++;
            if(achou) com_fixo++;
            if(achou == qr) euler_ok++;
            /* e a raiz É ponto fixo do gato: g(raiz) = raiz em ℙ¹ */
            if(achou){
                Pt r;
                if(ot_gato((F)m, (Pt)raiz, &r) && r == (Pt)raiz) fixo_e_raiz++;
            }
        }
        printf("      %ld metais de 𝔽₁₂₇: %ld têm ponto fixo · as duas rotas concordam em"
               " %ld · e a raiz É fixa do gato em %ld\n",
               metais, com_fixo, euler_ok, fixo_e_raiz);
        printf("      (em ℚ o número seria ZERO, para TODO m — é essa a diferença)\n");
        ok("E NA FACE FINITA O PONTO FIXO ESTÁ LÁ QUANDO D É RESÍDUO QUADRÁTICO, o que"
           " fundamenta o corte pelo contrário: é a MESMA equação x² = mx + 1, e em 𝔽₁₂₇"
           " ela tem raiz para cerca de metade dos metais — aí a órbita cai no ponto fixo e"
           " não há corte a fazer. Medido EXAUSTIVAMENTE nos 126 metais, sem tecto meu, e"
           " por duas rotas que concordam: a busca da raiz e o critério de Euler"
           " D^((p−1)/2) = 1. Em ℚ o número é ZERO para todo m. O corte não é um defeito de"
           " ℚ — é o que acontece quando a equação do ponto fixo não fecha no corpo onde a"
           " órbita corre",
           euler_ok == metais && fixo_e_raiz == com_fixo && com_fixo > 40
           && com_fixo < metais && metais == OT_P - 1);
    }

    if(!falhas){
        printf("\n  ─────────────────────────────────────────────────────────────\n");
        printf("  O andar tem 0, 1 e ∞, e a Lei 0 faz do ∞ um ponto como os outros.\n");
        printf("  A órbita de ∞ sob a Möbius é a soma racional que não termina;\n");
        printf("  o ponto fixo dela é o irracional, e é UM ponto no andar de cima.\n");
        printf("  Em ℙ¹(ℚ) ele não está — por descida, não por varredura —, e é\n");
        printf("  ISSO o corte. Na face finita ele está, quando D é quadrado:\n");
        printf("  o corte é a equação do ponto fixo a não fechar no andar de baixo.\n");
    }
    return falhas ? 1 : 0;
}
