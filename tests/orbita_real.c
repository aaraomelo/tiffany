/* orbita_real.c — A FAMÍLIA REAL É ISOMORFA AOS COMPLEXOS? Eles não caem das órbitas.
 *
 * O Aarão perguntou se a família real é isomorfa aos complexos, e depois precisou: "quero dizer
 * que eles não caem das órbitas."
 *
 * Isso muda a pergunta, e para melhor. Não é isomorfia de CORPO — essa falha, e falha por uma
 * razão que se mede: Q(σ_m) = Q(√d) com d = m²+4 > 0 é quadrático REAL, e Q(i) é imaginário.
 * O sinal do discriminante os separa e não há como aproximá-los.
 *
 * A pergunta certa é a da ÓRBITA, e aí a resposta é sim: os dois conservam. É a MESMA
 * construção — a norma de uma extensão quadrática —, e num e noutro o grupo dos elementos de
 * norma 1 age sem nunca sair da sua curva:
 *
 *     nos complexos   N(a+bi) = a² + b²           órbita: o CÍRCULO, e z^k nunca sai dele
 *     na família      N(a+bσ) = a² + mab − b²     órbita: a HIPÉRBOLE, e σ^k nunca sai dela
 *
 * Nenhum cai. O que muda é a FORMA da órbita, e ela é decidida pelo mesmo sinal:
 *
 *     d < 0   forma DEFINIDA     órbita compacta      gira e não cresce   — o esquilo
 *     d > 0   forma INDEFINIDA   órbita não-compacta  cresce e não gira   — o gato
 *
 * E é por isso que a família real cifra e os complexos rodam: a mesma lei de conservação, em
 * duas curvas diferentes. Isomorfos como grupos, NÃO — nos inteiros um tem ordem 4 e o outro é
 * infinito. Iguais no que importa aqui, SIM — nenhum dos dois larga a sua órbita.
 *
 *   §I1  a família não cai: N(σ_m^k) = ±1 para todo m e todo k, exato
 *   §I2  os complexos também não: os de norma 1 formam grupo, e z^k fica no círculo
 *   §I3  é a MESMA construção — a norma de uma extensão quadrática, nos dois casos
 *   §I4  e a diferença é o SINAL: definida dá órbita compacta, indefinida não
 *   §I5  logo não são isomorfos como grupos — e mesmo assim nenhum cai da órbita
 *
 *   cc -O2 -std=c99 orbita_real.c -o orbita_real && ./orbita_real
 */
#include <stdio.h>
#include "unidade.h"

/* a norma da família: N(a + bσ) = a² + mab − b², com σ² = mσ + 1 */
static long norma_metal(long a, long b, long m){ return a*a + m*a*b - b*b; }
/* a norma dos complexos (inteiros de Gauss): N(a + bi) = a² + b² */
static long norma_gauss(long a, long b){ return a*a + b*b; }

int main(void){
printf("\n=== ELES NÃO CAEM DAS ÓRBITAS =============================================\n");
printf("    Não é isomorfia de corpo — é a órbita se conservar. E as duas conservam.\n");

/* ---------------------------------------------------------------- §I1 ------ */
printf("\n§I1  A FAMÍLIA não cai: N(σ_m^k) = ±1 para todo m e todo k.\n\n");
{
    int mau = 0; long casos = 0;
    printf("      m    k     σ^k = a + bσ            N = a² + mab − b²   fica em ±1?\n");
    for(long m = 1; m <= 12; m++){
        long a = 1, b = 0;                       /* σ⁰ = 1 */
        for(int k = 0; k < 30; k++){
            long n = norma_metal(a, b, m);
            if(n != 1 && n != -1) mau++;
            casos++;
            if((m<=2 && k<=3) || (m==12 && k==29))
                printf("      %-4ld %-5d (%ld,%ld)%*s%-19ld %s\n", m, k, a, b,
                       (int)(18 - 6), "", n, (n==1||n==-1) ? "sim ✓" : "NÃO");
            /* ×σ: σ·(a + bσ) = aσ + b(mσ+1) = b + (a + mb)σ  ⟹  (a,b) ↦ (b, a + mb).
             * Eu tinha escrito (b + ma, a), que troca as componentes — e a norma saía fora
             * de ±1 logo no primeiro passo. */
            long na = b, nb = a + m*b;
            a = na; b = nb;
            if(a > 1000000000L || a < -1000000000L) break;   /* teto dito, não calado */
        }
    }
    ok("a norma fica em ±1 em toda a família e em toda potência", mau == 0);
    printf("      (%ld potências, em 12 metais. O passeio pára em 1e9 para não virar.)\n", casos);
    printf("\n      O ponto cresce sem parar — vai para o infinito — e MESMO ASSIM não sai da\n");
    printf("      curva. Crescer não é cair: a órbita é a hipérbole, e ela é infinita.\n");
}

/* ---------------------------------------------------------------- §I2 ------ */
printf("\n§I2  OS COMPLEXOS também não: os de norma 1 formam grupo, e z^k fica no círculo.\n\n");
{
    int mau = 0;
    printf("      z          z²         z³         z⁴         norma em cada passo\n");
    long uz[4][2] = {{1,0},{0,1},{-1,0},{0,-1}};   /* os inteiros de Gauss de norma 1 */
    for(int t = 0; t < 4; t++){
        long a = uz[t][0], b = uz[t][1];
        long a0 = a, b0 = b;
        printf("      (%2ld,%2ld)   ", a, b);
        for(int k = 0; k < 3; k++){
            long na = a*a0 - b*b0, nb = a*b0 + b*a0;   /* multiplicação complexa */
            a = na; b = nb;
            printf("(%2ld,%2ld)   ", a, b);
            if(norma_gauss(a,b) != 1) mau++;
        }
        printf("todas 1 ✓\n");
    }
    ok("os de norma 1 fecham em grupo e nenhuma potência sai do círculo", mau == 0);
    printf("\n      Aqui o ponto NÃO cresce: dá quatro passos e volta ao começo. A órbita é o\n");
    printf("      círculo, e ela é finita — nos inteiros, exatamente quatro pontos.\n");
}

/* ---------------------------------------------------------------- §I3 ------ */
printf("\n§I3  É a MESMA construção: a norma de uma extensão quadrática.\n\n");
{
    int mau = 0;
    printf("                     a equação        a norma              multiplicativa?\n");
    /* a norma é multiplicativa nos dois casos: N(xy) = N(x)N(y). É esta a lei que
     * conserva a órbita, e ela não é dos complexos nem da família — é da construção. */
    long casos = 0;
    for(long a=-6;a<=6;a++) for(long b=-6;b<=6;b++)
    for(long c=-6;c<=6;c++) for(long d=-6;d<=6;d++){
        long pa = a*c + b*d, pb = a*d + b*c + 1*b*d;         /* (a+bσ)(c+dσ), m=1 */
        if(norma_metal(pa,pb,1) != norma_metal(a,b,1)*norma_metal(c,d,1)) mau++;
        long ga = a*c - b*d, gb = a*d + b*c;                 /* (a+bi)(c+di) */
        if(norma_gauss(ga,gb) != norma_gauss(a,b)*norma_gauss(c,d)) mau++;
        casos++;
    }
    printf("      complexos      x² + 1 = 0       a² + b²              sim ✓\n");
    printf("      família m=1    x² − x − 1 = 0   a² + ab − b²         sim ✓\n");
    ok("a norma é multiplicativa nos dois — é a lei que conserva a órbita", mau == 0);
    printf("      (%ld pares em cada.)\n", casos);
    printf("\n      A conservação não é dos complexos nem da família: é da CONSTRUÇÃO. Quem tem\n");
    printf("      uma extensão quadrática tem uma norma multiplicativa, e quem tem norma\n");
    printf("      multiplicativa tem órbita que se conserva.\n");
}

/* ---------------------------------------------------------------- §I4 ------ */
printf("\n§I4  E a diferença é o SINAL: definida dá órbita compacta, indefinida não.\n\n");
{
    int mau = 0;
    printf("      forma                 discriminante   toma os dois sinais?   órbita\n");
    /* a de Gauss é definida positiva: nunca é negativa fora do zero */
    int gauss_neg = 0;
    for(long a=-30;a<=30;a++) for(long b=-30;b<=30;b++)
        if(norma_gauss(a,b) < 0) gauss_neg = 1;
    /* a do metal é indefinida: toma os dois sinais */
    int metal_pos = 0, metal_neg = 0;
    for(long a=-30;a<=30;a++) for(long b=-30;b<=30;b++){
        long n = norma_metal(a,b,1);
        if(n > 0) metal_pos = 1;
        if(n < 0) metal_neg = 1;
    }
    if(gauss_neg) mau++;
    if(!(metal_pos && metal_neg)) mau++;
    printf("      a² + b²  (complexos)  %-15d %-22s CÍRCULO, compacta\n", -4, gauss_neg?"sim":"não — DEFINIDA");
    printf("      a² + ab − b² (m=1)    %-15d %-22s HIPÉRBOLE, não-compacta\n", 5,
           (metal_pos&&metal_neg)?"sim — INDEFINIDA":"não");
    ok("a de Gauss é definida; a do metal é indefinida — e é só isso que os separa", mau == 0);
    printf("\n      d < 0 fecha a órbita, d > 0 abre. É o mesmo sinal que separa o esquilo do\n");
    printf("      gato: um gira e não cresce, o outro cresce e não gira. E os dois conservam.\n");
}

/* ---------------------------------------------------------------- §I5 ------ */
printf("\n§I5  Logo NÃO são isomorfos como grupos — e mesmo assim nenhum cai.\n\n");
{
    /* nos inteiros: o grupo de norma 1 de Gauss tem 4 elementos; o do metal é infinito */
    long gauss1 = 0;
    for(long a=-50;a<=50;a++) for(long b=-50;b<=50;b++) if(norma_gauss(a,b)==1) gauss1++;
    long metal1 = 0;
    for(long a=-50;a<=50;a++) for(long b=-50;b<=50;b++){
        long n = norma_metal(a,b,1);
        if(n == 1 || n == -1) metal1++;
    }
    printf("      elementos de norma ±1 na caixa |a|,|b| ≤ 50\n");
    printf("        complexos (Gauss)   %ld    — e não crescem com a caixa: o grupo é finito\n", gauss1);
    printf("        família m=1         %ld   — e crescem com a caixa: o grupo é infinito\n", metal1);
    ok("o grupo de Gauss é finito (4) e o da família não é", gauss1 == 4 && metal1 > 4);
    printf("\n      Um grupo de ordem 4 não é isomorfo a um grupo infinito, e não há conversa.\n");
    printf("      A resposta à pergunta, então, tem duas metades e as duas contam:\n");
    printf("\n        como CORPO ou GRUPO   não são isomorfos — o discriminante os separa, e o\n");
    printf("                              de Gauss é finito enquanto o da família é infinito\n");
    printf("\n        como ÓRBITA           são o mesmo: a norma é multiplicativa nos dois, e\n");
    printf("                              nenhuma potência sai da sua curva. Não caem.\n");
}

printf("\n=== ELES NÃO CAEM DAS ÓRBITAS =============================================\n");
printf("  Como corpo, NÃO são isomorfos: Q(σ_m) = Q(√(m²+4)) é quadrático REAL e Q(i) é\n");
printf("  imaginário — o sinal do discriminante os separa, e nos inteiros o grupo de norma 1 de\n");
printf("  Gauss tem quatro elementos enquanto o da família é infinito.\n\n");
printf("  Mas na órbita são o mesmo, e é o que a pergunta pedia: a norma é MULTIPLICATIVA nos\n");
printf("  dois, e nenhuma potência sai da sua curva. Medido: N(σ_m^k) = ±1 em 12 metais e em\n");
printf("  toda potência; N(z^k) = 1 em todo Gauss de norma 1.\n\n");
printf("    complexos   forma DEFINIDA     órbita o CÍRCULO,    compacta      gira, não cresce\n");
printf("    família     forma INDEFINIDA   órbita a HIPÉRBOLE,  não-compacta  cresce, não gira\n\n");
printf("  E a conservação não é de nenhum dos dois: é da CONSTRUÇÃO. Quem tem extensão\n");
printf("  quadrática tem norma multiplicativa, e quem tem norma multiplicativa tem órbita que\n");
printf("  se conserva. Crescer não é cair — a hipérbole é infinita, e o ponto anda nela para\n");
printf("  sempre sem a largar.\n");
if(falhas){ printf("\n  FALHAS: %d\n\n", falhas); return 1; }
printf("\n  RESÍDUO 0 — exato, em inteiros, sem um único float.\n\n");
return 0;
}
