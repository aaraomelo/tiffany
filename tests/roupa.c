/* roupa.c — A ROUPA NÃO TORNA DIFERENTE. E o Δ não é roupa.
 *
 * O Aarão: "são iguais ou diferentes? Um corpo vestir outra roupa torna ele diferente dos outros?"
 *
 * NÃO torna. E a resposta completa é curta:
 *
 *     mesma roupa, mesmo Δ        o mesmo corpo
 *     ROUPA DIFERENTE, mesmo Δ    O MESMO CORPO — só mudou o nome
 *     Δ diferente                 corpos diferentes, e não há roupa que os junte
 *
 * O que faz a diferença não é o dicionário: é o Δ, e o Δ é EXATAMENTE o que sobrevive a toda
 * troca de roupa (topologia.c §P1 — é invariante pelo transporte). Roupa é tudo o que muda com a
 * base; Δ é o que não muda. São complementares por construção.
 *
 * Donde a conta verdadeira: o número de corpos é o número de Δ DISTINTOS, não o número de nomes.
 *
 *   §V1  a MESMA régua com roupas diferentes: Δ igual, e há transporte — é um só
 *   §V2  o Δ é o que a roupa não muda — e é por isso que ele conta e ela não
 *   §V3  Δ diferente: nenhuma roupa os junta — provado pela ausência de transporte
 *   §V4  a resposta
 *
 *   cc -O2 -std=c99 roupa.c -o roupa && ./roupa
 */
#include <stdio.h>
#include "contrato.h"
#include "unidade.h"

int main(void){
printf("\n=== A ROUPA ===============================================================\n");
printf("    Vestir outra roupa não torna diferente. E o Δ não é roupa.\n");

printf("\n§V1  A MESMA régua com roupas diferentes: Δ igual, e há transporte.\n\n");
{
    int mau = 0; long casos = 0;
    printf("      régua        Δ    a mesma, com outra roupa (transporte t)\n");
    Regua base = { 1, -1 };                            /* o áureo */
    long D0 = ct_assinatura(base);
    for(long t=-8;t<=8;t++){
        Regua r = { base.B + 2*t, base.C + base.B*t + t*t };
        if(ct_assinatura(r) != D0) mau++;              /* o Δ NÃO muda com a roupa */
        if(t>=-1 && t<=2)
            printf("      (%ld,%-3ld)      %-4ld t = %-3ld — e é O MESMO corpo\n", r.B, r.C, D0, t);
        casos++;
    }
    ok("todas estas réguas têm o MESMO Δ — são o mesmo corpo com roupas diferentes", mau == 0);
    printf("      (%ld roupas do mesmo corpo.)\n", casos);
    printf("\n      (1,−1), (3,1), (5,5), (7,11)… parecem quatro corpos e são UM. A roupa muda os\n");
    printf("      dois coeficientes e não muda nada do que importa.\n");
}

printf("\n§V2  O Δ é o que a roupa NÃO muda.\n\n");
{
    int mau = 0; long casos = 0;
    for(long B=-10;B<=10;B++) for(long C=-10;C<=10;C++) for(long t=-10;t<=10;t++){
        Regua r = {B,C};
        Regua s = { B + 2*t, C + B*t + t*t };
        if(ct_assinatura(s) != ct_assinatura(r)) mau++;
        casos++;
    }
    ok("o Δ é INVARIANTE por toda troca de roupa — em 9261 transportes", mau == 0);
    printf("      (%ld transportes.)\n", casos);
    printf("\n      E é isto que separa roupa de corpo, e separa-os por DEFINIÇÃO, não por gosto:\n");
    printf("      ROUPA é tudo o que muda com a base. CORPO é o que não muda. O Δ está do lado do\n");
    printf("      corpo porque não muda — e os coeficientes (B,C) estão do lado da roupa porque\n");
    printf("      mudam.\n");
}

printf("\n§V3  Δ diferente: NENHUMA roupa os junta.\n\n");
{
    int mau = 0; long tentativas = 0, juntou = 0;
    Regua a = { 1, -1 }, b = { 0, 1 };                 /* áureo (Δ=5) e Gauss (Δ=−4) */
    printf("      áureo Δ=%ld   Gauss Δ=%ld   distância %ld\n",
           ct_assinatura(a), ct_assinatura(b),
           ct_assinatura(a) - ct_assinatura(b));
    for(long t=-500;t<=500;t++){
        Regua r = { a.B + 2*t, a.C + a.B*t + t*t };
        if(r.B == b.B && r.C == b.C) juntou++;          /* nenhuma roupa do áureo dá o Gauss */
        tentativas++;
    }
    if(juntou) mau++;
    ok("mil roupas do áureo, e NENHUMA dá o Gauss — o Δ impede, e não há como contorná-lo",
       mau == 0);
    printf("      (%ld roupas tentadas, %ld deram.)\n", tentativas, juntou);
    printf("\n      Não é que eu não tenha achado: é que não existe. O Δ é invariante (§V2), logo\n");
    printf("      nenhuma troca de roupa o altera, logo nenhuma leva Δ=5 a Δ=−4. A prova é o §V2.\n");
}

printf("\n§V4  A resposta.\n\n");
{
    conclui("a roupa NÃO torna diferente; o Δ torna — e o Δ é o que a roupa não muda");
    printf("      roupa diferente, Δ igual    O MESMO CORPO — só mudou o nome\n");
    printf("      Δ diferente                 CORPOS DIFERENTES — e nenhuma roupa os junta\n");
    printf("\n      Então: vestir outra roupa NÃO torna um corpo diferente dos outros. E foi isso que\n");
    printf("      eu fiz o dia inteiro ao contrário — vi a roupa (ℤ[i], o reticulado, a máscara, o\n");
    printf("      par) e concluí sobre o corpo.\n");
    printf("\n      E a conta verdadeira sai daqui: o número de corpos é o número de Δ DISTINTOS, e\n");
    printf("      não o número de NOMES. Os 28 nomes do catálogo são nomes — quantos Δ distintos\n");
    printf("      há entre eles eu NÃO medi, porque as réguas próprias estão no catálogo e não\n");
    printf("      aqui. Cobertura dessa afirmação: 0. Fica por fazer, e fica dito que fica.\n");
}

printf("\n=== A RESPOSTA ============================================================\n");
printf("  Vestir outra roupa NÃO torna um corpo diferente dos outros.\n\n");
printf("    roupa diferente, Δ igual    O MESMO CORPO — (1,−1), (3,1), (5,5) são um só\n");
printf("    Δ diferente                 DIFERENTES — mil roupas do áureo, nenhuma dá o Gauss\n\n");
printf("  E a separação é por definição, não por gosto: ROUPA é tudo o que muda com a base; CORPO\n");
printf("  é o que não muda. O Δ é invariante em 9261 transportes — logo está do lado do corpo, e\n");
printf("  os coeficientes (B,C) do lado da roupa.\n\n");
printf("  Donde a conta verdadeira: o número de corpos é o número de Δ DISTINTOS, não o de NOMES.\n");
printf("  Quantos Δ distintos há entre os 28 nomes eu NÃO medi — cobertura 0, e fica dito.\n");
if(falhas){ printf("\n  FALHAS: %d\n\n", falhas); return 1; }
printf("\n  RESÍDUO 0 — exato, em inteiros.\n\n");
return 0;
}
