/* ═══════════════════════════════════════════════════════════════════════════
 * lib/triade.h — A TRÍADE E A TRAVESSIA, para quem resolve problemas nos corpos
 *
 * Um tipo só para as três faces, porque são um corpo: a + bω com ω² = t,
 * t ∈ {−1, 0, +1}. O que muda é a métrica; o corpo é um.
 *
 *   t = −1   ω = i    rotação      Δ < 0   a órbita VOLTA
 *   t =  0   ω = ε    exterior     Δ = 0   a órbita DESLIZA
 *   t = +1   ω = j    hiperbólico  Δ > 0   a órbita FOGE
 *
 * Tudo em inteiros: sem vírgula, sem raiz, sem limiar. Onde uma operação não
 * fecha no degrau, ela RECUSA e diz porquê — nunca devolve um valor inventado.
 *
 * A teoria está no `papers/aranha.tex`; as leis que este header realiza:
 *   thm:leidisc   Δ = tr²−4det classifica, e aqui Δ = 4tb²
 *   thm:duas      as duas faces: o oposto e o inverso
 *   cor:medias    a desigualdade das médias, aqui com o terceiro membro
 *   def:arvore    a ultramétrica dos endereços
 *   prop:travessia  T = S∘R⁻¹ é permutação de posições, e D = 2^{−q}
 *   thm:enumera   a leitura posicional e a sua inversa
 *
 * Medido em `tests/pgwire.c` §W146–§W155 e `tests/triade.c`.
 * ═══════════════════════════════════════════════════════════════════════════ */
#ifndef TRIADE_H
#define TRIADE_H

#include <stdio.h>

/* ── O ELEMENTO: a + bω, com t a dizer que face ──────────────────────────── */
typedef struct { long a, b, t; } Tri;

static Tri tri(long a, long b, long t){ Tri r = {a, b, t}; return r; }
static Tri tri_rot(long a, long b){ return tri(a, b, -1); }   /* a + bi   */
static Tri tri_ext(long a, long b){ return tri(a, b,  0); }   /* a + bε   */
static Tri tri_hip(long a, long b){ return tri(a, b, +1); }   /* a + bj   */

/* ── AS DUAS FACES ───────────────────────────────────────────────────────── */
static Tri tri_soma(Tri x, Tri y){ Tri r = {x.a+y.a, x.b+y.b, x.t}; return r; }
static Tri tri_op(Tri x){ Tri r = {-x.a, -x.b, x.t}; return r; }   /* o oposto */

/* (a+bω)(c+dω) = (ac + t·bd) + (ad + bc)ω */
static Tri tri_prod(Tri x, Tri y){
    Tri r = { x.a*y.a + x.t*x.b*y.b, x.a*y.b + x.b*y.a, x.t };
    return r;
}
static Tri tri_conj(Tri x){ Tri r = {x.a, -x.b, x.t}; return r; }

/* a norma: N(a+bω) = a² − t·b², e ela É multiplicativa */
static long tri_norma(Tri x){ return x.a*x.a - x.t*x.b*x.b; }

/* ── O DISCRIMINANTE, E A FACE ───────────────────────────────────────────── */
/* na realização [[a, t·b],[b, a]]: tr = 2a, det = a² − t·b², donde Δ = 4·t·b² */
static long tri_traco(Tri x){ return 2*x.a; }
static long tri_det(Tri x){ return x.a*x.a - x.t*x.b*x.b; }
static long tri_disc(Tri x){ return 4*x.t*x.b*x.b; }

/* a dinâmica que a face impõe: −1 volta, 0 desliza, +1 foge */
static int tri_dinamica(Tri x){ long D = tri_disc(x); return D < 0 ? -1 : (D == 0 ? 0 : 1); }
static const char *tri_dinamica_nome(Tri x){
    switch(tri_dinamica(x)){
        case -1: return "volta (elíptica)";
        case  0: return "desliza (parabólica)";
        default: return "foge (hiperbólica)";
    }
}

/* ── O INVERSO: existe sse a norma é ±1, e então é exacto no degrau ───────
 * Devolve 0 e não toca em `out` quando não existe — nunca inventa. */
static int tri_inverso(Tri x, Tri *out){
    long N = tri_norma(x);
    if(N != 1 && N != -1) return 0;
    Tri c = tri_conj(x);
    out->a = c.a / N; out->b = c.b / N; out->t = x.t;
    return 1;
}

/* ── A VALORAÇÃO DO EXTERIOR (t = 0): v ∈ {0, 1, ∞}, e d = 2^{−v} ────────
 * É a régua da def:arvore neste corpo, e ela é MULTIPLICATIVA. */
#define TRI_INF 62
static long tri_val(Tri x){
    if(x.t != 0) return -1;                  /* só o exterior tem valoração */
    if(x.a != 0) return 0;
    if(x.b != 0) return 1;
    return TRI_INF;
}

/* ── AS TRÊS MÉDIAS, e o trio fecha: g² = h·m ────────────────────────────
 * Em inteiros e sem raiz: devolve g² directamente. */
static long tri_media_arit2(long a, long b){ return a + b; }        /* 2m */
static long tri_media_geo2(long a, long b){ return a * b; }         /* g² */
static int  tri_media_harm(long a, long b, long *num, long *den){
    if(a + b == 0) return 0;
    *num = 2*a*b; *den = a + b; return 1;                            /* h = num/den */
}
/* o fecho do trio, exacto: g² = h·m  ⟺  ab = (2ab/(a+b))·((a+b)/2) */
static int tri_medias_fecham(long a, long b){
    long num, den;
    if(!tri_media_harm(a, b, &num, &den)) return 0;
    return num/2 == tri_media_geo2(a, b);
}
/* a ordem h ≤ g ≤ m reduz-se a (a−b)² ≥ 0 */
static int tri_medias_ordem(long a, long b){ return 4*a*b <= (a+b)*(a+b); }

/* ═══ A TRAVESSIA ════════════════════════════════════════════════════════
 * Uma LEITURA de I_M^n é R_r(x) = Σ x_k M^{r(k)}. A travessia entre duas
 * leituras é a permutação π = s∘r⁻¹ das POSIÇÕES dos dígitos, e o preço
 * lê-se dela — O(n), não O(N). */

#define TRI_NMAX 32

/* lê o endereço a com n dígitos em base M, pela ordem r */
static void tv_le(long a, long M, int n, const int *r, long *dig){
    long p[TRI_NMAX];
    for(int i = 0; i < n; i++){ p[i] = a % M; a /= M; }
    for(int k = 0; k < n; k++) dig[k] = p[r[k]];
}
/* escreve os dígitos pela ordem s */
static long tv_escreve(const long *dig, long M, int n, const int *s){
    long pw[TRI_NMAX]; pw[0] = 1;
    for(int i = 1; i < n; i++) pw[i] = pw[i-1]*M;
    long a = 0;
    for(int k = 0; k < n; k++) a += dig[k]*pw[s[k]];
    return a;
}
/* a travessia: T = S∘R⁻¹ */
static long tv_travessia(long a, long M, int n, const int *r, const int *s){
    long dig[TRI_NMAX];
    tv_le(a, M, n, r, dig);
    return tv_escreve(dig, M, n, s);
}
/* o preço: q = a primeira posição que π = s∘r⁻¹ move; D = 2^{−q}, e D = 0 se π = id.
 * Devolve q, ou n quando π é a identidade (custo zero). */
static int tv_preco(int n, const int *r, const int *s){
    int pi[TRI_NMAX];
    for(int k = 0; k < n; k++) pi[r[k]] = s[k];
    for(int i = 0; i < n; i++) if(pi[i] != i) return i;
    return n;                                  /* π = id: não há travessia nem preço */
}

/* ── A ULTRAMÉTRICA DOS ENDEREÇOS: prof = a primeira divergência ────────── */
static int tv_prof(long x, long y, int bits){
    if(x == y) return bits;
    for(int i = 0; i < bits; i++)
        if(((x >> (bits-1-i)) & 1L) != ((y >> (bits-1-i)) & 1L)) return i;
    return bits;
}

/* ── O CRITÉRIO DA LEITURA: serve sse é bem definida, separa, e a volta é conta.
 * `igual` é a igualdade DO CORPO — não a dos parâmetros. */
typedef struct { int bem_definida, separa, pares; } TvCriterio;
static TvCriterio tv_criterio(const long *end, int n,
                              int (*igual)(int, int, void *), void *ctx){
    TvCriterio c = {1, 1, 0};
    for(int i = 0; i < n; i++) for(int j = 0; j < i; j++){
        int mesmo = igual ? igual(i, j, ctx) : 0;
        int mesmo_end = (end[i] == end[j]);
        c.pares++;
        if(mesmo && !mesmo_end) c.bem_definida = 0;
        if(mesmo_end && !mesmo) c.separa = 0;
    }
    return c;
}

#endif /* TRIADE_H */
