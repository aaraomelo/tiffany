/* grau.c — A SPLINE É DE GRAU n, E OS COEFICIENTES SÃO PASCAL. Resíduo 0, em inteiros.
 *
 * O Aarão: «vc não consegue desenhar e medir um polinómio do terceiro grau?»
 *
 * A pergunta é justa porque eu andei a tratar TrueType e OpenType como dois formatos, e
 * eles não são: a TrueType tem contornos de grau 2, a OpenType de grau 3, e é o MESMO
 * polinómio uma linha acima. O `tests/pascal.c` diz porquê, e não é analogia:
 *
 *     C(n,k) = C(n-1,k-1) + C(n-1,k)     a recorrência de Pascal
 *     A_{n+1} = A_n ⊕ A_n†               o passo da torre
 *     SÃO A MESMA COISA.
 *
 * A Bézier de grau n é B(t) = Σ C(n,k) t^k (1−t)^(n−k) P_k, e os C(n,k) são a linha n do
 * triângulo. Grau 2 dá 1,2,1; grau 3 dá 1,3,3,1. Um avaliador que gere a linha pela
 * recorrência lê os dois sem saber que são formatos diferentes.
 *
 * E A ELEVAÇÃO DE GRAU É EXACTA, o que é o que fecha isto: toda quadrática É uma cúbica,
 * com os pontos de controlo que a própria recorrência dá,
 *
 *     Q(P0,P1,P2)  =  C(P0,  (P0+2P1)/3,  (2P1+P2)/3,  P2)
 *
 * e daí que o resíduo entre as duas seja ZERO — não pequeno, zero. Tudo em inteiros: os
 * pontos multiplicam-se por 3 antes de elevar, e a igualdade mede-se por PRODUTO CRUZADO,
 * sem uma única divisão.
 *
 *   §G1  a linha n de Pascal, pela recorrência, e a soma dá 2^n
 *   §G2  B(t) de grau 2 avaliada com os coeficientes gerados == a fórmula fechada
 *   §G3  ELEVAR: a quadrática elevada a cúbica dá o MESMO ponto, resíduo 0 INTEIRO
 *   §G4  e o CONTROLO: uma cúbica qualquer NÃO é uma quadrática — a volta não fecha
 *   §G5  o mesmo avaliador lê grau 2 e grau 3 sem um `if` de formato
 *
 * Zero doubles. Zero divisões na comparação.
 *
 *   cc -O2 -std=c99 -Wall -I../lib grau.c -o grau && ./grau
 */
#include <stdio.h>
#include "unidade.h"

#define GMAX 8

/* a linha n do triângulo, pela recorrência e não pela fórmula — é o passo da torre */
static void pascal(int n, long *c){
    c[0] = 1;
    for(int i = 1; i <= n; i++){
        c[i] = 1;
        for(int k = i - 1; k >= 1; k--) c[k] = c[k] + c[k-1];   /* C(i,k)=C(i-1,k-1)+C(i-1,k) */
    }
}

/* B(t) de grau n, com t = a/b racional e os pontos inteiros. Devolve o NUMERADOR sobre b^n:
 * não se divide, e por isso não há resto nenhum a esconder o resultado. */
static long bezier_num(int n, const long *P, long a, long b){
    long c[GMAX + 1]; pascal(n, c);
    long s = 0;
    for(int k = 0; k <= n; k++){
        long termo = c[k] * P[k];
        for(int i = 0; i < k; i++)     termo *= a;          /* t^k */
        for(int i = 0; i < n - k; i++) termo *= (b - a);    /* (1-t)^(n-k) */
        s += termo;
    }
    return s;                                              /* sobre b^n */
}

int main(void){
    printf("=== A SPLINE E' DE GRAU n, E OS COEFICIENTES SAO PASCAL ===================\n\n");

    /* ─── §G1 a linha de Pascal, e a soma ─────────────────────────────────────────── */
    int g1 = 1;
    for(int n = 0; n <= 6; n++){
        long c[GMAX + 1]; pascal(n, c);
        long soma = 0, pot = 1;
        printf("   n=%d:", n);
        for(int k = 0; k <= n; k++){ printf(" %ld", c[k]); soma += c[k]; }
        for(int i = 0; i < n; i++) pot *= 2;
        printf("   soma %ld  2^%d %ld\n", soma, n, pot);
        if(soma != pot) g1 = 0;
        /* e a simetria C(n,k)=C(n,n-k), que e' a involucao que troca os lados */
        for(int k = 0; k <= n; k++) if(c[k] != c[n-k]) g1 = 0;
    }
    ok("a linha de Pascal soma 2^n e e' simetrica — a dimensao da torre e a involucao", g1);

    /* ─── §G2 o avaliador contra a forma fechada do grau 2 ────────────────────────── */
    const long Q[3] = { 100, 700, 400 };
    int g2 = 1, n2 = 0;
    for(long a = 0; a <= 12; a++){
        long b = 12;
        long meu = bezier_num(2, Q, a, b);
        /* a forma fechada: (1-t)^2 P0 + 2t(1-t) P1 + t^2 P2, tudo sobre b^2 */
        long u = b - a;
        long fech = u*u*Q[0] + 2*a*u*Q[1] + a*a*Q[2];
        if(meu != fech) g2 = 0;
        n2++;
    }
    printf("\n   grau 2 avaliado em %d pontos de t\n", n2);
    ok("o avaliador generico == a forma fechada do grau 2, resIduo 0 exacto", g2 && n2 == 13);

    /* ─── §G3 ELEVAR: a quadratica E' uma cubica ──────────────────────────────────── */
    /* multiplicam-se os pontos por 3 para que a elevacao seja INTEIRA:
     *   C0 = 3 P0,  C1 = P0 + 2 P1,  C2 = 2 P1 + P2,  C3 = 3 P2   (sobre 3) */
    const long C[4] = { 3*Q[0], Q[0] + 2*Q[1], 2*Q[1] + Q[2], 3*Q[2] };
    int g3 = 1, n3 = 0; long pior = 0;
    for(long a = 0; a <= 12; a++){
        long b = 12;
        long q = bezier_num(2, Q, a, b);      /* sobre b^2      */
        long c = bezier_num(3, C, a, b);      /* sobre 3 * b^3  */
        /* PRODUTO CRUZADO, sem divisao:  q/b^2 == c/(3 b^3)  <=>  q*3*b == c */
        long e = q * 3 * b, d = e > c ? e - c : c - e;
        if(d){ g3 = 0; if(d > pior) pior = d; }
        n3++;
    }
    printf("\n   elevacao 2->3 em %d pontos, pior desvio %ld INTEIRO\n", n3, pior);
    ok("a quadratica elevada a cubica da' o MESMO ponto: resIduo 0 INTEIRO", g3 && n3 == 13);

    /* ─── §G4 o CONTROLO: nem toda cubica e' uma quadratica ───────────────────────── */
    /* se a elevacao fechasse para qualquer cubica, §G3 nao estava a medir nada */
    const long X[4] = { 300, 900, 100, 1200 };     /* uma cubica qualquer */
    int discorda = 0;
    for(long a = 0; a <= 12; a++){
        long b = 12;
        long q = bezier_num(2, Q, a, b);
        long c = bezier_num(3, X, a, b);
        if(q * 3 * b != c) discorda++;
    }
    printf("\n   controlo: uma cubica qualquer discorda em %d de 13 pontos\n", discorda);
    ok("uma cubica QUALQUER nao e' a quadratica — sem isto o §G3 passava sozinho",
       discorda >= 11);

    /* ─── §G5 o mesmo avaliador, os dois graus ────────────────────────────────────── */
    /* nao ha' `if` de formato em lado nenhum: muda o n, e a linha de Pascal vem atras */
    int g5 = 1;
    for(int n = 1; n <= 5; n++){
        long P[GMAX + 1];
        for(int k = 0; k <= n; k++) P[k] = 100 + 37*k;
        /* nos extremos a curva passa PELOS pontos: B(0)=P0 e B(1)=Pn, em qualquer grau */
        long b = 7, pot = 1;
        for(int i = 0; i < n; i++) pot *= b;
        if(bezier_num(n, P, 0, b) != P[0]  * pot) g5 = 0;
        if(bezier_num(n, P, b, b) != P[n] * pot) g5 = 0;
    }
    ok("o mesmo avaliador serve grau 1 a 5 e passa pelos extremos — sem um `if` de formato", g5);

    printf("\n%s\n", "==========================================================================");
    if(!falhas){
        puts("  A TrueType e a OpenType nao sao dois formatos: sao a mesma spline em graus");
        puts("  diferentes, e os coeficientes sao as linhas de Pascal — 1,2,1 e 1,3,3,1.");
        puts("  A recorrencia que as gera E' o passo da torre (tests/pascal.c), e por isso a");
        puts("  elevacao de grau nao aproxima nada: e' EXACTA, e o residuo e' 0 INTEIRO.");
        puts("");
        puts("  E mede-se por PRODUTO CRUZADO, sem uma divisao — porque divisao inteira tem");
        puts("  resto, e um resto pequeno de mais para se ver e' onde os defeitos moram.");
    } else printf("  FALHOU: %d\n", falhas);
    return falhas ? 1 : 0;
}
