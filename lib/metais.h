/* ═══════════════════════════════════════════════════════════════════════════
 * lib/metais.h — Z[σ] com σ² = mσ + 1: o inverso sem fração, e a descida
 *
 * O metal m fixa o corpo. E ele tem uma relação que os outros não têm:
 *
 *     σ⁻¹ = σ − m          o inverso escreve-se SEM fração
 *
 * porque σ² − mσ = 1, isto é σ(σ−m) = 1. A face multiplicativa não sai do
 * degrau, e é por isso que a família metálica é a que é.
 *
 * A norma é N(a+bσ) = a² + mab − b², multiplicativa. E a fração contínua de σ
 * é [m;m,m,…], de período UM --- daí os convergentes terem norma ±1.
 *
 * Tudo em inteiros. Medido em `tests/metais.c` e `tests/pgwire.c` §W147.
 * ═══════════════════════════════════════════════════════════════════════════ */
#ifndef METAIS_H
#define METAIS_H

/* a + bσ, no metal m */
typedef struct { long a, b, m; } Met;

static Met met(long a, long b, long m){ Met r = {a, b, m}; return r; }
static Met met_soma(Met x, Met y){ Met r = {x.a+y.a, x.b+y.b, x.m}; return r; }
static Met met_op(Met x){ Met r = {-x.a, -x.b, x.m}; return r; }

/* (a+bσ)(c+dσ) = ac + bd·σ² + (ad+bc)σ, e σ² = mσ + 1 */
static Met met_prod(Met x, Met y){
    Met r = { x.a*y.a + x.b*y.b,
              x.a*y.b + x.b*y.a + x.m*x.b*y.b,
              x.m };
    return r;
}
/* o conjugado: σ ↦ σ' = m − σ */
static Met met_conj(Met x){ Met r = {x.a + x.m*x.b, -x.b, x.m}; return r; }
static long met_norma(Met x){ return x.a*x.a + x.m*x.a*x.b - x.b*x.b; }

/* ── A RELAÇÃO DESTE CORPO: σ⁻¹ = σ − m, sem fração ─────────────────────── */
static Met met_sigma(long m){ return met(0, 1, m); }
static Met met_sigma_inv(long m){ return met(-m, 1, m); }      /* σ − m */

/* o inverso geral: existe no degrau sse N = ±1, e RECUSA quando não existe */
static int met_inverso(Met x, Met *out){
    long N = met_norma(x);
    if(N != 1 && N != -1) return 0;
    Met c = met_conj(x);
    out->a = c.a / N; out->b = c.b / N; out->m = x.m;
    return 1;
}

/* ── A DESCIDA: a fração contínua de σ é [m;m,m,…], período UM ───────────
 * Os convergentes p_k/q_k saem da recorrência p_k = m·p_{k−1} + p_{k−2}, e
 * têm norma ±1 --- é daí que as unidades vêm. */
typedef struct { long p, q; } MetConv;

static MetConv met_conv(long m, int k){
    long p0 = 1, p1 = m, q0 = 0, q1 = 1;
    for(int i = 0; i < k; i++){
        long p2 = m*p1 + p0, q2 = m*q1 + q0;
        p0 = p1; p1 = p2; q0 = q1; q1 = q2;
    }
    MetConv c = {p1, q1};
    return c;
}
/* a norma do convergente, N(p − qσ) = p² − m·p·q − q², que é ±1 */
static long met_conv_norma(long m, MetConv c){ return c.p*c.p - m*c.p*c.q - c.q*c.q; }

/* ── O GATO: ×σ é o passo da régua de base m (aranha def:gato) ──────────── */
static Met met_gato(Met x){ return met_prod(x, met_sigma(x.m)); }
static int  met_esquilo(Met x, Met *out){                     /* ×σ⁻¹ = ×(σ−m) */
    *out = met_prod(x, met_sigma_inv(x.m));
    return 1;
}

#endif /* METAIS_H */
