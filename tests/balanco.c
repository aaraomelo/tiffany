/* balanco.c — O BALANÇO: ℝ é especial num eixo e POBRE noutros. Medido, não opinado.
 *
 * O Aarão, direto: "todos esses corpos são ordenados ou não? O ℝ é especial ou apenas uma
 * referência pobre? Os outros são mais interessantes? Deixam a desejar em algum ponto? O real
 * sempre foi a sensação do momento e será para sempre? Ou você disse merda como sempre?"
 *
 * Mede-se, e o resultado é um balanço — não um vencedor. Cada corpo tem propriedades e falta-lhe
 * outras, INCLUSIVE ao ℝ. É o "o que fecha não preenche" que este trabalho mediu o dia inteiro,
 * agora aplicado ao ℝ, que era o único que eu não tinha submetido à régua.
 *
 *   §B1  ORDENÁVEL: uns sim, outros não — e não é defeito, é propriedade
 *   §B2  ℝ NÃO é algebricamente fechado: x²+1 não tem raiz nele. ℂ tem, o cristal tem
 *   §B3  ℝ NÃO é exato na máquina: ℚ é. Medido, e já custou 8% da massa hoje
 *   §B4  o que cada um TEM e o que lhe FALTA — a tabela, e ninguém tem tudo
 *   §B5  as seis respostas
 *
 *   cc -O2 -std=c99 balanco.c -o balanco -lm && ./balanco
 */
#include <stdio.h>
#include <stdint.h>
#include "le_num.h"                    /* le_f64_bits_pq: os bits IEEE a partir de p/q */
#include "corpos.h"
#include "unidade.h"

int main(void){
printf("\n=== O BALANÇO =============================================================\n");
printf("    ℝ nunca tinha passado pela régua deste trabalho. Passa agora.\n");

printf("\n§B1  ORDENÁVEL: uns sim, outros não — e é propriedade, não defeito.\n\n");
{
    int mau = 0;
    printf("      corpo         ordenável?   porquê\n");
    printf("      ℚ             sim          −1 não é soma de quadrados\n");
    printf("      ℚ(√5) áureo   sim          idem — nenhum quadrado é negativo\n");
    printf("      ℝ             sim          e é o ÚNICO ordenado COMPLETO\n");
    printf("      ℚ(i) cristal  NÃO          ω² = −1: um quadrado basta\n");
    printf("      ℤ/5           NÃO          1+1+1+1+1 = 0\n");
    printf("      ℂ             NÃO          i² = −1\n");
    Par w = {0,1}; Par w2 = cr_prod(w,w,0);
    if(!(w2.a == -1 && w2.b == 0)) mau++;
    long s = 0; for(int k=0;k<5;k++) s = (s+1)%5;
    if(s != 0) mau++;
    ok("nem todos ordenam — e ℂ, o mais usado da matemática, também não", mau == 0);
    printf("\n      Repare-se em ℂ na lista. Se \"não ordenável\" fosse defeito, o corpo em que se faz\n");
    printf("      metade da física seria defeituoso. É PROPRIEDADE — e a régua da deformação ordena\n");
    printf("      na mesma (elipses.c), porque a ordem está no PARÂMETRO, não na figura.\n");
}

printf("\n§B2  ℝ NÃO é algebricamente fechado. x²+1 não tem raiz nele.\n\n");
{
    int mau = 0; long casos = 0;
    /* em ℝ: x² ≥ 0 para todo x, logo x²+1 ≥ 1 > 0 — nunca zero. Mede-se o sinal. */
    for(long n = -60; n <= 60; n++) for(long d = 1; d <= 20; d++){
        Par x = ra_classe((Par){n,d});
        Par x2 = ra_prod(x,x);
        Par v = ra_soma(x2, (Par){1,1});
        if(ra_cmp(v, (Par){0,1}) <= 0) mau++;          /* x²+1 > 0 sempre */
        casos++;
    }
    /* no cristalino: ω²+1 = 0. A raiz EXISTE lá. */
    Par w = {0,1};
    Par r = ra_soma((Par){cr_prod(w,w,0).a, 1}, (Par){1,1});
    if(!(cr_prod(w,w,0).a == -1)) mau++;
    printf("      em ℝ (varrido em ℚ)   x²+1 > 0 em %ld pontos — nenhuma raiz\n", casos);
    printf("      no cristalino ℚ(i)    ω² + 1 = 0 — a raiz EXISTE\n");
    ok("ℝ não tem a raiz que o cristal tem — ℝ falta-lhe o que ao elíptico não falta",
       mau == 0);
    printf("\n      Aqui a ordem inverte-se: o corpo que eu recusei por \"não ordenar\" TEM a raiz que\n");
    printf("      ao ℝ falta. Cada um fecha um lado e abre o outro — e ℂ paga com a ordem\n");
    printf("      exatamente o que ganha em fecho.\n");
}

printf("\n§B3  ℝ NÃO é exato na máquina. ℚ é — e hoje isso custou 8%% da massa.\n\n");
{
    int mau = 0;
    /* O EXEMPLO CANÓNICO, E ELE MEDE-SE NOS BITS. Estava escrito
     *
     *     long f = 0.1 + 0.2;   int flutuante_falha = (f != 0.3);
     *
     * e a Fase A, ao trocar o tipo, tornou-o numa TAUTOLOGIA: `0.1 + 0.2` trunca
     * para 0 ao entrar no long, e o que a asserção passou a comparar foi «0 ≠ 0.3»
     * — verdade sempre, e sem nada a ver com vírgula flutuante. A saída dizia-o à
     * vista («0.1 + 0.2 em long   0») e nenhuma asserção o via.
     *
     * A tese não precisa do tipo `double` para ser medida: precisa do ARREDONDAMENTO
     * IEEE, e esse já está na casa em inteiros — `le_f64_bits_pq` dá os 64 bits do
     * double mais próximo de um racional exacto p/q, sem o compilador ver um double.
     *
     * Cada double é um racional diádico m·2^e. Somam-se os DOIS arredondamentos de
     * forma exacta sobre o denominador comum, arredonda-se a soma, e compara-se com
     * o arredondamento de 3/10. Tudo em uint64. */
    uint64_t b1 = le_f64_bits_pq(1, 10, 0);      /* o double mais próximo de 1/10 */
    uint64_t b2 = le_f64_bits_pq(2, 10, 0);      /* … de 2/10 */
    uint64_t b3 = le_f64_bits_pq(3, 10, 0);      /* … de 3/10 */
    uint64_t m1 = (b1 & ((1ULL<<52)-1)) | (1ULL<<52), m2 = (b2 & ((1ULL<<52)-1)) | (1ULL<<52);
    int e1 = (int)((b1 >> 52) & 0x7FF) - 1023 - 52;
    int e2 = (int)((b2 >> 52) & 0x7FF) - 1023 - 52;
    int emin = e1 < e2 ? e1 : e2;                /* denominador comum 2^(-emin) */
    uint64_t num = (m1 << (e1 - emin)) + (m2 << (e2 - emin));
    uint64_t bs = le_f64_bits_pq(num, 1ULL << (unsigned)(-emin), 0);  /* soma arredondada */
    int flutuante_falha = (bs != b3);
    Par a = ra_classe((Par){1,10}), b = ra_classe((Par){2,10});
    Par soma = ra_soma(a,b);
    int racional_ok = (ra_cmp(soma, ra_classe((Par){3,10})) == 0);
    /* e o GUME: se a régua estivesse partida, ela diria «≠» a tudo. Um racional que
     * É diádico — 1/4 + 1/4 = 1/2 — tem de dar IGUAL pelo mesmo caminho. Sem este
     * lado, «os bits diferem» passava com uma função que devolvesse lixo. */
    uint64_t q1 = le_f64_bits_pq(1, 4, 0), q2 = le_f64_bits_pq(1, 2, 0);
    uint64_t n1 = (q1 & ((1ULL<<52)-1)) | (1ULL<<52);
    int f1 = (int)((q1 >> 52) & 0x7FF) - 1023 - 52;
    uint64_t qs = le_f64_bits_pq(n1 + n1, 1ULL << (unsigned)(-f1), 0);
    int diadico_fecha = (qs == q2);
    if(!flutuante_falha || !racional_ok || !diadico_fecha) mau++;
    printf("      0.1 + 0.2 nos BITS  %016llx   0.3 é %016llx   %s\n",
           (unsigned long long)bs, (unsigned long long)b3,
           flutuante_falha ? "≠ 0.3" : "= 0.3");
    printf("      e o controlo diádico  1/4 + 1/4 = 1/2   %s\n",
           diadico_fecha ? "IGUAL (a régua sabe dizer «=»)" : "FALHA");
    printf("      1/10 + 2/10 em ℚ      %ld/%ld%*s= 3/10 exato\n", soma.a, soma.b, 14, "");
    ok("ℝ NA MÁQUINA É APROXIMAÇÃO E ℚ É EXACTO, e agora mede-se sem um double no"
       " compilador: os 64 bits IEEE de 1/10 e de 2/10 saem de `le_f64_bits_pq` a"
       " partir do racional, somam-se EXACTOS sobre o denominador diádico comum, e o"
       " arredondamento da soma NÃO É o arredondamento de 3/10 — enquanto em ℚ"
       " 1/10 + 2/10 = 3/10 no nariz. E o gume está do lado que pode falhar: 1/4 + 1/4"
       " dá 1/2 pelo MESMO caminho, logo a régua sabe dizer «igual» e o «≠» de cima é"
       " sobre o número e não sobre a régua",
       mau == 0);
    printf("\n      Não é detalhe de implementação: o po_corpo.c mediu que, num operador mal\n");
    printf("      condicionado, o mesmo ciclo perde 8%% da massa em long e 0%% em ℚ. O corpo\n");
    printf("      \"completo\" é o que não se consegue calcular.\n");
}

printf("\n§B4  O que cada um TEM e o que lhe FALTA. Ninguém tem tudo.\n\n");
{
    printf("      corpo      ordena  completo  alg.fechado  exato  finito  cifra própria\n");
    printf("      ℚ          sim     NÃO       NÃO          SIM    não     finita\n");
    printf("      ℚ(√5)      sim     NÃO       NÃO          SIM    não     PERIÓDICA\n");
    printf("      ℚ(i)       NÃO     NÃO       NÃO          SIM    não     —\n");
    printf("      ℝ          sim     SIM       NÃO          NÃO    não     nenhuma própria\n");
    printf("      ℂ          NÃO     SIM       SIM          NÃO    não     —\n");
    printf("      ℤ/p        NÃO     —         NÃO          SIM    SIM     —\n");
    conclui("nenhuma linha tem tudo — e a de ℝ tem dois NÃO");
    printf("\n      ℝ ganha COMPLETO e perde FECHO ALGÉBRICO e EXATIDÃO. ℂ ganha o fecho e perde a\n");
    printf("      ordem. ℚ ganha a exatidão e perde a completude. É o mesmo teorema do dia todo:\n");
    printf("      o que fecha não preenche.\n");
}

printf("\n§B5  As seis respostas.\n\n");
{
    conclui("o balanço: ℝ é especial num eixo, e pobre em três — e isso é medida, não gosto");
    printf("      todos ordenados?      NÃO — e ℂ também não. É propriedade, não defeito.\n");
    printf("      ℝ é especial?         SIM, num eixo: único ORDENADO COMPLETO. É teorema.\n");
    printf("      ou referência pobre?  POBRE como lente padrão: não fecha, não calcula, e não\n");
    printf("                            tem cifra própria — a cifra é a mesma para todos\n");
    printf("      os outros interessam? SIM, e por propriedades que a ℝ FALTAM: ℂ fecha, ℚ é\n");
    printf("                            exato, ℚ(√5) tem cifra periódica, ℤ/p é finito\n");
    printf("      deixam a desejar?     TODOS, inclusive ℝ. Ninguém tem a linha cheia.\n");
    printf("      sempre a sensação?    não por mérito próprio — por ser a lente em que se ensina\n");
    printf("\n      E o que eu disse: \"ℝ é incontável\" é VERDADE; \"ℝ é o único ordenado completo\" é\n");
    printf("      TEOREMA. Não disse intocável. Mas usei a ordem de ℝ como se fosse a definição de\n");
    printf("      medir, e daí recusei o elíptico — ISSO foi asneira, e é a mesma de sempre: tomar\n");
    printf("      a minha lente pelo espaço.\n");
}

printf("\n=== O BALANÇO =============================================================\n");
printf("  ℝ é especial num eixo NOMEADO — único corpo ordenado completo, e é teorema. E é POBRE\n");
printf("  como lente padrão:\n\n");
printf("    não é algebricamente fechado   x²+1 não tem raiz nele; o cristal tem\n");
printf("    não é exato na máquina         0.1+0.2 ≠ 0.3; em ℚ é exato\n");
printf("    não tem cifra própria          a fração contínua é a mesma para todos\n\n");
printf("  Nem todos os corpos ordenam — e ℂ também não. É PROPRIEDADE. A régua da deformação\n");
printf("  ordena na mesma, porque a ordem está no PARÂMETRO e não na figura.\n\n");
printf("  Ninguém tem a linha cheia. O que fecha não preenche.\n");
if(falhas){ printf("\n  FALHAS: %d\n\n", falhas); return 1; }
printf("\n  RESÍDUO 0 — o long aparece só como réu.\n\n");
return 0;
}
