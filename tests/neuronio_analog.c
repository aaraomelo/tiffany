/*
 * neurônio ANALÓGICO — a DUALIDADE do gato, em CORRENTES (par de neuronio.c).
 *
 * O gato e o esquilo são os dois lados do corpo, e é a MESMA peça com o sinal trocado (o
 * espelho 𝒥, como no microprocessador.tex §1: ANTILOG(log a + s·log b − s·log ref)):
 *   ⊗ gato ×σ    (o negro, s=+1, a CONVOLUÇÃO)  d_0 = m·c_0 + c_{n-1}   — SOBE
 *   ⊘ esquilo ×σ' (o branco, s=−1, a DECONVOLUÇÃO) c_{n-1} = d_0 − m·d_1  — DESCE
 * σσ'=−1 (a mão que segura). O ganho m·(·) é o TRANSLINEAR (log/exp, §B.4); a soma/subtração
 * é o Kirchhoff (§B.5). O oráculo é o gato/esquilo DIGITAL: as correntes têm de bater —
 * resíduo 0. E gato∘esquilo = id (a volta exata): a deconvolução desfaz a convolução.
 *
 * Uso: ./neuronio_analog CAMINHO [m] [K] [N]     cc -O2 neuronio_analog.c -lm -o neuronio_analog
 */
#include <stdio.h>
#include "unidade.h"
#include <stdlib.h>
#include <math.h>

/* os modelos físicos (analog.c §B.2/B.4): a junção b-e I_c=I_s·e^{V_be/V_T}, o translinear */
static const double I_S = 1e-15, IU = 1e-9, VT = 0.025852;   /* ~25.85 mV a 300 K */
static double tl_mul(double a, double b) {                    /* ANTILOG(log a+log b−log ref)=a·b */
    double V1 = VT*log(a*IU/I_S), V2 = VT*log(b*IU/I_S), Vr = VT*log(IU/I_S);
    return I_S*exp((V1 + V2 - Vr)/VT) / IU;
}
static const char *metal(long m) {
    switch (m) { case 1: return "ouro (φ)"; case 2: return "prata"; case 3: return "bronze"; default: return "metal m"; }
}

/* ⊗ o gato ×σ em correntes: d_0 = m·c_0 (translinear) + c_{n-1} (Kirchhoff); d_i = c_{i-1}  — SOBE */
static void gato_analog(double *c, int n, double m) {
    double v0 = c[0], vn1 = c[n-1];
    for (int i = n-1; i >= 1; i--) c[i] = c[i-1];
    c[0] = tl_mul(m, v0) + vn1;
}
/* ⊘ o esquilo ×σ' em correntes: c_j = d_{j+1}; c_{n-1} = d_0 − m·d_1 (translinear subtraído) — DESCE */
static void esquilo_analog(double *d, int n, double m) {
    double d0 = d[0], d1 = d[1];
    for (int i = 0; i < n-1; i++) d[i] = d[i+1];
    d[n-1] = d0 - tl_mul(m, d1);
}
/* os mesmos, DIGITAIS (o oráculo, inteiros) */
static void gato_int(long *c, int n, long m)    { long v0=c[0],vn1=c[n-1]; for(int i=n-1;i>=1;i--) c[i]=c[i-1]; c[0]=m*v0+vn1; }
static void esquilo_int(long *d, int n, long m) { long d0=d[0],d1=d[1];    for(int i=0;i<n-1;i++) d[i]=d[i+1]; d[n-1]=d0-m*d1; }

int main(int argc, char **argv)
{
    if (argc < 2) return 1;
    FILE *f = fopen(argv[1], "rb");
    if (!f) return 1;
    long mi = (argc > 2) ? atol(argv[2]) : 1;
    int N = (argc > 4) ? atoi(argv[4]) : 8;
    if (mi < 1) mi = 1; if (N < 2) N = 2; if (N > 8) N = 8;
    double m = (double)mi;

    long b[8] = {0};
    int ch;
    while ((ch = fgetc(f)) != EOF)
        for (int p = 0; p < 8; p++) b[p] += (ch >> p) & 1;   /* ∑ Kirchhoff: bits por posição */
    fclose(f);

    double s = (mi + sqrt((double)mi*mi + 4)) / 2, sl = -1.0/s;
    long viol = 0, volta_falha = 0;
    printf("metal m=%ld (%s): NEGRO σ=%.6f · BRANCO σ'=−1/σ=%.6f · σσ'=%.1f\n", mi, metal(mi), s, sl, s*sl);
    printf("dualidade em correntes (gato ×σ sobe → esquilo ×σ' desce ; oráculo = o digital):\n");

    for (int n = 2; n <= N; n++) {
        double cc[8], up[8]; long di[8], orig[8];
        for (int i = 0; i < n; i++) {
            long v = 0; for (int p = 0; p < 8; p++) if (p % n == i) v += b[p];   /* ⊕ cisão em n fases */
            cc[i] = (double)v; di[i] = v; orig[i] = v;
        }
        gato_analog(cc, n, m);  gato_int(di, n, mi);          /* ⊗ sobe (analógico e oráculo) */
        for (int i = 0; i < n; i++) up[i] = cc[i];
        int gok = 1; for (int i = 0; i < n; i++) if (fabs(cc[i]-(double)di[i]) > 1e-6) { viol++; gok=0; }
        esquilo_analog(cc, n, m);                              /* ⊘ desce */
        int volta = 1; for (int i = 0; i < n; i++) if (fabs(cc[i]-(double)orig[i]) > 1e-6) { volta=0; volta_falha++; }
        printf("  R%-2d →σ→ [", n);
        for (int i = 0; i < n; i++) printf("%.1f%s", up[i], i<n-1?",":"");
        printf("] →σ'→ volta: %s  (gato==digital: %s)\n", volta?"sim (id)":"NÃO", gok?"sim":"NÃO");
    }

    printf("\n----------------------------------------------------------------\n");
    int ok = (viol == 0 && volta_falha == 0);
    printf("metal m=%ld  gato==digital: viol=%ld ; esquilo desfaz o gato (gato∘esquilo=id): viol=%ld\n",
           mi, viol, volta_falha);
    printf("resíduo total = %d   %s\n", ok ? 0 : 1, VD(!(ok), "A DUALIDADE FECHA — gato ×σ (negro) e esquilo ×σ' (branco), a MESMA peça, σσ'=−1"));
    return ok ? 0 : 1;
}
