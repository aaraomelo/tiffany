/* oito.h — OITO BITS, E A EXAUSTÃO: ℙ¹(𝔽₁₂₇) tem 128 pontos e cabe todo.
 *
 * O eval pôs a ordem, e pôs a EXAUSTÃO antes do tipo:
 *
 *      Teorema Universal → 0 ↔ ∞ → Möbius projectivo → aritmética natural
 *      → EXAUSTÃO → int8_t
 *
 * e a regra arquitectural que a sustenta:
 *
 *      «Não representar o infinito como um número grande.
 *       Representá-lo como o DUAL PROJECTIVO DE ZERO.»
 *
 * ── E HÁ UM ENCAIXE QUE NÃO FOI PROCURADO ─────────────────────────────────────
 * 127 é PRIMO, e é exactamente o maior valor de um `int8_t`. Logo 𝔽₁₂₇ — o corpo com 127
 * elementos — cabe inteiro no tipo mais pequeno que a máquina tem, e a recta projectiva
 * sobre ele tem
 *
 *      |ℙ¹(𝔽₁₂₇)| = 127 + 1 = 128 pontos,
 *
 * que é o número de valores não negativos de um `int8_t`. O infinito não é um número
 * grande: é o ponto [1:0], e ocupa um lugar como os outros.
 *
 * ── E ENTÃO O int8_t DEIXA DE SER UMA APOSTA ──────────────────────────────────
 * Num corpo finito nada cresce: toda operação fica dentro, porque o resto é a operação.
 * E o espaço é pequeno o bastante para se VARRER INTEIRO — 128 pontos, 16384 pares,
 * 127 metais. Deixa de haver amostra, tecto ou profundidade escolhida: as leis
 * verificam-se em TODOS os casos que existem, e uma varredura exaustiva não tem o defeito
 * que esta casa persegue nas outras, que é o de o regime interessante ficar de fora.
 *
 * ── O QUE ISTO NÃO É ──────────────────────────────────────────────────────────
 * Não é a prova das leis sobre ℚ ou ℝ. É a verificação EXAUSTIVA numa realização onde o
 * espaço é finito — e o Teorema do Gato já diz o que isso vale: a lei é universal, a face
 * é da instância. O que 𝔽₁₂₇ dá é uma face onde não há nada por varrer, e por isso onde
 * nenhuma afirmação pode estar a esconder o caso que faltava.
 *
 * A aritmética faz-se em `int` e reduz-se logo; o que se GUARDA é `int8_t`. É a regra da
 * casa outra vez — reduzir antes de crescer.
 *
 * Não precisa de nada. */
#ifndef OITO_H
#define OITO_H

#include <stdint.h>

#define OT_P 127                       /* o primo, e o topo do int8_t */
#define OT_PONTOS (OT_P + 1)           /* |ℙ¹(𝔽ₚ)| = p + 1 = 128 */

static long ot_fora = 0;               /* o que não coube — e num corpo finito é ZERO */

/* ── 𝔽₁₂₇: tudo em int8_t, com o resto a fazer o trabalho do tecto ─────────────*/
typedef int8_t F;

static F ot_red(int x){
    int r = x % OT_P;
    if(r < 0) r += OT_P;
    if(r > 127 || r < 0){ ot_fora++; return 0; }   /* não pode acontecer, e vigia-se */
    return (F)r;
}
static F ot_soma(F a, F b){ return ot_red((int)a + (int)b); }
static F ot_menos(F a, F b){ return ot_red((int)a - (int)b); }
static F ot_mult(F a, F b){ return ot_red((int)a * (int)b); }
static F ot_oposto(F a){ return ot_red(-(int)a); }

/* o inverso em 𝔽ₚ: por Fermat, a^(p−2). Total excepto no zero — e é EXACTAMENTE
 * essa a fibra que ℙ¹ vai tornar total, no ponto seguinte. */
static int ot_inverso(F a, F *r){
    if(a == 0) return 0;
    int b = 1, e = OT_P - 2, base = a;
    while(e){
        if(e & 1) b = (b * base) % OT_P;
        base = (base * base) % OT_P;
        e >>= 1;
    }
    *r = (F)b;
    return 1;
}
/* ── ℙ¹(𝔽ₚ): 128 pontos, e o infinito é [1:0] ──────────────────────────────────
 * A forma canónica: [x : 1] para os p finitos, e [1 : 0] para o infinito. Guarda-se um
 * só `int8_t` — o índice do ponto — e o 127 é o ∞. Nada de números grandes. */
typedef int8_t Pt;                     /* 0..126 são os finitos; 127 é o ∞ */

#define OT_INF ((Pt)127)
static int ot_e_inf(Pt x){ return x == OT_INF; }

/* o ponto [p:q] normalizado: divide-se por q quando q ≠ 0, senão é o ∞ */
static int ot_ponto(int p, int q, Pt *r){
    int pp = ((p % OT_P) + OT_P) % OT_P, qq = ((q % OT_P) + OT_P) % OT_P;
    if(pp == 0 && qq == 0) return 0;               /* [0:0] não é ponto */
    if(qq == 0){ *r = OT_INF; return 1; }
    F iq;
    if(!ot_inverso((F)qq, &iq)) return 0;
    *r = (Pt)ot_mult((F)pp, iq);
    return 1;
}
/* ── A INVERSÃO: a TROCA, e aqui ela é total sem excepção nenhuma ──────────────
 * [x:1] ↦ [1:x], que é [1/x : 1] para x ≠ 0, e [1:0] = ∞ para x = 0. E ∞ ↦ 0. */
static Pt ot_inverte(Pt x){
    if(ot_e_inf(x)) return (Pt)0;
    if(x == 0) return OT_INF;
    F r;
    ot_inverso((F)x, &r);
    return (Pt)r;
}
/* ── A ACÇÃO DE MÖBIUS sobre ℙ¹, com a matriz em 𝔽ₚ ────────────────────────────
 * [p:q] ↦ [a p + b q : c p + d q]. Total quando det ≠ 0, e aí é BIJECÇÃO das 128. */
static int ot_mobius(F a, F b, F c, F d, Pt x, Pt *r){
    if(ot_menos(ot_mult(a,d), ot_mult(b,c)) == 0) return 0;
    int p, q;
    if(ot_e_inf(x)){ p = 1; q = 0; } else { p = x; q = 1; }
    return ot_ponto((int)a*p + (int)b*q, (int)c*p + (int)d*q, r);
}
/* o GATO A_m sobre 𝔽ₚ: x ↦ (m x + 1)/x — e leva 0 a ∞, ∞ a m */
static int ot_gato(F m, Pt x, Pt *r){ return ot_mobius(m, 1, 1, 0, x, r); }
/* a involução da casa, ν(x) = −1/x, em ℙ¹ */
static Pt ot_nu(Pt x){ return ot_inverte(ot_e_inf(x) ? (Pt)0 : (Pt)ot_oposto((F)x)); }

/* ── E O PERÍODO, que num corpo finito é sempre finito e mede-se ───────────────*/
static int ot_periodo_gato(F m, long *per){
    Pt orb = (Pt)0, x;
    if(!ot_gato(m, orb, &x)) return 0;
    Pt inicio = x;
    for(long k = 1; k <= 4 * OT_PONTOS; k++){
        Pt y;
        if(!ot_gato(m, x, &y)) return 0;
        x = y;
        if(x == inicio){ *per = k; return 1; }
    }
    return 0;
}
#endif
