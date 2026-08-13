/* conecthus/backends/latex/claim_to_tex.c — backend LaTeX: Claim → proposição (projeção).
 * Não declara Result. Só renderiza a especificação.
 *
 *   cc -O2 -std=c99 -Wall -I../../lang claim_to_tex.c ../../lang/parse_claim.c -o claim_to_tex
 *   ./claim_to_tex ../../claims/pareto.claim
 */
#include <stdio.h>
#include <stdlib.h>
#include "claim.h"

int main(int argc, char **argv){
    if(argc < 2){ fprintf(stderr, "uso: claim_to_tex ficheiro.claim\n"); return 2; }
    Claim c;
    if(claim_parse_file(argv[1], &c) != 0){
        fprintf(stderr, "parse falhou: %s\n", argv[1]);
        return 1;
    }
    printf("%% gerado de %s — projeção LaTeX (não é Result)\n", argv[1]);
    printf("\\begin{proposicao}[%s]\n", c.name);
    printf("Lei~%d sobre \\texttt{%s}.\n", c.law, c.object);
    printf("Passo: \\texttt{%s}; volta: \\texttt{%s}; mede: \\texttt{%s}.\n",
           c.step, c.back, c.measure);
    if(c.invariant[0])
        printf("Invariante: \\texttt{%s}.\n", c.invariant);
    if(c.mutate[0])
        printf("Mutação: \\texttt{%s}.\n", c.mutate);
    if(c.classify[0])
        printf("Classificação pedida: \\texttt{%s} (hipótese até o Result).\n", c.classify);
    printf("\\end{proposicao}\n");
    printf("%% Resultado só após STEP → BACK → MEASURE → R.\n");
    return 0;
}
