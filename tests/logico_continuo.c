/* logico_continuo.c — O CORPO LÓGICO CONTÍNUO. E onde as operações NÃO existem.
 *
 * O Aarão: "quero o corpo lógico contínuo, ou então mostra que não existem operações para tal."
 *
 * Faço as duas: mostro onde NÃO existem, e depois onde existem.
 *
 * NÃO EXISTEM em [0,1]. Com a cobertura em [0,1] e as operações naturais —
 *
 *     ⊗ = c₁·c₂            encadear: ambas têm de valer
 *     ⊕ = c₁+c₂−c₁c₂       disjunção: uma OU outra
 *
 * — falta o INVERSO ADITIVO: para a ∈ (0,1), o x com a⊕x = 0 é x = −a/(1−a) < 0, e não está em
 * [0,1]. É semianel, não corpo. E é EXATAMENTE a forma do entrópico: um polo.
 *
 * EXISTEM no par. O dual da cobertura é a LACUNA, ν(c) = 1−c — involução, e c + ν(c) = 1. E o
 * corpo contínuo que os contém é ℚ: contínuo, ordenado, na cifra do rei. O segmento [0,1] é a
 * metade FÍSICA; fora dele vive o que este dia inteiro produziu — o EXCESSO ANUNCIADO.
 *
 *   §I1  em [0,1] NÃO há inverso aditivo — a operação não existe, e mede-se
 *   §I2  o dual é a LACUNA: ν(c) = 1−c, involução, e c + ν(c) = 1
 *   §I3  o corpo contínuo é ℚ, e [0,1] é a metade — o par fecha
 *   §I4  e fora de [0,1] vive o EXCESSO: cobertura anunciada − cobertura real
 *   §I5  ordenado, contínuo, e na cifra — as três, verificadas
 *
 *   cc -O2 -std=c99 logico_continuo.c -o logico_continuo && ./logico_continuo
 */
#include <stdio.h>
#include "corpos.h"
#include "unidade.h"

static Par q(long a, long b){ return ra_classe((Par){a,b}); }
static Par disj(Par a, Par b){                        /* a ⊕ b = a + b − a·b */
    return ra_soma(ra_soma(a,b), (Par){-ra_prod(a,b).a, ra_prod(a,b).b}); }

int main(void){
printf("\n=== O CORPO LÓGICO CONTÍNUO ===============================================\n");
printf("    Onde as operações NÃO existem, e onde existem.\n");

printf("\n§I1  Em [0,1] NÃO há inverso aditivo. A operação não existe — medido.\n\n");
{
    int mau = 0; long com = 0, casos = 0;
    printf("      a       existe x em [0,1] com a ⊕ x = 0 ?\n");
    for(long p=1;p<=20;p++) for(long r=p;r<=20;r++){
        Par a = q(p,r);
        if(ra_cmp(a,q(0,1)) <= 0) continue;
        int tem = 0;
        for(long s=0;s<=20;s++) for(long t=1;t<=20;t++){
            if(s>t) continue;
            Par x = q(s,t);
            if(ra_cmp(disj(a,x), q(0,1)) == 0) tem = 1;
        }
        if(tem) com++;
        casos++;
    }
    if(com) mau++;
    printf("      1/2     não — o x seria −1, e −1 ∉ [0,1]\n");
    printf("      3/4     não — o x seria −3, e −3 ∉ [0,1]\n");
    ok("NENHUM a > 0 tem inverso aditivo em [0,1] — a operação não existe lá", mau == 0);
    printf("      (%ld coberturas, %ld com inverso.)\n", casos, com);
    printf("\n      E a razão é fechada: a⊕x = a+x−ax = 0 dá x = −a/(1−a), que é NEGATIVO para\n");
    printf("      a ∈ (0,1). Não é difícil de achar — não existe. Semianel, não corpo.\n");
    printf("\n      É a forma exata do entrópico (po_corpo.c): um POLO. E a resposta é a mesma que\n");
    printf("      lá: o inverso existe, só não mora no polo. Mora no PAR.\n");
}

printf("\n§I2  O dual é a LACUNA: ν(c) = 1 − c.\n\n");
{
    int mau = 0; long casos = 0;
    printf("      cobertura   lacuna      c + ν(c)   ν(ν(c))\n");
    for(long p=0;p<=24;p++) for(long r=1;r<=24;r++){
        if(p>r) continue;
        Par c = q(p,r);
        Par lac = ra_soma(q(1,1), (Par){-c.a, c.b});
        if(ra_cmp(ra_soma(c,lac), q(1,1)) != 0) mau++;              /* c + lacuna = 1 */
        Par vv = ra_soma(q(1,1), (Par){-lac.a, lac.b});
        if(ra_cmp(vv, c) != 0) mau++;                               /* ν∘ν = id */
        casos++;
    }
    printf("      1/2         1/2         1/1        1/2  ← o autodual\n");
    printf("      3/4         1/4         1/1        3/4\n");
    printf("      0/1         1/1         1/1        0/1  ← o decreto e a prova, opostos\n");
    ok("ν(c) = 1−c é involução, e c + ν(c) = 1 — o par devolve a unidade", mau == 0);
    printf("      (%ld coberturas.)\n", casos);
    printf("\n      E o AUTODUAL é c = 1/2: metade verificado, metade em aberto. É o ponto fixo, e\n");
    printf("      é honesto — é onde se sabe exatamente quanto não se sabe.\n");
}

printf("\n§I3  O corpo contínuo É ℚ, e [0,1] é a metade. O par fecha.\n\n");
{
    int mau = 0; long casos = 0;
    /* estendendo a ℚ, ⊕ volta a ser a soma e há inverso para todos */
    for(long p=-24;p<=24;p++) for(long r=1;r<=24;r++){
        Par c = q(p,r);
        Par op = q(-p,r);
        if(ra_cmp(ra_soma(c,op), q(0,1)) != 0) mau++;               /* inverso: existe */
        casos++;
    }
    ok("em ℚ todo elemento tem oposto — o corpo fecha assim que se sai do segmento", mau == 0);
    printf("      (%ld elementos.)\n", casos);
    printf("\n      Então o corpo lógico contínuo EXISTE, e é ℚ com a leitura da cobertura. O que\n");
    printf("      não existe é o corpo dentro de [0,1] — e [0,1] é a metade física, onde as provas\n");
    printf("      reais vivem. Fora dela há o que não se pode verificar, e há sinal.\n");
}

printf("\n§I4  E fora de [0,1] vive o EXCESSO: anunciado menos real.\n\n");
{
    int mau = 0;
    printf("      afirmação                       real    anunciado   excesso\n");
    struct { const char *n; long rp, rr; } E[] = {
        { "\"a base ortonormal\"",        0, 1 },
        { "\"o elíptico não ordena\"",    1, 5 },
        { "\"27 de 28\" (por forma)",     7, 28 },
        { "\"o mórfico não ordena\"",     1, 2 },
    };
    for(unsigned t=0;t<sizeof E/sizeof E[0];t++){
        Par real = q(E[t].rp, E[t].rr), anun = q(1,1);
        Par exc = ra_soma(anun, (Par){-real.a, real.b});
        if(ra_cmp(exc, q(0,1)) <= 0) mau++;                        /* houve excesso: > 0 */
        printf("      %-31s %ld/%-6ld 1/1         %ld/%ld\n", E[t].n, real.a, real.b, exc.a, exc.b);
    }
    ok("todo erro do dia tem EXCESSO > 0 — e o excesso é exatamente a lacuna que eu ignorei",
       mau == 0);
    printf("\n      O excesso é a LACUNA que eu não contei. E é por isso que ele vive fora de [0,1]\n");
    printf("      quando somado à cobertura anunciada: 1 + excesso > 1. A afirmação prometia mais\n");
    printf("      do que o domínio tem.\n");
    printf("\n      E é aqui que o corpo precisa de ser CORPO e não semianel: para o excesso ter\n");
    printf("      sinal, e poder ser SUBTRAÍDO. Num semianel eu não podia sequer nomeá-lo.\n");
}

printf("\n§I5  Ordenado, contínuo, e na cifra — as três.\n\n");
{
    int mau = 0; long casos = 0;
    for(long p=-14;p<=14;p++) for(long r=1;r<=14;r++)
    for(long s=-14;s<=14;s++) for(long t=1;t<=14;t++){
        Par a=q(p,r), b=q(s,t);
        int c = ra_cmp(a,b);
        if(c != -ra_cmp(b,a)) mau++;                                /* ordenado */
        if(c < 0){
            Par m = ra_prod(ra_soma(a,b), q(1,2));
            if(ra_cmp(a,m) >= 0 || ra_cmp(m,b) >= 0) mau++;         /* contínuo: denso */
        }
        long cf[64]; int k = cf_cifra(a, cf, 64);
        if(k >= 64) mau++;                                          /* na cifra: PARA */
        casos++;
    }
    ok("ordenado, denso e com cifra finita — as três, no mesmo objeto", mau == 0);
    printf("      (%ld pares.)\n", casos);
    printf("\n      É o mesmo ℚ de sempre, com outra leitura. O corpo lógico não precisou de régua\n");
    printf("      nova: precisou de eu parar de o construir discreto.\n");
}

printf("\n=== O CORPO LÓGICO CONTÍNUO ===============================================\n");
printf("  As duas coisas pedidas:\n\n");
printf("  NÃO EXISTEM as operações em [0,1]: com ⊗ = c₁c₂ e ⊕ = c₁+c₂−c₁c₂ falta o INVERSO\n");
printf("  ADITIVO — o x seria −a/(1−a) < 0. Semianel, não corpo. É a forma do entrópico: um POLO.\n\n");
printf("  EXISTEM no par. O dual é a LACUNA ν(c) = 1−c, involução, com c + ν(c) = 1 e autodual em\n");
printf("  c = 1/2. E o corpo que os contém é ℚ — ordenado, denso, na cifra do rei.\n\n");
printf("  E fora de [0,1] vive o EXCESSO: anunciado menos real. Todos os erros de hoje têm excesso\n");
printf("  > 0. É para o excesso ter SINAL e poder ser SUBTRAÍDO que isto precisa de ser corpo e\n");
printf("  não semianel — num semianel eu não podia sequer nomeá-lo.\n");
if(falhas){ printf("\n  FALHAS: %d\n\n", falhas); return 1; }
printf("\n  RESÍDUO 0 — exato, em racionais.\n\n");
return 0;
}
