/*
 * neurônio DIGITAL — a DUALIDADE do gato (o circuito fractal), em BITS.
 *
 * O gato A_m=[[m,1],[1,0]] tem dois atratores DUAIS, os dois lados do corpo:
 *   NEGRO  σ  = (m+√(m²+4))/2   |σ|>1   o sorvedouro — ×σ,  a CONVOLUÇÃO  (a fala cai, SOBE a torre)
 *   BRANCO σ' = −1/σ            |σ'|<1  a fonte       — ×σ', a DECONVOLUÇÃO (a resposta emana, DESCE)
 * com σσ'=−1 (a mão que segura: o det, a área) e σ+σ'=m. O metal é σ_m=[m;m,m,…]
 * (m=1 ouro φ, m=2 prata, m=3 bronze). O gato SOBE a torre; o esquilo A⁻¹ a DESCE — e
 * uma desfaz a outra: gato∘esquilo = id (a volta exata). Par de neuronio_analog.c (correntes).
 *
 *   ⊕ cisão    b agrupado mod n        ∑ soma  popcount (o Kirchhoff)
 *   ⊗ gato ×σ  A_n (companion):  d_0 = m·c_0 + c_{n-1},  d_i = c_{i-1}   — sobe (convolução)
 *   ⊘ esquilo  A_n⁻¹ (×σ'=÷σ):   c_j = d_{j+1},  c_{n-1} = d_0 − m·d_1   — desce (deconvolução)
 *
 * Uso: ./neuronio CAMINHO [m] [K] [N]     (m=metal=1, K=altura temporal=12, N=dim máxima=8)
 */
#include <stdio.h>
#include "unidade.h"
#include <stdlib.h>
#include <math.h>

static const char *metal(long m) {
    switch (m) { case 1: return "ouro (φ)"; case 2: return "prata"; case 3: return "bronze"; default: return "metal m"; }
}
/* ⊗ o gato A_n = ×σ (companion de pₙ): d_0 = m·c_0 + c_{n-1},  d_i = c_{i-1}  — SOBE (o negro) */
static void gato_n(long *c, int n, long m) {
    long v0 = c[0], vn1 = c[n-1];
    for (int i = n-1; i >= 1; i--) c[i] = c[i-1];
    c[0] = m*v0 + vn1;
}
/* ⊘ o esquilo A_n⁻¹ = ×σ' = ÷σ (o dual): c_j = d_{j+1},  c_{n-1} = d_0 − m·d_1  — DESCE (o branco) */
static void esquilo_n(long *d, int n, long m) {
    long d0 = d[0], d1 = d[1];
    for (int i = 0; i < n-1; i++) d[i] = d[i+1];
    d[n-1] = d0 - m*d1;
}

int main(int argc, char **argv)
{
    if (argc < 2) return 1;
    FILE *f = fopen(argv[1], "rb");
    if (!f) return 1;
    long m = (argc > 2) ? atol(argv[2]) : 1;
    int K = (argc > 3) ? atoi(argv[3]) : 12;
    int N = (argc > 4) ? atoi(argv[4]) : 8;
    if (m < 1) m = 1; if (N < 2) N = 2; if (N > 8) N = 8;

    long b[8] = {0};
    int ch;
    while ((ch = fgetc(f)) != EOF)
        for (int p = 0; p < 8; p++) b[p] += (ch >> p) & 1;   /* ∑ o Kirchhoff: bits por posição */
    fclose(f);

    double s = (m + sqrt((double)m*m + 4)) / 2, sl = -1.0/s;
    printf("metal m=%ld (%s): NEGRO σ=%.6f (sobe, ×σ) · BRANCO σ'=−1/σ=%.6f (desce, ×σ') · σσ'=%.1f\n",
           m, metal(m), s, sl, s*sl);

    /* torre TEMPORAL: o gato Aᵏ·c em R², a razão → σ (o negro, o sorvedouro) */
    printf("torre temporal (gato Aᵏ, R², a razão → σ):\n");
    long c[8]; c[0] = b[0]+b[2]+b[4]+b[6]; c[1] = b[1]+b[3]+b[5]+b[7];   /* ⊕ pares, ímpares */
    for (int k = 0; k <= K; k++) {
        printf("  k=%-2d [%ld,%ld] %.9f\n", k, c[0], c[1], c[1] ? (double)c[0]/c[1] : 0.0);
        gato_n(c, 2, m);
    }

    int volta_falhou = 0;
    /* a DUALIDADE em Rⁿ: o gato ×σ sobe (convolução), o esquilo ×σ' desce (deconvolução), a volta fecha */
    printf("dualidade (Rⁿ: gato ×σ sobe → esquilo ×σ' desce → gato∘esquilo=id):\n");
    for (int n = 2; n <= N; n++) {
        long cc[8] = {0}, orig[8], up[8];
        for (int p = 0; p < 8; p++) cc[p % n] += b[p];       /* ⊕ cisão em n fases */
        for (int i = 0; i < n; i++) orig[i] = cc[i];
        gato_n(cc, n, m);    for (int i = 0; i < n; i++) up[i] = cc[i];   /* ⊗ sobe */
        esquilo_n(cc, n, m);                                             /* ⊘ desce */
        int volta = 1; for (int i = 0; i < n; i++) if (cc[i] != orig[i]) volta = 0;
        if(!volta) volta_falhou++;
        printf("  R%-2d [", n);   for (int i = 0; i < n; i++) printf("%ld%s", orig[i], i<n-1?",":"");
        printf("] →σ→ [");        for (int i = 0; i < n; i++) printf("%ld%s", up[i],   i<n-1?",":"");
        printf("] →σ'→ volta: %s\n", volta ? "sim (id)" : "NÃO");
    }

    /* Antes isto imprimia a volta linha a linha e NÃO concluía nada: saía 0 sempre, mesmo
     * quando alguma dimensão não fechava. Agora a volta é acumulada e afirmada. */
    ok("gato∘esquilo = id: a volta fecha em toda dimensão", volta_falhou == 0);
    return falhas ? 1 : 0;
}
