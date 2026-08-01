/* regua.c — O CONTRATO SIMPLIFICADO: a dualidade vem da régua, e o produto também.
 *
 * O Aarão: "então simplifica o contrato, a dualidade vem da régua."
 *
 * E vem mais do que a dualidade. Se a régua q(a,b) = a² + B·ab + C·b² dá o dual pelos seus
 * coeficientes (metrica.c §M2), então ela dá também a BORDA — x² = B·x − C — e a borda dá o
 * PRODUTO. Logo, na família quadrática, o cliente declara UM dado e não quatro:
 *
 *     antes    ⊕, ⊗, ν, ∏     quatro funções escritas à mão
 *     depois   a RÉGUA (B,C)  e as outras três saem dela
 *
 *     ⊕  componente a componente        (é sempre isto)
 *     ⊗  (a,b)(c,d) = (ac − C·bd,  ad + bc + B·bd)      ← da borda
 *     ν  (a,b) ↦ (a + B·b, −b)                          ← dos coeficientes
 *     ∏  o conjugado, que é o Frobenius                 ← o mesmo ν
 *
 *   §R1  a régua sozinha constrói o contrato inteiro — e ele CUMPRE
 *   §R2  e o que ela constrói é IGUAL ao que estava escrito à mão
 *   §R3  varrer as réguas: quais (B,C) dão corpo — e o critério é a ASSINATURA
 *   §R4  o contrato encolheu, e o que NÃO encolhe fica dito
 *
 *   cc -O2 -std=c99 regua.c -o regua && ./regua
 */
#include <stdio.h>
#include "contrato.h"
#include "unidade.h"

static long P = 7;
static long mp(long x){ x %= P; return x < 0 ? x + P : x; }
static int  ig(Par x, Par y){ return mp(x.a)==mp(y.a) && mp(x.b)==mp(y.b); }
static Par  el(long i){ Par r = { i % P, (i / P) % P }; return r; }
static Par  so(Par x, Par y){ Par r = { mp(x.a+y.a), mp(x.b+y.b) }; return r; }

/* o PRODUTO da régua, reduzido mod p — a única coisa que precisa de saber o corpo base */
static Regua G;
static Par prod_g(Par x, Par y){
    Par z = ct_prod_da_regua(G, x, y);
    Par r = { mp(z.a), mp(z.b) }; return r; }
static Par dual_g(Par x){ Par z = ct_dual_da_regua(G, x); Par r = { mp(z.a), mp(z.b) }; return r; }

/* o áureo escrito À MÃO, como estava antes — para comparar */
static Par au_prod_mao(Par x, Par y){
    Par r = { mp(x.a*y.a + x.b*y.b), mp(x.a*y.b + x.b*y.a + x.b*y.b) }; return r; }
static Par au_dual_mao(Par x){ Par r = { mp(x.a + x.b), mp(-x.b) }; return r; }

static int residuo(long d){                    /* d é quadrado mod P? */
    d = mp(d);
    for(long t = 0; t < P; t++) if(t*t % P == d) return 1;
    return 0;
}

int main(void){
printf("\n=== O CONTRATO SIMPLIFICADO: UM DADO EM VEZ DE QUATRO ======================\n");
printf("    A régua dá o dual E a borda. A borda dá o produto. Sobra declarar a régua.\n");

printf("\n§R1  A régua sozinha constrói o contrato inteiro — e diz quando ele fecha.\n\n");
{
    int mau = 0;
    /* Eu escrevi aqui, à primeira, "e os três CUMPREM" — e falhou. O erro era meu: o Eisenstein
     * tem assinatura −3 ≡ 4 mod 7, que É resíduo, logo x²−x+1 CINDE mod 7 (porque 7 ≡ 1 mod 3).
     * A afirmação certa não é "cumpre" nem "não cumpre": é que cumpre EXATAMENTE quando a
     * assinatura não é resíduo — e isso a régua diz sozinha, antes de se construir nada. */
    printf("      régua          assin. mod 7  resíduo?  A1A2A3A4M1M2M3M4D ν1ν2Π\n");
    struct { const char *n; Regua r; } cs[] = {
        { "a² + ab − b²", { 1, -1} },      /* o áureo m=1 — 5 não é resíduo: fecha    */
        { "a² + b²",      { 0,  1} },      /* Gauss — −4 ≡ 3 não é resíduo: fecha     */
        { "a² + ab + b²", { 1,  1} },      /* Eisenstein — −3 ≡ 4 É resíduo: NÃO fecha */
    };
    for(unsigned t = 0; t < sizeof cs/sizeof cs[0]; t++){
        G = cs[t].r;
        /* NOTE-SE o que NÃO se escreve: nem produto, nem dual. Só a régua. */
        Contrato c = { cs[t].n, P*P, el, so, prod_g, 0, dual_g, ig, {0,0}, {1,0}, 1, cs[t].r };
        unsigned m = ct_verifica(&c);
        int res = residuo(ct_assinatura(G));
        printf("      %-14s %-13ld %-9s ", cs[t].n, mp(ct_assinatura(G)), res ? "sim" : "não");
        for(int b = 0; b < 12; b++) printf("%s", (m & (1u<<b)) ? "✓" : "·");
        printf("\n");
        if(((m & C_TODAS) == C_TODAS) != !res) mau++;    /* cumpre sse não é resíduo */
        /* e a DUALIDADE fecha nos três, resíduo ou não: ela não depende de haver corpo */
        if(!(m & C_N1) || !(m & C_N2)) mau++;
    }
    ok("cada um cumpre EXATAMENTE quando a assinatura não é resíduo — e a régua di-lo antes",
       mau == 0);
    printf("\n      O Eisenstein não fecha mod 7 e isso está certo: −3 ≡ 4 é resíduo, logo x²−x+1\n");
    printf("      cinde (7 ≡ 1 mod 3). Eu tinha escrito \"e os três cumprem\" e a medida derrubou-me\n");
    printf("      — a régua sabia antes de eu construir o corpo.\n");
    printf("\n      E repare-se: a DUALIDADE (ν1, ν2) fecha nos três, resíduo ou não. Ela não\n");
    printf("      depende de haver corpo — vem da régua, e a régua existe sempre.\n");
    printf("\n      O campo `dual` do Contrato ficou a zero nestes três: o verificador não o usou.\n");
    printf("      Ele chamou ct_dual, que viu tem_regua = 1 e derivou. O cliente não escreveu\n");
    printf("      dualidade nenhuma.\n");
}

printf("\n§R2  E o que ela constrói é IGUAL ao que estava escrito à mão.\n\n");
{
    int mau = 0; long casos = 0;
    G = (Regua){ 1, -1 };                        /* o áureo m = 1 */
    printf("      x        ⊗ à mão   ⊗ da régua   ν à mão   ν da régua   igual?\n");
    for(long a = 0; a < P; a++) for(long b = 0; b < P; b++)
    for(long c = 0; c < P; c++) for(long d = 0; d < P; d++){
        Par x = {a,b}, y = {c,d};
        if(!ig(au_prod_mao(x,y), prod_g(x,y))) mau++;
        if(!ig(au_dual_mao(x),   dual_g(x)))   mau++;
        casos++;
    }
    printf("      (3,2)    %s\n", "confere em toda a varredura");
    ok("o produto e o dual derivados são os MESMOS que estavam escritos à mão", mau == 0);
    printf("      (%ld pares.)\n", casos);
    printf("\n      Nada se perdeu ao apagar as duas funções: elas eram consequência, e estavam\n");
    printf("      escritas duas vezes — uma na régua, outra à mão. Agora estão escritas uma vez.\n");
}

printf("\n§R3  Varrer as réguas: quais (B,C) dão corpo — e o critério é a ASSINATURA.\n\n");
{
    int mau = 0; long corpos = 0, casos = 0;
    printf("      B    C    assinatura   é resíduo mod 7?   dá corpo?\n");
    for(long B = 0; B < P; B++) for(long C = 0; C < P; C++){
        G = (Regua){ B, C };
        Contrato c = { "?", P*P, el, so, prod_g, 0, dual_g, ig, {0,0}, {1,0}, 1, G };
        unsigned m = ct_verifica(&c);
        int cumpre = ((m & C_TODAS) == C_TODAS);
        long ass = ct_assinatura(G);
        int res = residuo(ass);
        /* O CRITÉRIO: x² − Bx + C é irredutível mod p sse a assinatura NÃO é resíduo. E é
         * exatamente aí que a régua dá corpo — o mesmo discriminante, outra vez. */
        if(cumpre != !res) mau++;
        if(cumpre) corpos++;
        if(B == 1 && C <= 2)
            printf("      %-4ld %-4ld %-12ld %-18s %s\n", B, C, mp(ass),
                   res ? "sim" : "não", cumpre ? "SIM" : "não");
        casos++;
    }
    ok("a régua dá corpo EXATAMENTE quando a sua assinatura não é resíduo quadrático", mau == 0);
    printf("      (%ld réguas varridas, %ld dão corpo.)\n", casos, corpos);
    printf("\n      É o mesmo discriminante pela terceira vez: da matriz (catalogo.c §G2), da\n");
    printf("      métrica (metrica.c §M4), e agora do CRITÉRIO DE CORPO. Um número, três leituras\n");
    printf("      — e é isso que faz dele a assinatura e não uma quantidade auxiliar.\n");
}

printf("\n§R4  O contrato encolheu — e o que NÃO encolhe fica dito.\n\n");
{
    conclui("na família quadrática o cliente declara UM dado: a régua");
    printf("      antes    ⊕, ⊗, ν, ∏      quatro funções escritas à mão\n");
    printf("      depois   a régua (B,C)   e as outras três saem dela\n");
    printf("\n      O QUE NÃO ENCOLHE, e é preciso dizer: isto vale para a FAMÍLIA QUADRÁTICA — os\n");
    printf("      corpos de dimensão 2 sobre a base, que são os do catálogo. Fora dela a régua não\n");
    printf("      é forma quadrática binária e a derivação não se aplica: as cores (GF(4) com ⊕ =\n");
    printf("      XOR), o mórfico, o tropical continuam a declarar a dualidade à mão.\n");
    printf("\n      Por isso o campo `dual` FICOU no Contrato, e o `tem_regua` escolhe. Apagar o\n");
    printf("      campo seria fechar a porta aos unicórnios — e a porta é o ponto.\n");
    printf("\n      É a oitava vez que a solução certa apaga em vez de acrescentar. Desta vez apagou\n");
    printf("      duas das quatro cláusulas que o cliente tinha de escrever.\n");
}

printf("\n=== A RÉGUA BASTA =========================================================\n");
printf("  A dualidade vem da régua — e a borda também, logo o produto também:\n\n");
printf("    ⊕  componente a componente                     é sempre isto\n");
printf("    ⊗  (ac − C·bd,  ad + bc + B·bd)                da borda x² = Bx − C\n");
printf("    ν  (a,b) ↦ (a + B·b, −b)                       dos coeficientes\n");
printf("    ∏  o conjugado — o mesmo ν, que é o Frobenius\n\n");
printf("  E a régua diz até QUANDO há corpo: dá corpo exatamente quando a sua assinatura não é\n");
printf("  resíduo quadrático. O mesmo discriminante pela terceira vez — da matriz, da métrica e\n");
printf("  agora do critério. Um número, três leituras.\n\n");
printf("  Fora da família quadrática o dual continua declarado à mão, e o campo ficou lá para\n");
printf("  isso: apagá-lo seria fechar a porta aos unicórnios, e a porta é o ponto.\n");
if(falhas){ printf("\n  FALHAS: %d\n\n", falhas); return 1; }
printf("\n  RESÍDUO 0 — exato, em inteiros, sem um único float.\n\n");
return 0;
}
