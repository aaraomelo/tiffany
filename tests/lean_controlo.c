/* lean_controlo.c — O CONTROLO E A FRONTEIRA: os dois mecanismos das Obrigações, medidos.
 *
 * O paper conecthus/fundamento.tex deriva duas obrigações para a próxima fase do LMS e este
 * medidor mede o MECANISMO de cada uma, em miniatura — não os números do LMS, que são deles:
 *
 *   §N1  O CONTROLO: gerar o mesmo objeto AO ACASO com a mesma magnitude e medir. Se o acaso
 *        EMPATA, a métrica é degenerada — o ganho veio do grau de liberdade, não da escolha.
 *        Aqui: uma métrica que ignora a escolha (o número de ações do plano — um KPI real,
 *        constante por construção) empata com o acaso SEMPRE; a informativa (o tempo poupado,
 *        somado) discorda. O empate é o sinal, e distingue as duas réguas sem um limiar —
 *        sem precisar de ler o código de nenhuma delas.
 *
 *   §N2  A FRONTEIRA: dois artefactos internamente válidos podem discordar entre si, e só o
 *        resíduo NA FRONTEIRA o vê. Aqui: o ranking do GUT e a ordem das ações do 5W2H —
 *        trocas injetadas que nenhuma validação interna apanha (cada artefacto continua bem
 *        formado), e as inversões POR VALOR entre os dois apanham exatamente as injetadas.
 *
 * Tudo em inteiros, gerador determinista — a mesma semente dá a mesma conta, sempre.
 *
 *   cc -O2 -std=c99 -I../lib lean_controlo.c -o lean_controlo && ./lean_controlo
 */
#include <stdio.h>
#include "unidade.h"

static unsigned long ger = 20260810;
static long dado(long topo){ ger = ger*6364136223846793005UL + 1442695040888963407UL;
                             return (long)((ger >> 33) % (unsigned long)topo); }

#define NEST 12   /* estações na mesa do §N1        */
#define NPLA 3    /* ações que um plano escolhe     */
#define NCAU 5    /* causas nos artefactos do §N2   */

int main(void){
printf("\n=== O CONTROLO E A FRONTEIRA ==============================================\n");
printf("    Os dois mecanismos das Obrigações do paper, medidos em miniatura.\n");

printf("\n§N1  O CONTROLO: o acaso empata ⟺ a métrica é degenerada.\n\n");
{
    long mesas = 0, degen_empata_sempre = 0, info_acaso_discorda = 0, modelo_perde = 0;
    long sorteios_total = 0;
    for(long mesa = 0; mesa < 200; mesa++){
        long w[NEST];
        for(int i = 0; i < NEST; i++) w[i] = 1 + dado(99);

        /* o plano do MODELO: as NPLA estações de maior desperdício (seleção, em inteiros) */
        int usado[NEST] = {0}; long ganho_modelo = 0;
        for(int k = 0; k < NPLA; k++){
            int melhor = -1;
            for(int i = 0; i < NEST; i++)
                if(!usado[i] && (melhor < 0 || w[i] > w[melhor])) melhor = i;
            usado[melhor] = 1; ganho_modelo += w[melhor];
        }

        /* 40 planos AO ACASO, mesma magnitude: NPLA estações distintas */
        long empates_degen = 0, discorda_info = 0, acima = 0;
        for(int s = 0; s < 40; s++){
            int a = (int)dado(NEST), b, c;
            do { b = (int)dado(NEST); } while(b == a);
            do { c = (int)dado(NEST); } while(c == a || c == b);
            long ganho_acaso = w[a] + w[b] + w[c];

            /* a métrica DEGENERADA: o número de ações do plano — um KPI real («N
             * contramedidas implementadas») e CEGO à escolha: constante por construção.
             * O controlo existe para tornar isso visível SEM ler o código da métrica */
            long degen_modelo = NPLA;
            long degen_acaso  = NPLA;
            (void)ganho_modelo;
            if(degen_modelo == degen_acaso) empates_degen++;

            /* a métrica INFORMATIVA: o tempo poupado, somado — depende da escolha */
            if(ganho_acaso != ganho_modelo) discorda_info++;
            if(ganho_acaso >  ganho_modelo) modelo_perde++;
            acima += (ganho_acaso < ganho_modelo);
            sorteios_total++;
        }
        if(empates_degen == 40) degen_empata_sempre++;
        if(discorda_info  >  0) info_acaso_discorda++;
        mesas++;
    }
    ok("sob a métrica degenerada o acaso EMPATA nos 40 sorteios, em todas as mesas",
       degen_empata_sempre == mesas);
    ok("sob a informativa o acaso DISCORDA em todas as mesas — a régua separa as duas",
       info_acaso_discorda == mesas);
    ok("e o plano do modelo não perde para sorteio nenhum — o topo-3 é o máximo da soma",
       modelo_perde == 0);
    printf("      (%ld mesas × 40 sorteios = %ld controlos. O empate universal é o SINAL:\n",
           mesas, sorteios_total);
    printf("      a régua que o acaso empata não mediu a escolha — mediu-se a si própria.)\n");
}

printf("\n§N2  A FRONTEIRA: o que nenhuma validação interna vê, o resíduo entre DOIS vê.\n\n");
{
    long casos = 0, injetados = 0, interna_apanha = 0, fronteira_apanha = 0, falsos = 0;
    for(long caso = 0; caso < 300; caso++){
        /* o artefacto GUT: NCAU causas, score = G·U·T — e o ranking por score */
        long score[NCAU];
        for(int i = 0; i < NCAU; i++)
            score[i] = (1 + dado(5)) * (1 + dado(5)) * (1 + dado(5));
        int rank[NCAU]; int posto[NCAU] = {0};
        for(int k = 0; k < NCAU; k++){
            int melhor = -1;
            for(int i = 0; i < NCAU; i++)
                if(!posto[i] && (melhor < 0 || score[i] > score[melhor])) melhor = i;
            posto[melhor] = 1; rank[k] = melhor;
        }

        /* o artefacto 5W2H: as ações na ordem do ranking — o par consistente */
        int acao[NCAU];
        for(int k = 0; k < NCAU; k++) acao[k] = rank[k];

        /* a injeção: numa fração dos casos, troca-se um par ADJACENTE de scores DISTINTOS
         * (trocar iguais é mutação equivalente — a ordem entre empates é livre) */
        int errado = 0;
        if(dado(3) == 0){
            for(int p = 0; p < NCAU-1; p++)
                if(score[acao[p]] != score[acao[p+1]]){
                    int t = acao[p]; acao[p] = acao[p+1]; acao[p+1] = t;
                    errado = 1; injetados++;
                    break;
                }
        }

        /* a validação INTERNA de cada artefacto — e ela continua verde:
         * o GUT tem os scores re-deriváveis, o 5W2H é uma permutação completa das causas */
        int interna = 0;
        { int visto[NCAU] = {0}; int soma_ok = 1;
          for(int k = 0; k < NCAU; k++){
              if(acao[k] < 0 || acao[k] >= NCAU || visto[acao[k]]) soma_ok = 0;
              else visto[acao[k]] = 1;
          }
          if(!soma_ok) interna = 1;
        }
        if(interna && errado) interna_apanha++;

        /* o resíduo NA FRONTEIRA: inversões POR VALOR na ordem das ações — a prioridade
         * do GUT tem de reaparecer no 5W2H, e cada quebra é uma inversão contável */
        long inversoes = 0;
        for(int p = 0; p < NCAU-1; p++)
            if(score[acao[p]] < score[acao[p+1]]) inversoes++;
        if(inversoes > 0 &&  errado) fronteira_apanha++;
        if(inversoes > 0 && !errado) falsos++;
        casos++;
    }
    ok("a injeção é não-degenerada: há trocas, e não em todos os casos",
       injetados > 0 && injetados < casos);
    ok("a validação INTERNA não apanha nenhuma — os dois artefactos continuam bem formados",
       interna_apanha == 0);
    ok("o resíduo na FRONTEIRA apanha todas as injetadas — dois caminhos têm de concordar",
       fronteira_apanha == injetados);
    ok("e não acusa um falso — no par consistente as inversões são zero", falsos == 0);
    printf("      (%ld casos, %ld trocas injetadas: por dentro 0, na fronteira %ld, falsos 0.\n",
           casos, injetados, fronteira_apanha);
    printf("      É a Obrigação 4: a consistência não se valida DENTRO de um artefacto —\n");
    printf("      mede-se ENTRE os dois, e cada quebra é uma inversão que se conta.)\n");
}

printf("\n=== OS DOIS MECANISMOS FECHAM =============================================\n");
printf("  O controlo: o empate do acaso denuncia a régua degenerada, sem um limiar.\n");
printf("  A fronteira: o resíduo entre dois artefactos apanha o que nenhum vê sozinho.\n");
if(falhas){ printf("\n  FALHAS: %d\n\n", falhas); return 1; }
printf("\n  RESÍDUO 0 — exato, em inteiros.\n\n");
return 0;
}
