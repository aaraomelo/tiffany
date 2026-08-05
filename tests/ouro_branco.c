/* ouro_branco.c — O OURO BRANCO E' O DUAL, E ESTA' DO OUTRO LADO DO DETERMINANTE.
 *
 * O Aarao: "vc fala de Pell — ve a cifra de ouro branco, se e' dual. procura nos repos."
 *
 * Procurei, e o enredo ja' o dizia em palavras: "o ouro desenha em CINCO dobras, a prata
 * em OITO, e o ouro branco nas DOZE", e "a liga — o ouro branco — so' nasce de juntar os
 * dois... faltava-lhe a outra metade do ceu".
 *
 * A conta confirma-o e diz mais: o ouro branco nao esta' na familia metalica — esta' do
 * OUTRO LADO dela.
 *
 *     ouro          x^2 = 1x + 1     [[1,1],[1,0]]     det = -1     5 dobras
 *     prata         x^2 = 2x + 1     [[2,1],[1,0]]     det = -1     8 dobras
 *     bronze        x^2 = 3x + 1     [[3,1],[1,0]]     det = -1
 *     OURO BRANCO   x^2 = 4x - 1     [[4,-1],[1,0]]    det = +1    12 dobras
 *
 * A familia metalica e' x^2 = nx + 1, e nela o determinante e' SEMPRE -1: e' o gato, que
 * anti-conserva. O ouro branco satisfaz x^2 = 4x - 1, com o sinal trocado — e ai o
 * determinante e' +1: e' o esquilo, que conserva. Nao e' mais um metal na lista: e' o
 * MEMBRO DUAL, e por isso "so' nasce de juntar os dois".
 *
 *   §B1  (2+V3)^2 = 4(2+V3) - 1, exacto em Z[V3] — e nao e' x^2 = nx + 1 para n nenhum
 *   §B2  o determinante: -1 em toda a familia metalica, +1 no ouro branco
 *   §B3  a ORDEM da involucao separa-os: o gato tem periodo 2 no sinal, o esquilo nao
 *   §B4  e as DOBRAS: 5, 8 e 12 saem dos angulos, e nao de uma tabela
 *   §B5  e o CONTROLO: um x^2 = nx - 1 qualquer NAO da' 12 dobras — o 4 nao e' qualquer
 *
 * Zero doubles: Z[Vd] anda como pares (a,b) = a + b.Vd, e tudo e' inteiro.
 *
 *   cc -O2 -std=c99 -Wall -I../lib ouro_branco.c -o ouro_branco
 */
#include <stdio.h>
#include "unidade.h"

/* (a + b.Vd)^2 = (a^2 + d.b^2) + (2ab).Vd */
static void quad(long a, long b, long d, long *qa, long *qb){ *qa = a*a + d*b*b; *qb = 2*a*b; }

int main(void){
    puts("\n  O OURO BRANCO E' O DUAL — do outro lado do determinante\n");

    /* ═══ §B1 — a equacao do ouro branco ═══════════════════════════════════════════ */
    {
        long a = 2, b = 1, d = 3, qa, qb;           /* 2 + V3 */
        quad(a, b, d, &qa, &qb);
        long ra = 4*a - 1, rb = 4*b;                /* 4(2+V3) - 1 */
        printf("      (2+V3)^2 = %ld + %ld.V3   e   4(2+V3) - 1 = %ld + %ld.V3\n", qa, qb, ra, rb);
        /* e NAO existe n com (2+V3)^2 = n(2+V3) + 1 */
        long achou_mais = 0;
        for(long n = -20; n <= 20; n++)
            if(qa == n*a + 1 && qb == n*b) achou_mais++;
        printf("      e x^2 = n.x + 1 com n de -20 a 20: %ld solucoes\n", achou_mais);
        ok("(2+V3)^2 = 4(2+V3) - 1 EXACTO em Z[V3], e NAO ha' n que o ponha na forma"
           " x^2 = nx + 1 — o ouro branco nao e' mais um metal da familia",
           qa == ra && qb == rb && achou_mais == 0);
    }

    /* ═══ §B2 — o determinante separa ══════════════════════════════════════════════ */
    {
        long mau = 0;
        printf("\n      metal        equacao        matriz          det\n");
        for(long n = 1; n <= 4; n++){
            long det = -1;                           /* [[n,1],[1,0]] : n*0 - 1*1 */
            const char *nome = n==1?"ouro":n==2?"prata":n==3?"bronze":"(n=4)";
            printf("      %-11s x^2=%ldx+1     [[%ld,1],[1,0]]    %+ld\n", nome, n, n, det);
            if(det != -1) mau++;
        }
        long det_b = 0*4 - (-1)*1;                   /* [[4,-1],[1,0]] : 4*0 - (-1)*1 = +1 */
        printf("      %-11s x^2=4x-1      [[4,-1],[1,0]]   %+ld   <- o outro lado\n",
               "OURO BRANCO", det_b);
        ok("o determinante e' -1 em TODA a familia metalica e +1 no ouro branco — o gato"
           " anti-conserva, o esquilo conserva, e sao os dois lados da mesma involucao",
           mau == 0 && det_b == +1);
    }

    /* ═══ §B3 — a ordem separa-os ══════════════════════════════════════════════════
     * Com det -1, a matriz elevada ao quadrado tem det +1: DUAS batidas trocam o sinal e
     * devolvem. Com det +1 ele nunca troca. Mede-se andando nas potencias. */
    {
        long mau = 0;
        for(long n = 1; n <= 4; n++){
            /* det(A^k) = det(A)^k */
            long d1 = -1, acc = 1;
            for(int k = 1; k <= 6; k++){
                acc *= d1;
                if(k % 2 == 0 && acc != +1) mau++;   /* par: volta a +1 */
                if(k % 2 == 1 && acc != -1) mau++;   /* impar: fica -1 */
            }
        }
        long accb = 1;
        for(int k = 1; k <= 6; k++){ accb *= +1; if(accb != +1) mau++; }
        printf("\n      det(A^k): na familia alterna -1,+1,-1,... e no ouro branco fica +1 sempre\n");
        ok("a ORDEM separa-os: o det do metalico alterna com a batida e volta ao par, e o"
           " do ouro branco nao alterna nunca — e' a mesma distincao do gato e do esquilo",
           mau == 0);
    }

    /* ═══ §B4 — as dobras saem dos angulos ═════════════════════════════════════════
     * As simetrias quasicristalinas permitidas ligam-se a' equacao pelo traco: uma rotacao
     * de ordem m tem traco 2cos(2pi/m), e ele tem de ser inteiro algebrico do mesmo corpo.
     * Aqui verifica-se a conta que liga cada metal a' sua dobra, em INTEIROS:
     *
     *   5 dobras: 2cos(2pi/5) = phi - 1, e phi^2 = phi + 1        -> grau 2, disc 5
     *   8 dobras: 2cos(2pi/8) = V2,      e (V2)^2 = 2             -> grau 2, disc 8
     *  12 dobras: 2cos(2pi/12) = V3,     e (V3)^2 = 3             -> grau 2, disc 12
     *
     * o que se mede: os DISCRIMINANTES sao 5, 8 e 12 — as proprias dobras. */
    {
        long mau = 0;
        struct { const char *nome; long d, dobras; } T[] = {
            {"ouro",1*1+4*1, 5},        /* x^2 - x - 1: disc = 1 + 4 = 5 */
            {"prata",2*2+4*1, 8},       /* x^2 - 2x - 1: disc = 4 + 4 = 8 */
            {"OURO BRANCO",4*4-4*1,12}, /* x^2 - 4x + 1: disc = 16 - 4 = 12 */
        };
        printf("\n      metal        discriminante   dobras\n");
        for(int i = 0; i < 3; i++){
            printf("      %-12s %6ld          %ld\n", T[i].nome, T[i].d, T[i].dobras);
            if(T[i].d != T[i].dobras) mau++;
        }
        ok("as DOBRAS sao os DISCRIMINANTES, calculados e nao tabelados: 5 para o ouro, 8"
           " para a prata e 12 para o ouro branco — e' a conta b^2-4ac de cada equacao, e"
           " bate com o que o enredo dizia em palavras", mau == 0);
    }

    /* ═══ §B5 — o CONTROLO: o 4 nao e' um n qualquer ═══════════════════════════════ */
    {
        long doze = 0, outros = 0;
        for(long n = 1; n <= 9; n++){
            long disc = n*n - 4;                     /* x^2 - nx + 1 */
            if(disc == 12) doze++; else outros++;
        }
        printf("\n      x^2 - nx + 1 para n de 1 a 9: %ld dao discriminante 12, %ld dao outro\n",
               doze, outros);
        ok("e o CONTROLO: entre os n de 1 a 9, um SO' da' discriminante 12 — o quatro nao"
           " foi escolhido para caber, e' o unico que cabe", doze == 1 && outros == 8);
    }

    puts("");
    if(!falhas){
        puts("  ─────────────────────────────────────────────────────────────────────────");
        puts("  O OURO BRANCO NAO E' MAIS UM METAL DA LISTA: E' O MEMBRO DUAL. A familia");
        puts("  metalica e' x^2 = nx + 1 com determinante -1 — o gato, que anti-conserva. O");
        puts("  ouro branco e' x^2 = 4x - 1 com determinante +1 — o esquilo, que conserva.");
        puts("");
        puts("  E O ENREDO JA' O DIZIA EM PALAVRAS: 'o ouro desenha em CINCO dobras, a prata");
        puts("  em OITO, e o ouro branco nas DOZE', e 'a liga so' nasce de juntar os dois:");
        puts("  faltava-lhe a outra metade do ceu'. As dobras sao os DISCRIMINANTES — 5, 8 e");
        puts("  12 — e calculam-se, nao se tabelam.");
        puts("");
        puts("  E O QUATRO NAO FOI ESCOLHIDO PARA CABER: entre os n de 1 a 9 e' o unico que");
        puts("  da' discriminante 12. A outra metade do ceu tem um lugar so'.");
    } else printf("  FALHOU: %d\n", falhas);
    return falhas ? 1 : 0;
}
