/* ═══════════════════════════════════════════════════════════════════════════
 * lib/derivacao.h — d como DERIVAÇÃO, e a nilpotência que a liga ao exterior
 *
 *     d(fg) = f·dg + g·df        a regra do produto é a lei deste corpo
 *     d^{n+1} = 0 em grau n      o diferencial é o exterior SUBIDO DE ORDEM
 *
 * Ali ε² = 0; aqui d^{n+1} = 0, e a nilpotência é a mesma lei. O núcleo de d
 * são as constantes, pelo que a fibra da derivada é a classe «a menos de
 * constante» --- e é essa a dobra deste corpo.
 *
 * Polinómios por vector de coeficientes, em inteiros. Sem alocar.
 * Medido em `tests/metais.c` e `tests/pgwire.c` §W148.
 * ═══════════════════════════════════════════════════════════════════════════ */
#ifndef DERIVACAO_H
#define DERIVACAO_H

#define DV_MAX 32

/* d: (dp)_i = (i+1)·p_{i+1} */
static void dv_d(const long *p, long *out, int n){
    for(int i = 0; i + 1 < n; i++) out[i] = (long)(i+1) * p[i+1];
    if(n > 0) out[n-1] = 0;
}

/* o produto de polinómios, truncado em n */
static void dv_prod(const long *f, const long *g, long *out, int n){
    for(int i = 0; i < n; i++) out[i] = 0;
    for(int i = 0; i < n; i++) for(int j = 0; i + j < n; j++)
        out[i+j] += f[i]*g[j];
}

/* ── A LEI: d(fg) = f·dg + g·df. Devolve 1 se vale no par dado. ─────────── */
static int dv_regra_produto(const long *f, const long *g, int n){
    long fg[DV_MAX], dfg[DV_MAX], df[DV_MAX], dg[DV_MAX];
    long t1[DV_MAX], t2[DV_MAX];
    if(n > DV_MAX) return 0;
    dv_prod(f, g, fg, n);   dv_d(fg, dfg, n);
    dv_d(f, df, n);         dv_d(g, dg, n);
    dv_prod(f, dg, t1, n);  dv_prod(g, df, t2, n);
    for(int i = 0; i < n; i++) if(dfg[i] != t1[i] + t2[i]) return 0;
    return 1;
}

/* ── A NILPOTÊNCIA: quantas derivadas até zerar. Em grau g dá g+1. ──────── */
static int dv_grau_nilpotencia(const long *p, int n){
    long a[DV_MAX], b[DV_MAX];
    if(n > DV_MAX) return -1;
    for(int i = 0; i < n; i++) a[i] = p[i];
    for(int k = 0; k <= n; k++){
        int nulo = 1;
        for(int i = 0; i < n; i++) if(a[i]) nulo = 0;
        if(nulo) return k;
        dv_d(a, b, n);
        for(int i = 0; i < n; i++) a[i] = b[i];
    }
    return n + 1;
}

/* ── O NÚCLEO: dp = 0 sse p é constante --- a fibra é «a menos de constante» */
static int dv_no_nucleo(const long *p, int n){
    long d[DV_MAX];
    if(n > DV_MAX) return 0;
    dv_d(p, d, n);
    for(int i = 0; i < n; i++) if(d[i]) return 0;
    return 1;
}

#endif /* DERIVACAO_H */
