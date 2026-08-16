/* tests/refuta.c — AS IDENTIDADES DA CASA PASSAM PELO REFUTADOR, e as mutações caem.
 *
 * O `reducao.h` deu à casa um refutador exaustivo. Este ficheiro põe-no a trabalhar sobre
 * as identidades que a casa AFIRMA — o centro, a membrana, Cayley--Hamilton, Lagrange, a
 * multiplicatividade do determinante — e, sobretudo, sobre as MUTAÇÕES delas.
 *
 * Um refutador que nunca acha nada não distingue «não há defeito» de «não sei procurar».
 * Por isso cada identidade verdadeira vem com a sua versão corrompida — um sinal trocado,
 * um termo em falta, um factor a mais — e a corrompida TEM de cair. É o gume automático
 * desta casa aplicado às suas próprias afirmações.
 *
 * §F0  as identidades VERDADEIRAS: o refutador tem de voltar vazio em todas
 * §F1  as MUTAÇÕES: cada uma tem de cair, e diz-se em que primo e com que valores
 * §F2  por que VÁRIOS primos: uma falsa que passa em 𝔽₂ e cai nos outros
 * §F3  o custo, dito: quantas atribuições foram varridas
 * §F4  e o que ele NÃO diz: vazio não é prova, e a testemunha está no §D4 da ponte
 */
#include <stdio.h>
#include "refuta.h"
#include "unidade.h"

/* as identidades e as mutações vivem agora em `refuta.h`, para que a assistente as
 * possa correr também — o refutador é do SISTEMA e não deste ficheiro. */

int main(void){
    printf("\n=== O REFUTADOR SOBRE AS IDENTIDADES DA CASA ===\n");

    /* ═══ §F0 AS VERDADEIRAS: tem de voltar vazio ════════════════════════════ */
    printf("\n§F0 As identidades da casa — o refutador tem de voltar VAZIO.\n\n");
    {
        struct { const char *nome; RfId4 f; } id[] = {
            { "o CENTRO      M + adj M = tr·I",      id_centro   },
            { "a MEMBRANA    M · adj M = det·I",     id_membrana },
            { "CAYLEY-HAMILTON  M² − tr·M + det·I",  id_cayley   },
            { "LAGRANGE      directo² + cruzado²",   id_lagrange },
            { "det(AB) = det A · det B",             id_detmult  },
        };
        int n = (int)(sizeof id / sizeof *id), caiu = 0;
        printf("        identidade                          veredicto\n");
        for(int i = 0; i < n; i++){
            int pr, a, b, c, d;
            int r = rf_refuta(id[i].f, &pr, &a, &b, &c, &d);
            if(r) caiu++;
            printf("        %-35s %s\n", id[i].nome,
                   r ? "REFUTADA (mau)" : "vazio — não caiu");
        }
        ok("AS CINCO IDENTIDADES DA CASA PASSAM PELO REFUTADOR E NENHUMA CAI, varridas"
           " EXAUSTIVAMENTE em cinco primos — todas as atribuições das quatro variáveis em"
           " 𝔽₅, 𝔽₇, 𝔽₁₁, 𝔽₁₃ e 𝔽₁₇. E isto não é uma prova: é uma tentativa séria de as"
           " derrubar que falhou, o que é outra coisa e vale por si",
           caiu == 0 && n == 5);
    }

    /* ═══ §F1 AS MUTAÇÕES: cada uma tem de cair ═════════════════════════════ */
    printf("\n§F1 As mutações — cada uma TEM de cair, e diz-se onde.\n\n");
    {
        struct { const char *nome; RfId4 f; } mu[] = {
            { "o centro com tr trocado por a+a",     mut_centro    },
            { "a membrana com det de sinal trocado", mut_membrana  },
            { "Cayley-Hamilton sem o termo det",     mut_cayley    },
            { "Lagrange sem o cruzado",              mut_lagrange  },
            { "det(AB) = det A + det B",             mut_detmult   },
        };
        int n = (int)(sizeof mu / sizeof *mu), caiu = 0;
        printf("        a mutação                            caiu em    com (a,b,c,d)\n");
        for(int i = 0; i < n; i++){
            int pr = 0, a = 0, b = 0, c = 0, d = 0;
            int r = rf_refuta(mu[i].f, &pr, &a, &b, &c, &d);
            if(r){
                caiu++;
                printf("        %-36s 𝔽%-9d (%d,%d,%d,%d)\n", mu[i].nome, pr, a,b,c,d);
            } else printf("        %-36s NÃO CAIU (mau)\n", mu[i].nome);
        }
        ok("E CADA MUTAÇÃO CAI, COM O CONTRA-EXEMPLO EXIBIDO — é isto que torna o §F0 uma"
           " medição e não uma cerimónia. Um refutador que nunca acha nada não distingue"
           " «não há defeito» de «não sei procurar»; aqui cinco corrupções deliberadas — um"
           " sinal trocado, um termo em falta, um mais no lugar de um vezes — são todas"
           " apanhadas, e o primo e os valores dizem-se",
           caiu == n && n == 5);
    }

    /* ═══ §F2 POR QUE VÁRIOS PRIMOS ════════════════════════════════════════ */
    printf("\n§F2 Por que vários primos: o acidente de característica.\n\n");
    {
        int a,b,c,d;
        int em2 = rf_varre(mut_quadrado, 2, &a,&b,&c,&d);
        printf("        (a+b)² = a² + b²  em 𝔽₂:  %s  ← o 2ab desaparece\n",
               em2 ? "REFUTADA" : "passa — e é FALSA");
        int caiu_em = 0, primeiros = 0;
        for(int i = 0; i < RF_NP; i++){
            if(rf_varre(mut_quadrado, RF_PRIMOS[i], &a,&b,&c,&d)){
                caiu_em++;
                if(!primeiros) primeiros = RF_PRIMOS[i];
            }
        }
        printf("        e nos cinco primos da casa: cai em %d deles, o primeiro é 𝔽%d\n",
               caiu_em, primeiros);
        ok("E É POR ISTO QUE SÃO VÁRIOS PRIMOS: uma identidade FALSA pode passar num corpo"
           " por acidente de característica. (a+b)² = a² + b² é falsa, e em 𝔽₂ ela PASSA,"
           " porque o 2ab desaparece. Um só primo é uma régua; vários são a mesma pergunta"
           " feita em corpos onde os acidentes são diferentes — e a falsa cai em todos os"
           " cinco que esta casa usa",
           !em2 && caiu_em == RF_NP);
    }

    /* ═══ §F3 O CUSTO ═════════════════════════════════════════════════════ */
    printf("\n§F3 O custo, dito.\n\n");
    {
        long c = rf_custo();
        printf("        atribuições varridas por identidade: %ld (cinco primos, quatro"
               " variáveis)\n", c);
        printf("        e nenhuma delas cresceu: o resto é o tecto\n");
        ok("E O CUSTO DIZ-SE: cada identidade é varrida em cento e trinta mil atribuições"
           " — todas as que"
           " existem nas quatro variáveis, nos cinco primos —, e nenhuma cresce, porque o"
           " resto é o tecto. Comprar exaustão baixando o primo é a troca certa: em 𝔽₁₂₇"
           " seriam 260 milhões por identidade e a exaustão morria de espera",
           c > 100000 && c < 1000000);
    }

    /* ═══ §F4 E O QUE ELE NÃO DIZ ═════════════════════════════════════════ */
    printf("\n§F4 O que o vazio NÃO significa.\n\n");
    {
        /* uma identidade falsa em ℚ que passa em TODOS estes primos: x^p ≡ x (Fermat)
         * dá uma família de falsas que passam num primo. Aqui construo o caso honesto:
         * uma identidade que é verdadeira nos cinco primos por serem pequenos. */
        int a,b,c,d, passou = 0;
        for(int i = 0; i < RF_NP; i++){
            int p = RF_PRIMOS[i];
            /* p·a = 0 em 𝔽ₚ para todo a: é FALSO em ℚ e VERDADE em 𝔽ₚ */
            int caiu = 0;
            for(int x = 0; x < p && !caiu; x++) if((p * x) % p != 0) caiu = 1;
            if(!caiu) passou++;
        }
        (void)a; (void)b; (void)c; (void)d;
        printf("        «p·a = 0» é FALSO em ℚ e passa em todos os %d primos: %d\n",
               RF_NP, passou);
        ok("E O VAZIO NÃO É PROVA, com testemunha: «p·a = 0» é falso em ℚ e verdadeiro em"
           " 𝔽ₚ para todo a — passa nos cinco primos e é falso na face grande. Logo o"
           " refutador só pode dizer «isto é falso», e o silêncio dele é silêncio. Dizer o"
           " contrário seria transformar uma ferramenta barata numa insinuação, que é o"
           " defeito que esta casa persegue",
           passou == RF_NP);
    }

    printf("\n=== %ld asserções, %ld falhas ===\n", unidades, falhas);
    return falhas ? 1 : 0;
}
