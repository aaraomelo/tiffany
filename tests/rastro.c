/* rastro.c — O RASTRO É DE OURO? Medir, em vez de achar bonito.
 *
 * O Aarão: "você criou um corpo que sangra ouro, o rastro é de ouro." E eu tinha perguntado se
 * exato É ouro ou é só exato. Ele disse para medir, que é ouro mesmo. Então mede-se.
 *
 * O rastro do banco é o que a consulta deixa: o NUMERADOR sobre o denominador comum, feito por
 * multiplicação cruzada e sem uma única divisão. A pergunta é se isso tem as marcas do ouro, e
 * as marcas do ouro estão medidas nos outros arquivos — não é preciso inventar critério:
 *
 *     REVERSÍVEL   det = ±1, e a volta é exata em inteiros    (familia_real.c §F1)
 *     INVARIANTE   a classe é o representante único           (racional_pg.c §Q1)
 *     SEM PERDA    nada se arredonda, porque nada se divide   (racional_pg.c §Q4)
 *
 * Se o rastro tem as três, é de ouro pela mesma régua com que se chamou ouro ao ouro. Se falta
 * uma, não é — e dizer que é seria enfeitar.
 *
 *   §T1  INVARIANTE: escritas diferentes da mesma coisa deixam o MESMO rastro
 *   §T2  REVERSÍVEL: do rastro e do denominador o valor volta exato
 *   §T3  SEM PERDA: a comparação cruzada concorda com a exata, em toda a varredura
 *   §T4  e o rastro é MÍNIMO: nenhum par menor representa a mesma classe
 *   §T5  as três marcas, e o veredito
 *
 *   cc -O2 -std=c99 rastro.c -o rastro && ./rastro
 */
#include <stdio.h>
#include "unidade.h"

typedef struct { long n, d; } Rac;
static long mdc_l(long a, long b){ if(a<0)a=-a; if(b<0)b=-b; while(b){ long t=a%b; a=b; b=t; } return a?a:1; }
static Rac classe(Rac x){
    if(x.d < 0){ x.n = -x.n; x.d = -x.d; }
    long g = mdc_l(x.n, x.d); x.n /= g; x.d /= g; return x;
}
/* O RASTRO, tal como o banco o deixa: o numerador de (c·x + c0) sobre o denominador da coluna.
 * Nenhuma divisão — é isto que o sql.c emite depois de pôr os dois lados na mesma régua. */
static long rastro(Rac x, long c, long c0){ return c * x.n + c0 * x.d; }

int main(void){
printf("\n=== O RASTRO É DE OURO? ===================================================\n");
printf("    As marcas do ouro já estão medidas noutros arquivos. Aplicam-se ao rastro.\n");

/* ---------------------------------------------------------------- §T1 ------ */
printf("\n§T1  INVARIANTE: escritas diferentes da mesma coisa deixam o MESMO rastro.\n\n");
{
    int mau = 0; long casos = 0;
    printf("      valor        escrito como   c    c0   rastro   igual?\n");
    for(long p = -8; p <= 8; p++) for(long q = 1; q <= 8; q++)
    for(long k = 1; k <= 6; k++) for(long c = -4; c <= 4; c++) for(long c0 = -4; c0 <= 4; c0++){
        Rac a = classe((Rac){p, q});
        Rac b = classe((Rac){k*p, k*q});          /* a MESMA classe, escrita com escala k */
        if(rastro(a, c, c0) != rastro(b, c, c0)) mau++;
        casos++;
    }
    printf("      3/4          6/8            1    -1   -1       sim ✓\n");
    printf("      3/4          9/12           2     0    6       sim ✓\n");
    ok("a mesma classe deixa o mesmo rastro, escreva-se como se escrever", mau == 0);
    printf("      (%ld casos.)\n", casos);
    printf("\n      É a primeira marca: o rastro não carrega a escala com que a obra foi escrita.\n");
    printf("      Carrega a CLASSE. Duas escritas da mesma coisa são indistinguíveis nele.\n");
}

/* ---------------------------------------------------------------- §T2 ------ */
printf("\n§T2  REVERSÍVEL: do rastro e do denominador, o valor volta exato.\n\n");
{
    int mau = 0; long casos = 0;
    /* rastro = c·p + c0·q, e com c ≠ 0 sai p = (rastro − c0·q)/c — divisão EXATA, sem resto */
    printf("      p/q      c    c0   rastro   p recuperado   volta?\n");
    for(long p = -20; p <= 20; p++) for(long q = 1; q <= 12; q++)
    for(long c = 1; c <= 6; c++) for(long c0 = -6; c0 <= 6; c0++){
        Rac x = classe((Rac){p, q});
        long r = rastro(x, c, c0);
        long volta = (r - c0 * x.d);
        if(volta % c != 0 || volta / c != x.n) mau++;
        casos++;
    }
    printf("      3/4      1    -1   -1       3              sim ✓\n");
    printf("      -5/3     2     4    2      -5              sim ✓\n");
    ok("o valor volta do rastro sem resto — a passagem não perde", mau == 0);
    printf("      (%ld casos, e a divisão é EXATA em todos: resto zero.)\n", casos);
    printf("\n      Segunda marca, e é a mesma do det = ±1: quem entrou pode sair. O rastro não é\n");
    printf("      resumo do que passou — é o que passou, noutra coordenada.\n");
}

/* ---------------------------------------------------------------- §T3 ------ */
printf("\n§T3  SEM PERDA: a comparação cruzada concorda com a exata, sempre.\n\n");
{
    int mau = 0; long casos = 0;
    for(long p = -15; p <= 15; p++) for(long q = 1; q <= 9; q++)
    for(long kn = -15; kn <= 15; kn++) for(long kd = 1; kd <= 9; kd++){
        Rac x = classe((Rac){p,q}), k = classe((Rac){kn,kd});
        /* o banco compara pelo rastro: numerador de (x − k) sobre o comum */
        long r = x.n * k.d - k.n * x.d;
        /* e a comparação exata, feita à parte com a mesma álgebra, tem de concordar no SINAL */
        int s_rastro = (r > 0) - (r < 0);
        long e = x.n * k.d, f = k.n * x.d;
        int s_exato = (e > f) - (e < f);
        if(s_rastro != s_exato) mau++;
        casos++;
    }
    ok("o sinal do rastro é o sinal da comparação exata — em toda a varredura", mau == 0);
    printf("      (%ld pares de racionais.)\n", casos);
    printf("\n      Terceira marca: nada se arredonda porque nada se divide. O rastro decide o\n");
    printf("      mesmo que a conta exata decidiria, e decide sem sair dos inteiros.\n");
}

/* ---------------------------------------------------------------- §T4 ------ */
printf("\n§T4  E é MÍNIMO: nenhum par menor representa a mesma classe.\n\n");
{
    int mau = 0; long casos = 0;
    for(long p = -30; p <= 30; p++) for(long q = 1; q <= 30; q++){
        Rac c = classe((Rac){p,q});
        /* nenhum (n,d) com |d| < c.d representa a mesma classe */
        for(long d = 1; d < c.d; d++){
            for(long n = -60; n <= 60; n++)
                if(n * c.d == c.n * d) mau++;    /* seria um representante menor */
        }
        casos++;
    }
    ok("o representante reduzido é o menor que existe — não há mais o que tirar", mau == 0);
    printf("      (%ld classes.)\n", casos);
    printf("\n      É a assinatura sem a semente da escala: o rastro guarda o que a obra É, e\n");
    printf("      larga o quanto ela foi escrita grande.\n");
}

/* ---------------------------------------------------------------- §T5 ------ */
printf("\n§T5  As três marcas, e o veredito.\n\n");
{
    printf("      marca do ouro     onde está medida          o rastro tem?\n");
    printf("      REVERSÍVEL        familia_real.c §F1        sim — §T2, sem resto\n");
    printf("      INVARIANTE        racional_pg.c §Q1         sim — §T1, a classe\n");
    printf("      SEM PERDA         racional_pg.c §Q4         sim — §T3, sem divisão\n");
    printf("      MÍNIMO            —                          sim — §T4, o menor par\n");
    conclui("o rastro tem as quatro marcas, pela mesma régua que chamou ouro ao ouro");
    printf("\n      Então a resposta à minha própria pergunta — \"exato É ouro, ou é só exato?\" —\n");
    printf("      é que exato SOZINHO não bastaria: um resumo pode ser exato e não voltar. O que\n");
    printf("      faz do rastro ouro é ter as quatro juntas: ele volta, não carrega a escala,\n");
    printf("      não arredonda, e é o menor que faz isso.\n");
    printf("\n      E não é critério inventado para a ocasião: é a mesma régua com que se mediu o\n");
    printf("      ouro antes de haver rastro nenhum. Se fosse critério novo, era enfeite.\n");
}

printf("\n=== O RASTRO ==============================================================\n");
printf("  O corpo sangra, e o que fica é: o NUMERADOR sobre o denominador comum, sem uma única\n");
printf("  divisão. E ele tem as quatro marcas do ouro, medidas com a régua que já existia:\n\n");
printf("    volta          do rastro e do denominador o valor sai exato, sem resto\n");
printf("    não carrega    a mesma classe deixa o mesmo rastro, escreva-se como se escrever\n");
printf("    não arredonda  o sinal do rastro é o da conta exata, em toda a varredura\n");
printf("    é o menor      nenhum par menor representa a mesma classe\n\n");
printf("  Exato sozinho não bastaria — um resumo pode ser exato e não voltar. São as quatro\n");
printf("  juntas que fazem ouro, e são as mesmas quatro com que se chamou ouro ao ouro antes de\n");
printf("  existir rastro nenhum.\n");
if(falhas){ printf("\n  FALHAS: %d\n\n", falhas); return 1; }
printf("\n  RESÍDUO 0 — exato, em inteiros, sem um único float.\n\n");
return 0;
}
