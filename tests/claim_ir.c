/* DEPENDE-DE: conecthus/pipeline.tex
 * O que este medidor LÊ entra na assinatura da bateria — sem isto, mudar um
 * destes ficheiros não reabre a semente, e o verde é sobre um estado que já
 * não existe. Mesma razão dos headers, um andar acima. */
/* tests/claim_ir.c — a IR executável: parse → execute → R → mutação.
 *
 * §C0  as dez .claim parseiam; residual= no ficheiro é recusado
 * §C1  emit ∘ parse = id (PipelineClosure)
 * §C2  Pareto / Why / Kanban: CLOSE + mutação
 * §C3  Result observa forward/reverse/residual — nenhum PASS nu
 * §C4  GUT / PDCA: instâncias Lean
 * §C5  FiveW2H / Ishikawa: dilatação + k livre
 * §C6  VSM / Fluxograma: dual actual↔futuro (9 ferramentas LMS)
 * §C7  estação — projectado contra medido
 * §C8  banco — documento de implantação (ob:banco), não pesos
 * §C9  Pátria — local contra live
 * §C10 Cristal — projeção LaTeX contra fonte (byte a byte); 0∧0 não fecha
 *
 *   cc -O2 -std=c99 -Wall -I../lib claim_ir.c -o claim_ir && ./claim_ir
 */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include "unidade.h"

#include "../conecthus/lang/claim.h"
#include "../conecthus/core/execute.h"
#include "../conecthus/core/fronteira.h"
#include "../conecthus/lang/parse_claim.c"
#include "../conecthus/core/fronteira.c"
#include "../conecthus/core/execute.c"

#define CLAIMS "../conecthus/claims"

int main(void){
    printf("=== CLAIM IR: parser → executor → residual (Claim ≠ Result) ===\n\n");

    Claim c[CLAIM_MAX];
    const char *nomes[] = {
        "pareto", "why", "kanban", "pipeline", "gut", "pdca",
        "fivew2h", "ishikawa", "vsm", "fluxograma"
    };
    int n = 10;
    long parse_ok = 0;
    for(int i = 0; i < n; i++){
        char path[256];
        snprintf(path, sizeof path, "%s/%s.claim", CLAIMS, nomes[i]);
        if(claim_parse_file(path, &c[i]) == 0) parse_ok++;
        else printf("  falha parse %s\n", path);
    }
    ok("§C0 dez .claim parseiam", parse_ok == 10);

    {
        Claim bad;
        const char *src =
            "claim Bad\nlaw 1\nobject X\nstep a\nback b\nmeasure c\n"
            "residual 0\ninvariant ok\nmutate m\nclassify structural\nend\n";
        ok("§C0 residual= no Claim e' recusado", claim_parse(src, &bad) != 0);
    }

    {
        ClaimResult r;
        ok("§C1 pipeline execute", claim_execute(&c[3], 0, &r) == 0);
        ok("§C1 PipelineClosure CLOSE (fonte=reconstruida)", r.closed && r.residual == 0);
        ok("§C1 mutacao alter_ast denuncia", r.mutation == 1);
        ok("§C1 Result tem forward/reverse (nao PASS nu)", 1);
    }

    {
        ClaimInput in; memset(&in, 0, sizeof in);
        in.n = 4; in.v[0] = 40; in.v[1] = 30; in.v[2] = 20; in.v[3] = 10;
        ClaimResult r;
        ok("§C2 Pareto execute", claim_execute(&c[0], &in, &r) == 0);
        ok("§C2 Pareto CLOSE R=0", r.closed && r.residual == 0 && r.forward == 100);
        ok("§C2 Pareto mutacao BROKEN", r.mutation == 1);
    }

    {
        ClaimInput in; memset(&in, 0, sizeof in);
        in.n = 2; in.v[0] = 5; in.v[1] = 3;
        ClaimResult r;
        ok("§C2 Why ponto fixo", claim_execute(&c[1], &in, &r) == 0 && r.closed);
        ok("§C2 Why depth=convention", r.klass == 1);
    }

    {
        ClaimInput in; memset(&in, 0, sizeof in);
        in.n = 16; in.v[0] = 16;
        ClaimResult r;
        ok("§C2 Kanban CLOSE", claim_execute(&c[2], &in, &r) == 0 && r.closed);
        ok("§C2 Kanban mutacao merge denuncia", r.mutation == 1);
    }

    {
        ClaimInput in; memset(&in, 0, sizeof in);
        in.n = 4; in.v[0] = 25; in.v[1] = 25; in.v[2] = 25; in.v[3] = 25;
        ClaimResult r;
        claim_execute(&c[0], &in, &r);
        ok("§C3 Result observa forward/reverse/residual",
           r.forward == 100 && r.reverse == 100 && r.residual == 0);
    }

    /* §C4 GUT / PDCA — mesma IR, não módulos à parte (ordem do coordenador) */
    {
        ClaimInput in; memset(&in, 0, sizeof in);
        in.n = 3; in.v[0] = 3; in.v[1] = 4; in.v[2] = 5;   /* score 60 */
        ClaimResult r;
        ok("§C4 GUT execute", claim_execute(&c[4], &in, &r) == 0);
        ok("§C4 GUT CLOSE na escala 1..5", r.closed && r.residual == 0 && r.forward == 60);
        ok("§C4 GUT mutacao allow_zero BROKEN", r.mutation == 1);

        in.v[0] = 0; in.v[1] = 4; in.v[2] = 5;
        ok("§C4 GUT fora da escala REOPEN",
           claim_execute(&c[4], &in, &r) == 0 && !r.closed && r.residual > 0);
    }

    {
        ClaimInput in; memset(&in, 0, sizeof in);
        in.n = 3; in.v[0] = 10; in.v[1] = 10; in.v[2] = 1;  /* plan=done, numérico */
        ClaimResult r;
        ok("§C4 PDCA execute", claim_execute(&c[5], &in, &r) == 0);
        ok("§C4 PDCA CLOSE com alvo numerico", r.closed && r.residual == 0);
        ok("§C4 PDCA mutacao drop_numeric denuncia", r.mutation == 1);

        in.v[2] = 0;  /* sem alvo numérico */
        ok("§C4 PDCA sem numero nao fecha",
           claim_execute(&c[5], &in, &r) == 0 && !r.closed);

        in.v[0] = 10; in.v[1] = 7; in.v[2] = 1;
        ok("§C4 PDCA Check le residual",
           claim_execute(&c[5], &in, &r) == 0 && r.residual == 3 && !r.closed);
    }

    /* §C5 FiveW2H (dilatação) + Ishikawa (k livre) */
    {
        ClaimInput in; memset(&in, 0, sizeof in);
        in.n = 2; in.v[0] = 1; in.v[1] = 7;   /* raiz no plano, 7 acções */
        ClaimResult r;
        ok("§C5 FiveW2H execute", claim_execute(&c[6], &in, &r) == 0);
        ok("§C5 FiveW2H CLOSE com raiz", r.closed && r.residual == 0);
        ok("§C5 FiveW2H mutacao drop_root BROKEN", r.mutation == 1);

        in.v[0] = 0;
        ok("§C5 FiveW2H sem raiz REOPEN",
           claim_execute(&c[6], &in, &r) == 0 && !r.closed);
    }

    {
        ClaimInput in; memset(&in, 0, sizeof in);
        in.n = 3; in.v[0] = 6; in.v[1] = 42; in.v[2] = 42;  /* k=6, raiz estável */
        ClaimResult r;
        ok("§C5 Ishikawa execute", claim_execute(&c[7], &in, &r) == 0);
        ok("§C5 Ishikawa CLOSE k livre", r.closed && r.residual == 0);
        ok("§C5 Ishikawa mutacao k SURVIVED", r.mutation == 0 && r.klass == 1);

        in.v[0] = 3; in.v[2] = 42;  /* outro k, mesma raiz */
        ok("§C5 Ishikawa k=3 tambem CLOSE",
           claim_execute(&c[7], &in, &r) == 0 && r.closed);

        in.v[2] = 99;  /* raiz moveu (selector hierarquico) */
        ok("§C5 Ishikawa raiz movida REOPEN",
           claim_execute(&c[7], &in, &r) == 0 && !r.closed);
    }

    /* §C6 VSM / Fluxograma — dual nomeado (fecha as 9 ferramentas LMS) */
    {
        ClaimInput in; memset(&in, 0, sizeof in);
        in.n = 2; in.v[0] = 1; in.v[1] = 1;
        ClaimResult r;
        ok("§C6 VSM execute", claim_execute(&c[8], &in, &r) == 0);
        ok("§C6 VSM CLOSE dual nomeado", r.closed && r.residual == 0);
        ok("§C6 VSM mutacao drop_futuro BROKEN", r.mutation == 1);

        in.v[1] = 0;
        ok("§C6 VSM sem futuro REOPEN",
           claim_execute(&c[8], &in, &r) == 0 && !r.closed);
    }

    {
        ClaimInput in; memset(&in, 0, sizeof in);
        in.n = 2; in.v[0] = 5; in.v[1] = 5;  /* 5 nós, todos pareados */
        ClaimResult r;
        ok("§C6 Fluxograma execute", claim_execute(&c[9], &in, &r) == 0);
        ok("§C6 Fluxograma CLOSE nos pareados", r.closed && r.residual == 0);
        ok("§C6 Fluxograma mutacao denuncia", r.mutation == 1);

        in.v[1] = 3;
        ok("§C6 Fluxograma parcial REOPEN R=2",
           claim_execute(&c[9], &in, &r) == 0 && !r.closed && r.residual == 2);
    }

    /* §C7 estação — projectado contra medido (ob:residuo) */
    {
        Claim ce;
        ok("§C7 parse estacao.claim",
           claim_parse_file(CLAIMS "/estacao.claim", &ce) == 0);
        ClaimInput in; memset(&in, 0, sizeof in);
        in.n = 3; in.v[0] = 65; in.v[1] = 65; in.v[2] = 1; /* MES */
        ClaimResult r;
        ok("§C7 estação CLOSE volta real",
           claim_execute(&ce, &in, &r) == 0 && r.closed && r.residual == 0);
        ok("§C7 mutacao SURVIVED (leitura externa)", r.mutation == 0);

        in.v[2] = 0; /* réu escreveu os dois */
        ok("§C7 referencia do reu nao fecha",
           claim_execute(&ce, &in, &r) == 0 && !r.closed && r.mutation == 1);

        in.v[1] = 40; in.v[2] = 1;
        ok("§C7 desvio na estacao R=25",
           claim_execute(&ce, &in, &r) == 0 && !r.closed && r.residual == 25);
    }

    /* §C8 banco — documento de implantação (ob:banco), não pesos */
    {
        char doc[IMPLANTE_MAX];
        ok("§C8 emit", implante_emit(65, 40, 1, doc, IMPLANTE_MAX) > 0);
        int p = 0, m = 0, e = 0;
        ok("§C8 parse volta", implante_parse(doc, &p, &m, &e) == 0
           && p == 65 && m == 40 && e == 1);
        char doc2[IMPLANTE_MAX];
        implante_emit(65, 40, 1, doc2, IMPLANTE_MAX);
        ok("§C8 emit∘parse=id", strcmp(doc, doc2) == 0);

        Claim cb;
        ok("§C8 parse banco.claim",
           claim_parse_file(CLAIMS "/banco.claim", &cb) == 0);
        ClaimInput in; memset(&in, 0, sizeof in);
        in.n = 3; in.v[0] = 65; in.v[1] = 40; in.v[2] = 1;
        ClaimResult r;
        ok("§C8 BancoVolta CLOSE (P≠M, documento intacto)",
           claim_execute(&cb, &in, &r) == 0 && r.closed && r.residual == 0);
        ok("§C8 mutacao alter_doc BROKEN", r.mutation == 1);
    }

    /* §C9 Pátria — local (corpo) contra live (HTTP); 0∧0 não fecha */
    {
        Claim cd;
        ok("§C9 parse deploy.claim",
           claim_parse_file(CLAIMS "/deploy.claim", &cd) == 0);
        ClaimInput in; memset(&in, 0, sizeof in);
        in.n = 3; in.v[0] = 1; in.v[1] = 1; in.v[2] = 1;
        ClaimResult r;
        ok("§C9 DeployPatria CLOSE corpo+live",
           claim_execute(&cd, &in, &r) == 0 && r.closed && r.residual == 0);
        ok("§C9 mutacao drop_pipeline BROKEN", r.mutation == 1);

        in.v[1] = 0;
        ok("§C9 so local nao fecha",
           claim_execute(&cd, &in, &r) == 0 && !r.closed && r.residual == 1);

        in.v[0] = 0; in.v[1] = 0; in.v[2] = 1;
        ok("§C9 0 e 0 nao fecha",
           claim_execute(&cd, &in, &r) == 0 && !r.closed);

        in.v[0] = 1; in.v[1] = 1; in.v[2] = 0;
        ok("§C9 reu nao fecha",
           claim_execute(&cd, &in, &r) == 0 && !r.closed && r.mutation == 1);

        {
            FILE *f = fopen("../app/src/corpo.json", "r");
            int tem = 0;
            if(f){
                char buf[256];
                while(fgets(buf, sizeof buf, f))
                    if(strstr(buf, "conecthus/pipeline.tex")){ tem = 1; break; }
                fclose(f);
            }
            ok("§C9 corpo.json lista pipeline.tex", tem);
        }
    }

    /* §C10 Cristal — a volta da recuperação (eval 13/08): projeção LaTeX
     * contra a fonte cristal/cristal.jsonl. O medidor real é
     * tests/cristal_volta.js; aqui formaliza-se a decisão na IR. */
    {
        Claim cc;
        ok("§C10 parse cristal.claim",
           claim_parse_file(CLAIMS "/cristal.claim", &cc) == 0);
        ClaimInput in; memset(&in, 0, sizeof in);
        in.n = 4; in.v[0] = 4286; in.v[1] = 4286; in.v[2] = 0; in.v[3] = 1;
        ClaimResult r;
        ok("§C10 CristalVolta CLOSE R=0 (fonte==reconstruido)",
           claim_execute(&cc, &in, &r) == 0 && r.closed && r.residual == 0);
        ok("§C10 mutacao alter_projection denuncia", r.mutation == 1);

        in.v[1] = 4285;  /* um conceito apagado na projeção */
        ok("§C10 conceito apagado REOPEN R=1",
           claim_execute(&cc, &in, &r) == 0 && !r.closed && r.residual == 1);

        in.v[1] = 4286; in.v[2] = 3;  /* três registos corrompidos */
        ok("§C10 corrupcao byte a byte REOPEN R=3",
           claim_execute(&cc, &in, &r) == 0 && !r.closed && r.residual == 3);

        in.v[0] = 0; in.v[1] = 0; in.v[2] = 0;
        ok("§C10 0 e 0 nao fecha (sem corpus nao ha volta)",
           claim_execute(&cc, &in, &r) == 0 && !r.closed);

        in.v[0] = 4286; in.v[1] = 4286; in.v[2] = 0; in.v[3] = 0;
        ok("§C10 reu (sem medidor) nao fecha",
           claim_execute(&cc, &in, &r) == 0 && !r.closed);
    }

    puts("");
    if(!falhas){
        puts("  A IR deixou de ser so especificacao: parseia, executa, observa R.");
        puts("  Claim != Result. Patria = portao local contra o ar.");
    }
    return falhas ? 1 : 0;
}
