/* conecthus/core/control.c — runtime acima da IR; 𝒱 com 8 eixos. */
#include "control.h"
#include "eixos_texto.h"
#include <string.h>

static long labs_l(long x){ return x < 0 ? -x : x; }

static void copia_texto(char *dst, int cap, const char *src){
    if(!dst || cap <= 0) return;
    dst[0] = 0;
    if(!src) return;
    int i = 0;
    while(src[i] && i + 1 < cap){ dst[i] = src[i]; i++; }
    dst[i] = 0;
}

int control_init(ControlState *H, const Claim *c, const ClaimInput *base){
    return control_init_texto(H, c, base, 0);
}

int control_init_texto(ControlState *H, const Claim *c, const ClaimInput *base,
                       const char *texto){
    if(!H || !c || !base) return -1;
    memset(H, 0, sizeof *H);
    H->exige_fecho = 1;
    H->theta[CTRL_E_L1]       = 5;
    H->theta[CTRL_E_R]        = 0;
    H->theta[CTRL_E_N]        = 0;
    H->theta[CTRL_E_MUT]      = 1;
    H->theta[CTRL_E_CAIXA]    = 2;   /* cafe↔café = 1 ≤ 2 */
    H->theta[CTRL_E_TECLADO]  = 6;   /* vizinhança de teclas */
    H->theta[CTRL_E_FONETICO] = 2;
    H->theta[CTRL_E_FORMA]    = 5;
    H->retido = *base;
    copia_texto(H->texto, CTRL_TEXTO, texto);
    if(claim_execute(c, base, &H->medido) != 0) return -1;
    return 0;
}

void control_dist(const ClaimInput *a, const ClaimResult *ra,
                  const ClaimInput *b, const ClaimResult *rb,
                  long D[CTRL_AXES]){
    control_dist_full(a, ra, 0, b, rb, 0, D);
}

void control_dist_full(const ClaimInput *a, const ClaimResult *ra, const char *ta,
                       const ClaimInput *b, const ClaimResult *rb, const char *tb,
                       long D[CTRL_AXES]){
    long l1 = 0, forma_v = 0;
    int n = a->n < b->n ? a->n : b->n;
    for(int i = 0; i < n; i++){
        long d = labs_l((long)a->v[i] - (long)b->v[i]);
        l1 += d;
        if(d > forma_v) forma_v = d;
    }
    for(int i = n; i < a->n; i++) l1 += labs_l((long)a->v[i]);
    for(int i = n; i < b->n; i++) l1 += labs_l((long)b->v[i]);
    D[CTRL_E_L1]  = l1;
    D[CTRL_E_R]   = labs_l(ra->residual - rb->residual);
    D[CTRL_E_N]   = labs_l((long)a->n - (long)b->n);
    D[CTRL_E_MUT] = labs_l((long)ra->mutation - (long)rb->mutation);

    if(ta && tb && ta[0] && tb[0]){
        long Tx[8];
        eixos_dist_texto(ta, tb, Tx);
        D[CTRL_E_CAIXA]    = Tx[4];
        D[CTRL_E_TECLADO]  = Tx[5];
        D[CTRL_E_FONETICO] = Tx[6];
        D[CTRL_E_FORMA]    = Tx[7];
    } else {
        D[CTRL_E_CAIXA]    = (a->n > 0 && b->n > 0)
            ? labs_l((long)a->v[0] - (long)b->v[0]) : 0;
        D[CTRL_E_TECLADO]  = 0;
        D[CTRL_E_FONETICO] = 0;
        D[CTRL_E_FORMA]    = forma_v;
    }
}

int control_admissivel(const ControlState *H, const long D[CTRL_AXES]){
    for(int i = 0; i < CTRL_AXES; i++)
        if(D[i] > H->theta[i]) return 0;
    return 1;
}

int control_gerar(const ControlState *H, int eixo, int delta, ClaimInput *out){
    if(!H || !out) return -1;
    *out = H->retido;
    if((eixo == CTRL_E_L1 || eixo == CTRL_E_CAIXA) && out->n > 0){
        out->v[0] += delta;
        if(out->v[0] < 0) out->v[0] = 0;
    } else if(eixo == CTRL_E_FORMA && out->n > 1){
        out->v[1] += delta;
        if(out->v[1] < 0) out->v[1] = 0;
    } else if(eixo == CTRL_E_N){
        if(delta > 0 && out->n < 32){ out->v[out->n] = out->v[0]; out->n++; }
        if(delta < 0 && out->n > 1) out->n--;
    }
    return 0;
}

int control_seleccionar(ControlState *H, const Claim *c, const ClaimInput *cand,
                        ControlCandidate *out){
    return control_seleccionar_texto(H, c, cand, 0, out);
}

int control_seleccionar_texto(ControlState *H, const Claim *c,
                              const ClaimInput *cand, const char *texto,
                              ControlCandidate *out){
    if(!H || !c || !cand || !out) return -1;
    memset(out, 0, sizeof *out);
    out->in = *cand;
    copia_texto(out->texto, CTRL_TEXTO, texto);
    if(claim_execute(c, cand, &out->result) != 0) return -1;
    control_dist_full(&H->retido, &H->medido, H->texto[0] ? H->texto : 0,
                      cand, &out->result, texto,
                      out->D);

    if(H->exige_fecho && !out->result.closed){
        out->act = CTRL_RETRACT;
        return 0;
    }
    if(control_admissivel(H, out->D)){
        out->act = CTRL_RETAIN;
        return 0;
    }
    out->act = CTRL_MOVE;
    return 0;
}

int control_acaso(const Claim *c, const ClaimInput *base, unsigned seed,
                  ClaimResult *acaso_out){
    if(!c || !base || !acaso_out) return -1;
    ClaimInput rnd = *base;
    unsigned s = seed ? seed : 1u;
    for(int i = 0; i < rnd.n; i++){
        s = s * 1103515245u + 12345u;
        rnd.v[i] = (int)((s >> 16) % 50);
    }
    if(strstr(c->object, "Pareto") || strcmp(c->name, "ParetoClosure") == 0){
        long sum = 0;
        for(int i = 0; i < rnd.n; i++) sum += rnd.v[i];
        if(sum <= 0){ rnd.v[0] = 100; for(int i = 1; i < rnd.n; i++) rnd.v[i] = 0; }
        else {
            long acc = 0;
            for(int i = 0; i < rnd.n - 1; i++){
                rnd.v[i] = (int)((rnd.v[i] * 100) / sum);
                acc += rnd.v[i];
            }
            rnd.v[rnd.n - 1] = (int)(100 - acc);
        }
    }
    if(claim_execute(c, &rnd, acaso_out) != 0) return -1;
    return 0;
}

static void preenche_acaso(const Claim *c, const ClaimInput *base, unsigned *s,
                           ClaimInput *rnd){
    *rnd = *base;
    for(int i = 0; i < rnd->n; i++){
        *s = (*s) * 1103515245u + 12345u;
        rnd->v[i] = (int)((*s >> 16) % 50);
    }
    if(strstr(c->object, "Pareto") || strcmp(c->name, "ParetoClosure") == 0){
        long sum = 0;
        for(int i = 0; i < rnd->n; i++) sum += rnd->v[i];
        if(sum <= 0){ rnd->v[0] = 100; for(int i = 1; i < rnd->n; i++) rnd->v[i] = 0; }
        else {
            long acc = 0;
            for(int i = 0; i < rnd->n - 1; i++){
                rnd->v[i] = (int)((rnd->v[i] * 100) / sum);
                acc += rnd->v[i];
            }
            rnd->v[rnd->n - 1] = (int)(100 - acc);
        }
    }
}

int control_acaso_relatorio(const Claim *c, const ClaimInput *base, unsigned seed,
                            int n_sorteios, ControlAcasoRel *out){
    if(!c || !base || !out || n_sorteios < 1) return -1;
    memset(out, 0, sizeof *out);
    out->n_sorteios = n_sorteios;
    ClaimResult modelo;
    if(claim_execute(c, base, &modelo) != 0) return -1;
    unsigned s = seed ? seed : 1u;
    for(int t = 0; t < n_sorteios; t++){
        ClaimInput rnd;
        preenche_acaso(c, base, &s, &rnd);
        ClaimResult rr;
        if(claim_execute(c, &rnd, &rr) != 0) return -1;
        /* degenerada: cardinalidade — modelo.n == rnd.n sempre */
        if(base->n == rnd.n) out->degen_empates++;
        if(modelo.closed && rr.closed) out->empates_fecho++;
        long D[CTRL_AXES];
        control_dist(base, &modelo, &rnd, &rr, D);
        if(D[CTRL_E_L1] > 0) out->discorda_L1++;
    }
    return 0;
}
