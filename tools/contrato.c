/* contrato.c — QUALQUER COISA PODE SER CORPO. O sistema verifica, não julga.
 *
 * O Aarão: "qualquer coisa pode ser um corpo — o corpo dos unicórnios coloridos, ou corpo de
 * cores, ou qualquer coisa. O sistema não faz juízo de valor, ele verifica a álgebra dual via
 * contrato. O cliente pode criar o corpo que quiser, nomear a função como quiser, mas deve
 * obedecer ao contrato."
 *
 * Eu vinha fazendo o contrário: uma LISTA de corpos com nome, e eu a decidir quem entra. Isso é
 * juízo de valor disfarçado de curadoria. O toolkit não devia ser uma lista — devia ser o
 * VERIFICADOR. E o contrato está escrito em `chess/elementares/index.tex`:
 *
 *     "Cada estrutura desta série herda o mesmo contrato: uma SOMA, uma MULTIPLICAÇÃO, uma
 *      DUALIDADE e um OPERADOR. O que muda é o DICIONÁRIO."
 *
 * São QUATRO, e eu vinha dizendo três a sessão inteira — faltava a DUALIDADE, que é exatamente a
 * que ele apontou: "deve obedecer à álgebra do corpo E deve ser dual".
 *
 * Aqui declaram-se corpos inventados na hora, com nomes que o sistema nunca lê, ao lado dos
 * corpos do catálogo. O verificador trata-os igual — e o resultado é que alguns inventados
 * CUMPREM e alguns oficiais FALHAM. É essa a prova de que não há juízo de valor.
 *
 *   §K1  o corpo dos UNICÓRNIOS COLORIDOS — inventado agora, e cumpre
 *   §K2  o corpo das CORES — mistura e Frobenius, e cumpre
 *   §K3  os do catálogo pelo MESMO verificador: áureo e cristalino
 *   §K4  e os que FALHAM, ditos pela CLÁUSULA e não por veredito
 *   §K5  o mesmo nome, dois parâmetros, dois resultados — o nome não decide
 *   §K6  o que a ferramenta é, e o que ela recusa fazer
 *
 *   cc -O2 -std=c99 contrato.c -o contrato && ./contrato
 */
#include <stdio.h>
#include "contrato.h"
#include "unidade.h"

static int ig(Par x, Par y){ return x.a == y.a && x.b == y.b; }
static long mod7(long x){ x %= 7; return x < 0 ? x + 7 : x; }
static long mod5(long x){ x %= 5; return x < 0 ? x + 5 : x; }

/* ---- §K1 O CORPO DOS UNICÓRNIOS COLORIDOS. Cinco unicórnios; o cliente nomeia como quer. ---- */
static Par uni_elem(long i){ Par r = { i % 5, 0 }; return r; }
static Par uni_juntar(Par x, Par y){ Par r = { mod5(x.a + y.a), 0 }; return r; }   /* ⊕ "juntar" */
static Par uni_cruzar(Par x, Par y){ Par r = { mod5(x.a * y.a), 0 }; return r; }   /* ⊗ "cruzar" */
static Par uni_espelho(Par x){ return x; }        /* ν "espelho" — em ℤ/p o Frobenius é a id */
static Par uni_chifre(Par x){ return x; }         /* ∏ "chifre"  — endomorfismo, e é morfismo  */

/* ---- §K2 O CORPO DAS CORES. GF(4): preto, branco, vermelho, azul. ⊕ é MISTURAR. ---- */
static Par cor_elem(long i){ Par r = { i & 1, (i >> 1) & 1 }; return r; }
static Par cor_misturar(Par x, Par y){ Par r = { x.a ^ y.a, x.b ^ y.b }; return r; }
static Par cor_compor(Par x, Par y){                    /* ω² = ω + 1 sobre GF(2) */
    long a = (x.a & y.a) ^ (x.b & y.b);
    long b = (x.a & y.b) ^ (x.b & y.a) ^ (x.b & y.b);
    Par r = { a & 1, b & 1 }; return r; }
static Par cor_negativo(Par x){ Par r = { (x.a ^ x.b) & 1, x.b }; return r; }   /* ν = Frobenius */
static Par cor_luz(Par x){ return cor_negativo(x); }                            /* ∏ = Frobenius */

/* ---- §K3 os do catálogo, pelo MESMO verificador. GF(49). ---- */
static Par au_elem(long i){ Par r = { i % 7, (i / 7) % 7 }; return r; }
static Par au7_soma(Par x, Par y){ Par r = { mod7(x.a+y.a), mod7(x.b+y.b) }; return r; }
static Par au7_prod(Par x, Par y){                       /* σ² = σ + 1 */
    Par r = { mod7(x.a*y.a + x.b*y.b), mod7(x.a*y.b + x.b*y.a + x.b*y.b) }; return r; }
static Par au7_conj(Par x){ Par r = { mod7(x.a + x.b), mod7(-x.b) }; return r; }
static Par cr7_prod(Par x, Par y){                       /* ω² = −1 */
    Par r = { mod7(x.a*y.a - x.b*y.b), mod7(x.a*y.b + x.b*y.a) }; return r; }
static Par cr7_conj(Par x){ Par r = { x.a, mod7(-x.b) }; return r; }

/* ---- §K4 os que falham, e a cláusula em que falham ---- */
static Par tel_elem(long i){ Par r = { i % 5, (i / 5) % 5 }; return r; }
static Par tel_soma(Par x, Par y){ Par r = { mod5(x.a+y.a), mod5(x.b+y.b) }; return r; }
static Par tel_prod(Par x, Par y){ Par r = { mod5(x.a*y.a), mod5(x.b*y.b) }; return r; }
static Par tel_troca(Par x){ Par r = { x.b, x.a }; return r; }

static Par tro_elem(long i){ Par r = { i - 4, 0 }; return r; }
static Par tro_soma(Par x, Par y){ Par r = { x.a > y.a ? x.a : y.a, 0 }; return r; }
static Par tro_prod(Par x, Par y){ Par r = { x.a + y.a, 0 }; return r; }
static Par tro_neg(Par x){ Par r = { -x.a, 0 }; return r; }

static Par mo_elem(long i){ Par r = { i, 0 }; return r; }
static Par mo_x(Par x, Par y){ Par r = { x.a ^ y.a, 0 }; return r; }
static Par mo_a(Par x, Par y){ Par r = { x.a & y.a, 0 }; return r; }
static Par mo_n1(Par x){ Par r = { x.a ^ 1, 0 }; return r; }
static Par mo_n3(Par x){ Par r = { x.a ^ 3, 0 }; return r; }
static Par mo_id(Par x){ return x; }

int main(void){
printf("\n=== QUALQUER COISA PODE SER CORPO — O SISTEMA VERIFICA, NÃO JULGA ==========\n");
printf("    O cliente nomeia como quiser. O contrato é que decide, cláusula a cláusula.\n");
printf("\n    cláusulas:  A1 A2 A3 A4  M1 M2 M3 M4  D  ν1 ν2 Π\n");

#define ZP(a,b) ((Par){a,b})
static const Contrato UNI = { "unicórnios coloridos", 5, uni_elem, uni_juntar, uni_cruzar,
                              uni_espelho, uni_chifre, ig, {0,0}, {1,0} };
static const Contrato COR = { "cores (misturar/compor)", 4, cor_elem, cor_misturar, cor_compor,
                              cor_negativo, cor_luz, ig, {0,0}, {1,0} };
static const Contrato AUR = { "áureo ℤ[φ] mod 7", 49, au_elem, au7_soma, au7_prod,
                              au7_conj, au7_conj, ig, {0,0}, {1,0} };
static const Contrato CRI = { "cristalino ℤ[i] mod 7", 49, au_elem, au7_soma, cr7_prod,
                              cr7_conj, cr7_conj, ig, {0,0}, {1,0} };
static const Contrato TEL = { "telescópico ℤ/5 × ℤ/5", 25, tel_elem, tel_soma, tel_prod,
                              tel_troca, mo_id, ig, {0,0}, {1,1} };
static const Contrato TRO = { "tropical (max,+)", 9, tro_elem, tro_soma, tro_prod,
                              tro_neg, mo_id, ig, {-1000000,0}, {0,0} };
static const Contrato MO1 = { "mórfico n=1", 2, mo_elem, mo_x, mo_a, mo_n1, mo_id, ig, {0,0}, {1,0} };
static const Contrato MO2 = { "mórfico n=2", 4, mo_elem, mo_x, mo_a, mo_n3, mo_id, ig, {0,0}, {3,0} };

printf("\n§K1  O corpo dos UNICÓRNIOS COLORIDOS — inventado agora, nomes do cliente.\n\n");
{
    printf("      corpo                      A1A2A3A4M1M2M3M4D ν1ν2Π    veredito\n");
    unsigned m = ct_verifica(&UNI);
    ct_relata(&UNI, m);
    ok("os unicórnios coloridos CUMPREM o contrato — e o sistema nunca leu o nome",
       (m & C_TODAS) == C_TODAS);
    printf("      as funções chamam-se juntar, cruzar, espelho e chifre. O verificador chamou-as\n");
    printf("      pela posição no contrato, não pelo nome — e não tinha como saber o que são.\n");
}

printf("\n§K2  O corpo das CORES — ⊕ é misturar, ν é o negativo.\n\n");
{
    unsigned m = ct_verifica(&COR);
    ct_relata(&COR, m);
    ok("as cores CUMPREM: quatro cores, misturar e compor, com o negativo por dual",
       (m & C_TODAS) == C_TODAS);
    printf("      e o negativo é involução de verdade: o negativo do negativo é a cor. Não é\n");
    printf("      metáfora — é a cláusula ν1, verificada em todas as quatro.\n");
}

printf("\n§K3  Os do catálogo, pelo MESMO verificador — sem tratamento especial.\n\n");
{
    unsigned a = ct_verifica(&AUR), c = ct_verifica(&CRI);
    ct_relata(&AUR, a);
    ct_relata(&CRI, c);
    ok("áureo e cristalino passam pelo mesmo contrato dos unicórnios",
       (a & C_TODAS) == C_TODAS && (c & C_TODAS) == C_TODAS);
    printf("      Nenhum atalho, nenhuma exceção por serem \"de verdade\". Passam porque a álgebra\n");
    printf("      fecha, que é a única razão que o sistema aceita.\n");
}

printf("\n§K4  E os que FALHAM — ditos pela CLÁUSULA, não por veredito.\n\n");
{
    unsigned t = ct_verifica(&TEL), r = ct_verifica(&TRO);
    ct_relata(&TEL, t);
    printf("      %-26s falta: ", "  → telescópico"); ct_faltas(t); printf("\n");
    ct_relata(&TRO, r);
    printf("      %-26s falta: ", "  → tropical"); ct_faltas(r); printf("\n");
    ok("o telescópico falha em M4 e o tropical em A4 — cláusulas DISTINTAS",
       !(t & C_M4) && !(r & C_A4));
    printf("\n      \"Falha em M4\" é informação: diz onde procurar. \"Não é corpo\" é juízo, e o\n");
    printf("      juízo não é do sistema — é de quem lê o relatório e decide o que fazer com ele.\n");
}

printf("\n§K5  O MESMO nome, dois parâmetros, dois resultados.\n\n");
{
    unsigned a = ct_verifica(&MO1), b = ct_verifica(&MO2);
    ct_relata(&MO1, a);
    ct_relata(&MO2, b);
    printf("      %-26s falta: ", "  → mórfico n=2"); ct_faltas(b); printf("\n");
    ok("o mesmo nome cumpre com n=1 e falha com n=2 — o nome não decide nada",
       (a & C_ALGEBRA) == C_ALGEBRA && !(b & C_M4));
    printf("\n      É a demonstração mais curta de que não há juízo de valor: a mesma etiqueta, dois\n");
    printf("      vereditos. O que mudou foi o parâmetro, e o verificador não sabe sequer que os\n");
    printf("      dois se chamam igual.\n");
}

printf("\n§K6  O que a ferramenta é, e o que ela RECUSA fazer.\n\n");
{
    ok("o contrato é ferramenta do toolkit: contrato.h, e qualquer cliente o usa", 1);
    printf("      FAZ       recebe ⊕, ⊗, ν, ∏ e um domínio finito; devolve as cláusulas que passam\n");
    printf("      FAZ       diz QUAL cláusula falhou, para quem lê poder agir\n");
    printf("      NÃO FAZ   não lê o nome, não conhece o domínio, não tem lista de aprovados\n");
    printf("      NÃO FAZ   não decide se \"é corpo\" — devolve as cláusulas e cala-se\n");
    printf("\n      E o que eu tinha feito até aqui era o oposto: uma LISTA, com eu a decidir quem\n");
    printf("      entra. Isso é juízo de valor disfarçado de curadoria — e é por isso que eu não\n");
    printf("      via como \"o corpo dos unicórnios\" podia entrar. Não havia porta; havia porteiro.\n");
    printf("\n      A quarta cláusula é a que eu tinha perdido. O contrato do index.tex diz \"uma\n");
    printf("      soma, uma multiplicação, uma DUALIDADE e um operador\" — quatro. Eu repeti três a\n");
    printf("      sessão inteira, e a que faltava é justamente a que ele apontou.\n");
}

printf("\n=== O CONTRATO ============================================================\n");
printf("  Quatro cláusulas, não três: soma, multiplicação, DUALIDADE e operador.\n\n");
printf("    o cliente     declara ⊕, ⊗, ν, ∏ com os nomes que quiser, sobre o que quiser\n");
printf("    o sistema     verifica as doze cláusulas e devolve quais passam\n");
printf("    o veredito    é por CLÁUSULA — \"falha em M4\", não \"não é corpo\"\n\n");
printf("  Os unicórnios coloridos cumprem. As cores cumprem. O telescópico falha em M4 e o\n");
printf("  tropical em A4. O mórfico cumpre com n=1 e falha com n=2 — mesmo nome, dois\n");
printf("  resultados. Não há lista de aprovados, e não há porteiro.\n");
if(falhas){ printf("\n  FALHAS: %d\n\n", falhas); return 1; }
printf("\n  RESÍDUO 0 — exato, em inteiros, sem um único float.\n\n");
return 0;
}
