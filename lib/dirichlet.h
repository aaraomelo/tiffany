/* dirichlet.h — A CONVOLUÇÃO SOBRE A ÁRVORE DOS DIVISORES, E μ = 1⁻¹.
 *
 *  ordem do coordenador pede o pacote nesta ordem, e a ordem é o argumento:
 *
 *   funções aritméticas → convolução → multiplicativas → μ como INVERSOR → inversão
 *   → série de Dirichlet
 *
 * E o alvo é esta frase:
 *
 *     μ * 1 = ε      isto é      μ = 1⁻¹
 *
 * «na álgebra da convolução de Dirichlet» — e é aqui que a ideia de INVERSÃO desta casa
 * fica matemática limpa: o μ não é uma função esquisita com sinais, é o INVERSO da
 * função constante 1, e a identidade Σ_{d|n} μ(d) = ε(n) é a definição de inverso a
 * ser cumprida. A inversão de Möbius é, literalmente, a DECONVOLUÇÃO:
 *
 *     F = f * 1   ⟺   f = F * μ
 *
 * ── O CONTRADOMÍNIO, E PORQUE NÃO ENTRA DECIMAL ────────────────────────────────
 * Ele escreve f: ℕ → ℂ. As funções deste andar — 1, id, φ, μ, ε, σ, τ — são TODAS
 * inteiras, e a convolução de inteiras é inteira. Então a álgebra corre exata em `long`
 * e o ℂ da definição nunca chega a ser preciso. Não é uma restrição: é a observação de
 * que o andar vive em ℤ.
 *
 * ── E A SÉRIE DE DIRICHLET É FORMAL ────────────────────────────────────────────
 * D_f(s) = Σ f(n)/nˢ, e D_{f*g} = D_f·D_g. Avaliar isto num s pedia análise e decimais;
 * mas a identidade NÃO é sobre o valor da soma — é sobre os COEFICIENTES. No produto
 * formal, o coeficiente de 1/kˢ é Σ_{nm=k} f(n)g(m), que é (f*g)(k) por definição. O s
 * nunca se avalia, e por isso a identidade mede-se EXATA, termo a termo.
 *
 * Precisa de `inteiros.h`, `naturais.h` e `numeros.h`. */
#ifndef DIRICHLET_H
#define DIRICHLET_H

#define DL_MAX 240
typedef struct { long v[DL_MAX + 1]; } Arit;   /* f(1..DL_MAX), indexada a partir de 1 */

/* ── as funções do ficheiro: 1, id, φ, μ, e o neutro ε ─────────────────────────── */
static void dl_um (Arit *f){ for(long n = 1; n <= DL_MAX; n++) f->v[n] = 1; }
static void dl_id (Arit *f){ for(long n = 1; n <= DL_MAX; n++) f->v[n] = n; }
static void dl_phi(Arit *f){ for(long n = 1; n <= DL_MAX; n++) f->v[n] = nm_phi(n); }
static void dl_mu (Arit *f){ for(long n = 1; n <= DL_MAX; n++) f->v[n] = nm_mu(n); }
static void dl_eps(Arit *f){ for(long n = 1; n <= DL_MAX; n++) f->v[n] = (n == 1); }
/* e mais duas que aparecem sozinhas: τ = 1*1 (nº de divisores) e σ = id*1 (soma) */
static void dl_tau(Arit *f){
    for(long n = 1; n <= DL_MAX; n++){ f->v[n] = 0; for(long d = 1; d <= n; d++) if(n % d == 0) f->v[n]++; }
}
static void dl_sigma(Arit *f){
    for(long n = 1; n <= DL_MAX; n++){ f->v[n] = 0; for(long d = 1; d <= n; d++) if(n % d == 0) f->v[n] += d; }
}
/* ── A CONVOLUÇÃO DE DIRICHLET: a multiplicação sobre a ÁRVORE DOS DIVISORES ────
 *     (f*g)(n) = Σ_{d|n} f(d)·g(n/d)
 * É o produto do andar, e não o ponto-a-ponto: cada n consulta a sua árvore inteira. */
static void dl_conv(const Arit *f, const Arit *g, Arit *h){
    for(long n = 1; n <= DL_MAX; n++){
        long s = 0;
        for(long d = 1; d <= n; d++) if(n % d == 0) s += f->v[d] * g->v[n/d];
        h->v[n] = s;
    }
}
static int dl_igual(const Arit *f, const Arit *g, long ate){
    for(long n = 1; n <= ate; n++) if(f->v[n] != g->v[n]) return 0;
    return 1;
}
/* ── MULTIPLICATIVA: f(mn) = f(m)f(n) quando gcd(m,n) = 1 ──────────────────────
 * Devolve 0 e escreve o par que a derruba — a testemunha do contra-exemplo. */
static int dl_multiplicativa(const Arit *f, long ate, long *pm, long *pn){
    if(pm) *pm = 1;                            /* A TESTEMUNHA ESCREVE-SE SEMPRE: sair sem
                                                * a preencher fazia o chamador imprimir
                                                * memória por estrear, e o texto mentia
                                                * com a asserção verde. */
    if(pn) *pn = 1;
    if(f->v[1] != 1) return 0;                 /* toda multiplicativa tem f(1) = 1, e o
                                                * par (1,1) é a testemunha disso mesmo */
    for(long m = 1; m <= ate; m++) for(long n = 1; m*n <= ate && n <= ate; n++){
        if(iz_gcd(m, n, 0, 0) != 1) continue;
        if(f->v[m*n] != f->v[m] * f->v[n]){
            if(pm) *pm = m;
            if(pn) *pn = n;
            return 0;
        }
    }
    return 1;
}
/* ── A INVERSÃO DE MÖBIUS — a DECONVOLUÇÃO deste andar ─────────────────────────
 *     F(n) = Σ_{d|n} f(d)      ⟹      f(n) = Σ_{d|n} μ(d)·F(n/d)
 * A primeira é f*1; a segunda é F*μ. Como μ = 1⁻¹, uma desfaz a outra — e é isso que a
 * palavra «inversão» quer dizer aqui, sem metáfora. */
static void dl_soma_divisores(const Arit *f, Arit *F){
    for(long n = 1; n <= DL_MAX; n++){
        long s = 0;
        for(long d = 1; d <= n; d++) if(n % d == 0) s += f->v[d];
        F->v[n] = s;
    }
}
static void dl_inverte(const Arit *F, Arit *f){
    for(long n = 1; n <= DL_MAX; n++){
        long s = 0;
        for(long d = 1; d <= n; d++) if(n % d == 0) s += nm_mu(d) * F->v[n/d];
        f->v[n] = s;
    }
}
/* ── A SÉRIE DE DIRICHLET, FORMAL ──────────────────────────────────────────────
 * O coeficiente de 1/kˢ no produto D_f·D_g é Σ_{nm=k} f(n)g(m). O s não se avalia. */
static long dl_coef_produto(const Arit *f, const Arit *g, long k){
    long s = 0;
    for(long n = 1; n <= k; n++) if(k % n == 0) s += f->v[n] * g->v[k/n];
    return s;
}
#endif
