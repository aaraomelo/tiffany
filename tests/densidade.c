/* densidade.c — A DENSIDADE DE OURO DE CADA MEMBRO DA FAMÍLIA REAL.
 *
 * "Quão de ouro" é um metal? A pergunta tem resposta exata, e ela já estava medida sem esse
 * nome: é o DISCRIMINANTE.
 *
 * O que distingue o ouro não é ser bonito — é ser o PIOR aproximável por racionais. Para
 * σ_m = [m;m,m,…], a melhor constante de aproximação é √d com d = m²+4 (a constante de
 * Lagrange): quanto MAIOR o d, mais fácil é cercar o número por frações, e menos ouro ele tem.
 * O ouro tem d = 5, o menor de todos, e por isso é o mais difícil de alcançar.
 *
 * Então a densidade de ouro é a razão para o extremo, e é uma classe racional exata:
 *
 *     densidade(m) = d(1)/d(m) = 5/(m² + 4)
 *
 * Vale 1 no ouro — o extremo é ele — e cai a cada metal seguinte. Nada de raiz, nada de float:
 * é o par (5, m²+4) reduzido pelo mdc, a classe do racional_pg.c §Q1.
 *
 *   §D1  a densidade de cada metal, como classe reduzida
 *   §D2  o ouro vale 1 e é o MÁXIMO — nenhum metal é mais de ouro que o ouro
 *   §D3  e cai estritamente com m: cada metal seguinte é menos de ouro que o anterior
 *   §D4  a metade: onde a densidade cai abaixo de 1/2, e por quê
 *   §D5  e a leitura: densidade alta = difícil de cercar. É a mesma norma ±1 do §N3.
 *
 *   cc -O2 -std=c99 densidade.c -o densidade && ./densidade
 */
#include <stdio.h>
#include "unidade.h"

static long mdc_l(long a, long b){ if(a<0)a=-a; if(b<0)b=-b; while(b){ long t=a%b; a=b; b=t; } return a?a:1; }
static long disc(long m){ return m*m + 4; }

int main(void){
printf("\n=== A DENSIDADE DE OURO DA FAMÍLIA REAL ===================================\n");
printf("    densidade(m) = 5/(m²+4) — a razão do discriminante do ouro para o do metal.\n");

/* ---------------------------------------------------------------- §D1 ------ */
printf("\n§D1  A densidade de cada metal, como CLASSE reduzida.\n\n");
{
    int mau = 0;
    printf("      m     nome        d = m²+4   densidade = 5/d   classe reduzida\n");
    const char *nome[13] = {"","ouro","prata","bronze","","","","","","","","",""};
    for(long m = 1; m <= 12; m++){
        long d = disc(m), num = 5, den = d;
        long g = mdc_l(num, den);
        num /= g; den /= g;
        /* a classe tem de reconstruir a razão: num·d = 5·den */
        if(num * d != 5 * den) mau++;
        printf("      %-5ld %-11s %-10ld %ld/%-16ld %ld/%ld\n", m,
               (m <= 3 && nome[m][0]) ? nome[m] : "—", d, 5L, d, num, den);
    }
    ok("a densidade é uma classe racional exata, e ela fecha", mau == 0);
    printf("\n      Nada de raiz e nada de float: é o par (5, m²+4) reduzido pelo mdc. A\n");
    printf("      densidade de um metal é um NÚMERO RACIONAL, e é lido no discriminante.\n");
}

/* ---------------------------------------------------------------- §D2 ------ */
printf("\n§D2  O ouro vale 1, e é o MÁXIMO: nenhum metal é mais de ouro que o ouro.\n\n");
{
    int mau = 0;
    long dmin = disc(1);
    for(long m = 1; m <= 200; m++) if(disc(m) < dmin) mau++;
    printf("      densidade(1) = 5/5 = 1\n");
    printf("      menor discriminante em m ≤ 200   %ld  (é o do ouro)\n", dmin);
    ok("o ouro tem o menor discriminante, logo a maior densidade", mau == 0);
    printf("\n      É o mesmo ouro do coroa.c §A4: sem termo grande na fração contínua, nada\n");
    printf("      onde uma fração se agarre. Ser o mais difícil de cercar É ser o mais de ouro.\n");
}

/* ---------------------------------------------------------------- §D3 ------ */
printf("\n§D3  E cai ESTRITAMENTE: cada metal seguinte é menos de ouro que o anterior.\n\n");
{
    int mau = 0;
    printf("      m     densidade 5/d   contra o anterior\n");
    for(long m = 2; m <= 200; m++){
        /* 5/d(m) < 5/d(m−1)  ⟺  d(m) > d(m−1), e comparar inteiros basta */
        if(disc(m) <= disc(m-1)) mau++;
    }
    for(long m = 1; m <= 6; m++)
        printf("      %-5ld 5/%-13ld %s\n", m, disc(m),
               m == 1 ? "— é o topo" : "menor que o anterior ✓");
    ok("a densidade decresce estritamente em m, sem exceção até 200", mau == 0);
    printf("\n      A família é uma ESCADA que só desce: o ouro no topo, e cada degrau um pouco\n");
    printf("      menos de ouro. Não há dois metais com a mesma densidade.\n");
}

/* ---------------------------------------------------------------- §D4 ------ */
printf("\n§D4  A metade: onde a densidade cai abaixo de 1/2.\n\n");
{
    long primeiro = 0;
    for(long m = 1; m <= 200 && !primeiro; m++)
        if(5 * 2 < disc(m)) primeiro = m;         /* 5/d < 1/2  ⟺  10 < d */
    printf("      5/d < 1/2  ⟺  d > 10  ⟺  m² > 6\n");
    printf("      primeiro metal abaixo da metade   m = %ld  (d = %ld)\n", primeiro, disc(primeiro));
    printf("      o ouro                            5/5  = 1\n");
    printf("      a prata                           5/8  = 0,625  ainda acima\n");
    printf("      o bronze                          5/13 ≈ 0,385  já abaixo\n");
    ok("a queda abaixo da metade acontece no bronze, m = 3", primeiro == 3);
    printf("\n      Só o ouro e a prata ficam acima da metade. Do bronze em diante o metal é\n");
    printf("      mais fácil de cercar do que de escapar — e a diferença mede-se em inteiros.\n");
}

/* ---------------------------------------------------------------- §D5 ------ */
printf("\n§D5  E a leitura: densidade alta é ser DIFÍCIL DE CERCAR.\n\n");
{
    /* a norma é ±1 em toda a família (normal_circulo.c §N3): todos são cercados por frações
     * que chegam a uma unidade de norma. O que muda é o quão PERTO isso põe a fração, e é
     * aí que o discriminante entra: quanto maior d, mais perto a fração chega. */
    int mau = 0;
    printf("      m     norma dos convergentes   d = m²+4   a fração chega...\n");
    for(long m = 1; m <= 4; m++){
        long a = 1, b = 0, mau_n = 0;
        for(int k = 0; k < 20; k++){
            long n = a*a + m*a*b - b*b;
            if(n != 1 && n != -1) mau_n++;
            long na = b, nb = a + m*b; a = na; b = nb;
            if(b > 1000000000L) break;
        }
        if(mau_n) mau++;
        printf("      %-5ld %-24s %-10ld %s\n", m, "±1 sempre", disc(m),
               m == 1 ? "o mais longe possível" : "mais perto que o ouro");
    }
    ok("a norma é ±1 em toda a família — o que muda é a distância, não o cerco", mau == 0);
    printf("\n      Todos são cercados, e todos com norma ±1: é a família inteira no mesmo\n");
    printf("      círculo (§N3). O que a densidade mede não é SE a fração cerca — é o quão\n");
    printf("      perto ela consegue chegar, e o ouro é o que a mantém mais longe.\n");
}

printf("\n=== A DENSIDADE DE OURO ===================================================\n");
printf("  densidade(m) = 5/(m² + 4), classe racional exata, sem raiz e sem float.\n\n");
printf("      ouro     m=1    5/5   = 1        o topo, e é o topo por definição\n");
printf("      prata    m=2    5/8   = 0,625    ainda acima da metade\n");
printf("      bronze   m=3    5/13  ≈ 0,385    o primeiro abaixo\n");
printf("      m=12            5/148 ≈ 0,034\n\n");
printf("  Cai estritamente e sem exceção: a família é uma escada que só desce, e não há dois\n");
printf("  metais com a mesma densidade. Só o ouro e a prata ficam acima da metade.\n\n");
printf("  E o que ela mede: TODOS são cercados por frações de norma ±1 — a família inteira no\n");
printf("  mesmo círculo. O que a densidade diz não é se a fração cerca, mas o quão perto ela\n");
printf("  consegue chegar. O ouro é o que a mantém mais longe, e por isso é o mais de ouro.\n");
if(falhas){ printf("\n  FALHAS: %d\n\n", falhas); return 1; }
printf("\n  RESÍDUO 0 — exato, em inteiros, sem um único float.\n\n");
return 0;
}
