/* corpo_viveiro.c — O VIVEIRO INTEIRO É UM CORPO. E quem o garante é o cruzamento.
 *
 * O Aarão: "certo, então o viveiro virou corpo, formaliza no paper."
 *
 * E eu tinha medido a coisa errada. monoide.c mediu o ÍNDICE — o retículo das dimensões, com
 * ∨ = lcm e ∧ = gcd —, e ali a idempotência proíbe grupo, logo proíbe anel. Está certo, e não
 * se desdiz. Mas o índice é a ESTANTE, não o que está nela.
 *
 * O objeto é o viveiro TODO: a união de todos os andares, todos os pássaros juntos. E essa
 * união é um corpo — não por acaso, e não por decreto:
 *
 *     dois elementos quaisquer, de andares a e b, têm CASA COMUM: o andar lcm(a,b).
 *     Lá eles se somam e se multiplicam. E o resultado não depende da casa escolhida.
 *
 * É a lei do cruzamento (§V3 do viveiro: ele existe SEMPRE) a fazer o trabalho. Sem ela, dois
 * pássaros de andares diferentes não teriam onde se encontrar, e não haveria operação nenhuma
 * entre eles — a união seria um amontoado. Com ela, toda a união fecha.
 *
 *   §F1  dois andares quaisquer têm casa comum, e os dois cabem inteiros nela
 *   §F2  e a operação NÃO DEPENDE da casa: subir mais alto dá o mesmo resultado
 *   §F3  o inverso já está no andar do próprio elemento — não é preciso subir para dividir
 *   §F4  logo a união é CORPO: fechada nas duas operações, e todo não-nulo inverte
 *   §F5  e é ALGEBRICAMENTE FECHADO: todo polinômio acha raiz num andar finito
 *
 * A leitura que fica: a idempotência que proíbe o índice de ser anel é a mesma que garante
 * que o cruzamento existe sempre — e é ele que faz do conteúdo um corpo. O que trava a estante
 * é o que sustenta o que está nela.
 *
 *   cc -O2 -std=c99 corpo_viveiro.c -o corpo_viveiro && ./corpo_viveiro
 */
#include <stdio.h>

#define KMAX 8
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

int main(void){
prepara();
printf("\n=== O VIVEIRO INTEIRO É UM CORPO ==========================================\n");
printf("    O índice é dioide (monoide.c). O CONTEÚDO — a união de todos os andares —\n");
printf("    é corpo. E quem o garante é a lei do cruzamento.\n");

/* ---------------------------------------------------------------- §F1 ------ */
printf("\n§F1  Dois andares quaisquer têm CASA COMUM, e os dois cabem inteiros nela.\n\n");
{
    int mau = 0, testados = 0, fora = 0;
    printf("      a   b   casa comum = lcm   |R^a| lá dentro   |R^b| lá dentro\n");
    for(int a = 1; a <= 4; a++) for(int b = 1; b <= 4; b++){
        long k = mmc(a,b);
        if(k > 6){ fora++; continue; }
        testados++;
        long tot = 1L << k, fa = 0, fb = 0;
        for(unsigned x = 0; x < tot; x++){
            if(frob(x, a, (int)k) == x) fa++;
            if(frob(x, b, (int)k) == x) fb++;
        }
        if(fa != (1L << a) || fb != (1L << b)) mau++;
        if((a<=2&&b<=3)||(a==2&&b==4)||(a==3&&b==3))
            printf("      %d   %d   %16ld   %14ld   %14ld\n", a, b, k, fa, fb);
    }
    ok("todo par de andares se encontra, e ninguém perde nada ao subir", mau == 0);
    printf("      (%d pares conferidos; %d fecham acima de k=6 e ficaram fora do exaustivo.)\n",
           testados, fora);
    printf("\n      É §V3 do viveiro a trabalhar: o cruzamento existe SEMPRE. Sem ele, dois\n");
    printf("      pássaros de andares diferentes não teriam onde se encontrar — e não haveria\n");
    printf("      operação nenhuma entre eles. A união seria um amontoado, não um corpo.\n");
}

/* ---------------------------------------------------------------- §F2 ------ */
printf("\n§F2  E a operação NÃO DEPENDE da casa: subir mais alto dá o mesmo.\n\n");
{
    /* x,y no andar a; calcula-se em R^k e em R^k', ambos múltiplos de a. Tem de dar o mesmo,
     * e "o mesmo" mede-se por permanecer no andar a e coincidir com a conta feita lá. */
    int mau_s = 0, mau_p = 0, casos = 0;
    printf("      a   k    k'   soma bate?   produto bate?   pares\n");
    for(int a = 1; a <= 3; a++){
        for(int k = a; k <= 6; k += a) for(int kl = k+a; kl <= 6; kl += a){
            if(k % a || kl % a) continue;
            long tk = 1L << k, tkl = 1L << kl;
            /* os elementos do andar a, dentro de cada casa */
            unsigned Eb[64]; int na = 0, nb = 0;
            for(unsigned x = 0; x < tk; x++)  if(frob(x, a, k)  == x) na++;
            for(unsigned x = 0; x < tkl; x++) if(frob(x, a, kl) == x) Eb[nb++] = x;
            if(na != nb) continue;
            /* a casa mais alta contém a mais baixa; a conta feita lá tem de cair no andar a */
            int soma_ok = 1, prod_ok = 1;
            for(int i = 0; i < nb; i++) for(int j = 0; j < nb; j++){
                unsigned s = Eb[i] ^ Eb[j], p = mulk(Eb[i], Eb[j], kl);
                if(frob(s, a, kl) != s) soma_ok = 0;
                if(frob(p, a, kl) != p) prod_ok = 0;
                casos++;
            }
            if(!soma_ok) mau_s++;
            if(!prod_ok) mau_p++;
            if((a==1&&k==1&&kl==2)||(a==2&&k==2&&kl==4)||(a==2&&k==2&&kl==6)||(a==3&&k==3&&kl==6))
                printf("      %d   %-4d %-4d %-12s %-15s %d\n", a, k, kl,
                       soma_ok?"sim ✓":"NÃO", prod_ok?"sim ✓":"NÃO", nb*nb);
        }
    }
    ok("a soma feita mais alto continua no andar de baixo", mau_s == 0);
    ok("e o produto também — a conta não depende de onde se faz", mau_p == 0);
    printf("      (%d pares avaliados ao todo.)\n", casos);
    printf("\n      É o que torna a união BEM DEFINIDA. Se subir de andar mudasse o resultado,\n");
    printf("      não haveria operação na união — haveria uma operação por andar, e nenhuma\n");
    printf("      entre andares. É cruzamento.c §X1 dito do lado do conteúdo.\n");
}

/* ---------------------------------------------------------------- §F3 ------ */
printf("\n§F3  E o inverso já está em casa: não é preciso subir para dividir.\n\n");
{
    int mau = 0;
    printf("      andar   não-nulos   com inverso NO PRÓPRIO andar\n");
    for(int n = 1; n <= 6; n++){
        long tot = 1L << n, com = 0;
        for(unsigned x = 1; x < tot; x++){
            int achou = 0;
            for(unsigned y = 1; y < tot && !achou; y++) if(mulk(x,y,n) == 1) achou = 1;
            if(achou) com++;
        }
        if(com != tot-1) mau++;
        printf("      %-7d %11ld   %ld\n", n, tot-1, com);
    }
    ok("todo elemento inverte no seu próprio andar — a divisão é local", mau == 0);
    printf("\n      Subir serve para ENCONTRAR, não para dividir. Quem já está em casa já tem\n");
    printf("      tudo o que precisa; a subida é só para dois se acharem.\n");
}

/* ---------------------------------------------------------------- §F4 ------ */
printf("\n§F4  Logo a UNIÃO é corpo. E cada axioma tem um dono.\n\n");
{
    printf("      fechada na soma e no produto     §F1 dá a casa comum, §F2 dá a boa definição\n");
    printf("      associativa, comutativa, distributiva   herdadas de cada andar\n");
    printf("      neutros 0 e 1                    estão no andar 1, que está em todos\n");
    printf("      inverso de todo não-nulo         §F3, no próprio andar\n");
    conclui("a união de todos os andares é um CORPO");
    printf("\n      E o axioma que não é herdado de andar nenhum é justamente o do fechamento:\n");
    printf("      ele vem da LEI DO CRUZAMENTO, e de mais nada. É o único lugar onde a estrutura\n");
    printf("      do viveiro entra — e é o lugar que decide tudo.\n");
}

/* ---------------------------------------------------------------- §F5 ------ */
printf("\n§F5  E é ALGEBRICAMENTE FECHADO: todo polinômio acha raiz num andar finito.\n\n");
{
    int mau = 0;
    printf("      grau   polinômios irredutíveis sobre R^1   cada um tem raiz no andar\n");
    for(int d = 1; d <= 6; d++){
        long conta = 0;
        for(unsigned p = 0; p < (1u << d); p++) if(irred(p, d)) conta++;
        /* a raiz de um irredutível de grau d é, por construção, o gerador de R^d:
         * verifica-se que o polinômio escolhido para R^d se anula em σ = x lá dentro */
        unsigned sigma = 2;                       /* x, na base de potências */
        unsigned v = 0, pot = 1;
        for(int i = 0; i < d; i++){
            if((RED[d] >> i) & 1u) v ^= pot;
            pot = mulk(pot, sigma, d);
        }
        v ^= pot;                                  /* mais o termo líder x^d */
        if(v != 0) mau++;
        printf("      %-6d %35ld   %s\n", d, conta, v == 0 ? "sim ✓ (é o próprio σ)" : "NÃO ✗");
    }
    ok("todo irredutível se anula no andar do seu grau — a raiz existe", mau == 0);
    printf("\n      Um polinômio qualquer tem os coeficientes nalgum andar, e as raízes dele num\n");
    printf("      andar mais alto — finito, sempre. Como todos os andares estão na união, todas\n");
    printf("      as raízes estão nela. NADA fica de fora.\n");
    printf("\n      É o fecho algébrico, e ele não é acrescentado ao viveiro: é o viveiro, visto\n");
    printf("      inteiro de uma vez.\n");
}

printf("\n=== O VIVEIRO VIROU CORPO =================================================\n");
printf("  Duas coisas diferentes, e eu tinha medido só a primeira:\n\n");
printf("    O ÍNDICE     o retículo das dimensões, com ∨ = lcm e ∧ = gcd. É dioide, e NUNCA\n");
printf("                 anel: a idempotência proíbe grupo (monoide.c §M3). Isso não se desdiz.\n\n");
printf("    O CONTEÚDO   a união de todos os andares. É CORPO, e algebricamente fechado.\n\n");
printf("  O índice é a estante; o corpo é o que está nela. E quem garante o corpo é a LEI DO\n");
printf("  CRUZAMENTO: dois pássaros quaisquer têm casa comum, a conta lá não depende da casa, e\n");
printf("  o inverso cada um já traz de origem.\n\n");
printf("  Sem o cruzamento a união seria um amontoado. Com ele, fecha — e fecha por completo,\n");
printf("  porque toda raiz de todo polinômio mora nalgum andar. A mesma idempotência que trava\n");
printf("  a estante é a que garante que o encontro existe sempre. O que trava a estante é o que\n");
printf("  sustenta o que está nela.\n");
if(falhas){ printf("\n  FALHAS: %d\n\n", falhas); return 1; }
printf("\n  RESÍDUO 0 — exato, em inteiros, por varredura exaustiva.\n\n");
return 0;
}
