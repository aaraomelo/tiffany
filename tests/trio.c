/* trio.c — A SOMA DIRETA DOS TRÊS: pai, mãe e filho.
 *
 * Aarão: "vê o que dá a soma direta dos 3, pai mãe e filho." R^j ⊕ R^i ⊕ R^k, com k o corpo
 * onde o rolamento fecha. O que dá é uma coisa de duas caras, e é isso que o medidor separa:
 *
 *   COMO GRUPO é perfeito. Autodual, p^(i+j+k) caracteres todos distintos, e a transformada
 *   FATORA — o dual de uma soma é a soma dos duais, e a convolução continua a virar produto.
 *   Nada do lado aditivo se perde ao colar as três peças. §S4.
 *
 *   COMO ANEL é péssimo, e ele mesmo denuncia por quê: tem 2³ = 8 idempotentes. Os
 *   idempotentes CONTAM AS PEÇAS — um por subconjunto das três casas. Um corpo tem 2 (o 0 e o
 *   1) porque é uma peça só. Oito é a soma direta a dizer, sozinha, que são três coladas. §S2.
 *
 * E há a redundância, que é o achado que me interessa: os pais JÁ ESTAVAM no filho (viveiro
 * §V3 mediu que ele os contém). Então a soma direta dos três repete o que já tinha. O conjunto
 * fixo por φ^i não é uma cópia da mãe — é p^(2i+d), com d = gcd(i,j): ela aparece na casa dela
 * E na casa do filho, e o pai ainda partilha o comum com ela. §S5.
 *
 *   §S1  tamanho p^(i+j+k), e o divisor de zero achado por busca
 *   §S2  os idempotentes contam as peças: 8 = 2³, contra 2 de um corpo
 *   §S3  as unidades: (p^i−1)(p^j−1)(p^k−1), e a fração que gira despenca
 *   §S4  Pontryagin: autodual, e a transformada FATORA nas três
 *   §S5  a redundância: o fixo por φ^i é p^(2i+d), não p^i — a mãe repete
 *
 *   cc -O2 -std=c99 trio.c -o trio && ./trio
 */
#include <stdio.h>

#define KMAX 6
#include "unidade.h"
static long mdc(long a, long b){ while(b){ long t = a % b; a = b; b = t; } return a; }
static long mmc(long a, long b){ return a / mdc(a,b) * b; }

static unsigned RED[KMAX+1];
static unsigned mulk(unsigned a, unsigned b, int n){
    unsigned r = 0, red = RED[n];
    for(int i = 0; i < n; i++) if((b >> i) & 1u) r ^= a << i;
    for(int t = 2*n-2; t >= n; t--) if((r >> t) & 1u) r ^= red << (t - n);
    return r & ((1u << n) - 1u);
}
static int irred(unsigned poly, int n){
    unsigned m = 1u << n;
    for(unsigned a = 2; a < m; a++)
        for(unsigned b = 2; b < m; b++){
            unsigned p = 0;
            for(int i = 0; i < n; i++) if((b >> i) & 1u) p ^= a << i;
            if(p == (poly | (1u << n))) return 0;
        }
    return 1;
}
static void prepara(void){
    for(int n = 1; n <= KMAX; n++){
        RED[n] = 0;
        for(unsigned p = 0; p < (1u << n); p++)
            if(irred(p, n)){ RED[n] = p | (1u << n); break; }
    }
}
static unsigned frob(unsigned x, int t, int n){
    for(int s = 0; s < t; s++) x = mulk(x, x, n);
    return x;
}
static unsigned tra(unsigned x, int k){
    unsigned r = 0;
    for(int t = 0; t < k; t++) r ^= frob(x, t, k);
    return r;
}
/* os pares (i,j) do viveiro que cabem no exaustivo: k = onde o rolamento fecha */
static const int PI[] = {1,1,2,2,2,3,2,3};
static const int PJ[] = {1,2,2,3,4,3,6,6};
#define NPAR 8

int main(void){
prepara();
printf("\n=== A SOMA DIRETA DOS TRÊS: pai, mãe e filho ==============================\n");
printf("    R^j ⊕ R^i ⊕ R^k. Uma coisa de duas caras: grupo perfeito, anel péssimo.\n");

/* ---------------------------------------------------------------- §S1 ------ */
printf("\n§S1  O tamanho, e o divisor de zero — achado por busca, não afirmado.\n\n");
{
    int mau_t = 0, mau_d = 0;
    printf("      i   j   k   i+j+k   |trio| = 2^(i+j+k)   |filho|   divisor de zero\n");
    for(int t = 0; t < NPAR; t++){
        int i = PI[t], j = PJ[t]; long k = mmc(i,j);
        if(k > KMAX) continue;
        long s = i + j + k, ntrio = 1L << s;
        if(ntrio == (1L << k)) mau_t++;
        /* procura de verdade: dois não-nulos cujo produto componente a componente zera */
        long ni = 1L << i, nj = 1L << j, nk = 1L << k;
        int div = 0;
        for(long u = 1; u < ntrio && !div; u++) for(long v = 1; v < ntrio && !div; v++){
            unsigned ua = u/(nj*nk), ub = (u/nk)%nj, uc = u%nk;
            unsigned va = v/(nj*nk), vb = (v/nk)%nj, vc = v%nk;
            if(!mulk(ua,va,i) && !mulk(ub,vb,j) && !mulk(uc,vc,(int)k)) div = 1;
        }
        if(!div) mau_d++;
        printf("      %d   %d   %ld   %5ld   %19ld   %7ld   %s\n", i, j, k, s, ntrio,
               1L<<k, div?"achado ✓":"não achado");
    }
    ok("o trio nunca tem o tamanho do filho — colar aumenta, não completa", mau_t == 0);
    ok("e o divisor de zero está lá, achado em todo trio testado", mau_d == 0);
}

/* ---------------------------------------------------------------- §S2 ------ */
printf("\n§S2  Os IDEMPOTENTES contam as peças: 8 = 2³, contra 2 de um corpo.\n\n");
{
    int mau_t = 0, mau_c = 0;
    printf("      i   j   k   idempotentes do trio   2³   idempotentes do filho   2\n");
    for(int t = 0; t < NPAR; t++){
        int i = PI[t], j = PJ[t]; long k = mmc(i,j);
        if(k > KMAX) continue;
        long ni = 1L << i, nj = 1L << j, nk = 1L << k;
        /* e·e = e em cada componente. Num corpo só o 0 e o 1 cumprem. */
        long ei = 0, ej = 0, ek = 0;
        for(unsigned x = 0; x < ni; x++) if(mulk(x,x,i) == x) ei++;
        for(unsigned x = 0; x < nj; x++) if(mulk(x,x,j) == x) ej++;
        for(unsigned x = 0; x < nk; x++) if(mulk(x,x,(int)k) == x) ek++;
        long total = ei * ej * ek;
        if(total != 8) mau_t++;
        if(ek != 2) mau_c++;
        printf("      %d   %d   %ld   %20ld   %3d   %21ld   %d\n", i, j, k, total, 8, ek, 2);
    }
    ok("o trio tem exatamente 8 idempotentes — um por subconjunto das casas", mau_t == 0);
    ok("e o filho tem 2, que é o que um corpo tem: uma peça só", mau_c == 0);
    printf("\n      A soma direta não esconde a costura. O número de idempotentes é o número\n");
    printf("      de peças coladas, e ele denuncia sozinho: 2³ diz TRÊS. Um corpo diz UMA.\n");
    printf("      Não é preciso procurar o divisor de zero — os idempotentes já contaram.\n");
}

/* ---------------------------------------------------------------- §S3 ------ */
printf("\n§S3  As unidades: só quem é não-nulo nas TRÊS casas gira.\n\n");
{
    int mau = 0;
    printf("      i   j   k   unidades   (2^i−1)(2^j−1)(2^k−1)   não-nulos   fração que gira\n");
    for(int t = 0; t < NPAR; t++){
        int i = PI[t], j = PJ[t]; long k = mmc(i,j);
        if(k > KMAX) continue;
        long ni = 1L << i, nj = 1L << j, nk = 1L << k;
        long uni = 0;
        for(unsigned a = 0; a < ni; a++) for(unsigned b = 0; b < nj; b++) for(unsigned c = 0; c < nk; c++){
            /* unidade = tem inverso em cada casa = não é 0 em nenhuma */
            int inv = 1;
            if(a == 0 || b == 0 || c == 0) inv = 0;
            if(inv) uni++;
        }
        long prev = (ni-1)*(nj-1)*(nk-1), nn = ni*nj*nk - 1;
        if(uni != prev) mau++;
        printf("      %d   %d   %ld   %9ld   %22ld   %10ld   %ld/%ld\n", i, j, k, uni, prev,
               nn, uni, nn);
    }
    ok("as unidades são exatamente (2^i−1)(2^j−1)(2^k−1)", mau == 0);
    printf("\n      Num corpo TODO não-nulo gira. Aqui, quem tiver uma casa vazia esmaga o\n");
    printf("      espaço em vez de girá-lo — e basta uma casa. É a soma direta a cobrar o\n");
    printf("      preço de colar: quanto mais peças, mais fácil ter uma delas em zero.\n");
}

/* ---------------------------------------------------------------- §S4 ------ */
printf("\n§S4  PONTRYAGIN: como GRUPO o trio é perfeito — autodual, e a transformada FATORA.\n\n");
{
    int mau_c = 0, mau_f = 0, fora = 0;
    printf("      i   j   k   n=2^(i+j+k)   caracteres distintos   convolução → produto\n");
    for(int t = 0; t < NPAR; t++){
        int i = PI[t], j = PJ[t]; long k = mmc(i,j);
        if(k > KMAX) continue;
        if(i+j+k > 11){ fora++; continue; }   /* O(n²) aqui; quem passa disso fica DE FORA */
        long ni = 1L << i, nj = 1L << j, nk = 1L << k, n = ni*nj*nk;
        /* χ_(a,b,c)(x,y,z) = (−1)^(tr_i(ax)+tr_j(by)+tr_k(cz)): o caractere FATORA */
        long f[2048], g[2048], conv[2048];
        for(long u = 0; u < n; u++){ f[u] = ((u*7+1)%5)-2; g[u] = ((u*11+2)%7)-3; }
        for(long z = 0; z < n; z++){
            conv[z] = 0;
            for(long y = 0; y < n; y++){
                long za = z/(nj*nk), zb = (z/nk)%nj, zc = z%nk;
                long ya = y/(nj*nk), yb = (y/nk)%nj, yc = y%nk;
                conv[z] += f[y] * g[((za^ya)*nj + (zb^yb))*nk + (zc^yc)];
            }
        }
        int bom = 1;
        long distintos = 0;
        for(long c = 0; c < n; c++){
            long a = c/(nj*nk), b = (c/nk)%nj, e = c%nk, cf = 0, cg = 0, cc = 0;
            for(long y = 0; y < n; y++){
                long ya = y/(nj*nk), yb = (y/nk)%nj, yc = y%nk;
                unsigned s = tra(mulk((unsigned)a,(unsigned)ya,i), i)
                           ^ tra(mulk((unsigned)b,(unsigned)yb,j), j)
                           ^ tra(mulk((unsigned)e,(unsigned)yc,(int)k), (int)k);
                int ch = s ? -1 : 1;
                cf += f[y]*ch; cg += g[y]*ch; cc += conv[y]*ch;
            }
            if(cc != cf*cg) bom = 0;
            distintos++;      /* contados abaixo pela não-degenerescência de cada fator */
        }
        /* distintos: o caractere é nulo em todo o trio só se for nulo em cada casa, e cada
         * casa é não-degenerada (dualidade.c §P1). Confirma-se pela contagem do núcleo. */
        long nucleo = 0;
        for(long c = 0; c < n; c++){
            long a = c/(nj*nk), b = (c/nk)%nj, e = c%nk;
            int triv = 1;
            for(long y = 0; y < n && triv; y++){
                long ya = y/(nj*nk), yb = (y/nk)%nj, yc = y%nk;
                unsigned s = tra(mulk((unsigned)a,(unsigned)ya,i), i)
                           ^ tra(mulk((unsigned)b,(unsigned)yb,j), j)
                           ^ tra(mulk((unsigned)e,(unsigned)yc,(int)k), (int)k);
                if(s) triv = 0;
            }
            if(triv) nucleo++;
        }
        if(nucleo != 1) mau_c++;      /* só o caractere trivial é trivial ⟹ todos distintos */
        if(!bom) mau_f++;
        printf("      %d   %d   %ld   %13ld   %21ld   %s\n", i, j, k, n,
               nucleo == 1 ? n : n/nucleo, bom?"exato ✓":"FALHOU ✗");
    }
    ok("os 2^(i+j+k) caracteres são todos distintos — o trio é autodual", mau_c == 0);
    ok("e a transformada FATORA: convolução vira produto, colado e tudo", mau_f == 0);
    printf("      (%d trios ficaram de fora por i+j+k>11 — a conta aqui é O(n²) e essa é a\n", fora);
    printf("       fronteira do exaustivo nesta máquina. Não é resultado, é alcance.)\n");
    printf("\n      Do lado aditivo não se perde nada ao colar. O dual de uma soma é a soma dos\n");
    printf("      duais, e cada casa contribui o seu caractere: o de fora é o produto dos de\n");
    printf("      dentro. Colar peças é operação limpa para a SOMA — e só para ela.\n");
}

/* ---------------------------------------------------------------- §S5 ------ */
printf("\n§S5  A REDUNDÂNCIA: os pais já estavam no filho, e o trio repete-os.\n\n");
{
    int mau = 0;
    printf("      i   j   k   d=gcd   fixos por φ^i no trio   2^(2i+d)   uma cópia seria 2^i\n");
    for(int t = 0; t < NPAR; t++){
        int i = PI[t], j = PJ[t]; long k = mmc(i,j), d = mdc(i,j);
        if(k > KMAX) continue;
        long ni = 1L << i, nj = 1L << j, nk = 1L << k, fixos = 0;
        for(unsigned a = 0; a < ni; a++) for(unsigned b = 0; b < nj; b++) for(unsigned c = 0; c < nk; c++)
            if(frob(a,i,i) == a && frob(b,i,j) == b && frob(c,i,(int)k) == c) fixos++;
        long prev = 1L << (2*i + d);
        if(fixos != prev) mau++;
        printf("      %d   %d   %ld   %5ld   %22ld   %10ld   %16ld\n", i, j, k, d, fixos,
               prev, ni);
    }
    ok("o fixo por φ^i é 2^(2i+d), não 2^i — a mãe aparece mais de uma vez", mau == 0);
    printf("\n      É a conta da repetição: a mãe está na casa dela (2^i), está OUTRA VEZ dentro\n");
    printf("      do filho (2^i, porque o filho a contém), e o pai devolve ainda o que os dois\n");
    printf("      têm em comum (2^d). Somando os expoentes: 2i+d.\n");
    printf("\n      O filho já continha os dois pais. Pôr os pais ao lado dele não acrescenta\n");
    printf("      tecido nenhum — acrescenta CÓPIA, e cópia é o que estraga o produto.\n");
}

printf("\n=== O QUE DÁ ==============================================================\n");
printf("  Duas caras, e opostas:\n\n");
printf("    COMO GRUPO   perfeito. Autodual, 2^(i+j+k) caracteres todos distintos, e a\n");
printf("                 transformada FATORA nas três casas. Do lado aditivo, colar é\n");
printf("                 operação limpa e nada se perde.\n\n");
printf("    COMO ANEL    péssimo, e ele mesmo denuncia: 8 = 2³ idempotentes, quando um\n");
printf("                 corpo tem 2. Os idempotentes CONTAM AS PEÇAS. E só\n");
printf("                 (2^i−1)(2^j−1)(2^k−1) giram: basta uma casa em zero para esmagar.\n\n");
printf("  E é redundante: o filho JÁ continha os dois pais, então o fixo por φ^i é 2^(2i+d)\n");
printf("  e não 2^i. Pôr os pais ao lado do filho não acrescenta tecido — acrescenta cópia.\n");
printf("  Cópia é exatamente o que produz divisor de zero, e foi isto que o §V2 do viveiro\n");
printf("  já tinha visto pelo outro lado: com gcd>1 o tensorial dá d cópias e não voa.\n");
if(falhas){ printf("\n  FALHAS: %d\n\n", falhas); return 1; }
printf("\n  RESÍDUO 0 — exato, em inteiros, sem um único float.\n\n");
return 0;
}
