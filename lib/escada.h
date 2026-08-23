/* ═══════════════════════════════════════════════════════════════════════════
 * lib/escada.h — os degraus, a fibra G, e a COMPLETAÇÃO de um corpo
 *
 * A escada da `aranha sec:escada` constrói-se a partir da distinção e de mais
 * nada, e os degraus por quociente são realizações com G a contar a classe:
 *
 *     X_1 = I                    as palavras            (sem quociente)
 *     X_2 = X_1²/~   a+d = b+c   fecha o OPOSTO         (a,b) ↦ a−b
 *     X_3 = (X_2×X_2*)/~  ps=qr  fecha o INVERSO        (p,q) ↦ p/q
 *     X_4 = os cortes de X_3     fecha o CORTE
 *
 * E daí a lei que interessa a quem tem um corpo em mãos:
 *
 *     um corpo está COMPLETO exactamente quando G é constante.
 *
 * Quando não está, este header diz QUANTO falta e o cliente expande.
 * Medido em `tests/escada.c` e `tests/pgwire.c` §W142, §W145, §W154.
 * ═══════════════════════════════════════════════════════════════════════════ */
#ifndef ESCADA_H
#define ESCADA_H

#define ES_MAX 2048

/* ── A FIBRA: G(x) conta quantos vieram parar ao mesmo endereço (def:dobra) ── */
typedef struct {
    long fibras;        /* quantos endereços distintos */
    long menor, maior;  /* o tamanho da menor e da maior fibra */
    long soma;          /* Σ G, que tem de ser |I| --- o thm:escada */
    int  constante;     /* 1 se G é constante: o corpo está COMPLETO */
} EsFibra;

static EsFibra es_fibra(const long *end, long n){
    EsFibra f = {0, 0, 0, 0, 0};
    long vistos[ES_MAX];
    if(n > ES_MAX) return f;
    f.menor = n + 1;
    for(long i = 0; i < n; i++){
        int novo = 1;
        for(long j = 0; j < f.fibras; j++) if(vistos[j] == end[i]){ novo = 0; break; }
        if(!novo) continue;
        vistos[f.fibras] = end[i];
        long t = 0;
        for(long j = 0; j < n; j++) if(end[j] == end[i]) t++;
        if(t < f.menor) f.menor = t;
        if(t > f.maior) f.maior = t;
        f.soma += t;
        f.fibras++;
    }
    f.constante = (f.fibras > 0 && f.menor == f.maior);
    return f;
}

/* ── QUANTO FALTA para o corpo ficar completo ───────────────────────────
 * É a diferença de cada fibra para a maior. Zero sse já era quociente. */
static long es_falta(const long *end, long n){
    EsFibra f = es_fibra(end, n);
    if(f.fibras == 0) return 0;
    return f.fibras * f.maior - f.soma;
}

/* ── O TAMANHO DO CORPO EXPANDIDO: |X| × (a fibra maior) ────────────────
 * Ali G é constante e Σ G = |I| vale por construção. */
static long es_expandido(const long *end, long n){
    EsFibra f = es_fibra(end, n);
    return f.fibras * f.maior;
}

/* ── O thm:escada, verificável: Σ G = |I| ───────────────────────────────── */
static int es_soma_fecha(const long *end, long n){
    EsFibra f = es_fibra(end, n);
    return f.soma == n;
}

/* ═══ AS RELAÇÕES DOS DEGRAUS, para o cliente as usar directamente ═════════ */

/* X_2: (a,b) ~ (c,d) ⟺ a+d = b+c  --- representa a−b, e fecha o OPOSTO */
static int es_x2_igual(long a, long b, long c, long d){ return a + d == b + c; }
static long es_x2_repr(long a, long b){ return a - b; }

/* X_3: (p,q) ~ (r,s) ⟺ ps = qr --- representa p/q, e fecha o INVERSO.
 * É o MESMO degrau do projectivo: a relação é a mesma. */
static int es_x3_igual(long p, long q, long r, long s){ return p*s == q*r; }
static void es_x3_repr(long p, long q, long *rp, long *rq){
    long x = p < 0 ? -p : p, y = q < 0 ? -q : q;
    while(y){ long t = x % y; x = y; y = t; }
    if(x == 0) x = 1;
    *rp = p / x; *rq = q / x;
    if(*rq < 0){ *rp = -*rp; *rq = -*rq; }
}

/* X_4: o corte. Aqui a lei que interessa é a NEGATIVA --- que X_3 não o
 * atinge ---, e ela prova-se: (p,q) ↦ (3p+4q, 2p+3q) preserva p²−2q² e
 * aumenta a fração, logo {x : x² < 2} não tem máximo. Devolve o seguinte. */
static void es_x4_sobe(long p, long q, long *rp, long *rq){
    *rp = 3*p + 4*q; *rq = 2*p + 3*q;
}
static long es_x4_invariante(long p, long q){ return p*p - 2*q*q; }

#endif /* ESCADA_H */
