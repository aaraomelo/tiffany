/* sem_ramo.h — A ARITMÉTICA SEM UM ÚNICO `if`, e a razão de ela ser possível.
 *
 * O Aarão: «segue migrando o sistema para aritmética natural, foco em eliminar todos os
 * ifs.»
 *
 * ── A CAUSA DOS RAMOS, E ELA É UMA SÓ ─────────────────────────────────────────
 * Contados no `racionais.h`, os dez `if` da aritmética testam exactamente três coisas:
 *
 *      o SINAL     (a < 0, b < 0, q < 0)      — normalizar o representante
 *      o ZERO      (p == 0, a.p == 0)         — a fibra vazia da inversão
 *      o TECTO     (v > …, não cabe)          — o crescimento
 *
 * e as três são consequências da NORMALIZAÇÃO. Normaliza-se porque em ℚ os números
 * crescem: sem reduzir, o denominador estoura. E é a normalização que obriga a perguntar
 * pelo sinal e pelo zero.
 *
 * ── LOGO A ELIMINAÇÃO DOS RAMOS NÃO É UMA TÉCNICA: É UMA CONSEQUÊNCIA ─────────
 * Em 𝔽₁₂₇ nada cresce — o resto É a operação. Logo não é preciso normalizar. E se não se
 * normaliza:
 *
 *      · o SINAL desaparece, porque em ℕ não há sinal a pôr no sítio certo;
 *      · o ZERO desaparece, porque a inversão passa a ser a TROCA [p:q] ↦ [q:p], que não
 *        divide e portanto não pergunta;
 *      · o TECTO desaparece, porque não há para onde crescer.
 *
 * O par fica NÃO REDUZIDO de propósito. [2:4] e [1:2] são o mesmo ponto e não se
 * confundem, porque a igualdade é o produto cruzado — uma comparação, não um ramo.
 *
 * ── E O QUE FICA DE FORA, que é o preço e diz-se ──────────────────────────────
 * O par [0:0] não é ponto. Aqui isso não é um `if`: é uma condição sobre o DOMÍNIO — e
 * mede-se que ela nunca é violada, isto é, que nenhuma operação com det ≠ 0 produz [0:0]
 * a partir de um ponto legítimo. A exclusão está no enunciado, não no código.
 *
 * Precisa de `stdint.h`. */
#ifndef SEM_RAMO_H
#define SEM_RAMO_H

#include <stdint.h>

#define SR_P 127u

/* ── O CORPO 𝔽₁₂₇, e nenhuma destas linhas ramifica ────────────────────────────
 * Tudo é não negativo, logo o `%` não precisa de correcção de sinal. */
typedef uint8_t Fp;

static Fp sr_som(Fp a, Fp b){ return (Fp)(((unsigned)a + b) % SR_P); }
static Fp sr_mul(Fp a, Fp b){ return (Fp)(((unsigned)a * b) % SR_P); }
static Fp sr_opo(Fp a){ return (Fp)((SR_P - a) % SR_P); }
static Fp sr_sub(Fp a, Fp b){ return sr_som(a, sr_opo(b)); }

/* o INVERSO por Fermat, a^125 — e a cadeia de adição é FIXA, logo não há laço com teste.
 * 125 = 1111101₂: a₂=a², a₃=a²·a, a₆=a₃², a₇=a₆·a, a₁₄, a₁₅, a₃₀, a₆₀, a₆₂, a₁₂₄, a₁₂₅.
 * Onze multiplicações, escritas em linha reta. O zero devolve zero — e isso não é um caso
 * especial escondido: é o que a potência dá, e o ponto projectivo trata-o na troca. */
static Fp sr_inv_corpo(Fp a){
    Fp a2  = sr_mul(a, a);
    Fp a3  = sr_mul(a2, a);
    Fp a6  = sr_mul(a3, a3);
    Fp a7  = sr_mul(a6, a);
    Fp a14 = sr_mul(a7, a7);
    Fp a15 = sr_mul(a14, a);
    Fp a30 = sr_mul(a15, a15);
    Fp a60 = sr_mul(a30, a30);
    Fp a62 = sr_mul(a60, a2);
    Fp a124 = sr_mul(a62, a62);
    return sr_mul(a124, a);                 /* a¹²⁵ = a⁻¹ em 𝔽₁₂₇ */
}
/* ── O PONTO DE ℙ¹, NÃO NORMALIZADO — e é aí que os ramos morrem ──────────────*/
typedef struct { Fp p, q; } Pr;

static Pr sr_pt(Fp p, Fp q){ Pr r; r.p = p; r.q = q; return r; }
static Pr sr_zero(void){ return sr_pt(0, 1); }
static Pr sr_inf(void){ return sr_pt(1, 0); }

/* A INVERSÃO. Uma troca. Zero ramos, e é esta a linha inteira do corolário 0 ↔ ∞. */
static Pr sr_inverte(Pr x){ Pr r; r.p = x.q; r.q = x.p; return r; }

/* a IGUALDADE pelo produto cruzado — uma comparação, não um ramo, e é ela que faz o
 * par não normalizado funcionar: [2:4] e [1:2] dão igual sem se reduzir nenhum */
static int sr_igual(Pr a, Pr b){ return sr_mul(a.p, b.q) == sr_mul(b.p, a.q); }
static int sr_e_inf(Pr x){ return x.q == 0; }
static int sr_e_zero(Pr x){ return x.p == 0; }
static int sr_e_ponto(Pr x){ return !(x.p == 0 && x.q == 0); }   /* [0:0] fora */

/* ── A ACÇÃO DE MÖBIUS: duas combinações lineares, e nada mais ─────────────────*/
static Pr sr_mobius(Fp a, Fp b, Fp c, Fp d, Pr x){
    Pr r;
    r.p = sr_som(sr_mul(a, x.p), sr_mul(b, x.q));
    r.q = sr_som(sr_mul(c, x.p), sr_mul(d, x.q));
    return r;
}
static Fp sr_det(Fp a, Fp b, Fp c, Fp d){ return sr_sub(sr_mul(a,d), sr_mul(b,c)); }

/* o GATO A_m: x ↦ (m x + 1)/x, sem ramo nenhum */
static Pr sr_gato(Fp m, Pr x){ return sr_mobius(m, 1, 1, 0, x); }
/* a involução da casa, ν(x) = −1/x: trocar e opor. Também sem ramo. */
static Pr sr_nu(Pr x){ Pr r; r.p = sr_opo(x.q); r.q = x.p; return r; }

/* ── E A NORMALIZAÇÃO, quando se QUISER um representante — não para operar ─────
 * Fica aqui à parte, e de propósito: ela existe para IMPRIMIR, não para calcular. Repare
 * que ela também não ramifica — a multiplicação pelo inverso trata o ∞ sozinha, porque
 * o inverso de 0 é 0 e [p:0] fica [0:0]… que não é ponto. Por isso a normalização é a
 * ÚNICA coisa deste ficheiro que precisa de saber se q é zero, e por isso não se usa. */
static Fp sr_indice(Pr x){ return sr_mul(x.p, sr_inv_corpo(x.q)); }
#endif
