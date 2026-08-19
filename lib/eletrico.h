/* eletrico.h — O CORPO TRANSISTOR: resolver, simular e validar circuitos.
 *
 * A tríade encarnada, e nenhuma peça pede vírgula:
 *
 *     SOMA      (⊕, Clifford)   =  KIRCHHOFF   — o RESISTOR, linear
 *     PRODUTO   (⊗, La Hire)    =  o GANHO     — o POTENCIÔMETRO, V_out = α·V_in
 *     OPERADOR  (Π, Pontryagin) =  o TRANSISTOR — soma de tensões vira PRODUTO de correntes
 *
 * Π = exp∘Σ∘log não se avalia: a soma no expoente É o produto. A Gilbert cell é I1·I2/Iref,
 * exacta em ℚ. As raízes (ω₀, Z₀, fp) vivem ao QUADRADO: ω₀² = 1/(LC), Z₀² = L/C,
 * fp² = Re²/(Re²+Im²). O sinal de Δ é o de R²C − 4L, sem dividir.
 */
#ifndef ELETRICO_H
#define ELETRICO_H

#include <math.h>                      /* os medidores que ainda não migraram puxavam daqui */
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif
#define VT 0.025852                    /* 25852/1000000 V a 300 K — a constante, não o tipo */

typedef long long LL;
typedef __int128 I128;

/* ---- KIRCHHOFF: a SOMA. Série soma; paralelo lê-se sem dividir. ------------------- */
static long el_serie(const long *R, int n){
    long s = 0;
    for(int k = 0; k < n; k++) s += R[k];
    return s;
}
/* Rp = (R1 R2 R3) / (R2R3+R1R3+R1R2), reduzido. A lei é pn·soma = prod·pd. */
static void el_paralelo3(long R1, long R2, long R3, long *pn, long *pd){
    *pn = R1*R2*R3;
    *pd = R2*R3 + R1*R3 + R1*R2;
}

/* ---- O GANHO: o PRODUTO. α = Z2/(Z1+Z2), e compor multiplica os α. --------------- */
static void el_divisor(long z1, long z2, long *pn, long *pd){
    *pn = z2; *pd = z1 + z2;
}

/* ---- O OPERADOR: soma no expoente vira produto. k é I/Is na região exponencial. ---- */
static long el_shockley_prod(long k1, long k2){ return k1 * k2; }
/* Gilbert: log-Σ-antilog É o produto. I1·I2/Iref em ℚ. */
static void el_gilbert(long I1, long I2, long Iref, long *pn, long *pd){
    *pn = I1 * I2; *pd = Iref;
}

/* ---- as três impedâncias: |Z_L|=ωL, |Z_R|=R, |Z_C|=1/(ωC)  (s^{+1}, s^0, s^{-1}) ---- */
static long el_zL(long L, long w){ return w * L; }
static long el_zR(long R){ return R < 0 ? -R : R; }
static void el_zC(long C, long w, long *pn, long *pd){ *pn = 1; *pd = w * C; }

/* Im Z do RLC, homogeneizado: (ω² L C − 1). Zero na ressonância ω² = 1/(LC). */
static long el_rlc_im_num(long L, long C, long w2n, long w2d){
    return w2n * L * C - w2d;
}
static void el_w0q(long L, long C, long *pn, long *pd){ *pn = 1; *pd = L * C; }
static void el_Z0q(long L, long C, long *pn, long *pd){ *pn = L; *pd = C; }
/* fp = 1  <=>  fp² = 1  <=>  Im = 0 (e Re ≠ 0). Sem cabs. */
static int el_fp_unitario(long re, long im){ return im == 0 && re != 0; }

/* Δ = R² − 4L/C; o sinal é o de R²C − 4L, inteiro se R,L,C o forem. */
static int el_delta_sinal(long R, long L, long C){
    if(C <= 0) return 0;
    long v = R*R*C - 4*L;
    return v > 0 ? 1 : (v < 0 ? -1 : 0);
}

/* ---- Wheatstone: equilíbrio Z1·Zx = Z2·Z3. O detector é o numerador Z2 Z3 − Z1 Zx. ---- */
static void el_wheatstone(long z1, long z2, long z3, long *pn, long *pd){
    *pn = z2 * z3; *pd = z1;
}
static long el_detector_num(long z1, long z2, long z3, long zx_n, long zx_d){
    return z2 * z3 * zx_d - z1 * zx_n;
}

/* ---- Verlet em ℤ: L q'' + R q' + q/C = 0. h = hn/hd. Devolve q, i e as trocas de sinal. */
#ifndef EL_AMP
#define EL_AMP 1000000LL
#endif
static void el_simula(long R, long L, long C, long hn, long hd, int passos,
                      long *qf, long *if_, int *sings){
    LL q = EL_AMP, ii = 0;
    int prev = 1, sc = 0;
    I128 a_den = (I128)L * C;
    if(hd == 0 || a_den == 0){ if(qf) *qf = 0; if(if_) *if_ = 0; if(sings) *sings = 0; return; }
    for(int k = 0; k < passos; k++){
        I128 a_num = -(I128)R * ii * C - q;
        I128 i2 = (I128)ii * hd * 2 * a_den + (I128)hn * a_num;
        ii = (LL)(i2 / (2 * hd * a_den));
        I128 q2 = (I128)q * hd + (I128)hn * ii;
        q = (LL)(q2 / hd);
        a_num = -(I128)R * ii * C - q;
        i2 = (I128)ii * hd * 2 * a_den + (I128)hn * a_num;
        ii = (LL)(i2 / (2 * hd * a_den));
        int s = q > 0 ? 1 : q < 0 ? -1 : 0;
        if(s && prev && s != prev) sc++;
        if(s) prev = s;
    }
    if(qf) *qf = (long)q;
    if(if_) *if_ = (long)ii;
    if(sings) *sings = sc;
}
#endif
