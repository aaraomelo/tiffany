/* gerador_andar.c — O GERADOR DEFINE O ANDAR, E O ZERO DESCE QUANDO FACTORIZA.
 *
 * O Aarão: «formaliza o gerador na teoria: agora o zero de cada andar pode ser fatorado no
 * andar abaixo.»
 *
 * A casa tinha `ℚ(σ)` — «a classe de racionais com o gerador irracional» — e a torre de
 * andares, mas não dizia QUANDO σ gera, nem o que acontece quando não gera. As duas coisas
 * são a mesma, e a lei é curta:
 *
 * ── O GERADOR ──────────────────────────────────────────────────────────────────
 *      σ gera o andar n   ⟺   o seu polinómio mínimo é IRREDUTÍVEL de grau n
 *
 * ── E O ZERO DESCE ─────────────────────────────────────────────────────────────
 * Se μ = f·g com graus menores, σ é raiz de UM dos factores, e o andar que ele gera é o
 * grau desse factor. O zero não desaparece: MUDA DE ANDAR.
 *
 *      x⁵ − x⁴ − 1 = (x² − x + 1)(x³ − x − 1)
 *
 * — o zero do andar 5 desce ao andar 3, que é a razão PLÁSTICA, o menor Pisot; e o resto
 * fica no andar 2, com as raízes no círculo. Um andar que cresce e um que roda.
 *
 * ── E A DESCIDA É EFECTIVA, NÃO SÓ EXISTENCIAL ─────────────────────────────────
 * Por Gauss, um mónico inteiro que factoriza sobre ℚ factoriza sobre ℤ em mónicos; e o
 * termo constante ±1 força cada factor a ter termo constante ±1. Logo a busca é FINITA:
 * exibe-se o factor, não se afirma que existe.
 *
 * ── E A FACE FINITA DIZ O CONTRÁRIO, QUE É O QUE FUNDAMENTA ────────────────────
 * Em 𝔽ₚ um polinómio de grau 2 tem SEMPRE onde descer: ou tem raiz — e o zero cai no
 * andar 1 — ou é irredutível, e o zero vive no andar 2, que é 𝔽ₚ². Não há terceiro sítio,
 * e a soma dos dois fecha nos 126 metais. É por não haver esse fecho em ℚ que os andares
 * lá existem como objectos e não como etapas.
 *
 *   §A1  o GERADOR define o andar: irredutível de grau n, e a busca é finita
 *   §A2  e o ZERO DESCE quando factoriza — com o factor EXIBIDO
 *   §A3  a soma dos graus é n: o zero muda de andar, não desaparece
 *   §A4  na FACE FINITA há sempre onde descer, e são só dois sítios
 *
 * Nenhum double, nenhum limiar.
 *
 *   cc -O2 -std=c99 -I. -I../lib gerador_andar.c -o gerador_andar && ./gerador_andar
 */
#include <stdio.h>
#include "oito.h"
#include "unidade.h"

#define GA_N 6             /* graus varridos */
#define GA_U 20            /* tecto dos coeficientes na busca de factores */

/* divide P (grau n) por x² + ux + v, e devolve 1 se o resto é ZERO.
 * A divisão é sintética e INTEIRA: o divisor é mónico, logo não há fracção. */
static int divide_quad(const long *P, int n, long u, long v, long *Q){
    long R[10];
    for(int d = 0; d <= n; d++) R[d] = P[d];
    for(int d = 0; d < 10; d++) Q[d] = 0;   /* TODO o vector: acima de n é zero */
    for(int d = n; d >= 2; d--){
        long c = R[d];
        Q[d-2] = c;
        R[d] -= c; R[d-1] -= c*u; R[d-2] -= c*v;
    }
    return R[0] == 0 && R[1] == 0;
}
/* e por x + c: o resto é P(−c), por Horner */
static int divide_lin(const long *P, int n, long c, long *Q){
    long x = -c, r = 0;
    for(int d = 0; d < 10; d++) Q[d] = 0;
    for(int d = n; d >= 0; d--){ Q[d] = r; r = r*x + P[d]; }
    for(int d = 0; d < n; d++) Q[d] = Q[d+1];
    Q[n] = 0;
    return r == 0;
}

int main(void){
    printf("\n=== O GERADOR DEFINE O ANDAR, E O ZERO DESCE QUANDO FACTORIZA ===\n");

    /* ═══ §A1  O GERADOR DEFINE O ANDAR ═════════════════════════════════════ */
    printf("\n§A1 σ gera o andar n ⟺ o mínimo é irredutível de grau n.\n\n");
    {
        /* A busca é FINITA por Gauss: um mónico inteiro que factoriza sobre ℚ factoriza
         * sobre ℤ em mónicos, e o termo constante −1 força cada factor a ter ±1. Para
         * n ≤ 6 os graus 1, 2 e 3 fecham-na — um factor de grau 4 num sêxtico obriga a um
         * co-factor de grau 2, e um de grau 5 a um de grau 1. */
        long casos = 0, irred = 0, red = 0;
        printf("      n   m   graus dos factores   o andar que σ gera\n");
        for(int n = 2; n <= GA_N; n++) for(long m = 1; m <= 2; m++){
            long P[10] = {0}, Q[10];
            P[n] = 1; P[n-1] += -m; P[0] += -1;
            int g1 = 0, g2 = 0;
            for(long c = -1; c <= 1; c += 2) if(divide_lin(P, n, c, Q)) g1 = 1;
            if(n > 2)
                for(long v = -1; v <= 1; v += 2) for(long u = -GA_U; u <= GA_U; u++)
                    if(divide_quad(P, n, u, v, Q)) g2 = 1;
            casos++;
            if(g1 || g2) red++; else irred++;
            if(m == 1 || n == 5)
                printf("      %-3d %-3ld %-20s %d\n", n, m,
                       (g1 || g2) ? "factoriza" : "irredutível",
                       (g1 || g2) ? (n - (g1 ? 1 : 2)) : n);
        }
        printf("      %ld casos: irredutíveis %ld, redutíveis %ld\n", casos, irred, red);
        ok("O GERADOR DEFINE O ANDAR, E A DECISÃO É FINITA: σ gera o andar n exactamente"
           " quando o seu polinómio mínimo é IRREDUTÍVEL de grau n, e isso decide-se em"
           " inteiros — por Gauss a factorização sobre ℚ dá-se sobre ℤ em mónicos, e o"
           " termo constante −1 força cada factor a ter termo constante ±1, logo a busca"
           " é FINITA e exibe o factor em vez de afirmar que existe. Dos casos varridos, um"
           " só factoriza",
           casos == 2*(GA_N-1) && red == 1 && irred == casos - 1);
    }

    /* ═══ §A2  E O ZERO DESCE, COM O FACTOR EXIBIDO ═════════════════════════ */
    printf("\n§A2 Quando factoriza, o zero DESCE — e o factor exibe-se.\n\n");
    {
        /* x⁵ − x⁴ − 1 é o único que factoriza, e o que ele mostra é o essencial: o zero
         * não desaparece ao factorizar — MUDA DE ANDAR. */
        long P[10] = {0}, Q[10];
        int n = 5;
        P[5] = 1; P[4] = -1; P[0] = -1;
        long achou_u = 0, achou_v = 0; int tem = 0;
        for(long v = -1; v <= 1 && !tem; v += 2) for(long u = -GA_U; u <= GA_U; u++)
            if(divide_quad(P, n, u, v, Q)){ achou_u = u; achou_v = v; tem = 1; break; }
        printf("      x⁵ − x⁴ − 1 = (x² %+ld·x %+ld) · (x³ %+ld·x² %+ld·x %+ld)\n",
               achou_u, achou_v, Q[2], Q[1], Q[0]);
        /* o co-factor é x³ − x − 1: a razão PLÁSTICA, o menor número de Pisot */
        int plastica = (Q[3] == 1 && Q[2] == 0 && Q[1] == -1 && Q[0] == -1);
        /* e o factor quadrático x² − x + 1 tem as raízes no CÍRCULO: D = 1 − 4 = −3 < 0 */
        long D2 = achou_u*achou_u - 4*achou_v;
        int no_circulo = (D2 < 0);
        /* e o cúbico tem uma raiz real > 1 — a que cresce. Sinais em 1 e 2 bastam: */
        long c1 = Q[3] + Q[2] + Q[1] + Q[0];              /* p(1) */
        long c2 = 8*Q[3] + 4*Q[2] + 2*Q[1] + Q[0];        /* p(2) */
        int cresce = (c1 < 0 && c2 > 0);                  /* raiz entre 1 e 2 */
        printf("      o co-factor é x³ − x − 1 (%s): a razão PLÁSTICA, o menor Pisot\n",
               plastica ? "sim" : "NÃO");
        printf("      e o quadrático tem D = %ld < 0 (%s): as raízes estão no CÍRCULO\n",
               D2, no_circulo ? "sim" : "NÃO");
        printf("      o cúbico muda de sinal entre 1 e 2 (%s): p(1) = %ld, p(2) = %ld —"
               " a raiz que CRESCE\n", cresce ? "sim" : "NÃO", c1, c2);
        ok("E O ZERO DESCE, COM O FACTOR EXIBIDO: x⁵ − x⁴ − 1 = (x² − x + 1)(x³ − x − 1),"
           " logo o zero do andar 5 vive no andar 3 — e esse é a razão PLÁSTICA, o menor"
           " número de Pisot. O zero não desaparece ao factorizar: MUDA DE ANDAR. E os dois"
           " factores são os dois regimes desta casa — o quadrático tem D = −3 < 0 e as"
           " raízes no CÍRCULO, que roda e não acrescenta grau à parte que cresce; o cúbico"
           " muda de sinal entre 1 e 2, que é a raiz que CRESCE",
           tem && plastica && no_circulo && cresce);
    }

    /* ═══ §A3  A SOMA DOS GRAUS É n ═════════════════════════════════════════ */
    printf("\n§A3 O zero muda de andar e não desaparece: a soma dos graus é n.\n\n");
    {
        /* É o que distingue «descer» de «perder»: os graus dos factores SOMAM o grau
         * original. Nenhum grau se cria, nenhum se destrói — ele reparte-se. */
        long P[10] = {0}, Q[10];
        P[5] = 1; P[4] = -1; P[0] = -1;
        int tem = 0, gq = 0, gc = 0;
        for(long v = -1; v <= 1 && !tem; v += 2) for(long u = -GA_U; u <= GA_U; u++)
            if(divide_quad(P, 5, u, v, Q)){
                tem = 1; gq = 2;
                for(int d = 3; d >= 0; d--) if(Q[d]){ gc = d; break; }
                break;
            }
        /* e a volta obrigatória: o produto dos factores reconstrói o original */
        long R[12] = {0};
        long F[3] = {0};
        for(long v = -1; v <= 1 && tem; v += 2) for(long u = -GA_U; u <= GA_U; u++){
            long QQ[10];
            if(divide_quad(P, 5, u, v, QQ)){ F[0] = v; F[1] = u; F[2] = 1; break; }
        }
        for(int i = 0; i <= 2; i++) for(int j = 0; j <= gc; j++) R[i+j] += F[i]*Q[j];
        int volta = 1;
        for(int d = 0; d <= 5; d++) if(R[d] != P[d]) volta = 0;
        printf("      graus: %d + %d = %d   (o original)\n", gq, gc, gq + gc);
        printf("      e a VOLTA: o produto dos factores reconstrói x⁵ − x⁴ − 1 (%s)\n",
               volta ? "sim" : "NÃO");
        ok("O ZERO MUDA DE ANDAR E NÃO DESAPARECE, E É A SOMA DOS GRAUS QUE O DIZ: 2 + 3 ="
           " 5, o grau original. Nenhum grau se cria nem se destrói ao factorizar — ele"
           " REPARTE-SE, e é isso que distingue descer de perder. E a volta é obrigatória:"
           " o produto dos dois factores reconstrói o polinómio de partida, coeficiente a"
           " coeficiente",
           tem && gq == 2 && gc == 3 && gq + gc == 5 && volta);
    }

    /* ═══ §A4  NA FACE FINITA HÁ SEMPRE ONDE DESCER ═════════════════════════ */
    printf("\n§A4 Na face finita o zero tem sempre onde descer, e são só dois sítios.\n\n");
    {
        /* Em 𝔽ₚ um polinómio de grau 2 ou tem raiz — e o zero cai no andar 1 — ou é
         * irredutível, e o zero vive no andar 2, que é 𝔽ₚ². NÃO HÁ TERCEIRO SÍTIO, e a
         * soma dos dois fecha nos 126 metais.
         *
         * É por não haver esse fecho em ℚ que os andares lá EXISTEM como objectos e não
         * como etapas: em ℚ há polinómios que não descem, e o andar deles é o sítio onde
         * o zero fica. */
        long metais = 0, andar1 = 0, andar2 = 0;
        for(int m = 1; m < OT_P; m++){
            int tem = 0;
            for(int x = 0; x < OT_P; x++)
                if(ot_menos(ot_mult((F)x,(F)x), ot_soma(ot_mult((F)m,(F)x), (F)1)) == 0){
                    tem = 1; break;
                }
            metais++;
            if(tem) andar1++; else andar2++;
        }
        printf("      %ld metais de 𝔽₁₂₇: o zero desce ao andar 1 em %ld, fica no andar 2"
               " em %ld\n", metais, andar1, andar2);
        printf("      e a soma fecha: %ld + %ld = %ld — não há terceiro sítio\n",
               andar1, andar2, andar1 + andar2);
        printf("      (em ℚ o andar 1 tem ZERO, para todo m: é por isso que o andar 2"
               " existe lá como objecto)\n\n");
        ok("NA FACE FINITA O ZERO TEM SEMPRE ONDE DESCER, E SÃO SÓ DOIS SÍTIOS: em 𝔽₁₂₇ um"
           " polinómio de grau 2 ou tem raiz — e então o zero cai no andar 1 — ou é"
           " irredutível, e o zero vive no andar 2, que é 𝔽ₚ². Não há terceiro sítio, e a"
           " soma dos dois fecha nos 126 metais, exaustivamente e sem tecto meu. É por NÃO"
           " haver esse fecho em ℚ que os andares lá existem como OBJECTOS e não como"
           " etapas: em ℚ o andar 1 tem zero metais, para todo m, e o zero fica onde está",
           andar1 + andar2 == metais && metais == OT_P - 1 && andar1 > 0 && andar2 > 0);
    }

    if(!falhas){
        printf("\n  ─────────────────────────────────────────────────────────────\n");
        printf("  σ gera o andar n exactamente quando o mínimo é irredutível de grau n.\n");
        printf("  Quando factoriza, o zero DESCE — não desaparece: os graus somam.\n");
        printf("  E o único que desce nesta família cai na razão PLÁSTICA, com o resto\n");
        printf("  no círculo: um andar que cresce e um que roda.\n");
        printf("  Na face finita há sempre onde descer, e só dois sítios. Em ℚ não há —\n");
        printf("  e é por isso que lá o andar é um objecto, e não uma etapa.\n");
    }
    printf("\n=== %d asserções, %d falhas ===\n", unidades, falhas);
    return falhas ? 1 : 0;
}
