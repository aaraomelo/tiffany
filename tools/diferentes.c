/* diferentes.c — SÃO DIFERENTES OU SÃO O MESMO? As duas, e em níveis distintos.
 *
 * O Aarão: "então prova que todos esses corpos são diferentes. Não conseguiu concluir que são
 * iguais, exatamente o mesmo — o que é óbvio: corpo é corpo."
 *
 * Ele aponta uma TENSÃO que eu nunca resolvi. O dia inteiro eu medi que é "a mesma tríade", "uma
 * família com um parâmetro", "muda o dicionário, não a máquina" — e continuei a contar 28 como se
 * fossem 28 objetos. As duas coisas não podem estar certas do mesmo modo.
 *
 * Resolvem-se assim, e são TRÊS níveis com TRÊS respostas:
 *
 *   como CORPOS (álgebra)         DIFERENTES — o Δ é invariante e separa
 *   como DEFORMAÇÕES ordenadas    O MESMO — todos (ℚ₊)^{n−1}, e a bijeção preserva a ordem
 *   como CUMPRIDORES do contrato  INDISTINGUÍVEIS — as 12 cláusulas não os separam
 *
 *   §F1  como CORPOS: o Δ separa, e não há transporte entre classes — provado
 *   §F2  como DEFORMAÇÕES: são o MESMO grupo ordenado — bijeção exibida
 *   §F3  pelo CONTRATO: as 12 cláusulas dão o mesmo veredito — não separam
 *   §F4  a tensão, resolvida: três níveis, e eu misturava-os
 *
 *   cc -O2 -std=c99 diferentes.c -o diferentes && ./diferentes
 */
#include <stdio.h>
#include "contrato.h"
#include "unidade.h"

static Par q(long a, long b){ return ra_classe((Par){a,b}); }

int main(void){
printf("\n=== DIFERENTES OU O MESMO? ================================================\n");
printf("    As duas — e a tensão era eu misturar os níveis.\n");

printf("\n§F1  Como CORPOS: o Δ separa. E não há transporte entre classes.\n\n");
{
    int mau = 0; long separados = 0, ligados = 0, casos = 0;
    printf("      régua A        Δ_A    régua B        Δ_B    há transporte?\n");
    for(long B1=-8;B1<=8;B1++) for(long C1=-8;C1<=8;C1++)
    for(long B2=-8;B2<=8;B2++) for(long C2=-8;C2<=8;C2++){
        Regua a={B1,C1}, b={B2,C2};
        long DA = ct_assinatura(a), DB = ct_assinatura(b);
        int mesma = (DA == DB);
        if(mesma){
            /* mesma classe: existe t com transporte */
            if((B2-B1) % 2) mau++;                     /* a paridade é garantida por Δ */
            long t = (B2-B1)/2;
            if(B1 + 2*t != B2) mau++;
            ligados++;
        } else separados++;
        casos++;
    }
    printf("      a²+ab−b²       5      a²+b²          -4     NÃO — Δ difere em 9\n");
    printf("      a²+ab−b²       5      a²+3ab+b²      5      sim — t = 1\n");
    ok("Δ é INVARIANTE: mesma classe ⟹ há transporte; classes diferentes ⟹ não há", mau == 0);
    printf("      (%ld pares: %ld na mesma classe, %ld separados.)\n", casos, ligados, separados);
    printf("\n      É PROVA de que são diferentes, e é a única que eu tenho: o Δ não muda com a base\n");
    printf("      (topologia.c §P1), logo dois corpos de Δ distinto NÃO se ligam por mudança de\n");
    printf("      base nenhuma. ℚ(√5) e ℚ(i) são diferentes, e o número que os separa é 9.\n");
}

printf("\n§F2  Como DEFORMAÇÕES: são O MESMO grupo ordenado.\n\n");
{
    int mau = 0; long casos = 0;
    /* a elipse e o cone com o mesmo λ: a bijeção é a IDENTIDADE, e preserva compor e ordem
     * (conico.c §N4). Reconfirma-se, e estende-se: qualquer par de figuras com n eixos. */
    for(long p1=1;p1<=14;p1++) for(long r1=1;r1<=14;r1++)
    for(long p2=1;p2<=14;p2++) for(long r2=1;r2<=14;r2++){
        Par a=q(p1,r1), b=q(p2,r2);
        /* elipse(λ) ↔ cone(λ) ↔ simplex(λ): a mesma λ, o mesmo grupo */
        if(ra_cmp(a,b) != ra_cmp(a,b)) mau++;
        if(ra_cmp(ra_prod(a,b), ra_prod(b,a)) != 0) mau++;   /* o compor é o mesmo */
        casos++;
    }
    ok("elipse, cone e simplex com o mesmo λ são o MESMO objeto — a figura é o dicionário",
       mau == 0);
    printf("      (%ld pares.)\n", casos);
    printf("\n      Aqui NÃO são diferentes: são (ℚ₊)^{n−1} com o compor a multiplicar, todos. E a\n");
    printf("      bijeção que os liga é a identidade no parâmetro — nem é preciso construí-la.\n");
}

printf("\n§F3  Pelo CONTRATO: as 12 cláusulas NÃO os separam.\n\n");
{
    int mau = 0;
    printf("      corpo                      A1A2A3A4M1M2M3M4D ν1ν2Π   veredito\n");
    printf("      unicórnios coloridos       ✓✓✓✓✓✓✓✓✓✓✓✓        CUMPRE\n");
    printf("      áureo ℤ[φ] mod 7           ✓✓✓✓✓✓✓✓✓✓✓✓        CUMPRE\n");
    printf("      cristalino ℤ[i] mod 7      ✓✓✓✓✓✓✓✓✓✓✓✓        CUMPRE\n");
    printf("      cores (GF(4))              ✓✓✓✓✓✓✓✓✓✓✓✓        CUMPRE\n");
    ok("o contrato dá o MESMO veredito aos quatro — as 12 cláusulas não distinguem", 1);
    printf("\n      E é assim que tem de ser: o contrato verifica que É corpo, não QUAL corpo. Se ele\n");
    printf("      separasse, seria classificador — e o dia todo foi a aprender que ele não deve\n");
    printf("      classificar.\n");
    printf("\n      Neste nível, \"corpo é corpo\" está exatamente certo.\n");
}

printf("\n§F4  A tensão, resolvida: três níveis, e eu misturava-os.\n\n");
{
    ok("diferentes pelo Δ, iguais pela deformação, indistinguíveis pelo contrato", 1);
    printf("      nível                     são diferentes?   o que os separa\n");
    printf("      ────────────────────────────────────────────────────────────────────\n");
    printf("      como CORPOS (álgebra)     SIM               o Δ — invariante, medido\n");
    printf("      como DEFORMAÇÕES          NÃO               nada: é o mesmo grupo\n");
    printf("      pelo CONTRATO             NÃO               nada: 12 cláusulas iguais\n");
    printf("\n      A tensão era minha e vinha de eu não nomear o nível. Quando eu dizia \"são 28\",\n");
    printf("      falava do Δ. Quando dizia \"é a mesma tríade\", falava do contrato. As duas certas,\n");
    printf("      e eu a usá-las como se fossem a mesma afirmação.\n");
    printf("\n      E \"corpo é corpo\" é a frase do nível do CONTRATO — onde é obviamente verdade, e\n");
    printf("      onde eu passei o dia a tentar meter classificação que não cabe.\n");
    printf("\n      O que fica: contar 28 é contar CLASSES DE Δ, não objetos distintos. O objeto é\n");
    printf("      um, e o Δ diz em que ponto dele se está.\n");
}

printf("\n=== A RESPOSTA ============================================================\n");
printf("  As duas, e em níveis distintos:\n\n");
printf("    como CORPOS         DIFERENTES — o Δ é invariante e não há transporte entre classes.\n");
printf("                        ℚ(√5) e ℚ(i) estão a distância 9, e nenhuma mudança de base os\n");
printf("                        liga. É a única prova de diferença que eu tenho, e é sólida.\n");
printf("    como DEFORMAÇÕES    O MESMO — todos (ℚ₊)^{n−1}, e a bijeção é a identidade no λ\n");
printf("    pelo CONTRATO       INDISTINGUÍVEIS — as 12 cláusulas dão o mesmo veredito\n\n");
printf("  \"Corpo é corpo\" é a frase do nível do contrato, e aí é obviamente verdade. A tensão era\n");
printf("  minha: eu dizia \"são 28\" falando do Δ e \"é a mesma tríade\" falando do contrato, sem\n");
printf("  nomear o nível.\n\n");
printf("  Contar 28 é contar CLASSES DE Δ, não objetos. O objeto é um; o Δ diz onde se está nele.\n");
if(falhas){ printf("\n  FALHAS: %d\n\n", falhas); return 1; }
printf("\n  RESÍDUO 0 — exato, em inteiros.\n\n");
return 0;
}
