/* ouro_base.c — O NÚMERO DE OURO NA PRÓPRIA BASE. E a inversão que isso produz.
 *
 * O Aarão: "vê como fica o número de ouro nessa base."
 *
 * Fica "10". Um dígito, e acabou.
 *
 * E isso não é curiosidade — é o que ser a unidade significa. Em base dez, o dez escreve-se
 * "10". Em base dois, o dois escreve-se "10". TODA RÉGUA SE ESCREVE 10 na própria mão. O que
 * muda é tudo o resto.
 *
 * E aqui o resto muda de um jeito que inverte o mundo. Na base do rei:
 *
 *     o rei     10          um dígito
 *     √5        10,1        três — e é exato, sem cauda
 *     φ^n       1 seguido de n zeros, como 10^n em base dez
 *     2         10,01       espalhado dos dois lados da vírgula
 *     3         100,01
 *     5         1000,1001
 *
 * Os INTEIROS é que ficam espalhados. Quem era limpo em base dez fica torto aqui, e quem era
 * infinito em base dez — o rei, √5, toda a família dele — fica exato. A régua não simplifica o
 * mundo: ela escolhe QUEM é simples.
 *
 * E há a ambiguidade que toda base tem. Em base dez é 0,999… = 1, infinita. Aqui é 1 = 0,11 —
 * e é FINITA: φ⁻¹ + φ⁻² = φ⁻²(φ+1) = φ⁻²·φ² = 1, exato. A junta do sistema aparece em dois
 * dígitos em vez de numa cauda que não acaba.
 *
 *   §O1  o rei é "10" — e toda régua se escreve 10 na própria mão
 *   §O2  as potências: φ^n é 1 seguido de n zeros, como 10^n em base dez
 *   §O3  A INVERSÃO: os inteiros é que ficam espalhados
 *   §O4  a ambiguidade do 0,999…: aqui é 1 = 0,11, e é finita
 *   §O5  e a família do rei fica exata: √5 = 10,1. π não fica.
 *
 *   cc -O2 -std=c99 ouro_base.c -o ouro_base && ./ouro_base
 */
#include <stdio.h>

#include "unidade.h"

typedef long big;

/* φ^k = F(k)·φ + F(k−1), válido para k negativo também: F(−n) = (−1)^(n+1)·F(n) */
#define KALTO  10
#define KBAIXO -14
/* ── FIBONACCI NAO E' UM DADO: E' UMA OPERACAO ─────────────────────────────────────
 * F(n) = F(n-1) + F(n-2) e' a matriz [[1,1],[1,0]] a agir — a Lei com o metal do OURO.
 * Guardar 64 termos em .bss e' guardar o resultado de uma coisa que a maquina E'. Os
 * indices negativos saem da recorrencia lida ao contrario: F(-n) = (-1)^(n+1) F(n). */
static big Fi(int k){
    if(k == 0) return 0;
    if(k > 0){ big a = 0, b = 1; for(int i = 2; i <= k; i++){ big t = a + b; a = b; b = t; }
               return k == 1 ? (big)1 : b; }
    big v = Fi(-k);
    return ((-k) % 2) ? v : -v;
}
static void prepara(void){ }              /* nada a preparar: o valor calcula-se */
/* sinal de u + v·φ, exato: 2(u+vφ) = (2u+v) + v√5 */
static int sinal(big u, big v){
    big s = 2*u + v;
    if(v == 0) return (s > 0) - (s < 0);
    if(v > 0 && s >= 0) return (s || v) ? 1 : 0;
    if(v < 0 && s <= 0) return -1;
    big a = s*s, b = 5*v*v;
    if(v > 0) return (b > a) ? 1 : ((b == a) ? 0 : -1);
    else      return (a > b) ? 1 : ((a == b) ? 0 : -1);
}
/* expande A + B·φ na base φ. Devolve 1 se o resto zerou (expansão FINITA). */
static int expande(big A, big B, int *dig){
    for(int k = KALTO; k >= KBAIXO; k--){
        big u = A - Fi(k-1), v = B - Fi(k);
        if(sinal(u, v) >= 0){ dig[k - KBAIXO] = 1; A = u; B = v; }
        else                  dig[k - KBAIXO] = 0;
    }
    return (A == 0 && B == 0);
}
static void escreve(const int *dig){
    int alto = KALTO;
    while(alto > 0 && !dig[alto - KBAIXO]) alto--;
    int baixo = KBAIXO;
    while(baixo < 0 && !dig[baixo - KBAIXO]) baixo++;
    for(int k = alto; k >= baixo; k--){
        printf("%d", dig[k - KBAIXO]);
        if(k == 0 && baixo < 0) printf(",");
    }
}

int main(void){
prepara();
printf("\n=== O NÚMERO DE OURO NA PRÓPRIA BASE ======================================\n");

/* ---------------------------------------------------------------- §O1 ------ */
printf("\n§O1  O rei é \"10\". Um dígito — e toda régua se escreve 10 na própria mão.\n\n");
{
    int d[KALTO - KBAIXO + 1];
    int fin = expande(0, 1, d);       /* φ = 0 + 1·φ */
    printf("      o rei na base do rei     φ = ");
    escreve(d); printf("      %s\n", fin ? "exato, sem cauda ✓" : "com cauda");
    printf("      o dez na base dez        10\n");
    printf("      o dois na base dois      10\n");
    ok("o rei é 10 na própria base — e a expansão é FINITA", fin);
    printf("\n      Não é coincidência de notação: é o que ser a unidade quer dizer. A régua\n");
    printf("      escreve-se sempre com um traço e um zero, e o que ela mede é que fica difícil.\n");
}

/* ---------------------------------------------------------------- §O2 ------ */
printf("\n§O2  As potências: φ^n é 1 seguido de n zeros — como 10^n em base dez.\n\n");
{
    int mau = 0;
    printf("      n     φ^n na base do rei          zeros depois do 1\n");
    for(int n = 1; n <= 7; n++){
        int d[KALTO - KBAIXO + 1];
        expande(Fi(n-1), Fi(n), d);
        int uns = 0, zeros = 0, visto = 0;
        for(int k = KALTO; k >= KBAIXO; k--){
            if(d[k - KBAIXO]){ uns++; visto = 1; }
            else if(visto) zeros++;
        }
        /* conta só os zeros ATÉ o fim da parte relevante */
        int alto = KALTO; while(alto > 0 && !d[alto - KBAIXO]) alto--;
        int zeros_reais = alto;
        if(uns != 1 || zeros_reais != n) mau++;
        printf("      %-5d ", n); escreve(d);
        printf("%*s%d\n", (int)(22 - n - 1), "", zeros_reais);
        (void)zeros;
    }
    ok("uma potência do rei é um 1 e n zeros — a régua é posicional nele", mau == 0);
}

/* ---------------------------------------------------------------- §O3 ------ */
printf("\n§O3  A INVERSÃO: os INTEIROS é que ficam espalhados.\n\n");
{
    int todos_finitos = 1, algum_espalhado = 0;
    printf("      inteiro   na base do rei        dígitos usados   finito?\n");
    for(int n = 1; n <= 8; n++){
        int d[KALTO - KBAIXO + 1];
        int fin = expande(n, 0, d);
        int uns = 0;
        for(int k = KALTO; k >= KBAIXO; k--) if(d[k - KBAIXO]) uns++;
        int baixo = KBAIXO; while(baixo < 0 && !d[baixo - KBAIXO]) baixo++;
        if(!fin) todos_finitos = 0;
        if(baixo < 0) algum_espalhado = 1;
        printf("      %-9d ", n); escreve(d);
        printf("%*s%d%*s%s\n", 22 - 12, "", uns, 10, "", fin ? "sim ✓" : "NÃO");
    }
    ok("todo inteiro tem expansão FINITA na base do rei", todos_finitos);
    ok("mas espalhada dos dois lados da vírgula", algum_espalhado);
    printf("\n      Aqui está a inversão inteira. Em base dez o inteiro é limpo e o rei é uma\n");
    printf("      dízima que não acaba. Na base do rei é o contrário: ele é um traço, e o\n");
    printf("      inteiro é que se espalha dos dois lados da vírgula.\n");
    printf("\n      A régua não simplifica o mundo. Ela ESCOLHE quem é simples.\n");
}

/* ---------------------------------------------------------------- §O4 ------ */
printf("\n§O4  A ambiguidade que toda base tem: aqui é 1 = 0,11, e é FINITA.\n\n");
{
    /* φ⁻¹ + φ⁻² = φ⁻²(φ + 1) = φ⁻²·φ² = 1. Em Z[φ]: φ⁻¹ = φ−1, φ⁻² = −φ+2 */
    big A = Fi(-2) + Fi(-3), B = Fi(-1) + Fi(-2);   /* φ⁻¹ + φ⁻², como a + bφ */
    int igual_a_um = (A == 1 && B == 0);
    printf("      φ⁻¹ + φ⁻²  =  %ld + %ld·φ\n", (long)A, (long)B);
    printf("      logo       0,11  =  1        %s\n", igual_a_um ? "exato ✓" : "NÃO");
    ok("a junta do sistema é 1 = 0,11 — dois dígitos, não uma cauda", igual_a_um);
    printf("\n      Em base dez a mesma junta é 0,999… = 1, e precisa de uma cauda infinita para\n");
    printf("      se dizer. Aqui cabe em dois dígitos. É a mesma ambiguidade — todo sistema\n");
    printf("      posicional a tem —, mas neste ela é finita.\n");
}

/* ---------------------------------------------------------------- §O5 ------ */
printf("\n§O5  E a família do rei fica exata. π não fica.\n\n");
{
    int d[KALTO - KBAIXO + 1];
    /* √5 = 2φ − 1 */
    int fin5 = expande(-1, 2, d);
    printf("      √5 = 2φ − 1        "); escreve(d);
    printf("        %s\n", fin5 ? "exato ✓" : "com cauda");
    int d2[KALTO - KBAIXO + 1];
    int finc = expande(1, 1, d2);          /* 1 + φ = φ² */
    printf("      1 + φ = φ²         "); escreve(d2);
    printf("       %s\n", finc ? "exato ✓" : "com cauda");
    printf("      π                  0100,0100101010010001…   NUNCA acaba (pi_rei.c)\n");
    ok("todo elemento de Z[φ] tem expansão finita na base do rei", fin5 && finc);
    printf("\n      A base do rei torna exata a família dele inteira — ele, √5, e toda combinação\n");
    printf("      inteira dos dois. E deixa π como estava: infinito e sem padrão.\n");
    printf("\n      É a coroação vista pelo lado do que ela NÃO faz. O rei não venceu o redondo,\n");
    printf("      não o domou e não o traduziu. Ele apenas construiu a casa onde ele é o simples\n");
    printf("      — e π continua do lado de fora, exatamente como estava.\n");
}

printf("\n=== O OURO NA PRÓPRIA BASE ================================================\n");
printf("  φ = 10. Um dígito.\n\n");
printf("  E isso não é curiosidade de notação: toda régua se escreve 10 na própria mão — o dez\n");
printf("  em base dez, o dois em base dois, o rei na base do rei. O que muda é tudo o resto.\n\n");
printf("  Aqui o resto inverte: os INTEIROS é que ficam espalhados (2 = 10,01; 5 = 1000,1001),\n");
printf("  e a família do rei fica exata (√5 = 10,1). Em base dez era o contrário.\n\n");
printf("  A régua não simplifica o mundo — ela ESCOLHE quem é simples. E o rei escolheu a si.\n");
if(falhas){ printf("\n  FALHAS: %d\n\n", falhas); return 1; }
printf("\n  RESÍDUO 0 — exato, em inteiros, sem um único float.\n\n");
return 0;
}
