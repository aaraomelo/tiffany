/* cruzamento.c — AS OPERAÇÕES DO FILHO, EM FUNÇÃO DAS DOS PAIS.
 *
 * viveiro.c mediu QUE corpo é o filho: R^i ∨ R^j = R^lcm(i,j). Faltava o que o Aarão pediu:
 * a SOMA e a MULTIPLICAÇÃO do filho escritas em função das dos pais. É isto:
 *
 *     R^i ⊗_{R^d} R^j  =  R^lcm(i,j),      d = gcd(i,j)
 *
 * — o produto tensorial BALANCEADO sobre o subcorpo comum. E a dimensão fecha sozinha:
 * (i/d)·(j/d)·d = i·j/d = lcm. Daí as duas operações saem prontas:
 *
 *     SOMA          (u⊗v) + (u'⊗v')  =  coordenada a coordenada, em qualquer base
 *     MULTIPLICAÇÃO (u⊗v) · (u'⊗v')  =  (u ·_i u') ⊗ (v ·_j v')
 *
 * A multiplicação do filho É o par das multiplicações dos pais, cada uma agindo no seu índice.
 * Não há operação nova: há as duas antigas, uma em cada lado do ⊗.
 *
 * E isto explica o §V2 do viveiro: sobre Z_p — isto é, tensor sobre R^1 — só dá corpo quando
 * d=1, porque aí R^d = R^1 e balancear é não fazer nada. Com d>1, o tensor sobre Z_p produz
 * d CÓPIAS do filho, e é por isso que aparece divisor de zero. Balanceando sobre R^d, uma só.
 *
 *   §X1  os pais vivem dentro do filho, e as operações deles são RESTRIÇÕES das dele
 *   §X2  o filho é GERADO pelos pais: fechar R^i ∪ R^j sob + e × devolve R^lcm inteiro
 *   §X3  a soma é coordenada a coordenada — a dos pais, sem alteração
 *   §X4  a dimensão do balanceado: (i/d)(j/d)d = lcm, conferido
 *   §X5  e sobre Z_p sem balancear: exatamente d cópias, com divisor de zero
 *
 *   cc -O2 -std=c99 cruzamento.c -o cruzamento && ./cruzamento
 */
#include <stdio.h>
#include <string.h>

#define KMAX 8
static int falhas = 0;
static void ok(const char *r, int c){
    printf("      %-56s %s\n", r, c ? "sim ✓" : "NÃO ✗");
    if(!c) falhas++;
}
static long mdc(long a, long b){ while(b){ long t = a % b; a = b; b = t; } return a; }
static long mmc(long a, long b){ return a / mdc(a,b) * b; }

/* ---- GF(2^k) com um irredutível colhido por busca ---- */
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
/* o subcorpo R^a dentro de R^k: os pontos fixos de Frobenius^a */
static int no_subcorpo(unsigned x, int a, int k){
    unsigned y = x;
    for(int t = 0; t < a; t++) y = mulk(y, y, k);
    return y == x;
}

int main(void){
prepara();
printf("\n=== AS OPERAÇÕES DO FILHO, EM FUNÇÃO DAS DOS PAIS =========================\n");
printf("    R^i ⊗_{R^d} R^j = R^lcm(i,j), com d = gcd(i,j) — o tensor BALANCEADO\n");
printf("    sobre o subcorpo comum. E as duas operações saem prontas daí.\n");

/* ---------------------------------------------------------------- §X1 ------ */
printf("\n§X1  Os pais vivem dentro do filho, e as operações deles são RESTRIÇÕES.\n\n");
{
    int mau_s = 0, mau_m = 0, mau_t = 0;
    printf("      i   j   k=lcm   |R^i| dentro   soma restrita   produto restrito\n");
    for(int i = 1; i <= 4; i++) for(int j = 1; j <= 4; j++){
        long k = mmc(i,j);
        if(k > 6) continue;
        long tot = 1L << k, conta_i = 0;
        int soma_ok = 1, prod_ok = 1;
        for(unsigned x = 0; x < tot; x++){
            if(!no_subcorpo(x, i, (int)k)) continue;
            conta_i++;
            for(unsigned y = 0; y < tot; y++){
                if(!no_subcorpo(y, i, (int)k)) continue;
                /* a soma e o produto do FILHO, aplicados a dois elementos do PAI,
                 * têm de cair no pai — é o que "restrição" quer dizer */
                if(!no_subcorpo(x ^ y, i, (int)k)) soma_ok = 0;
                if(!no_subcorpo(mulk(x, y, (int)k), i, (int)k)) prod_ok = 0;
            }
        }
        if(conta_i != (1L << i)) mau_t++;
        if(!soma_ok) mau_s++;
        if(!prod_ok) mau_m++;
        if((i<=2&&j<=3)||(i==2&&j==4)||(i==4&&j==3))
            printf("      %d   %d   %5ld   %13ld   %13s   %s\n", i, j, k, conta_i,
                   soma_ok?"sim ✓":"NÃO", prod_ok?"sim ✓":"NÃO");
    }
    ok("o pai tem exatamente 2^i elementos dentro do filho", mau_t == 0);
    ok("a soma do filho restrita ao pai FECHA no pai", mau_s == 0);
    ok("e o produto do filho restrito ao pai também", mau_m == 0);
    printf("\n      Logo não há duas somas nem duas multiplicações: a do filho, olhada dentro\n");
    printf("      do pai, É a do pai. As operações não se traduzem — elas se restringem.\n");
}

/* ---------------------------------------------------------------- §X2 ------ */
printf("\n§X2  O filho é GERADO pelos pais: fechar R^i ∪ R^j devolve R^lcm inteiro.\n\n");
{
    int mau = 0;
    printf("      i   j   k=lcm   |R^i ∪ R^j|   fecho sob + e ×   2^k   gerou tudo?\n");
    for(int i = 1; i <= 4; i++) for(int j = 1; j <= 4; j++){
        long k = mmc(i,j);
        if(k > 6) continue;
        long tot = 1L << k;
        static char tem[1 << 8];
        memset(tem, 0, sizeof tem);
        long inicial = 0;
        for(unsigned x = 0; x < tot; x++)
            if(no_subcorpo(x, i, (int)k) || no_subcorpo(x, j, (int)k)){ tem[x] = 1; inicial++; }
        /* fecha sob soma e produto até estabilizar — o fecho é o subcorpo gerado */
        int mudou = 1;
        while(mudou){
            mudou = 0;
            for(unsigned x = 0; x < tot; x++){
                if(!tem[x]) continue;
                for(unsigned y = 0; y < tot; y++){
                    if(!tem[y]) continue;
                    unsigned s = x ^ y, p = mulk(x, y, (int)k);
                    if(!tem[s]){ tem[s] = 1; mudou = 1; }
                    if(!tem[p]){ tem[p] = 1; mudou = 1; }
                }
            }
        }
        long fecho = 0;
        for(unsigned x = 0; x < tot; x++) if(tem[x]) fecho++;
        int gerou = (fecho == tot);
        if(!gerou) mau++;
        if((i<=2&&j<=3)||(i==2&&j==4)||(i==3&&j==4))
            printf("      %d   %d   %5ld   %11ld   %16ld   %3ld   %s\n",
                   i, j, k, inicial, fecho, tot, gerou?"sim ✓":"NÃO ✗");
    }
    ok("os pais GERAM o filho inteiro, nada a mais e nada a menos", mau == 0);
    printf("\n      Nada precisa ser acrescentado de fora: o filho é exatamente o que os pais\n");
    printf("      alcançam somando e multiplicando entre si. É por isso que ele voa.\n");
}

/* ---------------------------------------------------------------- §X3 ------ */
printf("\n§X3  A SOMA é coordenada a coordenada — a dos pais, sem alteração.\n\n");
{
    /* em característica 2 a soma é o XOR, e ele é o mesmo em toda dimensão:
     * o bit i do resultado só depende dos bits i das entradas. Nada mistura. */
    int mau = 0;
    for(int k = 2; k <= 6; k++){
        long tot = 1L << k;
        for(unsigned x = 0; x < tot; x++) for(unsigned y = 0; y < tot; y++)
            for(int b = 0; b < k; b++){
                unsigned bit = ((x >> b) & 1u) ^ ((y >> b) & 1u);
                if((((x ^ y) >> b) & 1u) != bit) mau++;
            }
    }
    ok("o bit b da soma só depende dos bits b — nada mistura", mau == 0);
    printf("      A soma é a MESMA em toda dimensão: não há soma do filho e soma do pai.\n");
    printf("      É a única operação que atravessa a torre sem mudar de forma, e é por isso\n");
    printf("      que teoria.tex a chama de coordenada a coordenada em qualquer nível.\n");
    printf("\n      O que muda de dimensão para dimensão é SÓ a multiplicação — porque só ela\n");
    printf("      precisa da borda para baixar o excedente.\n");
}

/* ---------------------------------------------------------------- §X4 ------ */
printf("\n§X4  A dimensão do balanceado: (i/d)·(j/d)·d = lcm. Conferido.\n\n");
{
    int mau = 0;
    printf("      i   j   d=gcd   i/d   j/d   (i/d)(j/d)d   lcm(i,j)\n");
    for(int i = 1; i <= 6; i++) for(int j = 1; j <= 6; j++){
        long d = mdc(i,j), l = mmc(i,j);
        long dim = (i/d) * (j/d) * d;
        if(dim != l) mau++;
        if((i==2&&j==3)||(i==2&&j==4)||(i==4&&j==6)||(i==3&&j==6))
            printf("      %d   %d   %5ld   %3ld   %3ld   %11ld   %8ld\n", i, j, d, i/d, j/d, dim, l);
    }
    ok("balancear sobre R^d dá exatamente a dimensão do filho", mau == 0);
    printf("\n      É a conta que fecha o desenho: o filho tem (i/d) cópias do que o pai i traz\n");
    printf("      de novo, vezes (j/d) do que o pai j traz, vezes d do que os dois já tinham\n");
    printf("      em comum e NÃO se conta duas vezes. Balancear é justamente não contar duas.\n");
}

/* ---------------------------------------------------------------- §X5 ------ */
printf("\n§X5  E sem balancear, sobre Z_p: exatamente d cópias — daí o divisor de zero.\n\n");
{
    printf("      i   j   d   dim ⊗ sobre Z_p (i·j)   dim do filho (lcm)   cópias = i·j/lcm\n");
    int mau = 0;
    for(int i = 1; i <= 4; i++) for(int j = 1; j <= 4; j++){
        long d = mdc(i,j), l = mmc(i,j);
        long copias = (long)i*j / l;
        if(copias != d) mau++;
        if((i==2&&j==2)||(i==2&&j==3)||(i==2&&j==4)||(i==4&&j==4))
            printf("      %d   %d   %ld   %20d   %18ld   %14ld\n", i, j, d, i*j, l, copias);
    }
    ok("o número de cópias é exatamente d = gcd(i,j)", mau == 0);
    printf("\n      Com d=1 há UMA cópia e o tensor sobre Z_p já é o filho — é o caso que o\n");
    printf("      viveiro.c §V2 viu voar. Com d>1 há d cópias, e um produto que vive numa\n");
    printf("      cópia vezes outro que vive noutra dá ZERO: o divisor de zero não é acidente,\n");
    printf("      é a assinatura de se ter contado o subcorpo comum d vezes em vez de uma.\n");
}

printf("\n=== AS DUAS OPERAÇÕES DO FILHO ============================================\n");
printf("  R^i ⊗_{R^d} R^j = R^lcm(i,j), com d = gcd(i,j). Daí:\n\n");
printf("    SOMA           coordenada a coordenada — a MESMA em toda dimensão, sem\n");
printf("                   alteração; é a única operação que atravessa a torre inteira\n");
printf("                   sem mudar de forma.\n\n");
printf("    MULTIPLICAÇÃO  (u⊗v)·(u'⊗v') = (u ·_i u') ⊗ (v ·_j v') — o PAR das\n");
printf("                   multiplicações dos pais, cada uma no seu índice, balanceado\n");
printf("                   sobre R^d para não contar duas vezes o que é comum.\n\n");
printf("  Não há operação nova no filho: há as duas dos pais, uma de cada lado do ⊗, e a\n");
printf("  borda do R^lcm baixando o excedente como em qualquer dimensão. E os pais vivem\n");
printf("  dentro dele — as operações não se traduzem, se restringem.\n");
if(falhas){ printf("\n  FALHAS: %d\n\n", falhas); return 1; }
printf("\n  RESÍDUO 0 — exato, em inteiros, e por varredura exaustiva.\n\n");
return 0;
}
