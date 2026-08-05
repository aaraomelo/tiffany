#define _USE_MATH_DEFINES
/* enche.c — A SOMA COM CANTOR ENCHE, A MULTIPLICAÇÃO COM JULIA FECHA O CÍRCULO.
 *
 * O Aarão: "faltam as operações — soma com Cantor e multiplicação com Julia, e operador no centro
 * em PA e PG; aí preenche a área. Testa isso."
 *
 * O prisma.c mediu que o Cantor SOZINHO não enche: a medida vai a zero, é pó. Mas isso era o
 * conjunto, e não a OPERAÇÃO. A pergunta do Aarão é outra: e a SOMA dele consigo?
 *
 * E aí há um facto que decide, e não é opinião: C + C = [0,2]. Um conjunto de medida zero,
 * somado consigo próprio, dá um intervalo INTEIRO. A soma enche o que o conjunto não enchia.
 *
 * E a multiplicação: a Julia de z² é o círculo unitário, e multiplicar nele é rodar. O que a
 * soma abre em área, o produto fecha em círculo — que é o elíptico, que é o rei.
 *
 *   §E1  o Cantor é pó: a medida vai a zero (recordado do prisma.c)
 *   §E2  mas C + C ENCHE — a soma cobre o intervalo, e isso é medível
 *   §E3  a multiplicação: a Julia de z² é o círculo, e ele é invariante
 *   §E4  e o operador no centro: PA abre em parábola, PG em hipérbole
 *
 *   cc -O2 -std=c99 enche.c -lm -o enche && ./enche
 */
#include <stdio.h>
#include "../lib/disco.h"
#include <string.h>
#include <math.h>
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif
#include "unidade.h"

#define NIV 9                      /* níveis do Cantor */
#define NC  512                    /* 2^9 pedaços */
#define GR  2000                   /* grelha para medir a cobertura */

/* o Cantor em base 3: o pedaço k do nível n começa em soma dos dígitos 0 ou 2 */
static double cantor_ini(long k, int n){
    double x = 0, p = 1.0/3.0;
    for(int i = 0; i < n; i++){ if(k & (1L << (n-1-i))) x += 2*p; p /= 3.0; }
    return x;
}

int main(void){
printf("\n=== A SOMA ENCHE, O PRODUTO FECHA ========================================\n");
printf("    O Cantor sozinho é pó. Mas a pergunta é sobre a OPERAÇÃO, não o conjunto.\n");

printf("\n§E1  O Cantor é pó: a medida vai a zero.\n\n");
{
    double med = 1.0;
    for(int k = 0; k < NIV; k++) med = med * 2.0 / 3.0;
    printf("      nível %d: %d pedaços, medida total %.6f\n\n", NIV, NC, med);
    ok("a medida encolhe por 2/3 a cada nível e tende a zero", med < 0.03);
    printf("      É o que o prisma.c já dizia. E é por isso que o conjunto sozinho não enche.\n");
}

printf("\n§E2  Mas C + C ENCHE — e isto é o que decide.\n\n");
{
    /* soma-se cada par de pedacos e marca-se onde caiu, numa grelha de [0,2] */
    char *coberto = DISCO_FIXO(char, 222);
    disco_prende(DISCO_BASE(222),"dados/coberto_222.bin",(size_t)((GR + 1)),sizeof(char));
    memset(coberto, 0, ((size_t)((GR + 1))*sizeof(char)));
    for(long i = 0; i < NC; i++){
        double a = cantor_ini(i, NIV);
        for(long j = 0; j < NC; j++){
            double b = cantor_ini(j, NIV);
            double s = a + b;
            long g = (long)(s / 2.0 * GR);
            if(g >= 0 && g <= GR) coberto[g] = 1;
        }
    }
    long n = 0;
    for(long g = 0; g <= GR; g++) n += coberto[g];
    double frac = (double)n / (GR + 1);
    printf("      C tem medida %.4f, mas C + C cobre %.1f%% do intervalo [0,2]\n",
           pow(2.0/3.0, NIV), 100.0*frac);
    printf("      (%ld de %d células da grelha)\n\n", n, GR + 1);
    ok("a SOMA enche o que o conjunto não enchia — C + C cobre [0,2]", frac > 0.98);
    printf("      Um conjunto de medida ZERO, somado consigo, dá um intervalo INTEIRO. Não é\n");
    printf("      truque de amostragem: é o que C + C = [0,2] quer dizer, e a grelha só o vê.\n");
    printf("\n      E é a resposta ao que faltava: o pó não enche, a OPERAÇÃO enche.\n");
}

printf("\n§E3  A multiplicação: a Julia de z² é o círculo, e ele é invariante.\n\n");
{
    /* z -> z² : |z| = 1 fica; |z| < 1 cai para 0; |z| > 1 foge. O circulo e a fronteira. */
    /* UM PASSO, E NAO QUARENTA. Iterar z² quarenta vezes com virgula flutuante mede a DERIVA do
     * double e nao a invariancia: se |z| = 1+ε, ao fim de 40 passos e (1+ε)^(2^40). A afirmacao
     * verdadeira e |z²| = |z|², e essa mede-se num passo. Foi erro meu de medida, e apanhou-o o
     * teste — o circulo nunca deixou de ser invariante. */
    long fica = 0, cai = 0, foge = 0, mau = 0;
    for(int k = 0; k < 360; k++){
        double t = k * M_PI / 180.0;
        double x = cos(t), y = sin(t);
        double nx = x*x - y*y, ny = 2*x*y;
        double r = sqrt(nx*nx + ny*ny);
        if(fabs(r - 1.0) < 1e-12) fica++; else mau++;
    }
    for(int k = 1; k <= 20; k++){
        double x = k / 21.0, y = 0;                       /* dentro: |z| < 1 */
        for(int n = 0; n < 12; n++){ double nx = x*x - y*y, ny = 2*x*y; x = nx; y = ny; }
        if(sqrt(x*x + y*y) < 1e-6) cai++;
        double X = 1.0 + k/10.0, Y = 0;                   /* fora: |z| > 1 */
        for(int n = 0; n < 6; n++){ double nx = X*X - Y*Y, ny = 2*X*Y; X = nx; Y = ny; }
        if(sqrt(X*X + Y*Y) > 10.0) foge++;
    }
    printf("      no círculo |z|=1:  %ld ficam, %ld saem\n", fica, mau);
    printf("      dentro:            %ld caem para 0\n", cai);
    printf("      fora:              %ld fogem para o infinito\n\n", foge);
    ok("o círculo é INVARIANTE sob z²: |z|=1 => |z²|=1, nos 360", mau == 0 && fica == 360);
    ok("e é fronteira mesmo: dentro cai, fora foge", cai == 20 && foge == 20);
    printf("      Multiplicar no círculo é RODAR — e rodar é o elíptico, que é o rei. O que a\n");
    printf("      soma abriu em área, o produto fecha em círculo.\n");
}

printf("\n§E4  E o operador no centro: PA abre em parábola, PG em hipérbole.\n\n");
{
    /* o regime da cifra diz a figura, e ja estava medido: constante -> circulo, PA -> parabola,
     * PG -> hiperbole. Aqui confirma-se pelo CRESCIMENTO dos denominadores. */
    printf("      regime      termos          q cresce como     figura\n");
    long qc = 1, qa = 1, qg = 1, pc = 1, pa = 1, pg = 1;
    for(int k = 0; k < 8; k++){
        long nc = 1*qc + pc; pc = qc; qc = nc;                 /* constante: [1;1,1,...] */
        long na = (k+1)*qa + pa; pa = qa; qa = na;             /* PA: [1;2,3,4,...] */
        long ng = (1L<<k)*qg + pg; pg = qg; qg = ng;           /* PG: [1;2,4,8,...] */
    }
    printf("      CONSTANTE   [1;1,1,1,…]     %-17ld o círculo\n", qc);
    printf("      PA          [1;2,3,4,…]     %-17ld a parábola\n", qa);
    printf("      PG          [1;2,4,8,…]     %-17ld a hipérbole\n\n", qg);
    ok("o denominador da PG cresce mais que o da PA, e o da PA mais que o do círculo",
       qg > qa && qa > qc);
    printf("      O termo grande é a abertura: o rei não tem nenhum (só uns) e por isso é o mais\n");
    printf("      fechado — o círculo. A PA abre, a PG escancara. É a mesma cifra a dizer a\n");
    printf("      figura, e é isso o operador no centro: ele não muda de mecanismo, muda de passo.\n");
}

printf("\n");
return falhas ? 1 : 0;
}
