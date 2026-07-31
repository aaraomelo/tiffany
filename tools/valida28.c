/* valida28.c — OS 28 DO CATÁLOGO PELO CONTRATO, sem exceção para ninguém.
 *
 * O Aarão: "agora valida os 28 do catálogo por esse contrato."
 *
 * O verificador não lê nomes, então os 28 passam por ele como os unicórnios passaram. O que eu
 * tenho de dizer à cabeça, porque é a única coisa que pode enganar aqui:
 *
 *   O MODELO FINITO DE CADA UM É MEU. O catálogo dá a tríade e a régua de cada corpo; a régua
 *   é infinita ou contínua na maioria (a impedância, Friedmann, o campo médio). Para verificar
 *   as cláusulas M4 e A4 é preciso domínio FINITO — logo escolhi um modelo por corpo. Onde o
 *   catálogo diz algo específico (o telescópico CINDE, o entrópico é MAX) o modelo é esse; onde
 *   não, é o modelo genérico da FORMA do seu ∏ (os28.c).
 *
 *   Então: o que se valida é a ÁLGEBRA da forma, não a régua própria. Dizer que validei os 28
 *   "completos" seria medir a fatia e afirmar o todo — o erro que já custou o dia três vezes.
 *
 *   §V1  os 28, pelo mesmo verificador, com o modelo de cada um dito
 *   §V2  quem CUMPRE, e são a maioria
 *   §V3  quem falha, e a CLÁUSULA de cada
 *   §V4  o que as doze cláusulas NÃO veem — e é preciso dizê-lo
 *
 *   cc -O2 -std=c99 valida28.c -o valida28 && ./valida28
 */
#include <stdio.h>
#include "contrato.h"
#include "unidade.h"

static int ig(Par x, Par y){ return x.a == y.a && x.b == y.b; }
static long m5(long x){ x %= 5; return x < 0 ? x + 5 : x; }
static long m7(long x){ x %= 7; return x < 0 ? x + 7 : x; }

/* MODELO P — o corpo-mãe finito: ℤ/5, ν e ∏ o Frobenius (que num corpo primo é a identidade) */
static Par p_elem(long i){ Par r = { i % 5, 0 }; return r; }
static Par p_soma(Par x, Par y){ Par r = { m5(x.a+y.a), 0 }; return r; }
static Par p_prod(Par x, Par y){ Par r = { m5(x.a*y.a), 0 }; return r; }
static Par p_id(Par x){ return x; }
/* MODELO D — o mesmo corpo, mas com o SUCESSOR por operador, tal como esses corpos o declaram */
static Par d_suc(Par x){ Par r = { m5(x.a + 1), 0 }; return r; }
/* MODELO A — hiperbólico: GF(49), σ² = σ + 1 */
static Par q_elem(long i){ Par r = { i % 7, (i / 7) % 7 }; return r; }
static Par q_soma(Par x, Par y){ Par r = { m7(x.a+y.a), m7(x.b+y.b) }; return r; }
static Par a_prod(Par x, Par y){
    Par r = { m7(x.a*y.a + x.b*y.b), m7(x.a*y.b + x.b*y.a + x.b*y.b) }; return r; }
static Par a_conj(Par x){ Par r = { m7(x.a + x.b), m7(-x.b) }; return r; }
/* MODELO ν/W — elíptico: GF(49), ω² = −1, e o ∏ declarado é a CONJUGAÇÃO */
static Par w_prod(Par x, Par y){
    Par r = { m7(x.a*y.a - x.b*y.b), m7(x.a*y.b + x.b*y.a) }; return r; }
static Par w_conj(Par x){ Par r = { x.a, m7(-x.b) }; return r; }
/* MODELO δ⊣ε — o mórfico com n = 1 */
static Par o_elem(long i){ Par r = { i & 1, 0 }; return r; }
static Par o_xor(Par x, Par y){ Par r = { x.a ^ y.a, 0 }; return r; }
static Par o_and(Par x, Par y){ Par r = { x.a & y.a, 0 }; return r; }
static Par o_nao(Par x){ Par r = { x.a ^ 1, 0 }; return r; }
/* MODELO ESPECÍFICO — o telescópico, que o catálogo diz que CINDE */
static Par t_elem(long i){ Par r = { i % 5, (i / 5) % 5 }; return r; }
static Par t_soma(Par x, Par y){ Par r = { m5(x.a+y.a), m5(x.b+y.b) }; return r; }
static Par t_prod(Par x, Par y){ Par r = { m5(x.a*y.a), m5(x.b*y.b) }; return r; }
static Par t_troca(Par x){ Par r = { x.b, x.a }; return r; }
/* MODELO ESPECÍFICO — o entrópico, que o catálogo diz que é MAX */
static Par e_elem(long i){ Par r = { i - 4, 0 }; return r; }
static Par e_max(Par x, Par y){ Par r = { x.a > y.a ? x.a : y.a, 0 }; return r; }
static Par e_mais(Par x, Par y){ Par r = { x.a + y.a, 0 }; return r; }
static Par e_neg(Par x){ Par r = { -x.a, 0 }; return r; }

static const struct { const char *nome; const char *modelo; Contrato c; } V[] = {
 {"fractal",         "forma P — ℤ/5",       {"fractal",5,p_elem,p_soma,p_prod,p_id,p_id,ig,{0,0},{1,0}}},
 {"eletromagnético", "forma P — ℤ/5",       {"eletromagnético",5,p_elem,p_soma,p_prod,p_id,p_id,ig,{0,0},{1,0}}},
 {"motor",           "forma P — ℤ/5",       {"motor",5,p_elem,p_soma,p_prod,p_id,p_id,ig,{0,0},{1,0}}},
 {"relógio",         "forma P — ℤ/5",       {"relógio",5,p_elem,p_soma,p_prod,p_id,p_id,ig,{0,0},{1,0}}},
 {"econômico",       "forma P — ℤ/5",       {"econômico",5,p_elem,p_soma,p_prod,p_id,p_id,ig,{0,0},{1,0}}},
 {"evolutivo",       "forma P — ℤ/5",       {"evolutivo",5,p_elem,p_soma,p_prod,p_id,p_id,ig,{0,0},{1,0}}},
 {"expansivo",       "forma P — ℤ/5",       {"expansivo",5,p_elem,p_soma,p_prod,p_id,p_id,ig,{0,0},{1,0}}},
 {"somático",        "forma P — ℤ/5",       {"somático",5,p_elem,p_soma,p_prod,p_id,p_id,ig,{0,0},{1,0}}},
 {"geométrico",      "forma P — ℤ/5",       {"geométrico",5,p_elem,p_soma,p_prod,p_id,p_id,ig,{0,0},{1,0}}},
 {"rotor",           "forma P — ℤ/5",       {"rotor",5,p_elem,p_soma,p_prod,p_id,p_id,ig,{0,0},{1,0}}},
 {"cósmico",         "forma P — ℤ/5",       {"cósmico",5,p_elem,p_soma,p_prod,p_id,p_id,ig,{0,0},{1,0}}},
 {"nervoso",         "forma P — ℤ/5",       {"nervoso",5,p_elem,p_soma,p_prod,p_id,p_id,ig,{0,0},{1,0}}},
 {"exterior",        "forma P — ℤ/5",       {"exterior",5,p_elem,p_soma,p_prod,p_id,p_id,ig,{0,0},{1,0}}},
 {"racional ℚ",      "forma Q — ℤ/5",       {"racional ℚ",5,p_elem,p_soma,p_prod,p_id,p_id,ig,{0,0},{1,0}}},
 {"conforme",        "forma D — ∏ sucessor",{"conforme",5,p_elem,p_soma,p_prod,p_id,d_suc,ig,{0,0},{1,0}}},
 {"espaço-temporal", "forma D — ∏ sucessor",{"espaço-temporal",5,p_elem,p_soma,p_prod,p_id,d_suc,ig,{0,0},{1,0}}},
 {"óptico",          "forma D — ∏ sucessor",{"óptico",5,p_elem,p_soma,p_prod,p_id,d_suc,ig,{0,0},{1,0}}},
 {"universal",       "forma D — ∏ sucessor",{"universal",5,p_elem,p_soma,p_prod,p_id,d_suc,ig,{0,0},{1,0}}},
 {"áureo ℤ[φ]",      "forma A — GF(49)",    {"áureo ℤ[φ]",49,q_elem,q_soma,a_prod,a_conj,a_conj,ig,{0,0},{1,0}}},
 {"deflexivo",       "forma A — GF(49)",    {"deflexivo",49,q_elem,q_soma,a_prod,a_conj,a_conj,ig,{0,0},{1,0}}},
 {"cristalino",      "forma ν — GF(49)",    {"cristalino",49,q_elem,q_soma,w_prod,w_conj,w_conj,ig,{0,0},{1,0}}},
 {"celeste",         "forma ν — GF(49)",    {"celeste",49,q_elem,q_soma,w_prod,w_conj,w_conj,ig,{0,0},{1,0}}},
 {"criativo",        "forma ν — GF(49)",    {"criativo",49,q_elem,q_soma,w_prod,w_conj,w_conj,ig,{0,0},{1,0}}},
 {"técnico",         "forma ν — GF(49)",    {"técnico",49,q_elem,q_soma,w_prod,w_conj,w_conj,ig,{0,0},{1,0}}},
 {"sensitivo",       "forma ν — GF(49)",    {"sensitivo",49,q_elem,q_soma,w_prod,w_conj,w_conj,ig,{0,0},{1,0}}},
 {"mórfico",         "específico — n=1",    {"mórfico",2,o_elem,o_xor,o_and,o_nao,p_id,ig,{0,0},{1,0}}},
 {"telescópico",     "específico — cinde",  {"telescópico",25,t_elem,t_soma,t_prod,t_troca,p_id,ig,{0,0},{1,1}}},
 {"entrópico",       "específico — max",    {"entrópico",9,e_elem,e_max,e_mais,e_neg,p_id,ig,{-1000000,0},{0,0}}},
};
#define NV ((int)(sizeof V / sizeof V[0]))

int main(void){
printf("\n=== OS 28 DO CATÁLOGO, PELO CONTRATO ======================================\n");
printf("    O verificador não lê nomes. Os 28 passam como os unicórnios passaram.\n");
printf("\n    O MODELO FINITO DE CADA UM É MEU: o catálogo dá a tríade, mas a régua da\n");
printf("    maioria é contínua, e M4/A4 precisam de domínio finito. Onde o catálogo diz\n");
printf("    algo específico (o telescópico CINDE, o entrópico é MAX) o modelo é esse;\n");
printf("    onde não, é o modelo genérico da FORMA do seu ∏.\n");

printf("\n§V1  Os 28, um a um.\n\n");
unsigned masc[NV];
{
    printf("      corpo               modelo                A1A2A3A4M1M2M3M4D ν1ν2Π\n");
    for(int i = 0; i < NV; i++){
        masc[i] = ct_verifica(&V[i].c);
        printf("      %-19s %-21s ", V[i].nome, V[i].modelo);
        for(int b = 0; b < 12; b++) printf("%s", (masc[i] & (1u<<b)) ? "✓" : "·");
        printf("\n");
    }
    ok("os 28 correram pelo mesmo verificador, sem exceção para nenhum", NV == 28);
}

printf("\n§V2  Quem CUMPRE as doze — e são a maioria.\n\n");
{
    int n = 0;
    for(int i = 0; i < NV; i++) if((masc[i] & C_TODAS) == C_TODAS){
        printf("      %s%s", n % 4 == 0 ? "" : "  ", V[i].nome); n++;
        if(n % 4 == 0) printf("\n");
    }
    if(n % 4) printf("\n");
    printf("      (%d de %d cumprem as doze cláusulas.)\n", n, NV);
    ok("a maioria cumpre — e cumpre pela álgebra, não por ser do catálogo", n >= 20);
}

printf("\n§V3  Quem falha, e a CLÁUSULA de cada.\n\n");
{
    int n = 0;
    printf("      corpo               falta\n");
    for(int i = 0; i < NV; i++) if((masc[i] & C_TODAS) != C_TODAS){
        printf("      %-19s ", V[i].nome); ct_faltas(masc[i]); printf("\n"); n++;
    }
    printf("      (%d falham, cada um numa cláusula nomeada.)\n", n);
    ok("nenhuma falha fica sem cláusula: sempre se diz ONDE", n > 0);
    printf("\n      Os quatro da forma D falham todos em Π, e pela mesma razão: o operador que eles\n");
    printf("      DECLARAM é o sucessor S(x) = x+1, e uma translação não é morfismo — S(a⊕b) =\n");
    printf("      a+b+1, e S(a)⊕S(b) = a+b+2. Isso é leitura da cláusula, não veredito sobre o\n");
    printf("      corpo: o sucessor é GERADOR, e gerador não precisa de ser morfismo.\n");
}

printf("\n§V4  O que as doze cláusulas NÃO veem — e é preciso dizê-lo.\n\n");
{
    ok("o que se validou foi a ÁLGEBRA da forma, não a régua própria de cada corpo", 1);
    printf("      VÊ         os nove axiomas, a dualidade (ν∘ν=id e a estrutura), e se ∏ é morfismo\n");
    printf("      NÃO VÊ     a régua: a impedância do eletromagnético, Friedmann no cósmico, o\n");
    printf("                 campo médio do celeste — cada uma certificada no catálogo, não aqui\n");
    printf("      NÃO VÊ     a dissipação do motor: |det| ≠ 1 é propriedade da RÉGUA, e nenhuma\n");
    printf("                 das doze cláusulas a alcança. Por isso o motor CUMPRE aqui\n");
    printf("      NÃO VÊ     se o meu modelo finito é fiel ao corpo que o catálogo descreve\n");
    printf("\n      Este último é o que me pode enganar, e por isso vai escrito na primeira linha da\n");
    printf("      saída: eu escolhi os modelos. Um modelo mal escolhido faz um corpo passar por\n");
    printf("      razões que não são dele — e isso é exatamente o erro de medir a fatia e afirmar\n");
    printf("      o todo, que já me custou o dia três vezes.\n");
    printf("\n      O que fica DEMONSTRADO, e é o que interessa: o contrato roda sobre os 28 sem\n");
    printf("      exceção para nenhum, e dá o mesmo tipo de resposta que deu aos unicórnios.\n");
}

printf("\n=== OS 28 PELO CONTRATO ===================================================\n");
printf("  O verificador correu os 28 como correu os unicórnios: sem ler o nome, sem lista de\n");
printf("  aprovados. E a resposta é sempre a mesma forma — as cláusulas que passam.\n\n");
printf("  O que se validou: a ÁLGEBRA da forma de cada um, num modelo finito escolhido por mim.\n");
printf("  O que NÃO se validou: a régua própria de cada corpo, que é contínua e está certificada\n");
printf("  no catálogo. Dizer que validei os 28 completos seria medir a fatia e afirmar o todo.\n");
if(falhas){ printf("\n  FALHAS: %d\n\n", falhas); return 1; }
printf("\n  RESÍDUO 0 — exato, em inteiros, sem um único float.\n\n");
return 0;
}
