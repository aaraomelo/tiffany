/* ═══════════════════════════════════════════════════════════════════════════
 * lib/reguas.h — as réguas: uma por primo, a do prefixo, e a genérica
 *
 * A régua desta casa é a ultramétrica dos ENDEREÇOS (aranha def:arvore):
 *
 *     d(a,b) = 2^{−prof(a,b)},   prof = a primeira divergência
 *
 * e onde há VALORAÇÃO ela é ultramétrica e o produto vira soma; onde há NORMA
 * é arquimediana e o produto fica produto. O racional tem uma régua POR PRIMO.
 *
 * Devolve-se sempre a profundidade (um inteiro), nunca 2^{−q} --- porque a
 * potência sairia do degrau e o cliente compara profundidades, não decimais.
 * Medido em `tests/metais.c` e `tests/pgwire.c` §W147.
 * ═══════════════════════════════════════════════════════════════════════════ */
#ifndef REGUAS_H
#define REGUAS_H

#define RG_INF 62

/* ── A RÉGUA DO PREFIXO: prof = a primeira divergência de bits ───────────── */
static int rg_prefixo(long x, long y, int bits){
    if(x == y) return bits;
    for(int i = 0; i < bits; i++)
        if(((x >> (bits-1-i)) & 1L) != ((y >> (bits-1-i)) & 1L)) return i;
    return bits;
}

/* ── A RÉGUA p-ÁDICA: v_p(n) = quantas vezes p divide n ──────────────────
 * O racional tem uma destas por primo, e cada uma é ultramétrica. */
static int rg_vp(long n, long p){
    if(n == 0) return RG_INF;
    if(n < 0) n = -n;
    int v = 0;
    while(n % p == 0){ n /= p; v++; }
    return v;
}
/* a de um racional a/b */
static int rg_vp_rac(long a, long b, long p){
    if(a == 0) return RG_INF;
    return rg_vp(a, p) - rg_vp(b, p);
}

/* ── A DESIGUALDADE FORTE, verificável pelo cliente ──────────────────────
 * Devolve 1 se v(x−z) ≥ min{v(x−y), v(y−z)} --- a lei da def:arvore. */
static int rg_forte_p(long x, long y, long z, long p){
    int a = rg_vp(x - y, p), b = rg_vp(y - z, p), c = rg_vp(x - z, p);
    return c >= (a < b ? a : b);
}
static int rg_forte_pref(long x, long y, long z, int bits){
    int a = rg_prefixo(x, y, bits), b = rg_prefixo(y, z, bits), c = rg_prefixo(x, z, bits);
    return c >= (a < b ? a : b);
}

/* ── E O QUE DISTINGUE AS DUAS FAMÍLIAS DE RÉGUA ─────────────────────────
 * Onde há valoração, v(xy) = v(x) + v(y) --- o produto vira SOMA.
 * Onde há norma, N(xy) = N(x)·N(y) --- o produto fica PRODUTO.
 * Devolve 1 se a régua p-ádica é multiplicativa no par dado. */
static int rg_vp_multiplicativa(long x, long y, long p){
    if(x == 0 || y == 0) return 1;
    return rg_vp(x*y, p) == rg_vp(x, p) + rg_vp(y, p);
}
/* e a norma da rotação, que é arquimediana: a forte FALHA */
static int rg_norma_forte_falha(long a, long b, long c, long d){
    long n1 = a*a + b*b, n2 = c*c + d*d;
    long sr = a + c, si = b + d, ns = sr*sr + si*si;
    return ns > (n1 > n2 ? n1 : n2);        /* 1 quando a forte falha */
}

#endif /* REGUAS_H */
