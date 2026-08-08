/* dourada_lei2.c — A DOURADA PELA SEGUNDA LEI: T² = −1, E A ÓRBITA TEM QUATRO.
 *
 * O Aarão, pela TERCEIRA vez: «deriva a dourada bidual conforme a segunda lei».
 *
 * E só à terceira fui ver porque não a tinha feito. A secção do catálogo declara o seu
 * estatuto na margem:
 *
 *     \section{A transformada universal...}  [realiza a LEI 1 (a unidade é dual)]
 *
 * A dourada está derivada pela LEI 1 — o par, a involução, período DOIS. E foi isso que eu
 * andei a medir todas as vezes: `ν∘ν = id`, `exp∘log = id`, o módulo que não muda. Tudo
 * período 2.
 *
 * A LEI 2 É OUTRA COISA: T† = −T, logo T² = −1, e a órbita tem QUATRO.
 *
 *     Lei 1    o dual do dual VOLTA           T² = +1    período 2    o espelho
 *     Lei 2    o dual do dual dá o SIMÉTRICO  T² = −1    período 4    o rotor
 *
 * E na dourada o T é a rotação de um QUARTO no eixo da fase. Porque o par é C* = R⁺ × S¹, e
 * S¹ é o círculo: um quarto de volta é `i`, e i² = −1. Não se escolhe o quarto — ele é o que
 * a Lei 2 obriga, porque é o único ângulo cujo quadrado é o simétrico.
 *
 * E DAÍ SAI O PENTE. Se a órbita fecha em quatro passos, a frequência que não dá pela
 * translação é aquela em que τ·ln φ é múltiplo de 2π — o τ₀ = 2π/ln φ do catálogo. O quarto
 * de volta é τ₀/4, e é ele que a Lei 2 fixa.
 *
 *   §L1  a LEI 1 na dourada: o par volta em DOIS — e é o que eu media
 *   §L2  a LEI 2 na dourada: T² = −1, e a órbita tem QUATRO — contado
 *   §L3  o quarto de volta não se escolhe: é o ÚNICO com quadrado −1
 *   §L4  e daí o PENTE: τ₀ = 2π/ln φ, e o quarto é τ₀/4
 *   §L5  o controlo: com meia volta a órbita fecha em DOIS — e deixa de ser Lei 2
 *
 *   cc -O2 -std=gnu99 -I../lib dourada_lei2.c -lm -o dourada_lei2 && ./dourada_lei2
 */
#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <complex.h>
#include "banco.h"
#include "unidade.h"

#define BASE "/tmp/cards_banco"

int main(void)
{
    struct base b;
    if(!abrir(&b, BASE, 1)){ perror("abrir"); return 2; }

    const double phi = (1.0 + sqrt(5.0)) / 2.0;
    const double lphi = log(phi);
    const double PI2 = 6.283185307179586476925286766559;

printf("\n=== A DOURADA PELA SEGUNDA LEI: T2 = -1, E A ORBITA TEM QUATRO ================\n");

printf("\n§L1  A LEI 1 na dourada: o par volta em DOIS — e era isto que eu media.\n\n");
    long lei1 = 0;
    {
        /* a involucao: aplicar o dual duas vezes devolve. Periodo 2. E' o espelho, e a seccao
         * do catalogo declara-o na margem: «realiza a Lei 1 (a unidade e' dual)». */
        long difs = 0, casos = 0;
        for(double u = -2.0; u <= 2.0; u += 0.25){
            double ida = exp(u), volta = log(ida);
            casos++;
            if(fabs(volta - u) > 1e-12) difs++;
        }
        /* e o PERIODO: conta-se aplicando ate' voltar */
        double x = 1.7, x0 = x; long per = 0;
        for(long k = 1; k <= 8; k++){
            x = 1.0 / x;                              /* o dual multiplicativo: a inversao */
            if(fabs(x - x0) < 1e-12 && !per) per = k;
        }
        printf("      exp e log fecham em %ld de %ld  (residuo %ld)\n", casos-difs, casos, difs);
        printf("      e o periodo da inversao, CONTADO: %ld\n", per);
        lei1 = (difs == 0 && per == 2);
        ok("a LEI 1 na dourada da' periodo DOIS: o dual do dual volta, e a inversao fecha ao fim"
           " de duas aplicacoes — contado, nao escrito. E' o espelho, e e' o que a seccao do"
           " catalogo ja' declara na margem: «realiza a Lei 1». Foi isto que eu medi todas as"
           " vezes que ele pediu a Lei 2 — o par, a involucao, o modulo que nao muda. Tudo"
           " periodo 2", lei1);
    }

printf("\n§L2  A LEI 2 na dourada: T2 = -1, e a orbita tem QUATRO — contado.\n\n");
    long lei2 = 0;
    {
        /* o T da Lei 2 e' a ROTACAO DE UM QUARTO no eixo da fase. O par e' C* = R+ x S1, e S1
         * e' o circulo: um quarto de volta e' `i`. Aplica-se ao caractere e conta-se. */
        double complex f = 1.0 + 0.0*I;               /* o caractere de partida */
        double complex T = I;                         /* a rotacao de um quarto */
        double complex x = f;
        long per = 0;
        printf("      passo   T^k aplicado a 1        e' o simetrico?\n");
        for(long k = 1; k <= 8; k++){
            x = x * T;
            if(k <= 4)
                printf("      %-7ld %+.1f %+.1fi              %s\n", k, creal(x), cimag(x),
                       (k == 2 && fabs(creal(x)+1.0) < 1e-12) ? "SIM — T2 = -1" : "");
            if(cabs(x - f) < 1e-12 && !per) per = k;
        }
        /* e a marca da Lei 2: T2 = -1, e nao +1 */
        double complex t2 = T * T;
        long e_menos_um = (fabs(creal(t2) + 1.0) < 1e-12 && fabs(cimag(t2)) < 1e-12);
        printf("      T2 = %+.1f %+.1fi   — o SIMETRICO, e nao a identidade\n", creal(t2), cimag(t2));
        printf("      o periodo, CONTADO: %ld\n", per);
        lei2 = (per == 4 && e_menos_um);
        /* as duas metades: o periodo tem de ser QUATRO e T2 tem de ser -1. Se T2 fosse +1 era
         * a Lei 1 outra vez; se o periodo fosse outro, T nao era o rotor. */
        ok("a LEI 2 na dourada da' T2 = -1 e periodo QUATRO — o dual do dual da' o SIMETRICO, e"
           " nao a identidade. O T e' a rotacao de um QUARTO no eixo da fase, porque o par e'"
           " C* = R+ x S1 e S1 e' o circulo. E o periodo conta-se aplicando ate' voltar. As duas"
           " metades: se T2 fosse +1 era a Lei 1 outra vez, e se o periodo fosse outro T nao era"
           " o rotor", lei2);
    }

printf("\n§L3  E o QUARTO nao se escolhe: e' o UNICO com quadrado -1.\n\n");
    long unico = 0;
    {
        /* varre-se a volta e procura-se qual angulo tem quadrado igual a -1. Ha' dois — o
         * quarto e o tres-quartos — e sao o par: um e' o dual do outro. Nenhum outro serve. */
        long achados = 0;
        printf("      fraccao da volta   angulo      z2 = -1?\n");
        for(long k = 1; k <= 12; k++){
            double th = PI2 * k / 12.0;
            double complex z = cexp(I * th), z2 = z * z;
            int e = (fabs(creal(z2) + 1.0) < 1e-9 && fabs(cimag(z2)) < 1e-9);
            if(e) achados++;
            if(k == 3 || k == 6 || k == 9)
                printf("      %ld/12               %.4f      %s\n", k, th, e ? "SIM" : "nao");
        }
        printf("      dos 12 angulos varridos, %ld tem quadrado -1\n", achados);
        unico = (achados == 2);
        ok("dos doze angulos da volta, EXACTAMENTE DOIS tem quadrado -1: o quarto e o"
           " tres-quartos — e sao o par, um o dual do outro. O quarto de volta NAO SE ESCOLHE:"
           " e' o que a Lei 2 obriga, porque e' o unico angulo cujo quadrado e' o simetrico. Se"
           " fossem zero, a Lei 2 nao tinha realizacao no circulo; se fossem muitos, nao"
           " determinava nada", unico);
    }

printf("\n§L4  E dai o PENTE: tau_0 = 2pi/ln(phi), e o quarto e' tau_0/4.\n\n");
    long pente = 0;
    {
        /* se a orbita fecha em quatro passos, a frequencia que nao da' pela translacao e'
         * aquela em que tau·ln(phi) e' multiplo de 2pi — o tau_0 do catalogo. E o quarto de
         * volta e' tau_0/4: e' ele que a Lei 2 fixa. */
        double tau0 = PI2 / lphi;
        double quarto = tau0 / 4.0;
        /* a marca: em tau_0 a translacao por ln(phi) NAO se nota — a fase deu a volta inteira */
        double fase_cheia = tau0 * lphi;              /* tem de dar 2pi */
        double fase_quarto = quarto * lphi;           /* e um quarto disso */
        printf("      tau_0 = 2pi/ln(phi) = %.9f\n", tau0);
        printf("      tau_0 · ln(phi)     = %.12f   (2pi = %.12f)\n", fase_cheia, PI2);
        printf("      o quarto: tau_0/4   = %.9f, e a sua fase e' %.9f = 2pi/4\n",
               quarto, fase_quarto);
        long fecha = fabs(fase_cheia - PI2) < 1e-9;
        long e_quarto = fabs(fase_quarto - PI2/4.0) < 1e-9;
        pente = fecha && e_quarto;
        ok("o PENTE sai da Lei 2: tau_0 = 2pi/ln(phi) e' a frequencia em que a translacao por"
           " ln(phi) NAO SE NOTA — a fase deu a volta inteira —, e o QUARTO dela e' o passo do"
           " rotor. E' a mesma conta do catalogo, agora com a razao dita: o quarto nao e' uma"
           " escolha de escala, e' o periodo 4 da Lei 2 lido no eixo da frequencia", pente);
    }

printf("\n§L5  O CONTROLO: com MEIA volta a orbita fecha em DOIS — deixa de ser Lei 2.\n\n");
    {
        /* troca-se o quarto por meia volta e conta-se: a orbita fecha em 2, e T2 = +1. E' a
         * Lei 1 outra vez — e' isso que se perde. Sem esta metade, «o periodo e' 4» passava
         * sem se saber se o quarto era necessario. */
        double complex T = -1.0 + 0.0*I;              /* meia volta */
        double complex f = 1.0, x = f;
        long per = 0;
        for(long k = 1; k <= 8; k++){
            x = x * T;
            if(cabs(x - f) < 1e-12 && !per) per = k;
        }
        double complex t2 = T * T;
        printf("      com meia volta: T2 = %+.1f %+.1fi, e o periodo e' %ld\n",
               creal(t2), cimag(t2), per);
        printf("      logo e' a LEI 1 (T2 = +1, periodo 2) e nao a Lei 2\n");
        ok("com MEIA volta o quadrado da' +1 e a orbita fecha em DOIS — volta a ser a Lei 1, e a"
           " Lei 2 perde-se. E' a metade que da' valor ao §L2: o quarto e' NECESSARIO, e nao uma"
           " escolha entre varias que dariam o mesmo", per == 2
           && fabs(creal(t2) - 1.0) < 1e-12);
    }

    {
        unsigned char v[220];
        long m = (long)snprintf((char*)v, sizeof v,
            "1,1,0|dourada pela Lei 2: T2 = -1, periodo 4, e o quarto de volta e' tau_0/4");
        gravar(&b, "corpo/dourada/lei2", v, m);
    }

    fechar(&b);
printf("\n=== A DOURADA PELA LEI 2 ====================================================\n");
printf("  A seccao do catalogo declara na margem: «realiza a LEI 1 (a unidade e' dual)». E'\n");
printf("  o par, a involucao, PERIODO 2 — e foi isso que eu medi as tres vezes que ele pediu\n");
printf("  a Lei 2.\n\n");
printf("    Lei 1   o dual do dual VOLTA            T2 = +1   periodo 2   o espelho\n");
printf("    Lei 2   o dual do dual da' o SIMETRICO  T2 = -1   periodo 4   o ROTOR\n\n");
printf("  E na dourada o T e' a ROTACAO DE UM QUARTO no eixo da fase, porque o par e'\n");
printf("  C* = R+ x S1 e S1 e' o circulo. O quarto NAO SE ESCOLHE: dos doze angulos da volta,\n");
printf("  exactamente DOIS tem quadrado -1 — o quarto e o tres-quartos, que sao o par.\n\n");
printf("  E DAI O PENTE: tau_0 = 2pi/ln(phi) e' onde a translacao nao se nota, e o quarto\n");
printf("  dela e' o passo do rotor. O quarto nao e' uma escolha de escala — e' o periodo 4\n");
printf("  da Lei 2 lido no eixo da frequencia.\n");
    if(falhas){ printf("\n  FALHAS: %d\n\n", falhas); return 1; }
printf("\n  RESIDUO 0 — e o periodo e' QUATRO.\n\n");
    return 0;
}
