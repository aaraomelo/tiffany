/* api.h — a API do motor Chess, numa superfície só.
 *
 * Cola as duas superfícies que antes viviam separadas:
 *   (A) o MOTOR DINÂMICO   — motor/gato/esquilo sobre u64 (a iteração da mineração, o regime caos).
 *                            Declarado aqui, DEFINIDO em motor_impl.c (link).
 *   (B) a API ALGÉBRICA    — o corpo binário como projeção do Omnitrix (gato/esquilo matriciais →
 *                            Zhegalkin → portas e aritmética). Definida aqui, static inline (auto-contida).
 *   (C) as TRÊS OPERAÇÕES  — Clifford (g_ij, a soma), La Hire (a contração, o produto), Pontryagin
 *                            (subir/descer, o dual), na notação de índice. O estado atual (o magnífico).
 *
 * A referência: main/nucleo (o nível das operações) e elementares/magnifico.py, isomorfico.py.
 */
#ifndef UNIVERSE_TOOLS_API_H
#define UNIVERSE_TOOLS_API_H

#include <stdint.h>

typedef uint64_t u64;
typedef int64_t  i64;

/* ===================================================================
 * (A) O MOTOR DINÂMICO — u64 → u64 (definido em motor_impl.c)
 * =================================================================== */
u64 motor(u64 x);      /* motor(x) = gato(esquilo(x)) — a iteração */
u64 gato(u64 x);       /* gato(x)  = (x ^ rotl(x,1)) ^ (x & rotl(x,7)) */
u64 esquilo(u64 x);    /* esquilo(x) = rotl(x,29) — o flip/deslocamento */

/* ===================================================================
 * (B) A API ALGÉBRICA — as portas e a aritmética, do gato e do esquilo
 *     (o corpo binário como projeção mod 2 do Omnitrix matricial)
 * =================================================================== */
typedef struct { i64 a, b, c, d; } Mat;                        /* [[a,b],[c,d]] */

static inline Mat GATO(i64 m)   { Mat r = {m, 1, 1, 0}; return r; }   /* A_m: o meio-somador */
static inline Mat ESQUILO(void) { Mat r = {0, 1, -1, 0}; return r; }  /* G: o flip/deslocamento */
static inline Mat mat_lahire(Mat X, Mat Y) {                          /* ⊗ = produto dos gatos (La Hire) */
    Mat r = { X.a*Y.a + X.b*Y.c, X.a*Y.b + X.b*Y.d,
              X.c*Y.a + X.d*Y.c, X.c*Y.b + X.d*Y.d }; return r;
}
static inline i64 mat_det(Mat X) { return X.a*X.d - X.b*X.c; }        /* o esquilo: det = ±1 */

/* as duas saídas do meio-somador (o gato) = XOR e AND, bit a bit */
static inline u64 gato_soma (u64 x, u64 y) { return x ^ y; }          /* (x+y) mod 2 = XOR */
static inline u64 gato_vaium(u64 x, u64 y) { return x & y; }          /* (x+y) div 2 = AND */
static inline u64 esquilo_shift(u64 x)     { return x << 1; }         /* move o vai-um */

/* as clássicas — todas de Zhegalkin (⊕, ·) sobre o gato/esquilo */
static inline u64 XOR (u64 x, u64 y) { return gato_soma(x, y); }
static inline u64 AND (u64 x, u64 y) { return gato_vaium(x, y); }
static inline u64 NOT (u64 x)        { return XOR(x, ~0ULL); }
static inline u64 OR  (u64 x, u64 y) { return XOR(XOR(x, y), AND(x, y)); }   /* a⊕b⊕ab */
static inline u64 NAND(u64 x, u64 y) { return NOT(AND(x, y)); }
static inline u64 NOR (u64 x, u64 y) { return NOT(OR(x, y)); }
static inline u64 XNOR(u64 x, u64 y) { return NOT(XOR(x, y)); }
static inline u64 ADD (u64 x, u64 y) {                                 /* o ripple: gato∘esquilo iterado */
    while (y) { u64 s = gato_soma(x, y); u64 v = esquilo_shift(gato_vaium(x, y)); x = s; y = v; } return x;
}
static inline u64 SUB (u64 x, u64 y) { return ADD(x, ADD(NOT(y), 1)); }
static inline u64 MUL (u64 x, u64 y) {                                 /* shift-and-add */
    u64 r = 0; while (y) { if (y & 1) r = ADD(r, x); x = esquilo_shift(x); y >>= 1; } return r;
}

/* ===================================================================
 * (C) AS TRÊS OPERAÇÕES DO MAGNÍFICO — na notação de índice (Einstein)
 *     Clifford = a soma (a métrica); La Hire = o produto (a contração);
 *     Pontryagin = o operador (subir/descer). Generalizam de 2 (P2P) a N (PNP).
 * =================================================================== */

/* CLIFFORD: a forma quadrática Q = g_ij x^i x^j (a métrica, a soma ⊥) */
static inline double clifford_Q(int n, const double *g /* n*n */, const double *x) {
    double s = 0; for (int i = 0; i < n; i++) for (int j = 0; j < n; j++) s += g[i*n+j]*x[i]*x[j]; return s;
}
/* LA HIRE: a contração C^i_k = A^i_j B^j_k (o produto) */
static inline void lahire_contr(int n, const double *A, const double *B, double *C) {
    for (int i = 0; i < n; i++) for (int k = 0; k < n; k++) {
        double s = 0; for (int j = 0; j < n; j++) s += A[i*n+j]*B[j*n+k]; C[i*n+k] = s;
    }
}
/* PONTRYAGIN: baixar o índice x_i = g_ij x^j (o dual pela métrica) */
static inline void pontryagin_baixa(int n, const double *g, const double *x, double *xl) {
    for (int i = 0; i < n; i++) { double s = 0; for (int j = 0; j < n; j++) s += g[i*n+j]*x[j]; xl[i] = s; }
}
/* o pairing dual <x,y> = x_i y^i = g_ij x^i y^j (Pontryagin em índice) */
static inline double pairing(int n, const double *g, const double *x, const double *y) {
    double s = 0; for (int i = 0; i < n; i++) for (int j = 0; j < n; j++) s += g[i*n+j]*x[i]*y[j]; return s;
}


/* ===================================================================
 * (D) A TRANSFORMADA UNIVERSAL — 𝒢 = ℱ∘ℒ, em corpo finito
 *
 * A transformada nao e' definida: ela CAI DO DUAL. Os caracteres sao a orbita de um
 * unico elemento — a TORCAO —, e o emparelhamento com eles E' a transformada.
 * Trocar o grupo troca a transformada sem trocar a conta:
 *      grupo ADITIVO (Z/n, +)   -> Fourier
 *      grupo MULTIPLICATIVO     -> Mellin
 * e a segunda e' a primeira conjugada pelo log — o que o paper chama de dourada.
 *
 * Em corpo finito tudo isto e' EXATO: a raiz da unidade e' inteira, nao ha' seno,
 * cosseno nem epsilon. Parseval, a inversibilidade e a ortogonalidade sao igualdades
 * de inteiros. (Medido em universe/tools/{dourada_exata,ortogonalizacao}.c)
 * =================================================================== */

typedef struct { u64 p, g, n, w, ninv; } Univ;   /* corpo, gerador, ordem, raiz, 1/n */

static inline u64 u_md (const Univ *U, i64 x) { i64 r = x % (i64)U->p; return (u64)(r < 0 ? r + (i64)U->p : r); }
static inline u64 u_mul(const Univ *U, u64 a, u64 b) { return (u64)(((unsigned __int128)a * b) % U->p); }
static inline u64 u_pot(const Univ *U, u64 b, u64 e) {
    u64 r = 1; b %= U->p;
    while (e) { if (e & 1) r = u_mul(U, r, b); b = u_mul(U, b, b); e >>= 1; }
    return r;
}
static inline u64 u_inv(const Univ *U, u64 a) { return u_pot(U, a, U->p - 2); }

/* prepara o corpo: p primo, g gerador, n | p-1. A torcao e' w = g^((p-1)/n). */
static inline Univ universal_init(u64 p, u64 g, u64 n) {
    Univ U; U.p = p; U.g = g; U.n = n;
    U.w = u_pot(&U, g, (p - 1) / n);
    U.ninv = u_inv(&U, n % p);
    return U;
}

/* O CARACTERE: chi_k(j) = w^{jk}. E' a orbita da torcao — a base ortogonal, e ela
 * nao se escolhe: gera-se. <chi_a, chi_b> = n·delta_ab, exato. */
static inline u64 universal_chi(const Univ *U, i64 k, i64 j) {
    i64 e = (k * j) % (i64)U->n; if (e < 0) e += (i64)U->n;
    return u_pot(U, U->w, (u64)e);
}

/* ℱ — a transformada: o emparelhamento com os caracteres. */
static inline void universal_F(const Univ *U, const u64 *x, u64 *X) {
    for (u64 k = 0; k < U->n; k++) {
        u64 acc = 0;
        for (u64 j = 0; j < U->n; j++)
            acc = u_md(U, (i64)(acc + u_mul(U, x[j], universal_chi(U, (i64)k, (i64)j))));
        X[k] = acc;
    }
}
/* ℱ⁻¹ — a volta. Exata: nenhum residuo. */
static inline void universal_Finv(const Univ *U, const u64 *X, u64 *x) {
    for (u64 j = 0; j < U->n; j++) {
        u64 acc = 0;
        for (u64 k = 0; k < U->n; k++)
            acc = u_md(U, (i64)(acc + u_mul(U, X[k], universal_chi(U, -(i64)k, (i64)j))));
        x[j] = u_mul(U, acc, U->ninv);
    }
}

/* MELLIN — o mesmo, no grupo MULTIPLICATIVO: soma sobre a grade geometrica t=h^u.
 * E' Fourier conjugada pelo log, e por isso da' o MESMO resultado (medido, 16 de 16). */
static inline void universal_M(const Univ *U, const u64 *fx, u64 *M) {
    for (u64 s = 0; s < U->n; s++) {
        u64 acc = 0;
        for (u64 u = 0; u < U->n; u++) {
            u64 t = u_pot(U, U->w, u);                 /* a grade geometrica */
            acc = u_md(U, (i64)(acc + u_mul(U, fx[u], u_pot(U, t, s))));
        }
        M[s] = acc;
    }
}

/* PARSEVAL: <x,y> = Σ_k X_k Y_{-k}. A isometria, por igualdade de inteiros. */
static inline u64 universal_dot(const Univ *U, const u64 *a, const u64 *b) {
    u64 s = 0;
    for (u64 i = 0; i < U->n; i++) s = u_md(U, (i64)(s + u_mul(U, a[i], b[i])));
    return s;
}

/* DIRAC: somar a orbita inteira concentra num ponto. Σ_k chi_k(j) = n·delta_{j,0} */
static inline u64 universal_dirac(const Univ *U, i64 j) {
    u64 s = 0;
    for (u64 k = 0; k < U->n; k++) s = u_md(U, (i64)(s + universal_chi(U, (i64)k, j)));
    return s;
}

#endif /* UNIVERSE_TOOLS_API_H */