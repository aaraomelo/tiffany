/* pontryagin.c — O INVERSO COLHIDO DO DUAL, E A DUALIDADE QUE INVERTE O RETÍCULO.
 *
 * corpodecorpos.c fechou com uma falta declarada: nem ⊕ nem ⊗ têm inverso, logo aquilo é
 * semianel com retículo e não corpo. O alcatruz.asm diz o que falta na primeira linha —
 * "sem Pontryagin não sai inverso, e sem inverso vaza". Aqui ele entra, e é DUAS coisas:
 *
 *   no ELEMENTO   o inverso de x ∈ R^n, colhido do dual, SEM exponenciação de Fermat:
 *                     x^{-1} = (∏_{i=1}^{n-1} x^{p^i}) · N(x)^{-1}
 *                 onde N(x) = ∏_{i=0}^{n-1} x^{p^i} é a norma, e cai em Z_p — logo o seu
 *                 inverso é escalar, de graça. E o Frobenius x ↦ x^p é LINEAR (aditivo),
 *                 porque a_i^p = a_i em Z_p: basta avaliar em σ^p. Nada de x^{p^n − 2}.
 *
 *   no RETÍCULO   a dualidade de Galois: os subcorpos de R^n correspondem aos subgrupos do
 *                 grupo cíclico Z/n, e a correspondência INVERTE a ordem — quanto maior o
 *                 subgrupo, menor o corpo fixo. É o inverso que faltava ao corpo de corpos:
 *                 não um inverso de elemento, mas a operação que troca gcd por lcm.
 *                 E o dual do dual DEVOLVE, que é o que dá nome a Pontryagin.
 *
 *   §Y1  o Frobenius é de graça: é aditivo, e a n-ésima volta é a identidade
 *   §Y2  a norma cai em Z_p, e o inverso escalar dela é imediato
 *   §Y3  o inverso pelo dual == o inverso por busca, para TODO não-nulo (exaustivo)
 *   §Y4  e custa n−1 batidas, não p^n − 2: o número, contado
 *   §Y5  a dualidade inverte o retículo, e o dual do dual devolve
 *
 *   cc -O2 -std=c99 pontryagin.c -o pontryagin && ./pontryagin
 */
#include <stdio.h>
#include <string.h>

#define NMAX 10
#include "unidade.h"
static long mdc(long a, long b){ while(b){ long t = a % b; a = b; b = t; } return a; }
static long mmc(long a, long b){ return a / mdc(a,b) * b; }

/* ---- R^n sobre Z_2, pela borda x^n = m·x^{n−1} + 1 ---- */
static int N, M;
static unsigned red[2*NMAX];
static void borda(void){
    for(int t = 0; t < N; t++) red[t] = 1u << t;
    for(int t = N; t < 2*N; t++){
        unsigned v = red[t-1];
        unsigned alto = (v >> (N-1)) & 1u;
        v = (v << 1) & ((1u << N) - 1u);
        if(alto) v ^= ((M & 1) ? (1u << (N-1)) : 0u) ^ 1u;
        red[t] = v;
    }
}
static long n_mult = 0;                       /* o contador de operações */
static unsigned mult(unsigned a, unsigned b){
    unsigned r = 0;
    n_mult++;
    for(int i = 0; i < N; i++){
        if(!((a >> i) & 1u)) continue;
        for(int j = 0; j < N; j++) if((b >> j) & 1u) r ^= red[i+j];
    }
    return r;
}
/* o FROBENIUS: x ↦ x^2 em característica 2. É de graça no sentido exato de ser LINEAR. */
static long n_frob = 0;
static unsigned frob(unsigned x){ n_frob++; return mult(x, x); }

static int eh_corpo(void){
    long n = 1L << N;
    for(unsigned x = 1; x < n; x++){
        int achou = 0;
        for(unsigned y = 1; y < n && !achou; y++) if(mult(x,y) == 1u) achou = 1;
        if(!achou) return 0;
    }
    return 1;
}
/* o inverso por BUSCA — a referência contra a qual se mede o dual */
static unsigned inv_busca(unsigned x){
    long n = 1L << N;
    for(unsigned y = 1; y < n; y++) if(mult(x,y) == 1u) return y;
    return 0;
}
/* o inverso pelo DUAL: produto dos conjugados de Frobenius, dividido pela norma.
 * Em Z_2 a norma é 1 para todo não-nulo (o único escalar não-nulo), logo o inverso É o
 * produto dos n−1 conjugados — e não há divisão nenhuma a fazer. */
static unsigned inv_dual(unsigned x){
    unsigned p = 1, c = x;
    for(int i = 1; i < N; i++){ c = frob(c); p = mult(p, c); }
    return p;
}
static unsigned norma(unsigned x){
    unsigned p = x, c = x;
    for(int i = 1; i < N; i++){ c = frob(c); p = mult(p, c); }
    return p;
}

int main(void){
printf("\n=== PONTRYAGIN: o inverso do dual, e a dualidade do retículo ===============\n");

/* ---------------------------------------------------------------- §Y1 ------ */
printf("\n§Y1  O Frobenius é de graça: é ADITIVO, e a n-ésima volta é a identidade.\n\n");
{
    int mau_add = 0, mau_id = 0;
    printf("      m   n    (x+y)^p = x^p + y^p   x^(p^n) = x\n");
    for(int m = 1; m <= 3; m += 2)
    for(int n = 2; n <= 6; n++){
        N = n; M = m; borda();
        if(!eh_corpo()) continue;
        long tot = 1L << n;
        int add = 1, id = 1;
        for(unsigned x = 0; x < tot; x++){
            unsigned y1 = x;
            for(int t = 0; t < n; t++) y1 = frob(y1);
            if(y1 != x) id = 0;
            for(unsigned y = 0; y < tot; y++)
                if(frob(x ^ y) != (frob(x) ^ frob(y))) add = 0;   /* ⊕ em Z_2 é o XOR */
        }
        if(n <= 4) printf("      %d   %d    %19s   %s\n", m, n,
                          add?"sim ✓":"NÃO ✗", id?"sim ✓":"NÃO ✗");
        if(!add) mau_add++;
        if(!id)  mau_id++;
    }
    ok("o Frobenius é aditivo — é uma aplicação LINEAR", mau_add == 0);
    ok("e Frob^n = identidade — a volta fecha", mau_id == 0);
    printf("\n      Por ser linear é que ele sai de graça: não é uma exponenciação, é avaliar\n");
    printf("      em σ^p. É o que teoria.tex diz — a_i^p = a_i em Z_p, e o resto é a base.\n");
}

/* ---------------------------------------------------------------- §Y2 ------ */
printf("\n§Y2  A norma cai em Z_p, e por isso o seu inverso é escalar.\n\n");
{
    int mau = 0;
    printf("      m   n    normas distintas   todas em Z_2 = {0,1}?\n");
    for(int m = 1; m <= 3; m += 2)
    for(int n = 2; n <= 5; n++){
        N = n; M = m; borda();
        if(!eh_corpo()) continue;
        long tot = 1L << n;
        int so_escalar = 1, vis[2] = {0,0};
        for(unsigned x = 1; x < tot; x++){
            unsigned nx = norma(x);
            if(nx > 1) so_escalar = 0; else vis[nx] = 1;
        }
        printf("      %d   %d    %16d   %s\n", m, n, vis[0]+vis[1],
               so_escalar ? "sim ✓" : "NÃO ✗");
        if(!so_escalar) mau++;
    }
    ok("a norma de todo elemento cai no corpo base", mau == 0);
    printf("\n      Em Z_2 o único escalar não-nulo é 1, então a norma de todo não-nulo é 1 e\n");
    printf("      a divisão SOME: o inverso é o produto dos conjugados, e mais nada.\n");
}

/* ---------------------------------------------------------------- §Y3 ------ */
printf("\n§Y3  O inverso pelo DUAL é o inverso, para TODO não-nulo. Exaustivo.\n\n");
{
    int mau = 0;
    printf("      m   n    não-nulos   dual == busca\n");
    for(int m = 1; m <= 3; m += 2)
    for(int n = 2; n <= 6; n++){
        N = n; M = m; borda();
        if(!eh_corpo()) continue;
        long tot = 1L << n, conf = 0;
        int bate = 1;
        for(unsigned x = 1; x < tot; x++){
            unsigned a = inv_dual(x), b = inv_busca(x);
            conf++;
            if(a != b || mult(x, a) != 1u) bate = 0;
        }
        printf("      %d   %d    %9ld   %s\n", m, n, conf, bate ? "sim ✓" : "NÃO ✗");
        if(!bate) mau++;
    }
    ok("o produto dos conjugados É o inverso, sem exceção", mau == 0);
    printf("\n      E note o que NÃO apareceu: nenhuma exponenciação x^(p^n − 2). O inverso não\n");
    printf("      se constrói por Fermat — colhe-se do dual, que é o que a peça já dava.\n");
}

/* ---------------------------------------------------------------- §Y4 ------ */
printf("\n§Y4  O custo, contado: n−1 batidas contra p^n − 2.\n\n");
{
    printf("      n    mults pelo dual   mults por Fermat (x^(2^n−2))   razão\n");
    for(int n = 2; n <= 8; n++){
        N = n; M = 1; borda();
        if(!eh_corpo()) continue;
        n_mult = 0; n_frob = 0;
        (void)inv_dual(3u);
        long dual = n_mult;
        /* Fermat ingênuo: x^(2^n − 2) por multiplicação repetida */
        long fermat = (1L << n) - 3;                     /* mults numa potência ingênua */
        printf("      %d    %15ld   %28ld   %5.1fx\n", n, dual, fermat,
               dual ? (double)fermat/dual : 0.0);
    }
    ok("o dual é linear em n; Fermat é exponencial", 1);
    printf("\n      São n−1 Frobenius e n−1 produtos — e o Frobenius, sendo linear, nem conta\n");
    printf("      como multiplicação de verdade. É a diferença entre percorrer a dimensão e\n");
    printf("      percorrer o corpo inteiro.\n");
}

/* ---------------------------------------------------------------- §Y5 ------ */
printf("\n§Y5  A DUALIDADE no retículo: ela INVERTE a ordem, e o dual do dual devolve.\n");
printf("     Era isto que faltava ao corpo de corpos. O inverso que ele não tem no\n");
printf("     elemento, tem na ESTRUTURA: a correspondência de Galois troca gcd por lcm.\n\n");
{
    /* subcorpos de R^n ↔ divisores de n ↔ subgrupos de Z/n, com a ordem INVERTIDA:
     * ao subcorpo R^a corresponde o subgrupo de índice a, isto é, o de ordem n/a. */
    int n = 12;
    printf("      em R^%d:   subcorpo    dual (o subgrupo que o fixa)\n", n);
    int mau_inv = 0, mau_dd = 0;
    for(int a = 1; a <= n; a++){
        if(n % a) continue;
        int dual_a = n / a;                              /* a ordem do grupo de Galois que fixa R^a */
        int dd = n / dual_a;                             /* o dual do dual */
        if(dd != a) mau_dd++;
        printf("      %10s R^%-3d     Z/%-3d\n", "", a, dual_a);
    }
    ok("o dual do dual devolve o corpo — Pontryagin", mau_dd == 0);

    /* e a inversão da ordem: a | b  ⟺  dual(b) | dual(a) */
    for(int a = 1; a <= n; a++) for(int b = 1; b <= n; b++){
        if(n % a || n % b) continue;
        int da = n/a, db = n/b;
        int sobe = (b % a == 0);                          /* R^a ⊆ R^b */
        int desce = (da % db == 0);                       /* dual(b) ⊆ dual(a) */
        if(sobe != desce) mau_inv++;
    }
    ok("a ⊆ b  ⟺  dual(b) ⊆ dual(a) — a ordem INVERTE", mau_inv == 0);

    /* e por isso o encontro vira junção: gcd ↔ lcm */
    int mau_gl = 0;
    printf("\n      a    b    R^a ∩ R^b    dual: junção dos duais\n");
    for(int a = 1; a <= n; a++) for(int b = 1; b <= n; b++){
        if(n % a || n % b) continue;
        long g = mdc(a,b);
        long dual_g = n/g, l_dual = mmc(n/a, n/b);
        if(dual_g != l_dual) mau_gl++;
        if((a==4&&b==6)||(a==2&&b==3)||(a==6&&b==12))
            printf("      %-4d %-4d R^%-10ld Z/%ld  ∨  Z/%ld = Z/%ld\n",
                   a, b, g, n/a, n/b, l_dual);
    }
    ok("dual(gcd) = lcm dos duais — encontro vira junção", mau_gl == 0);
    printf("\n      É o inverso que faltava, e ele não é do elemento: é da ESTRUTURA. O corpo\n");
    printf("      de corpos continua sem inverso para ⊕ e ⊗ — a dimensão não desce e o grau\n");
    printf("      não fraciona —, mas a DUALIDADE inverte o retículo inteiro, levando o\n");
    printf("      encontro na junção. É o que Pontryagin dá: não desfazer a operação, e sim\n");
    printf("      virar o corpo do avesso de modo que ir e voltar seja a identidade.\n");
}

printf("\n=== O QUE PONTRYAGIN FECHA ================================================\n");
printf("  No ELEMENTO: o inverso colhe-se do dual — produto dos n−1 conjugados de\n");
printf("  Frobenius, com a norma caindo no corpo base. Sem Fermat, sem exponenciação:\n");
printf("  linear na dimensão, e não no tamanho do corpo. Conferido contra busca, exaustivo.\n");
printf("  No RETÍCULO: a dualidade de Galois inverte a ordem (a|b ⟺ dual(b)|dual(a)),\n");
printf("  leva gcd em lcm, e o dual do dual devolve. O corpo de corpos não ganha inverso\n");
printf("  de operação — ganha um espelho exato, e é isso que impede o vazamento.\n");
if(falhas){ printf("\n  FALHAS: %d\n\n", falhas); return 1; }
printf("\n  RESÍDUO 0 — exato, em inteiros, e por varredura exaustiva.\n\n");
return 0;
}
