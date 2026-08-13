/* conecthus/core/fronteira.h — resíduo ENTRE claims (Lei 7: ligar sem fundir).
 * Não funde artefactos: mede a interface.
 *   GUT ↔ 5W2H     inversões de ranking
 *   Why ↔ Ishikawa raiz contida no mapa
 *   PDCA ↔ VSM     alvo = futuro
 * fundamento.tex ob:sete; tests/lean_controlo.c §N2. */
#ifndef CONECTHUS_FRONTEIRA_H
#define CONECTHUS_FRONTEIRA_H
#include "execute.h"

/* scores[0..n), acao[0..n) = permutação de índices.
 * R = nº de pares adjacentes com score[acao[i]] < score[acao[i+1]]. */
long fronteira_gut_5w2h(const int *score, const int *acao, int n);

/* root ∈ causes[0..n) ? 0 : 1 */
long fronteira_why_ishikawa(int root, const int *causes, int n);

/* |plan_target - vsm_future| */
long fronteira_pdca_vsm(int plan_target, int vsm_future);

/* Volta contra o mundo (ob:residuo): |projectado − medido|.
 * Fecha só se a leitura é da estação (external≠0) e R=0.
 * Referência do réu (external=0) nunca fecha — mutação copy_reference. */
long residual_estacao(int projectado, int medido);
int  estacao_fecha(int projectado, int medido, int external);

/* Documento de implantação (ob:banco): emit ∘ parse = id. Não vai para pesos. */
#define IMPLANTE_MAX 128
int implante_emit(int p, int m, int ext, char *out, int cap);
int implante_parse(const char *src, int *p, int *m, int *ext);

/* Portão Pátria: local (corpo tem pipeline) contra live (HTTP 200).
 * 0∧0 não fecha — falta o artefacto. Réu (external=0) nunca fecha. */
long residual_patria(int local, int live);
int  patria_fecha(int local, int live, int external);

#endif
