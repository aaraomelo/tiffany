/* tests/claim_fronteira.c — resíduo ENTRE claims (Lei 7).
 *
 * §F0  parse das 3 fronteiras
 * §F1  GUT↔5W2H: ordem consistente R=0; swap adjacente R>0
 * §F2  Why↔Ishikawa: raiz no mapa / fora
 * §F3  PDCA↔VSM: alvo = futuro
 * §F4  interna verde + fronteira apanha (mecanismo §N2)
 *
 *   cc -O2 -std=c99 -Wall -I../lib claim_fronteira.c -o claim_fronteira && ./claim_fronteira
 */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "unidade.h"

#include "../conecthus/lang/claim.h"
#include "../conecthus/core/execute.h"
#include "../conecthus/core/fronteira.h"
#include "../conecthus/lang/parse_claim.c"
#include "../conecthus/core/fronteira.c"
#include "../conecthus/core/execute.c"

#define CLAIMS "../conecthus/claims"

int main(void){
    printf("=== FRONTEIRA: resíduo entre Claims (ligar sem fundir) ===\n\n");

    Claim cg, cw, cp;
    ok("§F0 parse fronteira_gut",
       claim_parse_file(CLAIMS "/fronteira_gut.claim", &cg) == 0);
    ok("§F0 parse fronteira_why",
       claim_parse_file(CLAIMS "/fronteira_why.claim", &cw) == 0);
    ok("§F0 parse fronteira_pdca",
       claim_parse_file(CLAIMS "/fronteira_pdca.claim", &cp) == 0);

    /* §F1 GUT↔5W2H — n=4, scores desc, acao = id */
    {
        ClaimInput in; memset(&in, 0, sizeof in);
        int n = 4;
        in.n = 1 + 2 * n;
        in.v[0] = n;
        in.v[1] = 60; in.v[2] = 48; in.v[3] = 30; in.v[4] = 12; /* scores */
        in.v[5] = 0; in.v[6] = 1; in.v[7] = 2; in.v[8] = 3;     /* ordem boa */
        ClaimResult r;
        ok("§F1 GUT/5W2H consistente", claim_execute(&cg, &in, &r) == 0);
        ok("§F1 CLOSE R=0", r.closed && r.residual == 0);

        /* swap adjacente 0↔1 com scores distintos */
        in.v[5] = 1; in.v[6] = 0;
        ok("§F1 swap → inversões",
           claim_execute(&cg, &in, &r) == 0 && !r.closed && r.residual > 0);
        ok("§F1 mutacao pode denunciar", r.mutation == 1);
    }

    /* §F2 Why↔Ishikawa */
    {
        ClaimInput in; memset(&in, 0, sizeof in);
        in.n = 2 + 4;
        in.v[0] = 7;              /* root */
        in.v[1] = 4;
        in.v[2] = 3; in.v[3] = 7; in.v[4] = 1; in.v[5] = 9;
        ClaimResult r;
        ok("§F2 raiz no mapa CLOSE",
           claim_execute(&cw, &in, &r) == 0 && r.closed);

        in.v[0] = 99;
        ok("§F2 raiz fora REOPEN",
           claim_execute(&cw, &in, &r) == 0 && !r.closed && r.residual == 1);
    }

    /* §F3 PDCA↔VSM */
    {
        ClaimInput in; memset(&in, 0, sizeof in);
        in.n = 2; in.v[0] = 42; in.v[1] = 42;
        ClaimResult r;
        ok("§F3 alvo=futuro CLOSE",
           claim_execute(&cp, &in, &r) == 0 && r.closed && r.residual == 0);

        in.v[1] = 40;
        ok("§F3 desvio R=2",
           claim_execute(&cp, &in, &r) == 0 && r.residual == 2 && !r.closed);
    }

    /* §F4 mecanismo: funções puras batem com lean_controlo §N2 */
    {
        int score[5] = {60, 48, 30, 30, 12};
        int acao[5] = {0, 1, 2, 3, 4};
        ok("§F4 consistente inv=0", fronteira_gut_5w2h(score, acao, 5) == 0);
        acao[0] = 1; acao[1] = 0;
        ok("§F4 swap inv>=1", fronteira_gut_5w2h(score, acao, 5) >= 1);
        int causes[3] = {1, 2, 3};
        ok("§F4 why hit", fronteira_why_ishikawa(2, causes, 3) == 0);
        ok("§F4 why miss", fronteira_why_ishikawa(9, causes, 3) == 1);
        ok("§F4 pdca eq", fronteira_pdca_vsm(10, 10) == 0);
        ok("§F5 estacao R=0 fecha MES",
           residual_estacao(65, 65) == 0 && estacao_fecha(65, 65, 1));
        ok("§F5 reu nao fecha", !estacao_fecha(65, 65, 0));
        ok("§F5 desvio", residual_estacao(65, 40) == 25);
    }

    puts("");
    if(!falhas){
        puts("  Fronteira = interface Lei 7: dois corpos, resíduo contável entre eles.");
        puts("  Validação interna não basta — GUT/5W2H, Why/Ishikawa, PDCA/VSM.");
    }
    return falhas ? 1 : 0;
}
