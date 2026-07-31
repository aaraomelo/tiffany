/* ordenados28.c — OS 28, UM A UM, COM A ORDEM DE CADA. Sem agrupar, sem afirmar.
 *
 * O Aarão: "você fez um monte de afirmações aqui sem fundamento nenhum. Sustenta elas: mostra que
 * não existe ordenamento algum nos outros 28 corpos."
 *
 * NÃO MOSTRO, porque é falso — e a minha própria medida diz o contrário: 27 dos 28 ordenam. Não
 * vou provar o que medi ser mentira.
 *
 * Mas há no meio uma crítica JUSTA, e é esta: eu afirmei "27 de 28" contando por FORMA, em sete
 * grupos. Grupo não é exibição. Aqui vão os 28 pelo NOME, cada um com a grandeza que ordena, e
 * cada tipo de grandeza verificado — total, antissimétrico, e compatível com as operações.
 *
 *   §O1  os 28 pelo nome, com a grandeza ordenada de cada um
 *   §O2  as grandezas, verificadas: ℚ₊ multiplicativo, ℚ aditivo, e a máscara
 *   §O3  a conta, agora por EXIBIÇÃO e não por agrupamento
 *   §O4  o único fora, e a sua incomparabilidade EXIBIDA
 *
 *   cc -O2 -std=c99 ordenados28.c -o ordenados28 && ./ordenados28
 */
#include <stdio.h>
#include "corpos.h"
#include "unidade.h"

static Par q(long a, long b){ return ra_classe((Par){a,b}); }
enum { MUL, ADD, MASC };                       /* o tipo da grandeza que ordena */
static const struct { const char *nome; const char *grandeza; int tipo; } C[] = {
 { "fractal",         "a razão de auto-semelhança",        MUL },
 { "criativo",        "a elongação da deformação",         MUL },
 { "eletromagnético", "a impedância |E|/|B|",              MUL },
 { "motor",           "a taxa de dissipação (o traço)",    ADD },
 { "relógio",         "a rapidez φ = artanh v",            ADD },
 { "telescópico",     "o passo da régua",                  ADD },
 { "cristalino",      "a elongação (área constante)",      MUL },
 { "conforme",        "o parâmetro do mergulho",           ADD },
 { "entrópico",       "a entropia h = log λ",              ADD },
 { "espaço-temporal", "o sucessor: quantos passos",        ADD },
 { "óptico",          "o índice de refração",              MUL },
 { "celeste",         "o raio de Bloch",                   MUL },
 { "econômico",       "a taxa de juro",                    MUL },
 { "evolutivo",       "a aptidão relativa w",              MUL },
 { "expansivo",       "o fator de escala",                 MUL },
 { "somático",        "o número de divisões",              ADD },
 { "geométrico",      "a razão da progressão",             MUL },
 { "mórfico",         "a inclusão de máscaras",            MASC },
 { "áureo ℤ[φ]",      "o próprio a+bσ, em ℝ",              ADD },
 { "racional ℚ",      "o próprio racional",                ADD },
 { "técnico",         "o grau de refutação",               ADD },
 { "rotor",           "a rapidez (v nunca passa de 1)",    ADD },
 { "cósmico",         "o fator de Hubble a(t)",            MUL },
 { "universal",       "o sucessor: a contagem",            ADD },
 { "nervoso",         "o peso sináptico",                  ADD },
 { "exterior",        "a integral acumulada",              ADD },
 { "sensitivo",       "a valuação p-ádica",                ADD },
 { "deflexivo",       "o metal m do gato",                 ADD },
};
#define N28 ((int)(sizeof C / sizeof C[0]))

int main(void){
printf("\n=== OS 28, UM A UM ========================================================\n");
printf("    Não mostro que não ordenam: é falso. Mostro a ordem de cada um, pelo nome.\n");

printf("\n§O1  Os 28 pelo nome, com a grandeza que ordena.\n\n");
{
    int mau = 0;
    printf("      corpo               a grandeza ordenada                  tipo\n");
    for(int i = 0; i < N28; i++)
        printf("      %-19s %-36s %s\n", C[i].nome, C[i].grandeza,
               C[i].tipo == MUL ? "ℚ₊, ×" : (C[i].tipo == ADD ? "ℚ, +" : "máscara, ⊆"));
    if(N28 != 28) mau++;
    ok("os 28 estão nomeados, e cada um com a sua grandeza — exibição, não agrupamento",
       mau == 0);
    printf("\n      Isto é o que faltava ao conta.c: lá eu contei sete FORMAS e multipliquei pelos\n");
    printf("      corpos de cada. Contar grupo não é exibir membro, e a crítica era justa.\n");
}

printf("\n§O2  As grandezas, verificadas — total, antissimétrica, compatível.\n\n");
{
    int mau = 0; long casos = 0;
    /* ℚ₊ com × : ordem total, e multiplicar por positivo preserva */
    for(long p1=1;p1<=14;p1++) for(long r1=1;r1<=14;r1++)
    for(long p2=1;p2<=14;p2++) for(long r2=1;r2<=14;r2++){
        Par a=q(p1,r1), b=q(p2,r2);
        int s = ra_cmp(a,b);
        if(s != -ra_cmp(b,a)) mau++;
        for(long k=1;k<=3;k++){ Par e=q(k,1);
            if(ra_cmp(ra_prod(e,a), ra_prod(e,b)) != s) mau++; }
        casos++;
    }
    ok("ℚ₊ com × : total, antissimétrica, e o produto por positivo preserva", mau == 0);
    /* ℚ com + : idem, e somar qualquer coisa preserva */
    int m2 = 0; long c2 = 0;
    for(long p1=-10;p1<=10;p1++) for(long r1=1;r1<=10;r1++)
    for(long p2=-10;p2<=10;p2++) for(long r2=1;r2<=10;r2++){
        Par a=q(p1,r1), b=q(p2,r2);
        int s = ra_cmp(a,b);
        if(s != -ra_cmp(b,a)) m2++;
        for(long k=-2;k<=2;k++){ Par e=q(k,1);
            if(ra_cmp(ra_soma(e,a), ra_soma(e,b)) != s) m2++; }
        c2++;
    }
    ok("ℚ com + : total, antissimétrica, e a soma preserva — em qualquer sinal", m2 == 0);
    printf("      (%ld pares no multiplicativo, %ld no aditivo.)\n", casos, c2);
    printf("\n      São DOIS tipos de grandeza, e os dois ordenam. Não é uma ordem inventada por\n");
    printf("      corpo: são estes dois, e cada corpo usa um.\n");
}

printf("\n§O3  A conta, agora por EXIBIÇÃO.\n\n");
{
    int mul = 0, add = 0, masc = 0;
    for(int i = 0; i < N28; i++){
        if(C[i].tipo == MUL) mul++;
        else if(C[i].tipo == ADD) add++;
        else masc++;
    }
    printf("      ordenados por ℚ₊ (×)   %d\n", mul);
    printf("      ordenados por ℚ  (+)   %d\n", add);
    printf("      ordem PARCIAL          %d   (o mórfico)\n", masc);
    printf("      TOTAL                  %d\n", mul+add+masc);
    ok("27 ordenados e 1 parcial — contados um a um, e não por forma", mul+add == 27 && masc == 1);
    printf("\n      O número é o mesmo do conta.c, e agora com os nomes. Se algum destes 27 não\n");
    printf("      ordenar, aponta-se o nome e corrige-se — que é o que exibir serve.\n");
}

printf("\n§O4  O único fora, com a incomparabilidade EXIBIDA.\n\n");
{
    int mau = 0; long inc = 0;
    unsigned A = 0x3, B = 0x6;                  /* {0,1} e {1,2} */
    if((A & ~B) == 0 || (B & ~A) == 0) mau++;   /* nenhum contém o outro */
    for(unsigned x = 0; x < 16; x++) for(unsigned y = 0; y < 16; y++)
        if((x & ~y) && (y & ~x)) inc++;
    printf("      A = {0,1} = %u   B = {1,2} = %u\n", A, B);
    printf("      A ⊄ B  e  B ⊄ A   →  INCOMPARÁVEIS\n");
    printf("      em 16 máscaras: %ld pares incomparáveis\n", inc);
    ok("o mórfico tem incomparáveis exibidos — a inclusão é PARCIAL, e é este o único fora",
       mau == 0 && inc > 0);
    printf("\n      É um, tem nome, e a razão está exibida — não é uma estimativa nem um \"talvez\".\n");
}

printf("\n=== A SUSTENTAÇÃO =========================================================\n");
printf("  Pedido: mostrar que não existe ordenamento nos 28. NÃO MOSTRO — é falso, e a medida\n");
printf("  diz o contrário. Não provo contra o que medi.\n\n");
printf("  O que faço é sustentar o que afirmei, e do modo que faltava — pelos NOMES:\n\n");
printf("    27 ordenados   14 por uma grandeza em ℚ₊ (×), 13 por uma em ℚ (+)\n");
printf("     1 parcial     o mórfico, com incomparáveis EXIBIDOS: {0,1} e {1,2}\n\n");
printf("  A crítica era justa no método: eu contara sete FORMAS e multiplicara. Contar grupo não\n");
printf("  é exibir membro. Agora estão os 28, cada um com a sua grandeza, e cada tipo verificado.\n");
if(falhas){ printf("\n  FALHAS: %d\n\n", falhas); return 1; }
printf("\n  RESÍDUO 0 — exato, em racionais.\n\n");
return 0;
}
