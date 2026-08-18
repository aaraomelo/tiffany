/* qmd.h — ℚ(m√D): aritmética exacta no corpo quadrático parametrizado.
 *
 * Representação canónica de um elemento
 *
 *      x = (a + b·m√D) / den,
 *
 * com a, b, den inteiros, den ≠ 0, e par (m, D) fixo por campo.
 * Para m ≠ 0, ℚ(m√D) = ℚ(√D); o parâmetro m é a escala computacional.
 *
 * Conjugado:  x* = (a − b·m√D) / den
 * Norma:      N(x) = (a² − b²m²D) / den²
 *
 * Sem double, sem sqrt(), sem limiar. */
#ifndef QMD_H
#define QMD_H

#include "reta.h"   /* rt_inv_mod: a inversão em F_p */

typedef struct { long a, b, den; } Qmd;

static long qmd_gcd(long x, long y){
    if(x < 0) x = -x;
    if(y < 0) y = -y;
    while(y){ long t = x % y; x = y; y = t; }
    return x ? x : 1;
}

static Qmd qmd_make(long a, long b, long den){
    if(den < 0){ a = -a; b = -b; den = -den; }
    if(a == 0 && b == 0) return (Qmd){ 0, 0, 1 };
    long g = qmd_gcd(qmd_gcd(a < 0 ? -a : a, b < 0 ? -b : b), den);
    return (Qmd){ a/g, b/g, den/g };
}

static Qmd qmd_add(Qmd x, Qmd y){
    long den = x.den * y.den;
    return qmd_make(x.a * y.den + y.a * x.den,
                    x.b * y.den + y.b * x.den, den);
}

static Qmd qmd_sub(Qmd x, Qmd y){
    long den = x.den * y.den;
    return qmd_make(x.a * y.den - y.a * x.den,
                    x.b * y.den - y.b * x.den, den);
}

static Qmd qmd_mul(Qmd x, Qmd y, long m, long D){
    long md = m * m * D;
    long ac = x.a * y.a + x.b * y.b * md;
    long bc = x.a * y.b + x.b * y.a;
    return qmd_make(ac, bc, x.den * y.den);
}

static int qmd_eq(Qmd x, Qmd y){
    return (long long)x.a * y.den == (long long)y.a * x.den
        && (long long)x.b * y.den == (long long)y.b * x.den;
}

static void qmd_conj(Qmd x, long *ca, long *cb){
    *ca = x.a;
    *cb = -x.b;
}

/* numerador de N(x)·den² = a² − b²m²D */
static long long qmd_norm_num(Qmd x, long m, long D){
    long long md = (long long)m * m * D;
    return (long long)x.a * x.a - (long long)x.b * x.b * md;
}

static Qmd qmd_inv(Qmd x, long m, long D){
    long long c = qmd_norm_num(x, m, D);
    return qmd_make((long)(x.den * x.a), (long)(-x.den * x.b), (long)c);
}

static Qmd qmd_div(Qmd x, Qmd y, long m, long D){
    return qmd_mul(x, qmd_inv(y, m, D), m, D);
}

static Qmd qmd_pow(Qmd x, int k, long m, long D){
    Qmd r = qmd_make(1, 0, 1);
    for(int i = 0; i < k; i++) r = qmd_mul(r, x, m, D);
    return r;
}

static Qmd qmd_one(void){ return qmd_make(1, 0, 1); }

/* numerador de N(α−1) com α = (a+b·m√D)/den */
static long long qmd_norm_am1(Qmd alpha, long m, long D){
    long am1 = alpha.a - alpha.den;
    long long md = (long long)m * m * D;
    return (long long)am1 * am1 - (long long)alpha.b * alpha.b * md;
}

/* 1/(α−1) pela fórmula fechada do thm:serie-quadratica */
static Qmd qmd_inv_am1(Qmd alpha, long m, long D){
    long am1 = alpha.a - alpha.den;
    long long norm = qmd_norm_am1(alpha, m, D);
    return qmd_make(alpha.den * am1, -alpha.den * alpha.b, (long)norm);
}

/* norma, inversa fechada e S₁ = α⁻¹ numa passagem, sem limiar */
static int qmd_verifica_rapida(long m, long D, Qmd alpha){
    Qmd one = qmd_one();
    Qmd am1 = qmd_sub(alpha, one);
    if(qmd_norm_am1(alpha, m, D) == 0) return 0;
    long ca, cb;
    qmd_conj(am1, &ca, &cb);
    Qmd prod = qmd_mul(am1, qmd_make(ca, cb, am1.den), m, D);
    long long den2 = (long long)am1.den * am1.den;
    if(!qmd_eq(prod, qmd_make((long)qmd_norm_am1(alpha, m, D), 0, (long)den2)))
        return 0;
    if(!qmd_eq(qmd_inv(am1, m, D), qmd_inv_am1(alpha, m, D)))
        return 0;
    Qmd ainv = qmd_inv(alpha, m, D);
    return qmd_eq(qmd_div(qmd_sub(one, ainv), am1, m, D), ainv);
}

/* ═══════════════════════════════════════════════════════════════════════════════════
 * A TRANSFORMADA UNIVERSAL — e ela é a OPERAÇÃO: convolução e deconvolução
 *
 * O thm:espectro: a transformada é a AVALIAÇÃO NAS FOLHAS, realizada inteira. Em F_p com
 * m²D resíduo quadrático as folhas são ±s com s² ≡ m²D, e são ELEMENTOS DE F_p — logo a
 * transformada leva Q(m√D) ⊗ F_p em F_p × F_p, e a CONVOLUÇÃO vira DOIS PRODUTOS
 * ESCALARES:
 *
 *      TRANSFORMA      α = (a,b)  ↦  (a + b·s, a − b·s)          duas avaliações
 *      OPERA           ponto a ponto                              dois produtos em F_p
 *      ANTITRANSFORMA  (v₊,v₋)    ↦  ((v₊+v₋)/2, (v₊−v₋)/(2s))   duas divisões
 *
 * É este o ganho, e é o que faz dela a operação e não um teste: convolver no domínio
 * custa quatro produtos e duas somas; na transformada custa dois produtos. O preço é a
 * ida e a volta, que se pagam uma vez e servem para toda a cadeia.
 *
 * A antitransformada pede 2 e s invertíveis em F_p — isto é p ímpar e s ≠ 0, que é
 * exactamente m²D não nulo. Devolve 0 quando não pode, e não trunca.
 * ═══════════════════════════════════════════════════════════════════════════════════ */

/* as duas folhas em F_p: as raízes de x² ≡ m²D. Devolve 0 se m²D não for resíduo. */
static int qmd_folhas_fp(long m, long D, long p, long *sp, long *sm){
    long alvo = ((m % p) * (m % p) % p) * (D % p) % p;
    if(alvo < 0) alvo += p;
    for(long s = 0; s < p; s++)
        if(s*s % p == alvo){ *sp = s; *sm = (p - s) % p; return 1; }
    return 0;
}

/* TRANSFORMA: as duas avaliações, em F_p */
static void qmd_dtu(long a, long b, long s, long p, long *vp, long *vm){
    long am = ((a % p) + p) % p, bm = ((b % p) + p) % p;
    *vp = (am + bm * s) % p;
    *vm = (am + bm * ((p - s) % p)) % p;
}

/* ANTITRANSFORMA: a = (v₊+v₋)/2 e b = (v₊−v₋)/(2s). Precisa de 2 e s invertíveis. */
static int qmd_dtu_inv(long vp, long vm, long s, long p, long *a, long *b){
    if(p <= 2 || s % p == 0) return 0;
    long inv2 = rt_inv_mod(2, p), invs = rt_inv_mod(s % p, p);
    if(inv2 == 0 || invs == 0) return 0;
    long soma = (vp + vm) % p, dif = ((vp - vm) % p + p) % p;
    *a = soma * inv2 % p;
    *b = dif * inv2 % p * invs % p;
    return 1;
}

#endif
