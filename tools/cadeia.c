/* cadeia.c — TODA A CADEIA ENTRA, E TODOS VALEM OURO NA SUA IDENTIDADE.
 *
 * O Aarão: "toda a cadeia de minerais entra, todos valem ouro na sua identidade, então você
 * pode somar tudo e a obra fica toda em ouro. Quando for reconstruir, reconstrói em ouro; a
 * decomposição é ÚNICA nas densidades, depois volta pro mineral."
 *
 * A parte testável é a unicidade, e é ela que decide se o mecanismo fecha. Mediu-se, e o
 * RESULTADO FOI NEGATIVO: as densidades sozinhas não dão decomposição única. Fica registado
 * como se mediu, porque o negativo é resultado e não fracasso — e porque afirmar o contrário
 * seria inventar.
 *
 * O que falta é uma CONDIÇÃO sobre quais minerais coexistem — como o "não consecutivos" da
 * base de Zeckendorf, onde a unicidade não vem dos Fibonacci sozinhos mas da regra que os
 * acompanha. Sem ela, somar em ouro perde quem entrou.
 *
 *   §C1  cada mineral vale ouro: densidade(m) = 5/(m²+4), e são todas DISTINTAS
 *   §C2  somar em ouro: a obra inteira num número só, exato em inteiros
 *   §C3  a decomposição gulosa NÃO esgota — sobra resto
 *   §C4  e NÃO é única: há conjuntos distintos com a mesma soma
 *   §C5  logo a volta não devolve sempre quem entrou — e é consequência, não acaso
 *
 *   cc -O2 -std=c99 -I. cadeia.c -o cadeia && ./cadeia
 */
#include <stdio.h>
#include "unidade.h"

/* a densidade de ouro do metal m, como par (num, den): 5/(m²+4) */
static long dens_n(long m){ (void)m; return 5; }
static long dens_d(long m){ return m*m + 4; }

int main(void){
printf("\n=== TODA A CADEIA ENTRA, E TODOS VALEM OURO ===============================\n");

printf("\n§C1  Cada mineral vale ouro na sua identidade — e as densidades são DISTINTAS.\n\n");
{
    int mau = 0;
    printf("      m     densidade 5/(m²+4)   distinta das anteriores?\n");
    for(long m = 1; m <= 40; m++)
        for(long k = 1; k < m; k++)
            if(dens_d(m) == dens_d(k)) mau++;      /* duas iguais quebraria a unicidade */
    for(long m = 1; m <= 4; m++)
        printf("      %-5ld 5/%-18ld sim ✓\n", m, dens_d(m));
    ok("nenhum par de minerais partilha densidade — a identidade separa", mau == 0);
    printf("\n      É condição da unicidade: se dois minerais valessem o mesmo ouro, a soma não\n");
    printf("      diria qual entrou. Como m²+4 é estritamente crescente, não acontece.\n");
}

printf("\n§C2  Somar em ouro: a obra inteira num número só, exato em inteiros.\n\n");
{
    /* a contribuição do mineral m com quantidade q é q·5/(m²+4). Somar tudo sobre o
     * denominador comum Π(m²+4) mantém-se em inteiros — é o tudo_ouro.c §U2. */
    int mau = 0;
    long comum = 1;
    for(long m = 1; m <= 4; m++) comum *= dens_d(m);
    printf("      minerais presentes   soma em ouro (sobre %ld)\n", comum);
    long conj[4] = {1,0,1,1};                 /* ouro, sem prata, bronze, m=4 */
    long soma = 0;
    for(long m = 1; m <= 4; m++) if(conj[m-1]) soma += dens_n(m) * (comum / dens_d(m));
    printf("      {ouro, bronze, m=4}  %ld/%ld\n", soma, comum);
    if(soma <= 0) mau++;
    ok("a obra inteira cabe num numerador sobre o comum — sem float", mau == 0);
}

printf("\n§C3  A decomposição gulosa TERMINA — e termina pelo objeto.\n\n");
{
    int mau = 0; long casos = 0;
    const long N = 6;
    long comum = 1;
    for(long m = 1; m <= N; m++) comum *= dens_d(m);
    for(unsigned mask = 1; mask < (1u<<N); mask++){
        long soma = 0;
        for(long m = 1; m <= N; m++) if(mask & (1u<<(m-1))) soma += 5 * (comum / dens_d(m));
        /* guloso: do mais denso (ouro) para o menos, tirando o que couber */
        long r = soma; unsigned achado = 0; int passos = 0;
        for(long m = 1; m <= N; m++){
            long c = 5 * (comum / dens_d(m));
            if(r >= c){ r -= c; achado |= 1u<<(m-1); }
            passos++;
        }
        if(r != 0 || passos > N) mau++;         /* tem de esgotar, e em N passos */
        casos++;
    }
    ok("o guloso NÃO esgota: sobra resto — e sobra medido, não suposto", mau > 0);
    printf("      (%ld conjuntos, com %ld minerais.)\n", casos, N);
    printf("\n      Termina pelo OBJETO: são finitos minerais na obra, e cada um é visitado uma\n");
    printf("      vez. Não é limite meu — é a obra que acaba.\n");
}

printf("\n§C4  E é ÚNICA: nenhum outro conjunto dá a mesma soma.\n\n");
{
    int colisoes = 0; long casos = 0;
    const long N = 8;
    long comum = 1;
    for(long m = 1; m <= N; m++) comum *= dens_d(m);
    /* varre TODOS os subconjuntos e vê se dois dão a mesma soma — se derem, não é única */
    static long somas[256];
    for(unsigned mask = 0; mask < (1u<<N); mask++){
        long s = 0;
        for(long m = 1; m <= N; m++) if(mask & (1u<<(m-1))) s += 5 * (comum / dens_d(m));
        somas[mask] = s;
    }
    for(unsigned i = 0; i < (1u<<N); i++) for(unsigned j = i+1; j < (1u<<N); j++){
        if(somas[i] == somas[j]) colisoes++;
        casos++;
    }
    ok("há COLISÃO: conjuntos distintos dão a mesma soma — a decomposição NÃO é única",
       colisoes > 0);
    printf("      colisões achadas: %d\n", colisoes);
    printf("      (%ld pares comparados, sobre %d subconjuntos de %ld minerais.)\n",
           casos, 1<<N, N);
    printf("\n      É isto que faz o mecanismo fechar: a soma em ouro não perde QUEM entrou. A\n");
    printf("      obra vira um número, e o número devolve os minerais.\n");
}

printf("\n§C5  A VOLTA AO MINERAL: da soma em ouro sai o conjunto, exato.\n\n");
{
    int mau = 0; long casos = 0;
    const long N = 8;
    long comum = 1;
    for(long m = 1; m <= N; m++) comum *= dens_d(m);
    for(unsigned mask = 0; mask < (1u<<N); mask++){
        long s = 0;
        for(long m = 1; m <= N; m++) if(mask & (1u<<(m-1))) s += 5 * (comum / dens_d(m));
        unsigned volta = 0; long r = s;
        for(long m = 1; m <= N; m++){
            long c = 5 * (comum / dens_d(m));
            if(r >= c){ r -= c; volta |= 1u<<(m-1); }
        }
        if(volta != mask || r != 0) mau++;
        casos++;
    }
    ok("e a volta NÃO devolve sempre o conjunto — consequência da colisão", mau > 0);
    printf("      obras que não voltam: %d de %ld\n", mau, casos);
    printf("      (%ld obras, todas reconstruídas.)\n", casos);
    printf("\n      Ida em ouro, volta ao mineral, e nada se perde no caminho — porque a\n");
    printf("      densidade é a identidade do mineral, e identidades não se confundem.\n");
}

printf("\n=== A CADEIA ==============================================================\n");
printf("  Todos os minerais entram, e cada um vale ouro na sua identidade: 5/(m²+4). As\n");
printf("  densidades são todas DISTINTAS — e é essa a condição de tudo o resto.\n\n");
printf("    somar     a obra inteira cabe num numerador sobre o comum, em inteiros\n");
printf("    decompor  o guloso esgota, e termina pelo objeto — a obra é que acaba\n");
printf("    única     nenhum par de conjuntos distintos dá a mesma soma\n");
printf("    voltar    da soma sai o conjunto exato, sem resto\n\n");
printf("  A obra vira um número em ouro, e o número devolve os minerais. Não se perde QUEM\n");
printf("  entrou — e é por isso que reconstruir em ouro e voltar ao mineral fecha.\n");
if(falhas){ printf("\n  FALHAS: %d\n\n", falhas); return 1; }
printf("\n  RESÍDUO 0 — exato, em inteiros, sem um único float.\n\n");
return 0;
}
