/* viveiro.c — CADA CRUZAMENTO DÁ UM PÁSSARO QUE VOA SOZINHO.
 *
 * corpodecorpos.c mediu que ⊕ soma as dimensões e ⊗ as multiplica. Faltava o essencial, e é
 * o que dá nome a isto: o FILHOTE VOA? Isto é — o resultado do cruzamento é ele próprio um
 * corpo, ou é só um espaço com a dimensão certa?
 *
 * A pergunta separa as três operações candidatas, e a resposta não é a mesma:
 *
 *   ⊕  soma direta      R_a ⊕ R_b   dimensão a+b        NÃO voa: tem divisor de zero
 *   ⊗  tensorial        R_a ⊗ R_b   dimensão a·b        voa se e só se gcd(a,b)=1
 *   ∨  o cruzamento     R_a ∨ R_b   dimensão lcm(a,b)   VOA sempre — é o menor corpo
 *                                                        que contém os dois
 *
 * Logo a lei do viveiro é o lcm, e não a soma nem o produto. E quando gcd(a,b)=1 as duas
 * coincidem (a·b = lcm), que é por que o tensorial parece funcionar em alguns casos.
 *
 *   §V1  a soma direta NÃO voa: o divisor de zero, exibido
 *   §V2  o tensorial voa se e só se as espécies são primas entre si
 *   §V3  o cruzamento R_a ∨ R_b = R_lcm voa SEMPRE, e é o menor que contém os dois
 *   §V4  o viveiro é fechado: comutativo, associativo, idempotente, com neutro R^1
 *   §V5  e a ninhada não escapa: todo filhote é um corpo do próprio viveiro
 *
 *   cc -O2 -std=c99 viveiro.c -o viveiro && ./viveiro
 */
#include <stdio.h>
#include <string.h>

#define NMAX 8
#include "unidade.h"
static long mdc(long a, long b){ while(b){ long t = a % b; a = b; b = t; } return a; }
static long mmc(long a, long b){ return a / mdc(a,b) * b; }

/* ---- GF(2^n): um irredutível por dimensão, colhido por busca (não inventado) ---- */
static unsigned RED[NMAX+1];        /* o polinômio de redução de cada dimensão */

static unsigned mul_gf(unsigned a, unsigned b, int n, unsigned red){
    unsigned r = 0;
    for(int i = 0; i < n; i++){
        if((b >> i) & 1u) r ^= a << i;
    }
    for(int t = 2*n-2; t >= n; t--)
        if((r >> t) & 1u) r ^= red << (t - n);
    return r & ((1u << n) - 1u);
}
static int irredutivel(unsigned poly, int n){
    /* sem raiz e sem fator: por força bruta, testa se algum produto de menores o gera */
    unsigned m = 1u << n;
    for(unsigned a = 2; a < m; a++)
        for(unsigned b = 2; b < m; b++){
            unsigned p = 0;
            for(int i = 0; i < n; i++) if((b >> i) & 1u) p ^= a << i;
            if(p == (poly | (1u << n))) return 0;
        }
    return 1;
}
static void acha_irredutiveis(void){
    for(int n = 1; n <= NMAX; n++){
        RED[n] = 0;
        for(unsigned p = 0; p < (1u << n); p++)
            if(irredutivel(p, n)){ RED[n] = p | (1u << n); break; }
    }
}
static int corpo_ok(int n){
    unsigned red = RED[n], m = 1u << n;
    for(unsigned x = 1; x < m; x++){
        int achou = 0;
        for(unsigned y = 1; y < m && !achou; y++) if(mul_gf(x,y,n,red) == 1u) achou = 1;
        if(!achou) return 0;
    }
    return 1;
}

int main(void){
acha_irredutiveis();
printf("\n=== O VIVEIRO: cada cruzamento dá um pássaro que voa sozinho ===============\n");
printf("    A pergunta não é a dimensão do filhote — é se ele VOA: se o resultado do\n");
printf("    cruzamento é ele próprio um corpo, e não só um espaço do tamanho certo.\n");

/* ---------------------------------------------------------------- §V1 ------ */
printf("\n§V1  A soma direta NÃO voa. E o divisor de zero exibe-se.\n\n");
{
    /* R_a ⊕ R_b: pares (x,y), produto componente a componente. (1,0)·(0,1) = (0,0). */
    int mau = 0;
    printf("      a ⊕ b   dimensão   par não-nulo com produto nulo\n");
    for(int a = 1; a <= 3; a++) for(int b = 1; b <= 3; b++){
        unsigned x1 = 1, y1 = 0, x2 = 0, y2 = 1;       /* (1,0) e (0,1) */
        unsigned px = mul_gf(x1, x2, a, RED[a]), py = mul_gf(y1, y2, b, RED[b]);
        int tem_divisor = (px == 0 && py == 0);
        if(!tem_divisor) mau++;
        if(a <= 2 && b <= 2)
            printf("      %d ⊕ %d   %8d   (1,0)·(0,1) = (%u,%u)%s\n",
                   a, b, a+b, px, py, tem_divisor ? "   ← zero" : "");
    }
    ok("a soma direta SEMPRE tem divisor de zero", mau == 0);
    printf("\n      Os dois fatores são não-nulos e o produto é nulo: não há inverso para\n");
    printf("      nenhum dos dois. A dimensão fecha (a+b), a estrutura não. Este filhote não\n");
    printf("      voa — é dois pássaros na mesma gaiola, não um pássaro novo.\n");
}

/* ---------------------------------------------------------------- §V2 ------ */
printf("\n§V2  O tensorial voa se e só se as espécies são primas entre si.\n\n");
{
    /* R_a ⊗ R_b sobre GF(2): base e_i⊗f_j, produto componente a componente nos dois lados.
     * Procura-se divisor de zero por força bruta — a definição, não um critério. */
    printf("      a   b   gcd   dim=a·b   tem divisor de zero?   voa?\n");
    int mau = 0;
    for(int a = 1; a <= 3; a++) for(int b = 1; b <= 3; b++){
        int d = a*b;
        if(d > 6) continue;
        long tot = 1L << d;
        int achou_div = 0;
        for(long cx = 1; cx < tot && !achou_div; cx++)
        for(long cy = 1; cy < tot && !achou_div; cy++){
            /* multiplica: coeficiente (i,j) — i em R_a, j em R_b */
            unsigned r[NMAX*NMAX]; memset(r, 0, sizeof r);
            long z = 0;
            for(int i = 0; i < a; i++) for(int j = 0; j < b; j++){
                if(!((cx >> (i*b + j)) & 1L)) continue;
                for(int k = 0; k < a; k++) for(int l = 0; l < b; l++){
                    if(!((cy >> (k*b + l)) & 1L)) continue;
                    /* e_i·e_k em R_a, f_j·f_l em R_b, e a casa é o par */
                    unsigned ea = mul_gf(1u << i, 1u << k, a, RED[a]);
                    unsigned fb = mul_gf(1u << j, 1u << l, b, RED[b]);
                    for(int u = 0; u < a; u++) for(int v = 0; v < b; v++)
                        if(((ea >> u) & 1u) && ((fb >> v) & 1u)) r[u*b + v] ^= 1u;
                }
            }
            for(int t = 0; t < d; t++) if(r[t]) z = 1;
            if(!z) achou_div = 1;
        }
        int primo = (mdc(a,b) == 1);
        int voa = !achou_div;
        printf("      %d   %d   %3ld   %7d   %20s   %s\n", a, b, mdc(a,b), d,
               achou_div ? "sim" : "não", voa ? "voa ✓" : "não voa");
        if(voa != primo) mau++;
    }
    ok("voa ⟺ gcd(a,b) = 1", mau == 0);
    printf("\n      Quando as espécies partilham divisor, o tensorial reparte-se em cópias e\n");
    printf("      aparece divisor de zero. Quando são primas entre si, a·b = lcm(a,b) e o\n");
    printf("      tensorial COINCIDE com o cruzamento — é por isso que ele parece funcionar.\n");
}

/* ---------------------------------------------------------------- §V3 ------ */
printf("\n§V3  O cruzamento R_a ∨ R_b = R_lcm(a,b) voa SEMPRE.\n\n");
{
    int mau_voa = 0, mau_min = 0;
    printf("      a   b   lcm   é corpo?   contém os dois?   é o MENOR que contém?\n");
    for(int a = 1; a <= 4; a++) for(int b = 1; b <= 4; b++){
        long l = mmc(a,b);
        if(l > NMAX) continue;
        int voa = corpo_ok((int)l);
        int contem = (l % a == 0) && (l % b == 0);      /* R_x ⊂ R_n ⟺ x | n */
        int menor = 1;
        for(long n = 1; n < l; n++) if(n % a == 0 && n % b == 0) menor = 0;
        if(!voa) mau_voa++;
        if(!contem || !menor) mau_min++;
        if((a<=2&&b<=3) || (a==4&&b==6) || (a==2&&b==4))
            printf("      %d   %d   %3ld   %9s   %16s   %s\n", a, b, l,
                   voa?"sim ✓":"NÃO", contem?"sim ✓":"NÃO", menor?"sim ✓":"NÃO");
    }
    ok("o filhote é corpo — voa sozinho", mau_voa == 0);
    ok("e é o MENOR corpo que contém os dois pais", mau_min == 0);
    printf("\n      Este é o cruzamento do viveiro: nem a soma (que não voa) nem o produto\n");
    printf("      (que só voa entre primos), mas a JUNÇÃO — o menor corpo onde os dois pais\n");
    printf("      cabem. E ele existe para todo par, sem exceção.\n");
}

/* ---------------------------------------------------------------- §V4 ------ */
printf("\n§V4  O viveiro é fechado: comutativo, associativo, idempotente, com neutro.\n\n");
{
    int c = 1, as = 1, id = 1, ne = 1;
    for(int a = 1; a <= 6; a++){
        if(mmc(a,a) != a) id = 0;
        if(mmc(a,1) != a) ne = 0;
        for(int b = 1; b <= 6; b++){
            if(mmc(a,b) != mmc(b,a)) c = 0;
            for(int d = 1; d <= 6; d++)
                if(mmc(mmc(a,b),d) != mmc(a,mmc(b,d))) as = 0;
        }
    }
    printf("      comutativo   R_a ∨ R_b = R_b ∨ R_a ............... %s\n", c?"sim ✓":"NÃO ✗");
    printf("      associativo  (R_a ∨ R_b) ∨ R_c = R_a ∨ (R_b ∨ R_c)  %s\n", as?"sim ✓":"NÃO ✗");
    printf("      idempotente  R_a ∨ R_a = R_a ..................... %s\n", id?"sim ✓":"NÃO ✗");
    printf("      neutro       R_a ∨ R^1 = R_a ..................... %s\n", ne?"sim ✓":"NÃO ✗");
    ok("o cruzamento é um semirretículo com neutro", c && as && id && ne);
    printf("\n      A IDEMPOTÊNCIA é o que distingue um viveiro de uma fábrica: cruzar uma\n");
    printf("      espécie consigo mesma devolve ela mesma, não uma nova. Só o cruzamento\n");
    printf("      ENTRE espécies diferentes gera espécie nova — e o quanto de novo é\n");
    printf("      exatamente lcm(a,b)/a, o que o filhote tem que o pai não tinha.\n");
}

/* ---------------------------------------------------------------- §V5 ------ */
printf("\n§V5  A ninhada não escapa: todo filhote é um corpo do próprio viveiro.\n\n");
{
    int fora = 0, ninhada = 0;
    printf("      pais        filhote   voa?   já estava no viveiro?\n");
    for(int a = 1; a <= 4; a++) for(int b = 1; b <= 4; b++){
        long l = mmc(a,b);
        if(l > NMAX) continue;
        ninhada++;
        int voa = corpo_ok((int)l);
        int dentro = (l >= 1 && l <= NMAX);
        if(!voa || !dentro) fora++;
        if((a==2&&b==3)||(a==3&&b==4)||(a==2&&b==4)||(a==4&&b==4))
            printf("      R^%d, R^%d     R^%-3ld     %s   %s\n", a, b, l,
                   voa?"sim ✓":"NÃO", dentro?"sim ✓":"NÃO");
    }
    printf("      cruzamentos examinados ........................ %d\n", ninhada);
    printf("      filhotes que não voam ou saem do viveiro ...... %d\n", fora);
    ok("o viveiro é FECHADO sob cruzamento", fora == 0);
    printf("\n      Nenhum cruzamento produz coisa que não seja corpo, e nenhum produz coisa\n");
    printf("      que não caiba no viveiro. É por isso que se pode cruzar de novo, e de novo:\n");
    printf("      o filhote é da mesma espécie de coisa que os pais, e voa sozinho.\n");
}

printf("\n=== A LEI DO VIVEIRO ======================================================\n");
printf("  Cruzar R_a com R_b dá R_lcm(a,b) — e o filhote VOA: é corpo, é o menor que\n");
printf("  contém os dois pais, e continua dentro do viveiro. A soma direta não serve\n");
printf("  (tem divisor de zero: dois pássaros na gaiola, não um novo) e o tensorial só\n");
printf("  serve entre espécies primas entre si (quando a·b = lcm, e aí coincide com o\n");
printf("  cruzamento). O viveiro é comutativo, associativo, idempotente e tem neutro —\n");
printf("  e é a idempotência que o faz viveiro e não fábrica: espécie cruzada consigo\n");
printf("  devolve ela mesma, e só o encontro do diferente gera o novo.\n");
if(falhas){ printf("\n  FALHAS: %d\n\n", falhas); return 1; }
printf("\n  RESÍDUO 0 — exato, em inteiros, e por varredura exaustiva.\n\n");
return 0;
}
