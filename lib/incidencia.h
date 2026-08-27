/* ═══════════════════════════════════════════════════════════════════════════
 * lib/incidencia.h — ζ ACUMULA, μ DESACUMULA. NA ORDEM TOTAL, μ É A DIFERENÇA.
 *
 * O `fisica.tex thm:mu` e `thm:zetamu` dizem o que o motor tinha por metade: ele
 * ACUMULA (o `GROUP BY`/`COUNT` constrói o G, que é a convolução com ζ) mas não
 * DESACUMULA. A inversa de ζ na álgebra da incidência é μ, e na ordem TOTAL μ é
 * a diferença finita:
 *
 *      μ(t,t) = 1,   μ(t-1,t) = -1,   μ(u,t) = 0 nos restantes,
 *
 *      logo   (b * μ)(t) = b(t) − b(t−1).
 *
 * E o par fecha: ζ soma o prefixo, μ desfaz a soma, e ζ∘μ = μ∘ζ = id (resíduo
 * 0). É o `thm:zetamu`:
 *
 *      escrever é convolução com ζ:     G_t(x) = (a_x * ζ)(t)     — o acumulado
 *      recuperar é deconvolução com μ:  a_x(t) = G_t(x) − G_{t−1}(x)  — a volta
 *
 * «Conhecida a família {G_t}, a trajetória inteira volta — sem que nada dela
 * tenha sido guardado.» O motor guarda o G; a diferença finita devolve-lhe os
 * passos.
 *
 * ── DUAS ORDENS, A MESMA ÁLGEBRA ─────────────────────────────────────────────
 * Este ficheiro é a ordem TOTAL (a cadeia 0,1,2,…). O `fis:cor:mobius` diz que a
 * mesma inversão na ordem por DIVISIBILIDADE é a função de Möbius clássica, e
 * essa já está no `lib/dirichlet.h` (`dl_acumula` = f*1, `dl_inverte` = F*μ). Não
 * se duplica: «a álgebra não muda — ζ acumula, μ desacumula — e só muda a ordem
 * sobre a qual se acumula». O `tests/zetamu.c` confronta as duas.
 *
 * ── EXACTO EM ℤ ──────────────────────────────────────────────────────────────
 * ζ e μ de funções inteiras são inteiras: a álgebra corre em `long`, sem vírgula.
 * A diferença finita não perde nada — é o inverso EXACTO da soma, não uma
 * aproximação dela.
 *
 * Medido em `tests/zetamu.c`.
 *   cc -O2 -std=c99 -Ilib -Itests -o /tmp/zetamu tests/zetamu.c && /tmp/zetamu
 * ═══════════════════════════════════════════════════════════════════════════ */
#ifndef INCIDENCIA_H
#define INCIDENCIA_H

/* ── ζ: acumula o prefixo. b(t) = Σ_{u≤t} a(u). É a convolução com ζ na cadeia. */
static void inc_zeta(const long *a, long *b, long n){
    long s = 0;
    for(long t = 0; t < n; t++){ s += a[t]; b[t] = s; }
}

/* ── μ: a diferença finita. a(t) = b(t) − b(t−1), com a(0) = b(0). ζ⁻¹, exacto. */
static void inc_mu(const long *b, long *a, long n){
    long ant = 0;
    for(long t = 0; t < n; t++){ a[t] = b[t] - ant; ant = b[t]; }
}

/* ── o núcleo μ(u,t) da ordem total, tal como o thm:mu o dá ────────────────────
 * É a definição pontual, para quem quiser a matriz em vez do operador: vale 1 na
 * diagonal, −1 logo abaixo, e zero em tudo o resto. `inc_mu` é isto aplicado. */
static long inc_mu_nucleo(long u, long t){
    if(u == t)     return  1;
    if(u == t - 1) return -1;
    return 0;
}

/* ── O G COMO ζ, E A SUA VOLTA ─────────────────────────────────────────────────
 * Dada a realização π (π[t] = o valor visitado no passo t) e um alvo x, o
 * indicador é a_x[t] = [π[t] = x], e o campo acumulado é G_t(x) = (a_x * ζ)(t):
 * quantas vezes x foi visto até t. É o G do `thm:mult`, lido ao longo da ordem.
 *
 * `inc_G` enche a família acumulada {G_t(x)}; `inc_mu` sobre ela devolve a_x —
 * a trajetória de x, recuperada da contagem sem se ter guardado a trajetória. */
static void inc_indicador(const long *pi, long n, long x, long *a){
    for(long t = 0; t < n; t++) a[t] = (pi[t] == x);
}
static void inc_G(const long *pi, long n, long x, long *G){
    long s = 0;
    for(long t = 0; t < n; t++){ if(pi[t] == x) s++; G[t] = s; }
}

/* ── O LEVANTAMENTO NA FIBRA (thm:zetamu (3)) ─────────────────────────────────
 * k(i) = G_i(π(i)) é a contagem acumulada da PRÓPRIA célula ao longo da fibra:
 * no r-ésimo encontro vale r. A sua diferença finita vale 1 em cada visita —
 * é o G̃ = 1 do levantamento, dito na linguagem da incidência. */
static void inc_levanta(const long *pi, long n, long *k){
    /* uma passagem: para cada valor, o contador sobe a cada visita, e k[t] recebe-o */
    /* usa uma tabela pequena indexada pelo valor; o chamador garante pi[t] em [0,VMAX) */
    long cont[256];
    for(int v = 0; v < 256; v++) cont[v] = 0;
    for(long t = 0; t < n; t++){
        long v = pi[t] & 255;
        k[t] = ++cont[v];
    }
}

#endif /* INCIDENCIA_H */
