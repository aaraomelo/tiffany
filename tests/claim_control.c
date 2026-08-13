/* tests/claim_control.c — Controlo de Histerese ACIMA da IR.
 *
 * §K0  init: Pareto na borda com R=0
 * §K1  variante admissível (+0) → RETAIN
 * §K2  variante que quebra soma → RETRACT (R≠0)
 * §K3  variante longe (Θ) → MOVE
 * §K4  acaso: plano aleatório mesma cardinalidade; se R=0 empata, métrica fraca
 * §K5  Controlo NÃO parseia — não há "control" no .claim
 *
 *   cc -O2 -std=c99 -Wall -I../lib claim_control.c -o claim_control && ./claim_control
 */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include "unidade.h"

#include "../conecthus/lang/claim.h"
#include "../conecthus/core/execute.h"
#include "../conecthus/core/control.h"
#include "../conecthus/core/eixos_texto.h"
#include "../conecthus/lang/parse_claim.c"
#include "../conecthus/core/fronteira.c"
#include "../conecthus/core/execute.c"
#include "../conecthus/core/eixos_texto.c"
#include "../conecthus/core/control.c"

int main(void){
    printf("=== CONTROLO: histerese acima da IR (RETAIN/MOVE/RETRACT) ===\n\n");

    Claim c;
    ok("§K0 parse pareto.claim",
       claim_parse_file("../conecthus/claims/pareto.claim", &c) == 0);

    ClaimInput base; memset(&base, 0, sizeof base);
    base.n = 4; base.v[0] = 40; base.v[1] = 30; base.v[2] = 20; base.v[3] = 10;

    ControlState H;
    ok("§K0 control_init na borda", control_init(&H, &c, &base) == 0);
    ok("§K0 retido fecha R=0", H.medido.closed && H.medido.residual == 0);

    /* §K1 RETAIN — mesma entrada */
    {
        ControlCandidate out;
        ok("§K1 seleccionar identidade",
           control_seleccionar(&H, &c, &base, &out) == 0);
        ok("§K1 act = RETAIN", out.act == CTRL_RETAIN && out.result.closed);
    }

    /* §K2 RETRACT — quebra a soma */
    {
        ClaimInput bad = base;
        bad.v[0] = 99;   /* soma ≠ 100 */
        ControlCandidate out;
        ok("§K2 seleccionar soma quebrada",
           control_seleccionar(&H, &c, &bad, &out) == 0);
        ok("§K2 act = RETRACT (R≠0)", out.act == CTRL_RETRACT && !out.result.closed);
    }

    /* §K3 MOVE — fecha mas longe de Θ (L1 grande, R ainda 0) */
    {
        /* redistribui mantendo soma 100 mas L1 grande */
        ClaimInput far = base;
        far.v[0] = 70; far.v[1] = 10; far.v[2] = 10; far.v[3] = 10;
        ControlCandidate out;
        ok("§K3 seleccionar redistribuicao",
           control_seleccionar(&H, &c, &far, &out) == 0);
        ok("§K3 ainda fecha", out.result.closed && out.result.residual == 0);
        ok("§K3 act = MOVE (fora de Theta)", out.act == CTRL_MOVE);
        ok("§K3 D[0] > theta[0]", out.D[0] > H.theta[0]);
    }

    /* §K4 acaso */
    {
        ClaimResult r_ok, r_rnd;
        claim_execute(&c, &base, &r_ok);
        ok("§K4 control_acaso corre", control_acaso(&c, &base, 42u, &r_rnd) == 0);
        /* acaso normalizado a 100 também fecha — empate no R denuncia métrica só-fecho */
        ok("§K4 acaso com soma 100 tambem R=0 (empate no fecho)",
           r_ok.closed && r_rnd.closed);
        /* a escolha do modelo vs acaso: L1 dos vectores deve diferir */
        long D[CTRL_AXES];
        ClaimInput rnd = base;
        unsigned s = 42u;
        for(int i = 0; i < rnd.n; i++){
            s = s * 1103515245u + 12345u;
            rnd.v[i] = (int)((s >> 16) % 50);
        }
        long sum = 0; for(int i = 0; i < rnd.n; i++) sum += rnd.v[i];
        long acc = 0;
        for(int i = 0; i < rnd.n - 1; i++){
            rnd.v[i] = (int)((rnd.v[i] * 100) / sum); acc += rnd.v[i];
        }
        rnd.v[rnd.n - 1] = (int)(100 - acc);
        ClaimResult rr;
        claim_execute(&c, &rnd, &rr);
        control_dist(&base, &r_ok, &rnd, &rr, D);
        ok("§K4 vectores diferem (L1>0) — controlo distingue escolha", D[0] > 0);
    }

    /* §K5 não há control na gramática */
    {
        Claim bad;
        ok("§K5 Controlo e runtime, nao campo do Claim",
           claim_parse_file("../conecthus/claims/pareto.claim", &bad) == 0
           && bad.step[0] && !strstr(bad.step, "control"));
    }

    /* §K6 eixos numéricos — V com 8 slots (teclado/φ a 0 sem texto) */
    {
        ok("§K6 CTRL_AXES == 8", CTRL_AXES == 8);
        ClaimInput near = base;
        near.v[0] = 42;  /* |Δv0|=2 ≤ theta caixa 2; rebalance */
        near.v[1] = 28;
        ControlCandidate out;
        ok("§K6 variante v0 pequena",
           control_seleccionar(&H, &c, &near, &out) == 0);
        ok("§K6 fecha", out.result.closed);
        ok("§K6 RETAIN ou MOVE coerente com L1",
           (out.act == CTRL_RETAIN && control_admissivel(&H, out.D))
           || (out.act == CTRL_MOVE && !control_admissivel(&H, out.D)));
    }

    /* §K7 eixos texto com dados reais (Peano: cafe↔café, d_K, d_φ) */
    {
        long dc = eixos_caixa("cafe", "café");
        long dk = eixos_teclado("casa", "casa");
        long dp = eixos_fonetico("cafe", "café");
        long df = eixos_forma("cafe", "café");
        ok("§K7 caixa cafe↔café = 1 (só acento)", dc == 1);
        ok("§K7 teclado identidade = 0", dk == 0);
        ok("§K7 fonético cafe≡café (mesmo φ)", dp == 0);
        ok("§K7 forma cafe≡café (base)", df == 0);

        long dk2 = eixos_teclado("que", "qeu"); /* e↔u vizinhos? u col6 e col2 → d=4 */
        ok("§K7 teclado que↔qeu > 0", dk2 > 0);

        long dp2 = eixos_fonetico("casa", "caza");
        ok("§K7 fonético casa↔caza > 0 (s≠z no mapa)", dp2 > 0);

        ControlState Ht;
        ok("§K7 init com texto cafe",
           control_init_texto(&Ht, &c, &base, "cafe") == 0);
        ControlCandidate out;
        ok("§K7 seleccionar café (acento)",
           control_seleccionar_texto(&Ht, &c, &base, "café", &out) == 0);
        ok("§K7 ainda fecha R=0", out.result.closed);
        ok("§K7 D_caixa=1 ≤ Θ → RETAIN",
           out.D[CTRL_E_CAIXA] == 1 && out.act == CTRL_RETAIN);

        ok("§K7 seleccionar palavra longe",
           control_seleccionar_texto(&Ht, &c, &base, "computador", &out) == 0);
        ok("§K7 longe → MOVE (forma/teclado)",
           out.act == CTRL_MOVE && out.result.closed);
    }

    /* §K8 quarto cenário: acaso — métrica degenerada empata; L1 discorda */
    {
        ControlAcasoRel rel;
        ok("§K8 acaso_relatorio",
           control_acaso_relatorio(&c, &base, 7u, 40, &rel) == 0);
        ok("§K8 degen (cardinalidade) empata sempre",
           rel.degen_empates == rel.n_sorteios);
        ok("§K8 L1 discorda em algum sorteio",
           rel.discorda_L1 > 0);
        ok("§K8 fecho tambem empata (Pareto normalizado)",
           rel.empates_fecho == rel.n_sorteios);
    }

    puts("");
    if(!falhas){
        puts("  Controlo acima da IR: Gerar → Medir(D) → Seleccionar → RETAIN|MOVE|RETRACT.");
        puts("  V: 8 eixos. Acaso = quarto cenario (degen empata; L1 informa).");
    }
    return falhas ? 1 : 0;
}
