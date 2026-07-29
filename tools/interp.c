/* interp.c — INVESTIGAÇÃO da matemática: o conversor discreto→contínuo é a interpolação polinomial.
 *
 * Um sinal DISCRETO de n amostras y_i (nos pontos x_i) determina UM polinômio P(x)=Σ c_k x^k de grau
 * < n --- e esse polinômio É a forma CONTÍNUA (avalia-se em qualquer x real). No corpo, P(x) é um
 * elemento de ℝⁿ (grau < n = dimensão n): as coordenadas c_k. A ligação amostras↔coordenadas é a
 * matriz de VANDERMONDE V (V_{ik}=x_i^k): y = V c, e o conversor é c = V⁻¹ y (interpolar) seguido da
 * avaliação. Quando os x_i são raízes da unidade, V é a transformada de Fourier/NTT --- a transformada
 * universal do corpo. E a avaliação (a saída contínua) é HORNER, que é uma REALIMENTAÇÃO:
 *     acc ← acc·x + c_k   (o gato ×x realimentado, um coeficiente por volta).
 * Investiga e verifica: (1) P passa pelas amostras (resíduo 0 nos nós); (2) Horner = avaliação direta;
 * (3) a forma contínua entre as amostras.
 *
 *   cc -O2 -std=c99 interp.c -lm -o interp
 */
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

/* c = V⁻¹ y : resolve o sistema de Vandermonde V c = y (Gauss com pivô) → as coordenadas do corpo. */
static void interpola(int n, const double *x, const double *y, double *c) {
    double A[16][17];
    for (int i = 0; i < n; i++) { double p = 1; for (int k = 0; k < n; k++) { A[i][k] = p; p *= x[i]; } A[i][n] = y[i]; }
    for (int k = 0; k < n; k++) {
        int piv = k; for (int i = k+1; i < n; i++) if (fabs(A[i][k]) > fabs(A[piv][k])) piv = i;
        for (int j = 0; j <= n; j++) { double t = A[k][j]; A[k][j] = A[piv][j]; A[piv][j] = t; }
        for (int i = 0; i < n; i++) if (i != k) { double f = A[i][k]/A[k][k]; for (int j = k; j <= n; j++) A[i][j] -= f*A[k][j]; }
    }
    for (int k = 0; k < n; k++) c[k] = A[k][n]/A[k][k];
}
/* HORNER = a realimentação: P(x) = c_0 + x(c_1 + x(c_2 + … + x·c_{n-1})). acc ← acc·x + c_k. */
static double horner(int n, const double *c, double x) {
    double acc = c[n-1]; for (int k = n-2; k >= 0; k--) acc = acc*x + c[k]; return acc;
}
/* avaliação DIRETA (o oráculo): Σ c_k x^k, potência a potência. */
static double direto(int n, const double *c, double x) {
    double s = 0, p = 1; for (int k = 0; k < n; k++) { s += c[k]*p; p *= x; } return s;
}

int main(void) {
    printf("O CONVERSOR DISCRETO→CONTÍNUO É A INTERPOLAÇÃO POLINOMIAL (a forma contínua do sinal)\n");
    printf("================================================================================\n");

    /* um sinal DISCRETO: n amostras contínuas (reais) nos pontos x_i = 0..n-1                        */
    int n = 6;
    double x[6] = {0, 1, 2, 3, 4, 5};
    double y[6] = {2.0, 3.5, 1.0, 4.2, 2.8, 0.7};      /* as amostras — valores CONTÍNUOS            */
    double c[8]; interpola(n, x, y, c);

    printf("\n  sinal discreto (n=%d amostras): ", n);
    for (int i = 0; i < n; i++) printf("y[%d]=%.1f ", i, y[i]);
    printf("\n  → o polinômio P(x)=Σ c_k x^k (o elemento de ℝⁿ, coordenadas c):\n     c = (");
    for (int k = 0; k < n; k++) printf("%.4f%s", c[k], k<n-1?", ":"");
    printf(")\n");

    /* (1) P passa pelas amostras: P(x_i) = y_i (resíduo 0 nos nós)                                   */
    double pior_no = 0;
    for (int i = 0; i < n; i++) { double e = fabs(horner(n, c, x[i]) - y[i]); if (e > pior_no) pior_no = e; }
    printf("\n  (1) P(x_i) = y_i nos nós — o contínuo passa por cada amostra: erro máx %.1e (resíduo 0)\n", pior_no);

    /* (2) Horner (a realimentação) == avaliação direta, em x contínuo                                */
    double pior_h = 0;
    for (int t = 0; t <= 200; t++) { double xx = 5.0*t/200; double e = fabs(horner(n,c,xx)-direto(n,c,xx)); if (e>pior_h) pior_h=e; }
    printf("  (2) Horner (acc←acc·x+c_k, a realimentação) == avaliação direta: erro máx %.1e\n", pior_h);

    /* (3) a forma CONTÍNUA entre as amostras (x não-inteiro) — o sinal reconstruído                   */
    printf("  (3) a forma contínua P(x), inclusive entre as amostras (x contínuo):\n");
    for (double xx = 0; xx <= 5.001; xx += 0.5) {
        printf("      P(%.1f) = %+.4f%s\n", xx, horner(n,c,xx), (fabs(xx-round(xx))<1e-9)?"   ← amostra":"");
    }

    printf("\n  ⇒ o sinal discreto (as amostras) É o polinômio P(x) do corpo ℝⁿ (grau < n = dim n);\n");
    printf("    interpolar = c = V⁻¹y (a transformada de Vandermonde; raízes da unidade ⇒ Fourier/NTT);\n");
    printf("    a saída contínua = Horner = a REALIMENTAÇÃO (o gato ×x, um coeficiente por volta).\n");
    return (pior_no < 1e-9 && pior_h < 1e-9) ? 0 : 1;
}
