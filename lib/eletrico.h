/* eletrico.h — O CORPO TRANSISTOR: resolver, simular e validar circuitos.
 *
 * O Aarão: "agora a assistente vai resolver, simular e validar circuitos elétricos. Recupera o
 * corpo transistor, e vamos seguir — é onde vive o operador."
 *
 * A fonte é chess/sandbox/corpo_transistor.tex, e o teorema central é a TRÍADE encarnada:
 *
 *     SOMA      (⊕, Clifford)   =  KIRCHHOFF   — o RESISTOR, linear
 *     PRODUTO   (⊗, La Hire)    =  o GANHO     — o POTENCIÔMETRO, V_out = α·V_in
 *     OPERADOR  (Π, Pontryagin) =  o TRANSISTOR — Shockley, I = I_s·e^{V/V_T}
 *
 * É por isso que "é onde vive o operador": Π = exp∘Σ∘log não é uma metáfora aqui, é a equação de
 * Shockley. O transistor leva SOMA de tensões em PRODUTO de correntes, e a Gilbert cell
 * (log-Σ-antilog) multiplica dois sinais somando os seus logs. Pontryagin em silício.
 *
 * E as multiplicidades fecham a tríade nos reativos: o INDUTOR é s^{+1} (deriva), o CAPACITOR é
 * s^{-1} (integra), e o RESISTOR é s^0. A dualidade L ⋈ C é +1 ⋈ -1 — soma 0 (o resistor) e
 * média geométrica √(L/C) = Z₀, o metal.
 */
#ifndef ELETRICO_H
#define ELETRICO_H

#include <math.h>
#include <complex.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

/* ---- as três impedâncias, e a multiplicidade de cada uma ------------------------------- */
static double complex z_R(double R, double w){ (void)w; return R; }                 /* s^0  */
static double complex z_L(double L, double w){ return I*w*L; }                      /* s^+1 */
static double complex z_C(double C, double w){ return (w == 0) ? INFINITY : 1.0/(I*w*C); } /* s^-1 */

/* ---- KIRCHHOFF: a SOMA. Em série somam as impedâncias; em paralelo, as admitâncias. ---- */
static double complex el_serie(const double complex *z, int n){
    double complex s = 0;
    for(int k = 0; k < n; k++) s += z[k];
    return s;
}
static double complex el_paralelo(const double complex *z, int n){
    double complex y = 0;
    for(int k = 0; k < n; k++) y += 1.0/z[k];
    return 1.0/y;
}
/* ---- O GANHO: o PRODUTO. O divisor de tensão, e compor divisores multiplica. ----------- */
static double complex el_divisor(double complex z1, double complex z2){
    return z2/(z1+z2);                       /* V_out/V_in = α */
}
/* ---- O OPERADOR: SHOCKLEY. Leva soma de tensões em produto de correntes. --------------- */
#define VT 0.025852                          /* a tensão térmica a 300 K */
static double el_shockley(double V, double Is){ return Is*(exp(V/VT) - 1.0); }
static double el_shockley_inv(double Ic, double Is){ return VT*log(Ic/Is + 1.0); }
/* a Gilbert cell: multiplicar É somar os logs, e depois antilog */
static double el_gilbert(double I1, double I2, double Iref){
    return exp(log(I1) + log(I2) - log(Iref));
}

/* ---- o RLC, e o casamento --------------------------------------------------------------- */
static double complex el_rlc(double R, double L, double C, double w){
    return R + I*(w*L - 1.0/(w*C));
}
static double el_ressonancia(double L, double C){ return 1.0/sqrt(L*C); }
static double el_fp(double complex Z){ return creal(Z)/cabs(Z); }   /* cos(arg Z) */
static double el_Z0(double L, double C){ return sqrt(L/C); }        /* o metal, La Hire */
/* a régua do RLC — a MESMA (B,C) do resto do projeto: L·s² + R·s + 1/C = 0 */
static double el_delta(double R, double L, double C){ return R*R - 4.0*L/C; }

/* ---- Wheatstone: a medida por ANULAÇÃO ------------------------------------------------- */
/* A convenção dos braços, e ela tem de ser a MESMA nas duas funções:
 *
 *        ramo A:  Z1 em cima, Z2 em baixo   ->  V_A = V·Z2/(Z1+Z2)
 *        ramo B:  Z3 em cima, Zx em baixo   ->  V_B = V·Zx/(Z3+Zx)
 *        equilíbrio: V_A = V_B  <=>  Z2·Z3 = Z1·Zx  <=>  Zx = Z2·Z3/Z1
 *
 * Na primeira versão o detector comparava `divisor(z1,zx)` com `divisor(z2,z3)` — outra
 * numeração — e por isso não zerava onde a fórmula dizia. O medidor apanhou: duas funções
 * minhas que tinham de concordar e não concordavam. */
static double complex el_wheatstone(double complex z1, double complex z2, double complex z3){
    return z2*z3/z1;                         /* o Z_x que zera o detector */
}
static double complex el_detector(double complex z1, double complex z2,
                                  double complex z3, double complex zx, double complex V){
    return V*(el_divisor(z1,z2) - el_divisor(z3,zx));
}

/* ---- simular no tempo, para ter o SEGUNDO caminho -------------------------------------- */
/* L·q'' + R·q' + q/C = V(t), integrado por Verlet de velocidade. É o caminho independente
 * que tem de concordar com a solução fechada — e é a comparação, não a asserção, que apanha. */
static void el_simula(double R, double L, double C, double q0, double i0,
                      double h, int passos, double *qf, double *if_){
    double q = q0, ii = i0;
    for(int k = 0; k < passos; k++){
        double a = (-R*ii - q/C)/L;
        ii += 0.5*h*a;
        q  += h*ii;
        double a2 = (-R*ii - q/C)/L;
        ii += 0.5*h*a2;
        (void)a2;
    }
    *qf = q; *if_ = ii;
}
#endif
