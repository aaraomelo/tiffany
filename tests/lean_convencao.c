/* lean_convencao.c — CONVENÇÃO OU LEI? A mutação do número decide, não o gosto.
 *
 * O paper conecthus/fundamento.tex dizia «não afirmo que o 6M seja a hexal nem o 5 a pental» —
 * e não afirmar é barato: pode-se não afirmar qualquer coisa. O certo é MEDIR e afirmar o que
 * a medição disser. O teste é um só, e é o da teoria: muta-se o número; se nada do que desce
 * muda, o número é LIVRE (convenção); se algo contável quebra, o número COBRA (estrutura).
 *
 *   §V1  o 6 do Ishikawa é LIVRE: a escolha da causa raiz é por CAUSA, e re-agrupar as
 *        categorias (2..12 grupos) não a move — resíduo 0 em todas as partições. E o
 *        contraste: um seletor HIERÁRQUICO (grupo primeiro, causa depois) move com o
 *        agrupamento — logo o que protege o fluxo é escolher por causa, não o seis.
 *   §V2  o 3 do Kanban COBRA: fundir To Do com In Progress colide histórias que os três
 *        estados separam — e colide EXATAMENTE as que diferem só no início, contadas por
 *        dois caminhos que têm de concordar.
 *   §V3  o 5 dos Porquês é TETO, e a lei é o PONTO FIXO: a raiz chega no passo d (não no 5),
 *        perguntar além dela é idempotente (resíduo 0), e o 5 nem basta (d>5 escapa) nem é
 *        preciso (d<5 sobra).
 *
 *   cc -O2 -std=c99 -I../lib lean_convencao.c -o lean_convencao && ./lean_convencao
 */
#include <stdio.h>
#include "unidade.h"

static unsigned long ger = 8102026;
static long dado(long topo){ ger = ger*6364136223846793005UL + 1442695040888963407UL;
                             return (long)((ger >> 33) % (unsigned long)topo); }

#define NCAU 30   /* causas na mesa do §V1  */
#define TFIM 20   /* horizonte do §V2       */

int main(void){
printf("\n=== CONVENÇÃO OU LEI? A MUTAÇÃO DECIDE ====================================\n");
printf("    Muta-se o número: se nada desce diferente, é livre; se algo quebra, cobra.\n");

printf("\n§V1  O 6 do Ishikawa é LIVRE — a escolha por causa não vê o agrupamento.\n\n");
{
    long mesas = 0, direta_move = 0, hier_move_nalguma = 0;
    for(long mesa = 0; mesa < 40; mesa++){
        long f[NCAU];
        for(int i = 0; i < NCAU; i++) f[i] = 1 + dado(99);

        /* a escolha DIRETA: a causa de maior frequência — é o que o fluxo do LMS faz */
        int raiz_direta = 0;
        for(int i = 1; i < NCAU; i++) if(f[i] > f[raiz_direta]) raiz_direta = i;

        int primeira_hier = -1, hier_variou = 0;
        for(long t = 0; t < 20; t++){
            /* a mutação do catálogo: re-agrupar as causas em k grupos, k de 2 a 12 —
             * o «6» vira 2, 7, 12... e o fluxo tem de não notar */
            long k = 2 + dado(11);
            long g[NCAU];
            for(int i = 0; i < NCAU; i++) g[i] = dado(k);

            /* a direta, recomputada COM o agrupamento à vista: não o lê, não move */
            int r2 = 0;
            for(int i = 1; i < NCAU; i++) if(f[i] > f[r2]) r2 = i;
            if(r2 != raiz_direta) direta_move++;

            /* o contraste: o seletor HIERÁRQUICO (grupo de maior soma, causa dentro dele)
             * lê o agrupamento — e move com ele */
            long soma_g[12] = {0};
            for(int i = 0; i < NCAU; i++) soma_g[g[i]] += f[i];
            long gmax = 0;
            for(long j = 1; j < k; j++) if(soma_g[j] > soma_g[gmax]) gmax = j;
            int rh = -1;
            for(int i = 0; i < NCAU; i++)
                if(g[i] == gmax && (rh < 0 || f[i] > f[rh])) rh = i;
            if(primeira_hier < 0) primeira_hier = rh;
            else if(rh != primeira_hier) hier_variou = 1;
        }
        if(hier_variou) hier_move_nalguma++;
        mesas++;
    }
    ok("a escolha por CAUSA não se move em partição nenhuma — resíduo 0, o 6 é livre",
       direta_move == 0);
    ok("e o contraste mede: a escolha HIERÁRQUICA move com o agrupamento nalguma mesa",
       hier_move_nalguma > 0);
    printf("      (%ld mesas × 20 partições de 2 a 12 grupos: a direta 0 movimentos, a\n", mesas);
    printf("      hierárquica move em %ld mesas. O seis não protege nada: quem protege é\n",
           hier_move_nalguma);
    printf("      escolher por causa — e aí QUALQUER k serve, que é ser convenção.)\n");
}

printf("\n§V2  O 3 do Kanban COBRA — fundir dois estados colide o que os três separam.\n\n");
{
    long pares = 0, colide3 = 0, colide2 = 0, so_no_inicio = 0;
    for(long p = 0; p < 300; p++){
        /* duas histórias (início, fim), com início ≤ fim no horizonte */
        long s1 = dado(TFIM), f1 = s1 + dado(TFIM - s1 + 1);
        long s2 = dado(TFIM), f2 = s2 + dado(TFIM - s2 + 1);
        if(s1 == s2 && f1 == f2) continue;          /* histórias iguais não medem separação */

        /* as trajetórias, instante a instante: 3 estados (todo/doing/done) e 2 (done?) */
        int igual3 = 1, igual2 = 1;
        for(long t = 0; t <= TFIM; t++){
            int e1 = (t < s1) ? 0 : (t < f1) ? 1 : 2;
            int e2 = (t < s2) ? 0 : (t < f2) ? 1 : 2;
            if(e1 != e2) igual3 = 0;
            if((e1 == 2) != (e2 == 2)) igual2 = 0;
        }
        if(igual3) colide3++;
        if(igual2) colide2++;
        /* o segundo caminho: a forma fechada — colidem no 2-estados exatamente as que
         * diferem SÓ no início */
        if(f1 == f2 && s1 != s2) so_no_inicio++;
        pares++;
    }
    ok("com os TRÊS estados, histórias distintas nunca colidem — o trânsito tem endereço",
       colide3 == 0);
    ok("fundido To Do com In Progress, há colisões — o que se perde é contável", colide2 > 0);
    ok("e os dois caminhos concordam: colidem EXATAMENTE as que diferem só no início",
       colide2 == so_no_inicio);
    printf("      (%ld pares de histórias distintas: 0 colisões com 3 estados, %ld com 2 —\n",
           pares, colide2);
    printf("      e %ld é também a conta fechada. O terceiro estado é o que lê o ATRAVESSAR;\n",
           so_no_inicio);
    printf("      sem ele o começado-e-não-acabado não tem onde morar. O 3 cobra.)\n");
}

printf("\n§V3  O 5 dos Porquês é TETO — a lei é o PONTO FIXO, e ela é idempotente.\n\n");
{
    long cadeias = 0, raiz_fora_do_passo_d = 0, nao_idempotente = 0;
    long escapa_ao_5 = 0, sobra_do_5 = 0;
    for(long d = 1; d <= 8; d++){
        /* a cadeia causal: 0 é o sintoma, parent(i) = i+1, e a raiz d é ponto fixo */
        long no = 0, passos = 0;
        while(passos < 100){
            long prox = (no < d) ? no + 1 : no;    /* parent */
            if(prox == no) break;
            no = prox; passos++;
        }
        if(passos != d || no != d) raiz_fora_do_passo_d++;

        /* perguntar ALÉM da raiz: k porquês extra não movem nada — a erosão é idempotente */
        for(long k = 1; k <= 5; k++){
            long m = no;
            for(long j = 0; j < k; j++) m = (m < d) ? m + 1 : m;
            if(m != no) nao_idempotente++;
        }

        /* e o 5: nem basta, nem é preciso */
        long no5 = 0;
        for(long j = 0; j < 5; j++) no5 = (no5 < d) ? no5 + 1 : no5;
        if(d > 5 && no5 != d) escapa_ao_5++;       /* parou antes da raiz     */
        if(d < 5 && no5 == d) sobra_do_5++;        /* a raiz chegou antes do 5 */
        cadeias++;
    }
    ok("a raiz chega EXATAMENTE no passo d, em todas as profundidades 1..8",
       raiz_fora_do_passo_d == 0);
    ok("e perguntar além dela é idempotente — resíduo 0 em todos os porquês extra",
       nao_idempotente == 0);
    ok("o 5 nem basta (d>5 escapa) nem é preciso (d<5 sobra) — há dos dois",
       escapa_ao_5 > 0 && sobra_do_5 > 0);
    printf("      (%ld cadeias: raiz no passo d em todas; %ld escapam ao teto de 5, %ld\n",
           cadeias, escapa_ao_5, sobra_do_5);
    printf("      chegam antes. A lei é «até ao ponto fixo» — o 5 é um teto de bolso.)\n");
}

printf("\n=== A MUTAÇÃO DECIDIU =====================================================\n");
printf("  O 6 do Ishikawa é LIVRE (resíduo 0 sob re-partição — convenção).\n");
printf("  O 3 do Kanban COBRA (fundir estados colide histórias — estrutura, o trial).\n");
printf("  O 5 dos Porquês é TETO (a lei é o ponto fixo idempotente — a erosão).\n");
if(falhas){ printf("\n  FALHAS: %d\n\n", falhas); return 1; }
printf("\n  RESÍDUO 0 — exato, em inteiros.\n\n");
return 0;
}
