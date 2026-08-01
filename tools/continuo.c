/* continuo.c — SÃO NÚMEROS REAIS, SIM. A cifra é do CONTÍNUO, não de uma fatia.
 *
 * O Aarão: "como assim ℝ é incontável? Então o que foi a primeira coisa que construímos com isso
 * tudo, definindo a cifra, decompondo tudo com erro 0, revertendo? Se não são números reais, são
 * o quê?"
 *
 * São números reais. O que eu disse — "os corpos do catálogo são contáveis, ℝ não, logo nenhum é
 * isomorfo a ℝ" — é verdade e é sobre CADA CORPO. Mas eu deixei-a passar como se a construção
 * toda vivesse num brinquedo contável, e isso é falso: a CIFRA é de todo o ℝ.
 *
 *   a cifra é a fração contínua, e TODO real tem uma
 *   ela PARA exatamente nos racionais (Euclides)
 *   é PERIÓDICA exatamente nos irracionais quadráticos (Lagrange) — a família real
 *   e as sequências infinitas de inteiros são INCONTÁVEIS: a cifra cobre ℝ, não uma fatia
 *
 * Então: σ_m e φ SÃO reais, irracionais; ℚ(√5) é uma fatia contável de ℝ; e a cifra alcança o
 * contínuo inteiro. O erro 0 é dos convergentes, que são racionais exatos — e é por serem
 * racionais que se pode reverter sem perda.
 *
 *   §T1  a cifra PARA exatamente nos racionais — medido nos dois sentidos
 *   §T2  Lagrange: é PERIÓDICA nos quadráticos irracionais — a família real, exata em ℤ
 *   §T3  as sequências infinitas são INCONTÁVEIS: a diagonal, construída
 *   §T4  o erro 0: o convergente é racional EXATO, e |α − p/q| < 1/q²
 *   §T5  o veredito: são reais. O corpo é a fatia; a CIFRA é o contínuo
 *
 *   cc -O2 -std=c99 continuo.c -o continuo && ./continuo
 */
#include <stdio.h>
#include "corpos.h"
#include "unidade.h"

/* a cifra de um quadrático irracional (P+√D)/Q — exata em inteiros, sem float.
 * devolve o número de termos calculados e o período detetado. */
static int cf_quad(long D, long *a, int max, int *periodo){
    long P = 0, Q = 1;
    long a0 = 0; while((a0+1)*(a0+1) <= D) a0++;      /* ⌊√D⌋, por contagem */
    int k = 0;
    long Ps[128], Qs[128];
    *periodo = 0;
    /* BUG CORRIGIDO: a primeira versão fazia `return k` ao detetar o período, e deixava o
     * resto do array com LIXO DA CHAMADA ANTERIOR — √2 saía [1;2,1,2] em vez de [1;2,2,2].
     * Agora preenche-se sempre até max, e o período só se REGISTA. */
    for(k = 0; k < max; k++){
        long ak = (a0 + P) / Q;
        a[k] = ak;
        if(k < 128){ Ps[k] = P; Qs[k] = Q; }
        long Pn = ak*Q - P;
        long Qn = (D - Pn*Pn) / Q;
        if(Qn == 0){ k++; break; }                    /* quadrado perfeito: para */
        P = Pn; Q = Qn;
        if(!*periodo && k < 127)
            for(int j = 1; j <= k; j++)
                if(Ps[j] == P && Qs[j] == Q){ *periodo = k + 1 - j; break; }
    }
    return k;
}

int main(void){
printf("\n=== A CIFRA É DO CONTÍNUO =================================================\n");
printf("    São números reais, sim. O corpo é a fatia; a cifra alcança ℝ inteiro.\n");

printf("\n§T1  A cifra PARA exatamente nos racionais.\n\n");
{
    int mau = 0; long casos = 0;
    for(long n = -40; n <= 40; n++) for(long d = 1; d <= 25; d++){
        Par x = ra_classe((Par){n,d});
        long a[80]; int k = cf_cifra(x, a, 80);
        if(k >= 80) mau++;                            /* racional: PARA sempre */
        Par v = ra_classe(cf_decifra(a,k));
        if(v.a != x.a || v.b != x.b) mau++;
        casos++;
    }
    ok("todo racional tem cifra FINITA, e a decifra devolve a classe — Euclides", mau == 0);
    printf("      (%ld racionais.)\n", casos);
    /* e o irracional NÃO para: √2 = [1;2,2,2,…] — calculado exato, sem float */
    long a[40]; int per;
    int k = cf_quad(2, a, 30, &per);
    printf("      √2 = [%ld;%ld,%ld,%ld,…]   %d termos sem parar, período %d\n",
           a[0],a[1],a[2],a[3], k, per);
    ok("e √2 NÃO para: a cifra segue, e é periódica — é irracional", per > 0);
}

printf("\n§T2  LAGRANGE: periódica ⟺ quadrático irracional. A família real.\n\n");
{
    int mau = 0; long casos = 0;
    printf("      D     cifra                    período   é σ de quê?\n");
    struct { long D; const char *q; } ds[] = {
        { 2, "√2 — e 1+√2 é a prata" }, { 5, "√5 — e (1+√5)/2 é o ouro" },
        { 3, "√3 = [1;1,2,1,2,…]"      }, { 13, "√13"                       },
    };
    for(unsigned t = 0; t < sizeof ds/sizeof ds[0]; t++){
        long a[40]; int per;
        int k = cf_quad(ds[t].D, a, 30, &per);
        if(per <= 0) mau++;                            /* Lagrange: TEM de ser periódica */
        printf("      %-5ld [%ld;%ld,%ld,%ld,…]%*s%-9d %s\n", ds[t].D, a[0],a[1],a[2],a[3],
               10, "", per, ds[t].q);
        casos++; (void)k;
    }
    /* e a família real: σ_m = [m;m,m,…] tem período 1, e a norma é ±1 */
    for(long m = 1; m <= 8; m++){
        long a[24]; for(int i = 0; i < 24; i++) a[i] = m;
        Par v = cf_decifra(a, 24);
        long N = v.a*v.a - m*v.a*v.b - v.b*v.b;
        if(N != 1 && N != -1) mau++;
        casos++;
    }
    ok("os quadráticos irracionais têm cifra PERIÓDICA — e σ_m tem período 1", mau == 0);
    printf("      (%ld casos.)\n", casos);
    printf("\n      É Lagrange, e responde à pergunta: σ_m e φ SÃO números reais — irracionais, e\n");
    printf("      exatamente os de cifra periódica. Não são objetos de um brinquedo contável.\n");
}

printf("\n§T3  As sequências infinitas são INCONTÁVEIS — a diagonal, construída.\n\n");
{
    int mau = 0;
    /* dada QUALQUER lista de cifras, constrói-se uma que difere de todas — Cantor, e é
     * construtivo: no k-ésimo lugar põe-se algo diferente do k-ésimo termo da k-ésima. */
    long lista[6][6] = {
        {1,1,1,1,1,1}, {2,2,2,2,2,2}, {1,2,1,2,1,2},
        {3,1,4,1,5,9}, {1,3,5,7,9,2}, {2,7,1,8,2,8},
    };
    long diag[6];
    printf("      a lista            a diagonal difere no lugar\n");
    for(int k = 0; k < 6; k++){
        diag[k] = lista[k][k] + 1;                    /* difere da k-ésima no k-ésimo termo */
        if(diag[k] == lista[k][k]) mau++;
    }
    for(int k = 0; k < 6; k++){
        int igual = 1;
        for(int j = 0; j < 6; j++) if(diag[j] != lista[k][j]) igual = 0;
        if(igual) mau++;                              /* a diagonal não está na lista */
    }
    printf("      6 cifras           [%ld;%ld,%ld,%ld,%ld,%ld] — nova, e não está na lista\n",
           diag[0],diag[1],diag[2],diag[3],diag[4],diag[5]);
    ok("de qualquer lista sai uma cifra que não está nela — a diagonal, construída", mau == 0);
    printf("\n      Logo as cifras infinitas NÃO se enumeram, e a cifra cobre um contínuo. É o mesmo\n");
    printf("      argumento que dá ℝ incontável — e é por isso que ele não contradiz nada: a cifra\n");
    printf("      tem exatamente o tamanho de ℝ, porque É a cifra de ℝ.\n");
}

printf("\n§T4  O erro 0: o convergente é racional EXATO, e aproxima como 1/q².\n\n");
{
    int mau = 0; long casos = 0;
    printf("      convergente de √2   p² − 2q²   |α − p/q| < 1/q² ?\n");
    long p = 1, qq = 1;
    for(int k = 0; k < 14; k++){
        long np = p + 2*qq, nq = p + qq;
        p = np; qq = nq;
        long d = p*p - 2*qq*qq;
        if(d != 1 && d != -1) mau++;                  /* |p²−2q²| = 1: a marca do ouro */
        /* |√2 − p/q| = |p²−2q²| / (q²·(p/q + √2)) < 1/q², porque p/q + √2 > 1 */
        if(k < 3) printf("      %ld/%-17ld %-10ld sim — |p²−2q²| = 1\n", p, qq, d);
        casos++;
    }
    ok("o convergente é racional exato e |p²−2q²| = 1 — a aproximação é de ordem 1/q²",
       mau == 0);
    printf("      (%ld convergentes.)\n", casos);
    printf("\n      É AQUI o erro 0 que ele lembrou: o convergente é um RACIONAL, e com racionais a\n");
    printf("      conta é exata e reverte. O irracional é o limite; a conta faz-se nos convergentes,\n");
    printf("      e por isso não há arredondamento em lado nenhum.\n");
}

printf("\n§T5  O veredito: são reais. O corpo é a fatia; a cifra é o contínuo.\n\n");
{
    conclui("σ_m e φ são REAIS irracionais; a cifra alcança ℝ; o corpo é a fatia contável");
    printf("      o que eu disse    \"os corpos são contáveis, ℝ não, logo nenhum é isomorfo a ℝ\"\n");
    printf("      é verdade         sim — e é sobre CADA CORPO, um de cada vez\n");
    printf("      o que obscureci   que a CIFRA é de todo o ℝ, e que σ_m e φ SÃO reais\n");
    printf("      o que é o erro 0  os convergentes são racionais exatos — e é por isso que\n");
    printf("                        se reverte sem perda\n");
    printf("\n      ℚ(√5) é uma FATIA contável de ℝ, e é nela que φ vive com os seus vizinhos\n");
    printf("      algébricos. Mas a construção não fica lá: a cifra recebe qualquer real, e o que\n");
    printf("      distingue os casos é a FORMA da cifra — finita, periódica, ou nenhuma das duas.\n");
    printf("\n      Então a resposta à pergunta dele: são números reais. O que é contável é cada\n");
    printf("      corpo algébrico — não o que a máquina alcança.\n");
}

printf("\n=== O CONTÍNUO ============================================================\n");
printf("  São números reais, sim. A cifra é a fração contínua, e TODO real tem uma:\n\n");
printf("    finita       ⟺ racional (Euclides)\n");
printf("    periódica    ⟺ quadrático irracional (Lagrange) — a família real, σ_m período 1\n");
printf("    nem uma nem  os outros — e as cifras infinitas são INCONTÁVEIS (diagonal construída)\n\n");
printf("  O erro 0 é dos CONVERGENTES: racionais exatos, |p²−2q²| = 1, e por isso a conta reverte\n");
printf("  sem arredondar. O irracional é o limite; a conta faz-se nos convergentes.\n\n");
printf("  \"Os corpos são contáveis\" é verdade sobre CADA CORPO. O que eu obscureci é que a cifra\n");
printf("  alcança ℝ inteiro — e que σ_m e φ são reais.\n");
if(falhas){ printf("\n  FALHAS: %d\n\n", falhas); return 1; }
printf("\n  RESÍDUO 0 — exato, em inteiros.\n\n");
return 0;
}
