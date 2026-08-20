/* enche.c — A SOMA COM CANTOR ENCHE, A MULTIPLICAÇÃO COM JULIA FECHA O CÍRCULO.
 *
 * C+C cobre [0,2] em ℤ (dígitos 0,1,2 sem carry). |z²|=|z|² no círculo em Z[i] / ISA.
 *
 *   cc -O2 -std=c99 -Wall -I lib tests/enche.c -o enche
 */
#include <stdio.h>
#include "unidade.h"
#include "isa_disk.h"

#define NIV 9

int main(void){
printf("\n=== A SOMA ENCHE, O PRODUTO FECHA ========================================\n");

printf("\n§E1  O Cantor é pó: a medida vai a zero.\n\n");
{
    /* medida = 2^n / 3^n. Encolhe porque 2 < 3; fica < 1 porque 2^n < 3^n. */
    int niveis = 0, encolhe = 0, abaixo = 0;
    long n2 = 1, n3 = 1;
    printf("      nível   2^n      3^n      2^n < 3^n?\n");
    for(int k = 1; k <= NIV; k++){
        long a2 = n2, a3 = n3;
        n2 *= 2; n3 *= 3;
        niveis++;
        if(n2 * a3 < a2 * n3) encolhe++;   /* 2^{k}/3^{k} < 2^{k-1}/3^{k-1} ⇔ 2 < 3 */
        if(n2 < n3) abaixo++;
        if(k <= 4 || k == NIV)
            printf("      %-7d %-8ld %-8ld %s\n", k, n2, n3, n2<n3?"sim":"NAO");
    }
    printf("\n");
    ok("a medida encolhe por 2/3 a cada nível e tende a zero — em Z: 2^n < 3^n e a razao"
       " cai porque 2 < 3. Sem 0,03 de limiar",
       niveis == NIV && encolhe == NIV && abaixo == NIV);
}

printf("\n§E2  Mas C + C ENCHE — e isto é o que decide.\n\n");
{
    /* Pontos de C_n: 2·u / 3^n, u com dígitos {0,1} em base 3.
     * u+v: cada dígito 0+0, 0+1, 1+0, 1+1 cobre {0,1,2} SEM carry. Logo
     * u+v cobre TODOS os inteiros 0..3^n−1, e C_n+C_n cobre a grelha 2k/3^n de [0,2]. */
    long p3 = 1;
    for(int i = 0; i < NIV; i++) p3 *= 3;
    long mau = 0, cobertos = 0;
    for(long k = 0; k < p3; k++){
        long ki = k, ui = 0, uj = 0, p = 1;
        while(ki){
            int d = (int)(ki % 3);
            if(d == 1) ui += p;
            else if(d == 2){ ui += p; uj += p; }
            ki /= 3; p *= 3;
        }
        if(ui + uj != k) mau++;
        else cobertos++;
    }
    printf("      3^%d = %ld pontos da grelha 2k/3^n em [0,2]\n", NIV, p3);
    printf("      cobertos por u+v (digitos {0,1}+{0,1}): %ld, falhas %ld\n\n", cobertos, mau);
    ok("a SOMA enche o que o conjunto não enchia — C + C cobre [0,2]:"
       " cada k < 3^n e' u+v sem carry, em Z, sem grelha de double",
       mau == 0 && cobertos == p3);
}

printf("\n§E3  A multiplicação: a Julia de z² é o círculo, e ele é invariante.\n\n");
{
    long fica = 0, mau = 0, casos = 0;
    printf("      |z|² = t²+e²  =>  |z²|² = (|z|²)²\n");
    for(int t = -6; t <= 6; t++) for(int e = -6; e <= 6; e++){
        long n2 = t*t + e*e;
        if(n2 == 0) continue;
        long zr = t*t - e*e, zi = 2*t*e;
        long n2sq = zr*zr + zi*zi;
        casos++;
        if(n2sq == n2*n2) fica++; else mau++;
    }
    /* círculo unitário em ISA: ESQUILO preserva norma 1, e z→z² com (1,0) fica */
    isa_word(ISA_S_A, 1, 0);
    isa_MOVE(ISA_S_ESQUILO, 1);
    isa_MOVE(ISA_S_ESQUILO, 1);
    int at, ae; isa_read(ISA_S_A, &at, &ae);
    long n_esq = at*at + ae*ae;

    /* dentro: |z|<1, |z²| < |z|; fora: |z|>1, |z²| > |z| — em fracções */
    long cai = 0, foge = 0;
    for(int k = 1; k <= 20; k++){
        long num = k, den = 21;                 /* |z| = k/21 < 1 */
        if(num*num < den*den){
            /* após um quadrado: num²/den² vs num/den */
            if(num*num * den < num * den*den) cai++;  /* num < den já, num²/den² < num/den */
        }
        long Num = 10 + k, Den = 10;            /* |z| = (10+k)/10 > 1 */
        if(Num*Num > Den*Den){
            if(Num*Num * Den > Num * Den*Den) foge++;
        }
    }
    printf("      pares Z[i]: %ld com |z²|² = |z|⁴, falhas %ld\n", fica, mau);
    printf("      ESQUILO² de (1,0): (%d,%d) norma²=%ld\n", at, ae, n_esq);
    printf("      dentro (k/21): %ld encolhem; fora ((10+k)/10): %ld fogem\n\n", cai, foge);
    ok("o círculo é INVARIANTE sob z²: |z²|² = |z|⁴ em Z[i], e ESQUILO² tem norma 1",
       mau == 0 && fica == casos && n_esq == 1);
    ok("e é fronteira mesmo: dentro |z²|<|z|, fora |z²|>|z| — em fracções, sem 100.0",
       cai == 20 && foge == 20);
}

printf("\n§E4  E o operador no centro: PA abre em parábola, PG em hipérbole.\n\n");
{
    printf("      regime      termos          q cresce como     figura\n");
    long qc = 1, qa = 1, qg = 1, pc = 1, pa = 1, pg = 1;
    for(int k = 0; k < 8; k++){
        long nc = 1*qc + pc; pc = qc; qc = nc;
        long na = (k+1)*qa + pa; pa = qa; qa = na;
        long ng = (1L<<k)*qg + pg; pg = qg; qg = ng;
    }
    printf("      CONSTANTE   [1;1,1,1,…]     %-17ld o círculo\n", qc);
    printf("      PA          [1;2,3,4,…]     %-17ld a parábola\n", qa);
    printf("      PG          [1;2,4,8,…]     %-17ld a hipérbole\n\n", qg);
    ok("o denominador da PG cresce mais que o da PA, e o da PA mais que o do círculo",
       qg > qa && qa > qc);
}

printf("\n");
return falhas ? 1 : 0;
}
