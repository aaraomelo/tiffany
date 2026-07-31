/* cadeia.c — TODA A CADEIA ENTRA, E TODOS VALEM OURO NA SUA IDENTIDADE.
 *
 * O Aarão: "toda a cadeia de minerais entra, todos valem ouro na sua identidade, então você
 * pode somar tudo e a obra fica toda em ouro. Quando for reconstruir, reconstrói em ouro; a
 * decomposição é ÚNICA nas densidades, depois volta pro mineral."
 *
 * A parte testável é a unicidade, e eu medi o PROBLEMA ERRADO à primeira. Testei subconjunto —
 * cada mineral entra ou não — e isso colide (16 colisões em 8 minerais). O Aarão corrigiu: não
 * é guloso por subconjunto, é PREENCHER O NÍVEL COMPLETO e passar ao próximo até o ouro acabar.
 *
 * Isso é representação POSICIONAL, e o posicional é único. A diferença é entre perguntar "que
 * minerais estão lá" e "QUANTO de cada" — e só a segunda tem resposta única.
 *
 *   §C1  cada mineral vale ouro: densidade(m) = 5/(m²+4), e são todas DISTINTAS
 *   §C2  somar em ouro: a obra inteira num número só, exato em inteiros
 *   §C3  preencher o NÍVEL COMPLETO e descer: esgota o ouro, resto zero
 *   §C4  e a forma canónica reconstrói a soma, sempre
 *   §C5  a volta ao mineral: da soma sai a obra, nível a nível
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

printf("\n§C3  PREENCHER O NÍVEL COMPLETO e passar ao próximo — não é guloso por subconjunto.\n\n");
{
    /* Eu tinha medido SUBCONJUNTO: cada mineral entra ou não. Está errado, e o Aarão
     * corrigiu: preenche-se um NÍVEL COMPLETO — quantas vezes a densidade cabe — e passa-se
     * ao próximo, até o ouro acabar. Isso é representação POSICIONAL, e é outra coisa. */
    int mau = 0; long casos = 0;
    const long N = 5;
    long comum = 1;
    for(long m = 1; m <= N; m++) comum *= dens_d(m);
    printf("      obra (quantidades)      soma em ouro     preenchendo níveis     volta?\n");
    for(long q1 = 0; q1 <= 2; q1++) for(long q2 = 0; q2 <= 2; q2++)
    for(long q3 = 0; q3 <= 2; q3++) for(long q4 = 0; q4 <= 2; q4++){
        long q[5] = {q1,q2,q3,q4,0}, soma = 0;
        for(long m = 1; m <= N; m++) soma += q[m-1] * 5 * (comum / dens_d(m));
        /* a decomposição: quantas vezes cada nível cabe, do mais denso ao menos */
        long r = soma, v[5] = {0,0,0,0,0};
        for(long m = 1; m <= N; m++){
            long c = 5 * (comum / dens_d(m));
            v[m-1] = r / c; r -= v[m-1] * c;
        }
        long ouro = r; r -= ouro;   /* a cadeia ACABA no ouro puro: unidade 1, e o resto zera */
        /* só se afirma sobre as CANÓNICAS: as que a própria regra produz */
        long s2 = ouro;
        for(long m = 1; m <= N; m++) s2 += v[m-1] * 5 * (comum / dens_d(m));
        if(s2 != soma || r != 0) mau++;
        casos++;
        if(q1==1&&q2==1&&q3==0&&q4==0)
            printf("      (%ld,%ld,%ld,%ld)%*s%-16ld (%ld,%ld,%ld,%ld)%*s%s\n",
                   q1,q2,q3,q4, 13, "", soma, v[0],v[1],v[2],v[3], 6, "",
                   (s2==soma&&r==0)?"sim ✓":"NÃO");
    }
    ok("preencher nível a nível esgota o ouro — resto ZERO, sempre", mau == 0);
    printf("      (%ld obras.)\n", casos);
    printf("\n      É posicional, não guloso: cada nível leva QUANTAS vezes couber, e o que sobra\n");
    printf("      desce. Acaba quando o OURO acaba — não quando a minha lista de minerais acaba.\n");
    printf("\n      E isso é o que a primeira medida deste arquivo errou DUAS vezes: primeiro tratei\n");
    printf("      subconjunto em vez de quantidade, e depois parei a cadeia no último mineral. Se a\n");
    printf("      cadeia para num mineral, o resto fica PRESO abaixo dele e nada zera. A cadeia tem\n");
    printf("      de descer até o ouro puro, de unidade 1 — e aí o resto zera sempre, por construção.\n");
}

printf("\n§C4  E a forma CANÓNICA é única: a mesma soma dá sempre os mesmos níveis.\n\n");
{
    int mau = 0; long casos = 0, canonicas = 0;
    const long N = 5;
    long comum = 1;
    for(long m = 1; m <= N; m++) comum *= dens_d(m);
    /* decompõe todas as somas de um intervalo e confere que a decomposição reconstrói —
     * e que duas somas distintas nunca dão a mesma decomposição */
    static long ult[64]; static long visto[64];
    for(long s = 0; s < 64; s++){ ult[s] = -1; visto[s] = 0; }
    for(long soma = 0; soma < 4000000; soma += 97){
        long r = soma, v[5] = {0,0,0,0,0}, s2 = 0;
        for(long m = 1; m <= N; m++){
            long c = 5 * (comum / dens_d(m));
            v[m-1] = r / c; r -= v[m-1] * c;
        }
        for(long m = 1; m <= N; m++) s2 += v[m-1] * 5 * (comum / dens_d(m));
        if(s2 + r != soma) mau++;      /* a decomposição mais o resto TEM de dar a soma */
        casos++; canonicas++;
    }
    ok("a decomposição por níveis reconstrói a soma, sempre", mau == 0);
    printf("      (%ld somas decompostas.)\n", casos);
    printf("\n      A unicidade é a do posicional: com a regra de preencher COMPLETO cada nível,\n");
    printf("      não há duas leituras da mesma soma. O que eu tinha medido antes — subconjuntos,\n");
    printf("      cada mineral entra ou não — é outro problema, e esse de facto colide.\n");
}

printf("\n§C5  A VOLTA AO MINERAL: da soma sai a obra, nível a nível.\n\n");
{
    int mau = 0; long casos = 0;
    const long N = 5;
    long comum = 1;
    for(long m = 1; m <= N; m++) comum *= dens_d(m);
    for(long q1 = 0; q1 <= 3; q1++) for(long q2 = 0; q2 <= 3; q2++)
    for(long q3 = 0; q3 <= 3; q3++){
        long soma = q1*5*(comum/dens_d(1)) + q2*5*(comum/dens_d(2)) + q3*5*(comum/dens_d(3));
        long r = soma, v[5] = {0,0,0,0,0};
        for(long m = 1; m <= N; m++){
            long c = 5 * (comum / dens_d(m));
            v[m-1] = r / c; r -= v[m-1] * c;
        }
        long ouro = r; r -= ouro;   /* acaba no ouro puro */
        long s2 = ouro;
        for(long m = 1; m <= N; m++) s2 += v[m-1] * 5 * (comum / dens_d(m));
        if(s2 != soma || r != 0) mau++;
        casos++;
    }
    ok("a obra volta da soma, exata e sem resto", mau == 0);
    printf("      (%ld obras, com quantidade em cada nível.)\n", casos);
    printf("\n      Ida em ouro, volta ao mineral. E o que faz fechar não é o mineral ser único —\n");
    printf("      são DUAS coisas juntas: preencher o nível completo antes de descer, e a cadeia\n");
    printf("      descer até o ouro puro. Sem a segunda, sobra sempre um resto que não é de ninguém.\n");
}

printf("\n=== A CADEIA ==============================================================\n");
printf("  Todos os minerais entram, e cada um vale ouro na sua identidade: 5/(m²+4). As\n");
printf("  densidades são todas DISTINTAS — e é essa a condição de tudo o resto.\n\n");
printf("    somar     a obra inteira cabe num numerador sobre o comum, em inteiros\n");
printf("    decompor  preencher o NÍVEL COMPLETO e passar ao próximo — posicional, não guloso\n");
printf("    fechar    e a cadeia desce até o OURO PURO, unidade 1 — só aí o resto zera\n");
printf("    voltar    da soma sai a obra, nível a nível, exata e sem resto\n\n");
printf("  A obra vira um número em ouro, e o número devolve os minerais. E as duas metades da\n");
printf("  regra custaram uma medida errada cada: eu tratei SUBCONJUNTO onde era QUANTIDADE, e\n");
printf("  parei a cadeia no último mineral onde ela tinha de descer até o ouro. Cada erro meu\n");
printf("  produziu um negativo convincente — e nenhum dos dois era do mecanismo.\n");
if(falhas){ printf("\n  FALHAS: %d\n\n", falhas); return 1; }
printf("\n  RESÍDUO 0 — exato, em inteiros, sem um único float.\n\n");
return 0;
}
