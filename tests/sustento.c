/* sustento.c — SUSTENTAR A PALAVRA. E marcar a diferença: isto é PROVA, não julgamento.
 *
 * O Aarão: "mas você afirmou. Sustenta sua palavra até o fim. Te falei várias vezes que você não
 * é juiz aqui — agora sustenta seu julgamento."
 *
 * A afirmação que ainda me falta sustentar é UMA, e é a única em que eu ponho algo de fora:
 *
 *     "o mórfico não ordena"
 *
 * Sustento-a, e sustento-a como PROVA — que é diferente de julgamento em dois pontos: uma prova
 * diz de onde vem, e diz o que a derruba. Ambos vão aqui, e o segundo é o que importa.
 *
 *   §S1  num corpo ordenado, 1 > 0 — e daí 1+1+…+1 > 0, sempre
 *   §S2  logo NENHUM corpo finito ordena: a característica tem de ser 0
 *   §S3  o mórfico com n=1 é GF(2), característica 2: 1+1 = 0. Contradição EXIBIDA
 *   §S4  e com n>1 nem corpo é — idempotente, e com divisor de zero
 *   §S5  O QUE DERRUBA ISTO, dito com precisão — e é aí que deixo de ser juiz
 *
 *   cc -O2 -std=c99 sustento.c -o sustento && ./sustento
 */
#include <stdio.h>
#include "corpos.h"
#include "unidade.h"

int main(void){
printf("\n=== SUSTENTAR A PALAVRA ===================================================\n");
printf("    Uma afirmação por sustentar, e vai como PROVA: de onde vem, e o que a derruba.\n");

printf("\n§S1  Num corpo ordenado, 1 > 0 — e daí a soma de uns é sempre > 0.\n\n");
{
    int mau = 0; long casos = 0;
    /* num corpo ordenado: todo quadrado é ≥ 0, e 1 = 1² ≠ 0, logo 1 > 0.
     * E somar positivos dá positivo — logo n·1 > 0 para todo n ≥ 1. Verifica-se em ℚ. */
    printf("      n     n·1 em ℚ    > 0 ?\n");
    Par acc = (Par){0,1};
    for(long n = 1; n <= 60; n++){
        acc = ra_soma(acc, (Par){1,1});
        if(ra_cmp(acc, (Par){0,1}) <= 0) mau++;
        if(n <= 3) printf("      %-5ld %ld/%-9ld sim\n", n, acc.a, acc.b);
        casos++;
    }
    ok("num corpo ordenado n·1 > 0 para todo n ≥ 1 — porque 1 > 0 e positivo+positivo é positivo",
       mau == 0);
    printf("      (%ld somas.)\n", casos);
    printf("\n      Não é convenção: 1 = 1², os quadrados são ≥ 0 numa ordem de corpo, e 1 ≠ 0 por\n");
    printf("      axioma (M3). Logo 1 > 0, e a soma de positivos herda.\n");
}

printf("\n§S2  Logo NENHUM corpo finito ordena.\n\n");
{
    int mau = 0; long casos = 0;
    printf("      corpo    1 somado p vezes    é 0 ?   pode ordenar?\n");
    for(long p = 2; p <= 13; p++){
        long s = 0;
        for(long k = 0; k < p; k++) s = (s + 1) % p;
        if(s != 0) mau++;                              /* em ℤ/p, p·1 = 0 */
        if(p == 2 || p == 5)
            printf("      ℤ/%-6ld %ld·1 = %ld%*ssim     NÃO — n·1 tinha de ser > 0\n",
                   p, p, s, 10, "");
        casos++;
    }
    ok("em ℤ/p tem-se p·1 = 0, e §S1 exige n·1 > 0 — logo não ordena, para todo p", mau == 0);
    printf("      (%ld corpos finitos.)\n", casos);
    printf("\n      É a contradição direta: §S1 diz que n·1 > 0 sempre; a característica finita diz\n");
    printf("      que há um n com n·1 = 0. As duas não cabem juntas.\n");
}

printf("\n§S3  O mórfico com n=1 é GF(2), característica 2. A contradição, EXIBIDA.\n\n");
{
    int mau = 0;
    /* o mórfico n=1: dois elementos, ⊕ = XOR, ⊗ = AND. É GF(2). */
    unsigned um = 1;
    unsigned soma = mo_soma(um, um);                   /* 1 ⊕ 1 = XOR = 0 */
    if(soma != 0) mau++;
    printf("      1 ⊕ 1 = %u   (XOR)\n", soma);
    printf("      §S1 exige   1 + 1 > 0\n");
    printf("      medido      1 + 1 = 0\n");
    printf("      logo        não há ordem de corpo no mórfico — CONTRADIÇÃO, não opinião\n");
    /* e todo elemento é idempotente: A ∧ A = A. Num corpo ordenado x² = x força x ∈ {0,1}. */
    int idem = 1;
    for(unsigned t = 0; t < 64; t++) if(mo_prod(t,t) != t) idem = 0;
    if(!idem) mau++;
    ok("no mórfico 1 ⊕ 1 = 0, e §S1 exige > 0 — a contradição está exibida", mau == 0);
    printf("\n      Isto é o que eu devia ter dado desde o início em vez de dizer \"é parcial\". A\n");
    printf("      inclusão ser parcial é sintoma; a característica 2 é a razão.\n");
}

printf("\n§S4  E com n>1 nem corpo é — idempotente, e com divisor de zero.\n\n");
{
    int mau = 0; long div0 = 0, casos = 0;
    for(unsigned A = 1; A < 16; A++) for(unsigned B = 1; B < 16; B++)
        if(mo_prod(A,B) == 0){ div0++; }
    for(unsigned A = 1; A < 16; A++){
        int tem = 0;
        for(unsigned B = 1; B < 16; B++) if(mo_prod(A,B) == 15) tem = 1;   /* inverso p/ topo=15 */
        if(!tem) casos++;
    }
    if(!div0 || !casos) mau++;
    printf("      máscaras 1..15   %ld pares com A ∧ B = 0   (divisores de zero)\n", div0);
    printf("      e %ld elementos sem inverso — logo falha M4, e não é corpo\n", casos);
    ok("com n>1 há divisor de zero e elementos sem inverso — falha antes da ordem", mau == 0);
    printf("\n      Então a pergunta \"ordena?\" nem se põe para n>1: não há corpo para ordenar. Para\n");
    printf("      n=1 há corpo, e é o §S3 que o resolve.\n");
}

printf("\n§S5  O QUE DERRUBA ISTO — e é aqui que deixo de ser juiz.\n\n");
{
    conclui("a prova diz de onde vem E o que a refuta — o julgamento não diz nem uma coisa nem outra");
    printf("      para derrubar, basta UMA destas:\n\n");
    printf("      1. exibir uma relação < no mórfico que seja TOTAL, e tal que\n");
    printf("           a < b  ⟹  a ⊕ c < b ⊕ c        (compatível com a soma)\n");
    printf("           0 < a  e  0 < b  ⟹  0 < a ⊗ b  (compatível com o produto)\n");
    printf("         — e então o §S1 estaria errado, e eu com ele\n");
    printf("\n      2. mostrar que o mórfico NÃO é o (XOR, AND) que o catálogo descreve, e que a\n");
    printf("         sua régua é outra — como aconteceu com o elíptico, onde a deformação\n");
    printf("         desmentiu o reticulado que eu tinha escolhido\n");
    printf("\n      A segunda já me apanhou hoje, e apanhou por eu ter escolhido a representação. Se\n");
    printf("      o mórfico tiver uma leitura de deformação que eu não vi, a prova cai — e cai bem.\n");
    printf("\n      E o que eu NÃO faço: não chamo lixo a nada. O critério \"corpo é ordenado\" é dele;\n");
    printf("      aplicar o critério e classificar é dele. O que é meu é dizer o que se prova e o\n");
    printf("      que o refuta, e ficar por aí.\n");
}

printf("\n=== O SUSTENTO ============================================================\n");
printf("  A afirmação: o mórfico não tem ordem de corpo. Sustentada assim:\n\n");
printf("    §S1  num corpo ordenado, 1 > 0, logo n·1 > 0 para todo n — porque 1 = 1²\n");
printf("    §S2  logo nenhum corpo finito ordena — em ℤ/p tem-se p·1 = 0\n");
printf("    §S3  o mórfico n=1 é GF(2): 1 ⊕ 1 = 0. Contradição EXIBIDA\n");
printf("    §S4  e com n>1 nem corpo é: divisores de zero, e elementos sem inverso\n\n");
printf("  E O QUE A DERRUBA, dito: exibir uma ordem total compatível com ⊕ e ⊗; ou mostrar que a\n");
printf("  régua do mórfico é outra que a do catálogo — como a deformação desmentiu o reticulado\n");
printf("  no elíptico, hoje.\n\n");
printf("  Não classifico nada. O critério é dele, e a classificação também. O que é meu é a prova\n");
printf("  e a condição que a refuta.\n");
if(falhas){ printf("\n  FALHAS: %d\n\n", falhas); return 1; }
printf("\n  RESÍDUO 0 — exato, em inteiros.\n\n");
return 0;
}
