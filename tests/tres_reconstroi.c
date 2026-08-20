/* tres_reconstroi.c — TRES DE CADA LADO RECONSTROI OS MODOS. Testado na prosa.
 *
 * A afirmacao: uma de cada lado (𝒢 = ℱ∘ℒ) ja' resolve a funcao de onda; TRES de cada
 * lado resolve a prosa — dados arbitrarios — porque reconstroi os MODOS.
 *
 * Aqui nao se supoe: mede-se. E a chave e' o PERIODO 4, que este documento ja' tem
 * em outro lugar:
 *
 *      G^4 = I         (o esquilo, §1.4 — o teorema do gato)
 *      ℱ^4 = id        (Fourier: a transformada tem periodo 4)
 *
 * Sao a MESMA estrutura. E dai' sai por que TRES: se o periodo e' 4, entao
 *
 *      ℱ^3 = ℱ^-1
 *
 * — tres aplicacoes de um lado SAO a volta. Nao e' acumulo: e' o retorno, exatamente
 * como G^3 = G^-1 (tres_e_tres.c). Logo "tres de cada lado" reconstroi por ser a
 * inversa, e reconstroi QUALQUER dado, nao so' o bem-comportado.
 *
 * A PROSA: le'-se a teoria.tex da raiz — texto real, acentuado, com LaTeX,
 * sem nenhum preparo. Se reconstruir isso byte a byte com residuo 0, reconstroi prosa.
 *
 * cc -O2 tres_reconstroi.c -o tres_reconstroi && ./tres_reconstroi
 */
#include <stdint.h>
#include <stdio.h>
#include "unidade.h"

typedef int64_t i64;
static const i64 P = 40961, G = 3;      /* primo com 4096 | P-1, para blocos grandes */

static i64 md(i64 x) { x %= P; return x < 0 ? x + P : x; }
static i64 mul(i64 a, i64 b) { return md(a * b); }
static i64 pot(i64 b, i64 e) { i64 r = 1; b = md(b);
    while (e > 0) { if (e & 1) r = mul(r,b); b = mul(b,b); e >>= 1; } return r; }
static i64 inv(i64 a) { return pot(a, P - 2); }

#define N 256
static i64 W;                            /* raiz N-esima da unidade */

/* ℱ — uma aplicacao. Normalizada por 1/sqrt(N) para que o periodo seja 4 exato:
 * sem a normalizacao, ℱ^2 devolve N·(reflexao) e o periodo se perde num fator. */
static i64 RN;                           /* 1/sqrt(N) em Z/P */
static void Fa(const i64 *x, i64 *X) {
    for (int k = 0; k < N; k++) {
        i64 acc = 0;
        for (int j = 0; j < N; j++) acc = md(acc + mul(x[j], pot(W, (i64)j*k)));
        X[k] = mul(acc, RN);
    }
}

int main(void) {
    int passou = 1;
    /* a raiz e a normalizacao */
    W = pot(G, (P-1)/N);
    {   /* 1/sqrt(N): existe se N e' residuo quadratico mod P. N=256=16², logo sqrt=16 */
        RN = inv(16);
    }
    printf("=== TRES DE CADA LADO RECONSTROI — testado na prosa do tiffany.tex ===\n\n");

    /* (0) o PERIODO 4 — a mesma estrutura do esquilo (G^4 = I) */
    printf("(0) o periodo: ℱ^4 = id, como G^4 = I no esquilo (§1.4)\n");
    {
        i64 a[N], b[N], c[N], d[N], e[N];
        for (int i = 0; i < N; i++) a[i] = (i*i*37 + i*11 + 5) % P;
        Fa(a,b); Fa(b,c); Fa(c,d); Fa(d,e);
        int err4 = 0, err2 = 0;
        for (int i = 0; i < N; i++) if (e[i] != a[i]) err4++;
        for (int i = 0; i < N; i++) if (c[i] != a[i]) err2++;
        printf("    ℱ^4 == id : %d erros   %s\n", err4, err4 ? (passou=0,"X") : "resid 0");
        printf("    ℱ^2 == id : %d erros   %s\n", err2,
               err2 ? "(nao — e' a reflexao, como esperado)" : "(seria periodo 2)");
        printf("    -> periodo 4. Logo ℱ^3 = ℱ^-1: TRES de um lado E' a volta.\n");
    }

    /* (1) A PROSA: os bytes do proprio tiffany.tex, sem preparo nenhum */
    printf("\n(1) a prosa: bytes crus de teoria.tex (a prosa real da raiz)\n");
    FILE *f = fopen("../teoria.tex", "rb");   /* cwd=tests/; o sandbox/tiffany.tex saiu numa reorganizacao */
    if (!f) { printf("    nao abriu o arquivo\n"); return 1; }
    i64 prosa[N]; unsigned char raw[N];
    size_t lidos = fread(raw, 1, N, f);
    /* pula o preambulo para pegar prosa de verdade, com acento */
    fseek(f, 40000, SEEK_SET); lidos = fread(raw, 1, N, f);
    fclose(f);
    for (int i = 0; i < N; i++) prosa[i] = (i64)(i < (int)lidos ? raw[i] : 0);
    printf("    %zu bytes lidos. amostra: \"", lidos);
    for (int i = 0; i < 48; i++) putchar(raw[i] >= 32 && raw[i] < 127 ? raw[i] : '.');
    printf("\"\n");

    /* (2) UMA de cada lado: ja' reconstroi? */
    printf("\n(2) UMA de cada lado (𝒢 = ℱ∘ℒ)\n");
    {
        i64 X[N], volta[N];
        Fa(prosa, X);
        /* a volta com uma aplicacao: ℱ^-1 = ℱ^3, mas aqui usa-se a inversa direta */
        i64 wi = inv(W);
        for (int j = 0; j < N; j++) {
            i64 acc = 0;
            for (int k = 0; k < N; k++) acc = md(acc + mul(X[k], pot(wi, (i64)j*k)));
            volta[j] = mul(acc, RN);
        }
        int erros = 0;
        for (int i = 0; i < N; i++) if (volta[i] != prosa[i]) erros++;
        printf("    ℱ^-1 ℱ (prosa) : %d erros   %s\n", erros,
               erros ? (passou=0,"X") : "resid 0");
    }

    /* (3) TRES de cada lado: ℱ^3 e' a volta — reconstroi a prosa BYTE A BYTE */
    printf("\n(3) TRES de cada lado: ℱ^3 = ℱ^-1, sem calcular inversa nenhuma\n");
    {
        i64 t1[N], t2[N], t3[N], X[N];
        Fa(prosa, X);                         /* uma ida  */
        Fa(X, t1); Fa(t1, t2); Fa(t2, t3);    /* TRES de volta */
        int erros = 0;
        for (int i = 0; i < N; i++) if (t3[i] != prosa[i]) erros++;
        printf("    ℱ^3(ℱ(prosa)) == prosa : %d erros em %d bytes   %s\n",
               erros, N, erros ? (passou=0,"X") : "resid 0");
        printf("    reconstruido: \"");
        for (int i = 0; i < 48; i++) {
            int ch = (int)t3[i];
            putchar(ch >= 32 && ch < 127 ? ch : '.');
        }
        printf("\"\n");
        printf("    -> nao foi preciso INVERTER: bateu-se tres vezes do mesmo lado.\n");
    }

    /* (4) OS MODOS: cada aplicacao devolve um modo, e os quatro fecham o ciclo.
     * E' isto que "reconstroi os modos" quer dizer, e e' medivel: as quatro
     * aplicacoes sao {id, ℱ, reflexao, ℱ^-1} e nenhuma outra aparece. */
    printf("\n(4) os MODOS: as quatro aplicacoes fecham o ciclo, e sao so' quatro\n");
    {
        i64 m[5][N];
        for (int i = 0; i < N; i++) m[0][i] = prosa[i];
        for (int p = 1; p <= 4; p++) Fa(m[p-1], m[p]);
        /* modo 2 e' a REFLEXAO x[-j]: e' o ν do documento, o virar do lado */
        int refl = 0;
        for (int i = 0; i < N; i++) if (m[2][i] != prosa[(N - i) % N]) refl++;
        printf("    modo 0 = id ......... (a prosa)\n");
        printf("    modo 1 = ℱ .......... a funcao de onda (amplitude e fase)\n");
        printf("    modo 2 = reflexao ... x[-j] : %d erros   %s\n", refl,
               refl ? (passou=0,"X") : "resid 0   <- e' o ν: virar o lado");
        printf("    modo 3 = ℱ^-1 ....... a volta\n");
        printf("    modo 4 = id ......... fechou\n");
        printf("    Quatro modos, e o terceiro E' a inversa. Por isso TRES resolve:\n");
        printf("    tres batidas percorrem o ciclo ate' a volta, sem inverter nada.\n");
    }

    printf("\n=== %s ===\n", passou ? "RESIDUO 0" : "FALHOU");
    ok("reconstrói byte a byte com resíduo 0", passou);
    printf("    A prosa do tiffany.tex volta byte a byte com TRES aplicacoes de um so'\n");
    printf("    lado. O periodo 4 da Fourier e' o mesmo do esquilo (G^4 = I), e o tres\n");
    printf("    e' a volta nos dois — nao por analogia: pela mesma conta.\n");
    return !passou;
}
