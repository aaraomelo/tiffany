/* condicao_pisot.c — A CONDIÇÃO É «n ≤ t», E É NECESSÁRIA E SUFICIENTE.
 *
 * O paper afirmava que a unidade |N(σ)| = 1 é NECESSÁRIA ao encaixe, e provava-o com
 * x² − 2x − 4 (determinante −4, e o mecanismo falha). O contraexemplo está certo; a
 * CONCLUSÃO estava a mais, e o que a derruba é uma linha:
 *
 *      x² − 3x − 3   tem |det| = 3 ≠ 1   E É PISOT   (|σ†| = 0,7913)
 *
 * Logo a unidade não é necessária — é apenas o caso extremo mínimo. A condição que
 * classifica sai da álgebra, e sai INTEIRA. Para x² − tx − n com t, n ≥ 1:
 *
 *      σ† = (t − √(t²+4n))/2 ,      |σ†| = (√(t²+4n) − t)/2
 *
 *      |σ†| < 1  ⟺  √(t²+4n) < t+2  ⟺  t² + 4n < (t+2)²  ⟺  4n < 4t + 4
 *
 *                             ⟺   n ≤ t
 *
 * O termo constante não excede o traço. E a unidade (n = 1) satisfaz sempre, para todo
 * t ≥ 1 — é por isso que a família metálica é toda Pisot, e é por isso que ela é o
 * EXTREMO e não a regra.
 *
 * A fronteira exacta é n = t+1, e ela não é acidente: aí |σ†| = 1 exactamente, e o
 * polinómio FACTORIZA,
 *
 *      x² − tx − (t+1) = (x − (t+1))(x + 1)
 *
 * — a raiz de módulo 1 é −1, o polinómio é redutível, e o irracional desapareceu. A
 * condição não corta o contínuo ao meio: corta exactamente onde ele deixa de existir.
 *
 *   §N1  A EQUIVALÊNCIA: |σ†| < 1 ⟺ n ≤ t, pelas duas rotas, sem avaliar raiz
 *   §N2  A UNIDADE É SUFICIENTE E NÃO NECESSÁRIA — e o contraexemplo à necessidade
 *   §N3  A FRONTEIRA n = t+1: |σ†| = 1 exacto, e o polinómio factoriza
 *   §N4  E O VAZAMENTO, ANDAR A ANDAR, EM INTEIROS: o teste de f(±1) no traço
 *
 * Nenhum double, nenhum limiar.
 *
 *   cc -O2 -std=c99 -I. -I../lib condicao_pisot.c -o condicao_pisot && ./condicao_pisot
 */
#include <stdio.h>
#include "unidade.h"

#define CP_T   60          /* traços varridos: t = 1..60 */
#define CP_N  140          /* termos constantes: n = 1..140 */
#define CP_K   14          /* andares do vazamento */

int main(void){
    printf("\n=== A CONDIÇÃO DE PISOT É «n ≤ t» — necessária E suficiente ===\n");

    /* ═══ §N1  A EQUIVALÊNCIA, PELAS DUAS ROTAS ══════════════════════════════ */
    printf("\n§N1 |σ†| < 1 ⟺ n ≤ t, e nenhuma das rotas avalia raiz.\n\n");
    {
        /* Rota A — o critério simples:      n ≤ t
         * Rota B — a desigualdade da raiz:   t² + 4n < (t+2)²
         *
         * São a mesma coisa por três linhas de álgebra, e é isso que se mede: as duas
         * rotas têm de coincidir em TODO par (t,n), sem excepção. Uma discordância
         * única mataria a equivalência, e é essa a asserção.
         *
         * E mede-se também que a fronteira é ATINGIDA — que existem pares dos dois
         * lados —, senão a equivalência valeria por vacuidade. */
        long pares = 0, concorda = 0, dentro = 0, fora = 0, na_borda = 0;
        for(long t = 1; t <= CP_T; t++) for(long n = 1; n <= CP_N; n++){
            long D = t*t + 4*n;
            long A = (n <= t);                        /* rota A */
            long B = (D < (t+2)*(t+2));               /* rota B */
            long E = (D == (t+2)*(t+2));              /* |σ†| = 1 exacto */
            pares++;
            if(A == B) concorda++;
            if(A) dentro++; else fora++;
            if(E) na_borda++;
        }
        printf("      %ld pares (t,n): as duas rotas concordam em %ld\n", pares, concorda);
        printf("      e a fronteira é atingida dos dois lados: %ld com n ≤ t, %ld sem,"
               " e %ld exactamente em |σ†| = 1\n", dentro, fora, na_borda);
        ok("A CONDIÇÃO É «n ≤ t», E É NECESSÁRIA E SUFICIENTE: as duas rotas — o critério"
           " n ≤ t e a desigualdade da raiz t²+4n < (t+2)² — coincidem em todos os pares"
           " varridos, o que é a equivalência |σ†| < 1 ⟺ n ≤ t, obtida sem avaliar raiz"
           " nenhuma. E não vale por vacuidade: há pares dos DOIS lados, e a fronteira"
           " |σ†| = 1 é atingida exactamente",
           concorda == pares && dentro > 0 && fora > 0 && na_borda == CP_T
           && pares == CP_T*CP_N);
    }

    /* ═══ §N2  A UNIDADE É SUFICIENTE, E NÃO NECESSÁRIA ══════════════════════ */
    printf("\n§N2 |det| = 1 é suficiente e não necessária — e o det não classifica.\n\n");
    {
        /* A unidade é n = 1, e n = 1 ≤ t para todo t ≥ 1: SUFICIENTE, sempre.
         *
         * Mas não é necessária, e a prova é exibir os dois lados COM O MESMO |det|
         * seria impossível — o que se exibe é mais forte: para cada |det| = n ≥ 2 há
         * polinómios Pisot (t ≥ n) e não-Pisot (t < n). Logo |det| NÃO classifica:
         * conhecer o determinante não decide o encaixe, e é preciso o traço. */
        long ns = 0, suf = 0, tem_pisot = 0, tem_nao = 0;
        printf("      |det| = n   há Pisot (t ≥ n)?   há não-Pisot (t < n)?   exemplo Pisot\n");
        for(long n = 1; n <= 12; n++){
            long p = 0, q = 0, tp = 0;
            for(long t = 1; t <= CP_T; t++){
                if(n <= t){ p++; if(!tp) tp = t; } else q++;
            }
            ns++;
            if(n == 1 && q == 0) suf++;               /* n=1: Pisot para TODO t */
            if(p > 0) tem_pisot++;
            if(q > 0) tem_nao++;
            if(n <= 4 || n == 12)
                printf("      %-11ld %-21s %-23s x² − %ldx − %ld\n", n,
                       p ? "sim" : "NÃO", q ? "sim" : "não (n=1: sempre Pisot)", tp, n);
        }
        printf("      e o contraexemplo à NECESSIDADE, explícito: x² − 3x − 3 tem"
               " |det| = 3 ≠ 1 e é Pisot (3 ≤ 3)\n");
        printf("      enquanto x² − 2x − 4 tem |det| = 4 e NÃO é Pisot (4 > 2) —"
               " mesmo lado do det, lados opostos do encaixe\n");
        ok("A UNIDADE É SUFICIENTE E NÃO NECESSÁRIA, E O DETERMINANTE NÃO CLASSIFICA:"
           " n = 1 dá Pisot para TODO traço, o que faz da família metálica o extremo e"
           " não a regra — mas para todo n ≥ 2 há polinómios dos DOIS lados, consoante o"
           " traço. O par decisivo é x² − 3x − 3 (|det| = 3, É Pisot) contra x² − 2x − 4"
           " (|det| = 4, NÃO é): conhecer o determinante não decide o encaixe. É por isso"
           " que a condição tem de nomear os dois coeficientes, e não só um",
           suf == 1 && tem_pisot == 12 && tem_nao == 11 && ns == 12);
    }

    /* ═══ §N3  A FRONTEIRA n = t+1 ═══════════════════════════════════════════ */
    printf("\n§N3 Em n = t+1 o módulo é 1 EXACTO — e o polinómio factoriza.\n\n");
    {
        /* A fronteira não é um limiar escolhido: é onde o objecto muda de natureza.
         *
         *      n = t+1  ⟹  D = t² + 4t + 4 = (t+2)²  — quadrado perfeito
         *               ⟹  x² − tx − (t+1) = (x − (t+1))(x + 1)
         *
         * A raiz de módulo 1 é −1, o polinómio é REDUTÍVEL, e não há irracional nenhum
         * para encaixar. A condição não corta o contínuo ao meio: corta exactamente
         * onde ele deixa de existir. Mede-se a factorização por expansão em inteiros. */
        long ts = 0, quadrado = 0, factoriza = 0, raiz_menos1 = 0;
        printf("      t     n=t+1   D = t²+4n   é (t+2)²?   (x−(t+1))(x+1) = x²−tx−n ?\n");
        for(long t = 1; t <= CP_T; t++){
            long n = t + 1, D = t*t + 4*n;
            /* a expansão: (x−(t+1))(x+1) = x² + (1−(t+1))x − (t+1) = x² − tx − (t+1) */
            long coef1 = 1 - (t+1), coef0 = -(t+1);
            ts++;
            if(D == (t+2)*(t+2)) quadrado++;
            if(coef1 == -t && coef0 == -n) factoriza++;
            /* e −1 é raiz: (−1)² − t(−1) − n = 1 + t − (t+1) = 0 */
            if(1 + t - n == 0) raiz_menos1++;
            if(t <= 3 || t == CP_T)
                printf("      %-5ld %-7ld %-11ld %-11s %s\n", t, n, D,
                       D == (t+2)*(t+2) ? "sim" : "NÃO",
                       (coef1 == -t && coef0 == -n) ? "sim" : "NÃO");
        }
        printf("      %ld traços: D é quadrado perfeito em %ld, factoriza em %ld,"
               " e −1 é raiz em %ld\n", ts, quadrado, factoriza, raiz_menos1);
        ok("E A FRONTEIRA NÃO É UM LIMIAR ESCOLHIDO — É ONDE O OBJECTO DEIXA DE EXISTIR:"
           " em n = t+1 o discriminante é o quadrado perfeito (t+2)², logo |σ†| = 1"
           " EXACTAMENTE, e o polinómio factoriza em (x−(t+1))(x+1) — verificado por"
           " expansão em inteiros em todos os traços. A raiz de módulo 1 é −1, o"
           " polinómio é REDUTÍVEL, e não há irracional nenhum para encaixar. A condição"
           " corta exactamente onde o irracional acaba",
           quadrado == ts && factoriza == ts && raiz_menos1 == ts && ts == CP_T);
    }

    /* ═══ §N4  O VAZAMENTO ANDAR A ANDAR, EM INTEIROS ════════════════════════ */
    printf("\n§N4 E o vazamento mede-se sem raiz: f(1) < 0 e f(−1) > 0 sobre o traço.\n\n");
    {
        /* σ^k e (σ†)^k são as raízes de  y² − t_k y + (−n)^k,  onde
         *
         *      t_k = σ^k + (σ†)^k       com  t_0 = 2, t_1 = t,  t_k = t·t_{k−1} + n·t_{k−2}
         *
         * é o traço, INTEIRO por Newton. E como σ^k > 1, o menor em módulo está em
         * (−1,1) exactamente quando 1 fica ENTRE as raízes e −1 fica ABAIXO de ambas:
         *
         *      f(1) = 1 − t_k + (−n)^k < 0        e       f(−1) = 1 + t_k + (−n)^k > 0
         *
         * Tudo inteiro, tudo exacto, e nenhuma raiz avaliada. É o teste de sinal a
         * fazer o trabalho que um limiar faria — e a fazê-lo sem escolher número nenhum.
         *
         * A tese: o teste concorda com «n ≤ t» em TODO andar. Se concordasse só em
         * alguns, a condição classificaria o polinómio e não o mecanismo. */
        long casos = 0, bate = 0, vaza = 0, encolhe = 0;
        printf("      polinómio          n ≤ t?   vazamento < 1 nos %d andares?\n", CP_K);
        long tab[8][2] = {{1,1},{2,1},{3,1},{3,3},{5,4},{2,4},{1,3},{4,7}};
        for(int c = 0; c < 8; c++){
            long t = tab[c][0], n = tab[c][1];
            long a = 2, b = t, pn = -n, todos = 1, k;
            for(k = 2; k <= CP_K; k++){
                long novo = t*b + n*a;
                a = b; b = novo; pn *= -n;
                /* guarda: os traços crescem, e os testes têm de caber */
                if(b > 1000000000L || pn > 1000000000L || pn < -1000000000L) break;
                long f1 = 1 - b + pn, fm = 1 + b + pn;
                casos++;
                long dentro = (f1 < 0 && fm > 0);
                if(dentro == (n <= t)) bate++;
                if(dentro) encolhe++; else vaza++;
                if(!dentro) todos = 0;
            }
            printf("      x² − %ldx − %-8ld %-8s %s (até k = %ld)\n", t, n,
                   (n <= t) ? "sim" : "não", todos ? "sim" : "não", k-1);
        }
        printf("      %ld andares medidos: o teste de sinal bate com «n ≤ t» em %ld"
               " · encolhe em %ld, vaza em %ld\n", casos, bate, encolhe, vaza);
        ok("E O VAZAMENTO MEDE-SE ANDAR A ANDAR SEM AVALIAR RAIZ NENHUMA: σ^k e (σ†)^k"
           " são as raízes de y² − t_k y + (−n)^k com t_k o traço inteiro de Newton, e"
           " |(σ†)^k| < 1 é exactamente f(1) < 0 e f(−1) > 0 — um teste de SINAL sobre"
           " inteiros, onde um limiar teria escolhido um número. E ele concorda com"
           " «n ≤ t» em todos os andares dos oito polinómios, incluindo os dois lados da"
           " fronteira: a condição classifica o MECANISMO, e não só o polinómio",
           bate == casos && encolhe > 0 && vaza > 0 && casos > 60);
    }

    if(!falhas){
        printf("\n  ─────────────────────────────────────────────────────────────\n");
        printf("  |σ†| < 1  ⟺  n ≤ t.  O termo constante não excede o traço.\n");
        printf("  A unidade n = 1 é o caso extremo mínimo — suficiente para todo t,\n");
        printf("  e por isso a família metálica é toda Pisot. NÃO é necessária:\n");
        printf("  x² − 3x − 3 tem |det| = 3 e encaixa. O determinante não classifica.\n");
        printf("  E em n = t+1 o polinómio factoriza: a condição corta onde o\n");
        printf("  irracional acaba, e não a meio dele.\n");
    }
    return falhas ? 1 : 0;
}
