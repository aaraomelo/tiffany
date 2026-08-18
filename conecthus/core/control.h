/* conecthus/core/control.h — Controlo de Histerese ACIMA da IR.
 * Não é sintaxe do .claim. Gerar → Medir(D) → Seleccionar → Reter/Fechar.
 * Teorema: papers/corpo_topologico.tex thm:controle-histerese.
 * Eixos V: L1, |R|, |n|, mut, caixa, teclado, fonético, forma. */
#ifndef CONECTHUS_CONTROL_H
#define CONECTHUS_CONTROL_H
#include "execute.h"

#define CTRL_AXES 8
#define CTRL_MAX_VAR 16
#define CTRL_TEXTO 128

/* índices dos eixos */
#define CTRL_E_L1       0
#define CTRL_E_R        1
#define CTRL_E_N        2
#define CTRL_E_MUT      3
#define CTRL_E_CAIXA    4   /* acento/segmentação (texto) ou |v0| (só número) */
#define CTRL_E_TECLADO  5   /* d_K geometria de teclas */
#define CTRL_E_FONETICO 6   /* d_φ grafema→fonema PT */
#define CTRL_E_FORMA    7   /* forma / max |Δvi| */

typedef enum {
    CTRL_RETAIN  = 0,
    CTRL_MOVE    = 1,
    CTRL_RETRACT = 2
} ControlAction;

typedef struct {
    int exige_fecho;
    int theta[CTRL_AXES];
    ClaimInput retido;
    ClaimResult medido;
    char texto[CTRL_TEXTO];   /* realização em fala (opcional) */
} ControlState;

typedef struct {
    ClaimInput in;
    long D[CTRL_AXES];
    ClaimResult result;
    ControlAction act;
    char texto[CTRL_TEXTO];
} ControlCandidate;

int control_init(ControlState *H, const Claim *c, const ClaimInput *base);
int control_init_texto(ControlState *H, const Claim *c, const ClaimInput *base,
                       const char *texto);

void control_dist(const ClaimInput *a, const ClaimResult *ra,
                  const ClaimInput *b, const ClaimResult *rb,
                  long D[CTRL_AXES]);

void control_dist_full(const ClaimInput *a, const ClaimResult *ra, const char *ta,
                       const ClaimInput *b, const ClaimResult *rb, const char *tb,
                       long D[CTRL_AXES]);

int control_admissivel(const ControlState *H, const long D[CTRL_AXES]);

int control_gerar(const ControlState *H, int eixo, int delta, ClaimInput *out);

int control_seleccionar(ControlState *H, const Claim *c, const ClaimInput *cand,
                        ControlCandidate *out);

int control_seleccionar_texto(ControlState *H, const Claim *c,
                              const ClaimInput *cand, const char *texto,
                              ControlCandidate *out);

int control_acaso(const Claim *c, const ClaimInput *base, unsigned seed,
                  ClaimResult *acaso_out);

/* Quarto cenário (fundamento): acaso ao lado do ganho.
 * degen_empates ≈ n_sorteios ⟺ métrica cega (cardinalidade).
 * discorda_L1: vectores diferem do modelo (métrica informativa). */
typedef struct {
    int n_sorteios;
    int empates_fecho;   /* acasos com R=0 como o modelo */
    int discorda_L1;     /* L1(modelo,acaso) > 0 */
    int degen_empates;   /* métrica n: sempre empata (= n_sorteios) */
} ControlAcasoRel;

int control_acaso_relatorio(const Claim *c, const ClaimInput *base, unsigned seed,
                            int n_sorteios, ControlAcasoRel *out);

#endif
