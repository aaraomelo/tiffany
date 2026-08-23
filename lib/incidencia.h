/* ═══════════════════════════════════════════════════════════════════════════
 * lib/incidencia.h — S, ζ, μ: o deslocamento gera a soma e a inversão
 *
 * Um operador e o trio inteiro sai dele:
 *
 *     S    a subdiagonal de uns          S_ij = [i = j+1]
 *     ζ    a série de S                  ζ = Σ_{j≥0} S^j   (a triangular de uns)
 *     μ    a diferença finita            μ = 1 − S
 *     μζ = ζμ = id                       (aranha thm:shift)
 *
 * A série é FINITA porque S é nilpotente, e o trio vive na face Δ = 0 --- a
 * mesma nilpotência do corpo exterior. É por isso que o inverso sai sem fração.
 *
 * Tudo em inteiros, e sem alocar: o cliente dá o vector e o tamanho.
 * Medido em `tests/incidencia.c` e `tests/pgwire.c` §W153.
 * ═══════════════════════════════════════════════════════════════════════════ */
#ifndef INCIDENCIA_H
#define INCIDENCIA_H

#define IN_MAX 64

/* ── ζ: a acumulação, (ζa)(t) = Σ_{u≤t} a(u) ─────────────────────────────
 * É a convolução pela triangular de uns, feita num passo por índice. */
static void in_zeta(const long *a, long *out, int n){
    long acc = 0;
    for(int i = 0; i < n; i++){ acc += a[i]; out[i] = acc; }
}

/* ── μ: a diferença finita, (μa)(t) = a(t) − a(t−1) ─────────────────────
 * É 1 − S, e desfaz ζ exactamente --- sem divisão, sem resto. */
static void in_mu(const long *a, long *out, int n){
    for(int i = n - 1; i > 0; i--) out[i] = a[i] - a[i-1];
    if(n > 0) out[0] = a[0];
}

/* ── S: o deslocamento, (Sa)(t) = a(t−1) ────────────────────────────────── */
static void in_desloca(const long *a, long *out, int n){
    for(int i = n - 1; i > 0; i--) out[i] = a[i-1];
    if(n > 0) out[0] = 0;
}

/* ── O grau de nilpotência: S^k = 0 exactamente em k = n ─────────────────
 * É ele que faz a série de ζ terminar, e devolve-se para o cliente saber
 * quantos termos a série tem. */
static int in_grau_nilpotente(int n){ return n; }

/* ── A VOLTA, verificada: μ(ζa) = a e ζ(μa) = a ──────────────────────────
 * Devolve 1 se a volta é exacta em todo o vector. Não é decoração: é a lei
 * que garante ao cliente que acumular e diferenciar não perde nada. */
static int in_volta_exacta(const long *a, int n){
    long z[IN_MAX], v[IN_MAX];
    if(n > IN_MAX) return 0;
    in_zeta(a, z, n);  in_mu(z, v, n);
    for(int i = 0; i < n; i++) if(v[i] != a[i]) return 0;
    in_mu(a, z, n);    in_zeta(z, v, n);
    for(int i = 0; i < n; i++) if(v[i] != a[i]) return 0;
    return 1;
}

/* ── E O GUME, que o cliente pode chamar: com o deslocamento CÍCLICO a série
 * não fecha. Devolve 1 se o cíclico volta à identidade em n passos --- e nesse
 * caso 1 − S é singular, pelo que não há inversa. */
static int in_ciclico_nao_fecha(int n){
    /* S_cic^n = I: a soma de cada linha de 1 − S_cic é zero, logo é singular */
    (void)n;
    return 1;
}

#endif /* INCIDENCIA_H */
