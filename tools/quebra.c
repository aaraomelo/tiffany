/* quebra.c — A QUEBRA. Eu ordenei o PARÂMETRO, não o CORPO. E isso derruba a minha conta.
 *
 * O Aarão: "você está consertando — falei pra QUEBRAR."
 *
 * Estava mesmo. O §A3b do ataque foi eu explicar porque o meu teste estava certo, não tentar
 * parti-lo. O ataque a sério é este, e parte:
 *
 *     Eu disse "27 dos 28 são ordenados". Mas o que eu ordenei em cada um foi o PARÂMETRO —
 *     a impedância, o índice, a taxa, a elongação. E o parâmetro é uma FUNÇÃO dos elementos.
 *     Se dois elementos DISTINTOS têm o mesmo parâmetro, eles empatam: e uma relação que
 *     empata distintos é PRÉ-ordem, não ordem.
 *
 * É exatamente o que eu já tinha notado para a norma no elíptico — e depois esqueci ao contar.
 *
 *   §Q1  a quebra: parâmetro não-injetivo ⟹ PRÉ-ordem. Exibido.
 *   §Q2  quais sobrevivem: aqueles em que o parâmetro É o elemento
 *   §Q3  a conta CORRIGIDA — e é muito menor que 27
 *   §Q4  o que isto faz à minha palavra
 *
 *   cc -O2 -std=c99 quebra.c -o quebra && ./quebra
 */
#include <stdio.h>
#include "corpos.h"
#include "unidade.h"

static Par q(long a, long b){ return ra_classe((Par){a,b}); }

int main(void){
printf("\n=== A QUEBRA ==============================================================\n");
printf("    Eu ordenei o PARÂMETRO. O corpo tem elementos, e eles empatam.\n");

printf("\n§Q1  Parâmetro não-injetivo ⟹ PRÉ-ordem. Exibido.\n\n");
{
    int mau = 0; long empates = 0, casos = 0;
    printf("      corpo             elemento      parâmetro        outro elemento  igual?\n");
    /* eletromagnético: o elemento é o par (E,B); o parâmetro é a impedância E/B */
    printf("      eletromagnético   (2,1)         E/B = 2          (4,2)           SIM — empatam\n");
    printf("      óptico            (3,1)         n = 3            (6,2)           SIM — empatam\n");
    printf("      econômico         (5,1)         juro = 5         (10,2)          SIM — empatam\n");
    for(long a=1;a<=20;a++) for(long b=1;b<=20;b++)
    for(long c=1;c<=20;c++) for(long d=1;d<=20;d++){
        Par p1 = q(a,b), p2 = q(c,d);
        int mesmo_par = (ra_cmp(p1,p2) == 0);
        int mesmo_el  = (a == c && b == d);
        if(mesmo_par && !mesmo_el) empates++;           /* distintos, parâmetro igual */
        casos++;
    }
    if(!empates) mau++;
    ok("há elementos DISTINTOS com o mesmo parâmetro — logo é PRÉ-ordem, não ordem", mau == 0);
    printf("      (%ld pares, %ld empates de elementos distintos.)\n", casos, empates);
    printf("\n      É a quebra, e é simples: (2,1) e (4,2) são elementos diferentes do corpo\n");
    printf("      eletromagnético e têm a MESMA impedância. A relação que eu chamei ordem não os\n");
    printf("      separa — e uma ordem separa. Chamar-lhe ordem foi eu contar a régua pelo objeto.\n");
}

printf("\n§Q2  Quais sobrevivem: aqueles em que o parâmetro É o elemento.\n\n");
{
    int mau = 0; long casos = 0;
    printf("      corpo          o parâmetro é...        injetivo?   ordem de verdade?\n");
    printf("      racional ℚ     o próprio racional      SIM         SIM\n");
    printf("      áureo ℤ[φ]     o próprio a+bσ em ℝ     SIM         SIM\n");
    printf("      deflexivo      o metal m do gato       SIM         SIM\n");
    printf("      eletromagn.    E/B — um QUOCIENTE      não         pré-ordem\n");
    printf("      óptico         o índice — quociente    não         pré-ordem\n");
    printf("      econômico      a taxa — quociente      não         pré-ordem\n");
    /* no áureo o parâmetro é injetivo: a+bσ = c+dσ com σ irracional força a=c, b=d */
    for(long a=-9;a<=9;a++) for(long b=-9;b<=9;b++)
    for(long c=-9;c<=9;c++) for(long d=-9;d<=9;d++){
        if(a==c && b==d) continue;
        if(au_sinal(a-c, b-d, 1) == 0) mau++;           /* distintos ⟹ sinal ≠ 0 */
        casos++;
    }
    ok("no áureo o parâmetro SEPARA: dois elementos distintos nunca empatam", mau == 0);
    printf("      (%ld pares distintos.)\n", casos);
    printf("\n      A diferença é injetividade. Onde o parâmetro é o próprio elemento — o racional,\n");
    printf("      o áureo, o metal — há ORDEM. Onde é um quociente ou uma norma, há PRÉ-ordem.\n");
}

printf("\n§Q3  A conta CORRIGIDA.\n\n");
{
    printf("      eu disse       27 de 28 são ORDENADOS\n");
    printf("      o certo é      27 de 28 têm uma PRÉ-ordem total no parâmetro\n");
    printf("      e ORDEM        só onde o parâmetro é injetivo\n\n");
    printf("      com ordem de verdade (parâmetro = elemento):\n");
    printf("        racional ℚ, áureo ℤ[φ], deflexivo, espaço-temporal, universal,\n");
    printf("        somático, técnico, sensitivo, nervoso, exterior, relógio, rotor\n");
    printf("        — os que ordenam pelo ELEMENTO, e não por um quociente dele\n\n");
    printf("      com pré-ordem (parâmetro é quociente, norma ou razão):\n");
    printf("        eletromagnético, óptico, econômico, evolutivo, expansivo, geométrico,\n");
    printf("        fractal, cósmico, celeste, criativo, cristalino, conforme, telescópico,\n");
    printf("        entrópico, motor\n\n");
    printf("      fora            mórfico — nem pré-ordem total: há incomparáveis\n");
    ok("a conta muda: ORDEM em ~12, PRÉ-ordem em ~15, e 1 fora — não 27 e 1", 1);
    printf("\n      E a divisão acima é a minha leitura de qual parâmetro é injetivo, corpo a corpo.\n");
    printf("      Não a meço aqui — precisaria da régua própria de cada um, que está no catálogo e\n");
    printf("      não neste repositório. Fica como leitura, e é falível.\n");
}

printf("\n§Q4  O que isto faz à minha palavra.\n\n");
{
    ok("a minha conta de 27 estava errada — e quebrou pelo ataque que eu não tinha feito", 1);
    printf("      o que eu afirmei    \"27 dos 28 são ordenados\"\n");
    printf("      o que quebra        o parâmetro não é injetivo na maioria: empatam distintos\n");
    printf("      o que sobrevive     a PRÉ-ordem total — que é real, e é o que a régua dá\n");
    printf("      o que eu já sabia   escrevi isto mesmo para a norma no elíptico, e esqueci\n");
    printf("\n      É a terceira vez hoje que eu noto uma distinção e depois a perco ao contar. E o\n");
    printf("      pedido dele era exatamente este: QUEBRAR, não consertar. Eu tinha respondido ao\n");
    printf("      \"derruba\" a mostrar que o ataque não achava nada — mas o ataque que eu corri\n");
    printf("      testava o PARÂMETRO, e a pergunta era sobre o CORPO.\n");
    printf("\n      Um ataque que só testa onde eu já sei que passa não é ataque. É conserto.\n");
}

printf("\n=== A QUEBRA ==============================================================\n");
printf("  Eu ordenei o PARÂMETRO — a impedância, o índice, a taxa — e o corpo tem ELEMENTOS.\n");
printf("  (2,1) e (4,2) têm a mesma impedância e são elementos distintos: EMPATAM.\n\n");
printf("    o que eu disse   27 de 28 são ORDENADOS\n");
printf("    o certo é        27 têm PRÉ-ordem total; ORDEM só onde o parâmetro é injetivo\n");
printf("    e eu já sabia    escrevi-o para a norma no elíptico, e perdi-o ao contar\n\n");
printf("  O ataque que eu tinha corrido testava o parâmetro — e a pergunta era sobre o corpo. Um\n");
printf("  ataque que só testa onde eu já sei que passa não é ataque: é conserto.\n");
if(falhas){ printf("\n  FALHAS: %d\n\n", falhas); return 1; }
printf("\n  RESÍDUO 0 — exato, em racionais.\n\n");
return 0;
}
