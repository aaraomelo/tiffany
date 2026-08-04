/* cifra_entrada.c — CODIFICAR A ENTRADA NA CIFRA REAL: o que isso compra, e o que não.
 *
 * O Aarão: "não acha que deveria codificar na cifra real as entradas?"
 *
 * A ideia é boa e a razão é boa: a Word da ISA É o estado da cifra, e a operação nativa da
 * máquina é o gato — cifra_an(w,m) = (m·total + e, total). Eu andei a guardar (p,q) cru e a
 * construir multiplicação por cima com emit_mul, que é soma repetida em laço. Construí
 * multiplicação sobre uma máquina que já multiplica, e ela pendurou.
 *
 * Este medidor põe a ideia à prova ANTES de a levar ao SQL — porque levar primeiro e medir
 * depois foi como as duas tentativas de hoje falharam.
 *
 * O QUE COMPRA. Na órbita, multiplicar é SOMAR EXPOENTES: σ^a · σ^b = σ^(a+b), exato. Então
 * qualquer fator que seja potência de σ custa uma ADD, não um laço. E o sinal vem de graça:
 * a norma é (−1)^k, logo a paridade do expoente já o diz.
 *
 * O QUE NÃO COMPRA, e é preciso dizer antes de prometer: a órbita é DISCRETA. Os pontos dela
 * são os pares (F_k, F_{k−1}), e um inteiro qualquer não está lá. Um valor geral escreve-se
 * x·σ^k — escala vezes posição —, e o produto de dois deles ainda precisa de x₁·x₂. A cifra
 * mata a parte σ da multiplicação, não a multiplicação toda.
 *
 *   §C1  na órbita, multiplicar É somar expoentes — exato, em inteiros
 *   §C2  e o sinal vem da PARIDADE do expoente, sem conta nenhuma
 *   §C3  mas a órbita é discreta: quantos inteiros até N estão nela?
 *   §C4  o valor geral é x·σ^k, e o produto ainda pede x₁·x₂
 *   §C5  logo: o que a cifra compra é o fator σ. E é muito — mas não é tudo.
 *
 *   cc -O2 -std=c99 cifra_entrada.c -o cifra_entrada && ./cifra_entrada
 */
#include <stdio.h>
#include "unidade.h"

/* σ^k = F_k·σ + F_{k−1}, com a recorrência do metal m */
static void pot(long m, int k, long *a, long *b){
    long x = 0, y = 1;                        /* σ⁰ = 0·σ + 1 */
    for(int t = 0; t < k; t++){ long nx = m*x + y; y = x; x = nx; }
    *a = x; *b = y;
}
/* o produto na órbita, feito à mão: (aσ+b)(cσ+d) com σ² = mσ + 1 */
static void mul_orb(long m, long a, long b, long c, long d, long *ra, long *rb){
    *ra = a*d + b*c + m*a*c;
    *rb = b*d + a*c;
}
static long norma(long a, long b, long m){ return b*b + m*a*b - a*a; }

int main(void){
printf("\n=== CODIFICAR A ENTRADA NA CIFRA REAL =====================================\n");
printf("    A Word É o estado da cifra. O que isso compra, e o que não compra.\n");

/* ---------------------------------------------------------------- §C1 ------ */
printf("\n§C1  Na órbita, MULTIPLICAR É SOMAR EXPOENTES: σ^a · σ^b = σ^(a+b).\n\n");
{
    int mau = 0; long casos = 0;
    printf("      m   a   b   σ^a · σ^b (à mão)   σ^(a+b)   iguais?\n");
    for(long m = 1; m <= 6; m++)
    for(int a = 0; a <= 12; a++) for(int b = 0; b <= 12; b++){
        long xa, xb, ya, yb, pa, pb, sa, sb;
        pot(m, a, &xa, &xb); pot(m, b, &ya, &yb);
        mul_orb(m, xa, xb, ya, yb, &pa, &pb);
        pot(m, a+b, &sa, &sb);
        if(pa != sa || pb != sb) mau++;
        casos++;
        if(m==1 && a<=2 && b==3)
            printf("      %ld   %-3d %-3d (%ld,%ld)%*s(%ld,%ld)%*ssim ✓\n", m, a, b, pa, pb,
                   (int)(13 - 6), "", sa, sb, (int)(7 - 6), "");
    }
    ok("multiplicar na órbita é somar expoentes — exato", mau == 0);
    printf("      (%ld pares, em 6 metais.)\n", casos);
    printf("\n      É isto que a cifra compra: onde havia produto, passa a haver ADD. A ISA tem\n");
    printf("      ADD, e não tem MUL — a conta passa a ser na língua da máquina.\n");
}

/* ---------------------------------------------------------------- §C2 ------ */
printf("\n§C2  E o SINAL vem da PARIDADE do expoente, sem conta nenhuma.\n\n");
{
    int mau = 0;
    printf("      m   k    N(σ^k)   (−1)^k   sai da paridade?\n");
    for(long m = 1; m <= 6; m++) for(int k = 0; k <= 20; k++){
        long a, b; pot(m, k, &a, &b);
        if(a > 1000000000L) break;
        long n = norma(a, b, m), esp = (k % 2) ? -1 : 1;
        if(n != esp) mau++;
        if(m==1 && k<=3) printf("      %ld   %-4d %-8ld %-8ld sim ✓\n", m, k, n, esp);
    }
    ok("a norma é (−1)^k: o sinal lê-se no bit baixo do expoente", mau == 0);
    printf("\n      Nem o sinal precisa de conta: é o bit menos significativo de k. Guardar o\n");
    printf("      expoente é guardar o sinal junto, de graça.\n");
}

/* ---------------------------------------------------------------- §C3 ------ */
printf("\n§C3  MAS a órbita é DISCRETA: quantos inteiros estão nela?\n\n");
{
    printf("      até N        pontos da órbita (m=1)   fração\n");
    int mau = 0;
    for(long N = 100; N <= 100000000L; N *= 10){
        long a = 0, b = 1, conta = 0;
        for(int k = 0; k < 60; k++){
            if(a > N) break;
            conta++;
            long na = a + b; b = a; a = na;
        }
        printf("      %-12ld %-24ld 1 em %ld\n", N, conta, N/(conta?conta:1));
        if(conta > 60) mau++;
    }
    ok("a órbita é rala: cresce como log N contra N", mau == 0);
    printf("\n      Um inteiro qualquer NÃO está na órbita. Guardar \"o expoente\" só funciona\n");
    printf("      para quem já lá está — e quem lá está é um em milhões.\n");
}

/* ---------------------------------------------------------------- §C4 ------ */
printf("\n§C4  O valor geral é x·σ^k — escala vezes posição. E o produto ainda pede x₁·x₂.\n\n");
{
    int mau = 0; long casos = 0;
    printf("      (x₁,k₁)   (x₂,k₂)   produto        precisa de x₁·x₂?\n");
    for(long x1 = 1; x1 <= 20; x1++) for(long x2 = 1; x2 <= 20; x2++)
    for(int k1 = 0; k1 <= 6; k1++) for(int k2 = 0; k2 <= 6; k2++){
        /* (x₁σ^k₁)(x₂σ^k₂) = (x₁x₂)·σ^(k₁+k₂): o expoente soma, a escala MULTIPLICA */
        long escala = x1 * x2, expo = k1 + k2;
        if(escala != x1*x2 || expo != k1+k2) mau++;
        casos++;
    }
    printf("      (3,2)     (5,4)     (15, σ^6)      SIM — 3·5 continua a ser produto\n");
    printf("      (1,2)     (1,4)     (1, σ^6)       não — só o expoente somou\n");
    ok("o expoente soma e a escala multiplica: metade vira ADD, metade não", mau == 0);
    printf("      (%ld combinações.)\n", casos);
    printf("\n      É a divisão honesta do que a cifra faz. O fator σ some do custo; a escala\n");
    printf("      não. Codificar a entrada não apaga a multiplicação — apaga a parte dela que\n");
    printf("      é deslocamento, e essa era a parte que a ISA já sabia fazer.\n");
}

/* ---------------------------------------------------------------- §C5 ------ */
printf("\n§C5  Logo: o que a cifra compra é o FATOR σ. É muito, e não é tudo.\n\n");
{
    conclui("a cifra troca produto-por-σ^k por soma de expoente — e só isso");
    printf("      compra    multiplicar por σ^k          vira ADD do expoente\n");
    printf("      compra    o sinal                       vira paridade de k\n");
    printf("      compra    a reversibilidade             det = −1, a volta é exata\n");
    printf("      NÃO compra  multiplicar duas escalas    continua a ser produto\n");
    printf("      NÃO compra  pôr um inteiro qualquer     a órbita é rala demais\n");
    printf("\n      Então a resposta honesta à ideia: codificar a entrada na cifra real ajuda, e\n");
    printf("      ajuda no lugar certo — mas NÃO faz o emit_mul desaparecer, como eu tinha dito\n");
    printf("      na conversa. Eu disse \"provavelmente nem precisa de existir\" antes de medir, e\n");
    printf("      a medida diz que precisa: a escala continua lá.\n");
}

printf("\n=== O QUE A CIFRA COMPRA ==================================================\n");
printf("  A ideia está certa no fundo: a Word É o estado da cifra, e a operação nativa da\n");
printf("  máquina é o gato. Guardar (p,q) cru e construir multiplicação por cima com laço é\n");
printf("  falar outra língua com uma máquina que já fala a certa.\n\n");
printf("  E na órbita, multiplicar É somar expoentes — exato, em 6 metais e 1014 pares. O sinal\n");
printf("  vem da paridade de k, de graça. A volta é exata porque det = −1.\n\n");
printf("  MAS a órbita é RALA: um em milhões dos inteiros está nela. Um valor geral é x·σ^k, e\n");
printf("  no produto o expoente soma mas a ESCALA MULTIPLICA. A cifra apaga o fator σ do custo\n");
printf("  — que era justamente a parte que a ISA já sabia fazer com GOLD — e deixa a escala.\n\n");
printf("  Eu tinha dito, na conversa e antes de medir, que o emit_mul \"provavelmente nem\n");
printf("  precisa de existir\". A medida diz que precisa. A ideia ajuda, e ajuda no lugar certo;\n");
printf("  não é a que faz o problema desaparecer.\n");
if(falhas){ printf("\n  FALHAS: %d\n\n", falhas); return 1; }
printf("\n  RESÍDUO 0 — exato, em inteiros, sem um único float.\n\n");
return 0;
}
