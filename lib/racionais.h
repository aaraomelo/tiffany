/* racionais.h — ℚ: A REVERSIBILIDADE DA MULTIPLICAÇÃO NÃO NULA.
 *
 *  ordem do coordenador põe a escada e o que cada andar acrescenta:
 *
 *     ℕ: + ×        ℤ: + × −  (reversibilidade da SOMA)
 *     ℚ: + × − ÷    (reversibilidade da MULTIPLICAÇÃO não nula)
 *
 * e a cadeia: construção → oposto → inverso.
 *
 * O gume do andar é dito na língua desta casa, e não podia ser melhor:
 *
 *     «divisão por zero não é uma aproximação ruim; é uma operação SEM FIBRA.»
 *
 * A fibra é a divisão das cinco operações — dado o produto e um fator, achar o outro.
 * Com o fator zero não há fibra nenhuma: nenhum x cumpre 0·x = 3, e TODOS cumprem
 * 0·x = 0. Não é um valor difícil de achar: é a ausência da fibra, e diz-se.
 *
 * O racional é uma CLASSE, não o par que calhou escrito: (a,b) ~ (c,d) ⟺ ad = bc. Por
 * isso cada operação tem de estar BEM DEFINIDA — trocar o representante não pode mudar
 * o resultado —, e é isso que aqui se mede. */
/* ═══ O RACIONAL MIGRADO PARA O TEOREMA DO GATO — ENVELOPE E₁₆ ════════════════
 * Três coisas mudam aqui, e as três saem do Gato e de `racionais.tex` §Q16:
 *
 *   1. O racional vive em 16 bits. O par (`dual16.h`) segura os intermédios exactos de
 *      16×16; produtos triplos escalam para `dual32.h` sem materializar ℚ.
 *   2. REDUZ-SE ANTES DE MULTIPLICAR. Cancelar em cruz mantém os números pequenos, e é
 *      a fibra a ser tirada primeiro — que é a ordem certa em todo o resto desta casa.
 *   3. O que não cabe CONTA-SE, e não enrola em silêncio. `qz_saturou` é um sítio
 *      separado dos defeitos, porque uma falha de representação não é um contra-exemplo.
 *
 * E foi o instrumento que obrigou a isto: com `-DQZ_MEDE` a assistente registava
 * 8,4·10¹⁸ — 91% do tecto do `long` — e eu não conseguia distinguir um número grande
 * legítimo de um já enrolado, porque a medição via o valor DEPOIS da conta. A detecção
 * tem de estar DENTRO da operação, e para isso o racional tem de caber no par. */
#ifndef RACIONAIS_H
#define RACIONAIS_H

#include "dual16.h"
#include "dual32.h"
#include "i128.h"

typedef struct { int16_t p, q; } Qz;         /* p/q em E₁₆, com q ≠ 0 */
typedef struct { I128 p, q; } QzP;           /* par promovido — classe exacta fora de E₁₆ */

static long qz_saturou = 0;    /* o que não coube — contado À PARTE dos defeitos */

/* ── A PERGUNTA DO TEOREMA DO GATO, FEITA AO PRÓPRIO TIPO ──────────────────────
 * Com -DQZ_MEDE regista-se a maior magnitude que passa por um racional. Agora ela é um
 * complemento e não o instrumento principal: o instrumento é o `qz_saturou`, que vê o
 * número ANTES de ele enrolar. */
#ifdef QZ_MEDE
#include <stdio.h>
#include <stdlib.h>
static long qz_max_pq = 0;
static int qz_reg = 0;
static void qz_rodape(void){
    fprintf(stderr, "#QZMAX %ld %ld\n", qz_max_pq, qz_saturou);
}
static void qz_ve(long v){
    long a = v < 0 ? -v : v;
    if(!qz_reg){ qz_reg = 1; atexit(qz_rodape); }
    if(a > qz_max_pq) qz_max_pq = a;
}
#else
#define qz_ve(v)   ((void)0)
#endif

static long qz_mdc(long a, long b){
    if(a < 0) a = -a;
    if(b < 0) b = -b;
    while(b){ long t = a % b; a = b; b = t; }
    return a ? a : 1;
}
/* cabe em E₁₆? — a pergunta do Gato, feita a cada valor que se guarda */
static int qz_cabe(long v){ return v <= 32767L && v >= -32767L; }

/* a forma REDUZIDA, com o sinal no numerador — e é ela a única por classe.
 * Aceita `long` para que os chamadores não mudem, e é AQUI que se decide se cabe. */
static Qz qz(long p, long q){
    Qz r;
    if(q < 0){ p = -p; q = -q; }
    long g = qz_mdc(p, q);
    p /= g; q /= g;
    if(p == 0) q = 1;
    if(!qz_cabe(p) || !qz_cabe(q)){
        qz_saturou++;                    /* não cabe: conta-se, e NÃO se enrola calado */
        r.p = (p < 0) ? (int16_t)-32767 : (int16_t)32767;
        r.q = 1;
        return r;
    }
    qz_ve(p); qz_ve(q);
    r.p = (int16_t)p; r.q = (int16_t)q;
    return r;
}
/* ── A ORDEM E A IGUALDADE: PELO PAR, e nunca saturam ─────────────────────────
 * O produto cruzado de dois de 16 é exacto no par D32, sempre. */
static int qz_igual(Qz a, Qz b){ return d16_cmp_prod(a.p, b.q, b.p, a.q) == 0; }
static int qz_menor(Qz a, Qz b){ return d16_cmp_prod(a.p, b.q, b.p, a.q) < 0; }

/* ── A SOMA: cancela-se em cruz ANTES de multiplicar ──────────────────────────
 * a/b + c/d = (a·(d/g) + c·(b/g)) / (b·(d/g)), com g = mdc(b,d). Assim o denominador é o
 * MENOR múltiplo comum e não o produto — e o que antes estourava por causa de um factor
 * repetido deixa de estourar. É a fibra tirada primeiro. */
static Qz qz_soma(Qz a, Qz b){
    long g = qz_mdc(a.q, b.q);
    long bq = b.q / g, aq = a.q / g;
    long n1 = (long)a.p * bq, n2 = (long)b.p * aq;
    long den = (long)a.q * bq;
    if(!qz_cabe(n1) || !qz_cabe(n2) || !qz_cabe(den) || !qz_cabe(n1 + n2)){
        qz_saturou++;
        return qz(n1 + n2, den);         /* deixa o qz() decidir e contar */
    }
    return qz(n1 + n2, den);
}
/* ── O PRODUTO: cancela-se em cruz, dos DOIS lados ────────────────────────────
 * (a/b)·(c/d): cancela-se mdc(a,d) e mdc(c,b) antes de multiplicar. É a mesma ideia,
 * e é o que mantém os números pequenos numa cadeia longa de produtos. */
static Qz qz_mult(Qz a, Qz b){
    long g1 = qz_mdc(a.p, b.q), g2 = qz_mdc(b.p, a.q);
    long ap = a.p / g1, bq = b.q / g1;
    long bp = b.p / g2, aq = a.q / g2;
    return qz(ap * bp, aq * bq);
}
static Qz qz_oposto(Qz a){ Qz r; r.p = -a.p; r.q = a.q; return r; }

/* O INVERSO — e a sua ausência. Devolve 0 quando não existe, e é o gume do andar:
 * o zero não tem inverso porque a fibra não existe, não porque seja difícil. */
static int qz_inverso(Qz a, Qz *r){
    if(a.p == 0) return 0;                     /* 0⁻¹ NÃO EXISTE — sem fibra */
    *r = qz(a.q, a.p);
    return 1;
}
static int qz_divide(Qz a, Qz b, Qz *r){
    Qz i;
    if(!qz_inverso(b, &i)) return 0;           /* dividir por zero: recusa-se */
    *r = qz_mult(a, i);
    return 1;
}
/* a DENSIDADE: entre dois há sempre o ponto médio — e é ele a testemunha */
static Qz qz_medio(Qz a, Qz b){
    I128 n = i128_add(i128_smul((int64_t)a.p, (int64_t)b.q),
                        i128_smul((int64_t)b.p, (int64_t)a.q));
    I128 den = i128_smul_i128(i128_from_i64(2LL * (int64_t)a.q), (int64_t)b.q);
    if(!i128_fits_i64(n) || !i128_fits_i64(den)){ qz_saturou++; return qz(0,1); }
    int64_t num = i128_to_i64(n), d = i128_to_i64(den);
    long g = qz_mdc(num < 0 ? -num : num, d < 0 ? -d : d);
    num /= g; d /= g;
    if(!qz_cabe(num) || !qz_cabe(d)){ qz_saturou++; return qz(0,1); }
    return qz((long)num, (long)d);
}
/* o ARQUIMEDIANO: existe n natural com n > |q| — e o n exibe-se */
static long qz_arquimediano(Qz a){
    long n = a.p < 0 ? -(long)a.p : (long)a.p;
    return n / a.q + 1;                        /* ⌊|p|/q⌋ + 1 serve sempre */
}
/* ── COMPARAR |a − b| COM ε SEM FORMAR A DIFERENÇA ────────────────────────────
 *      |a.p/a.q − b.p/b.q| < e.p/e.q   ⟺   |a.p·b.q − b.p·a.q| · e.q  <  e.p · a.q·b.q
 *
 * Pares 16×16 via `d16_mult`; o produto triplo escala para D64 sem construir ℚ. */
static int qz_dist_menor(Qz a, Qz b, Qz eps){
    if(eps.p <= 0 || eps.q <= 0) return 0;           /* ε inválido ou saturado */
    I128 x = i128_smul((int64_t)a.p, (int64_t)b.q);
    I128 y = i128_smul((int64_t)b.p, (int64_t)a.q);
    I128 n = i128_cmp(x, y) >= 0 ? i128_sub(x, y) : i128_sub(y, x);
    if(i128_negativo(n)) n = i128_neg(n);
    I128 esq = i128_smul_i128(n, (int64_t)(uint16_t)d16_abs_u(eps.q));
    I128 d = i128_smul((int64_t)(uint16_t)d16_abs_u(a.q), (int64_t)(uint16_t)d16_abs_u(b.q));
    I128 dir = i128_smul_i128(d, (int64_t)(uint16_t)d16_abs_u(eps.p));
    return i128_cmp(esq, dir) < 0;
}
/* ── COMPARAR x² COM UM INTEIRO SEM FORMAR x² ─────────────────────────────────
 * p² e c·q² são exactos no par; a comparação decide sem construir x². */
static int qz_cmp_quad(Qz x, long c, int *bom){
    if(bom) *bom = 1;
    if(c < 0) return 1;
    D32 e = d16_mult(x.p, x.p);
    D32 q2 = d16_mult(x.q, x.q);
    D64 d;
    if(!d64_esc(d64_de((unsigned)d32_to_i32(q2)), (unsigned)c, &d)){ if(bom) *bom = 0; return 0; }
    return d64_cmp(d64_de((unsigned)d32_to_i32(e)), d);
}
/* e o mesmo com a diferença dos DOIS lados de um encaixe: |a − b| ≤ ε */
static int qz_dist_menor_ig(Qz a, Qz b, Qz eps){
    if(eps.p <= 0 || eps.q <= 0) return 0;
    I128 x = i128_smul((int64_t)a.p, (int64_t)b.q);
    I128 y = i128_smul((int64_t)b.p, (int64_t)a.q);
    I128 n = i128_cmp(x, y) >= 0 ? i128_sub(x, y) : i128_sub(y, x);
    if(i128_negativo(n)) n = i128_neg(n);
    I128 esq = i128_smul_i128(n, (int64_t)(uint16_t)d16_abs_u(eps.q));
    I128 d = i128_smul((int64_t)(uint16_t)d16_abs_u(a.q), (int64_t)(uint16_t)d16_abs_u(b.q));
    I128 dir = i128_smul_i128(d, (int64_t)(uint16_t)d16_abs_u(eps.p));
    return i128_cmp(esq, dir) <= 0;
}
/* a inclusão ℤ ↪ ℚ: n ↦ n/1 */
static Qz qz_de_inteiro(long n){ return qz(n, 1); }

/* ── PROMOÇÃO: saturo → par exacto no andar I128 (caminho novo; qz() estreito intacto) ──
 * O detector é o mesmo princípio: não cabe em E₁₆ ⟺ alto ≠ 0 no sentido do par reduzido.
 * Não substitui qz() no caminho quente até decisão do coordenador — só expõe o resultado
 * exacto para quem precisa de ler a classe depois de qz_saturou. */
static int qz_prom_sat(QzP w){
    if(!i128_fits_i64(w.p) || !i128_fits_i64(w.q)) return 1;
    long p = i128_to_i64(w.p), q = i128_to_i64(w.q);
    return !qz_cabe(p) || !qz_cabe(q);
}
static QzP qz_prom(long p, long q){
    QzP r;
    if(q < 0){ p = -p; q = -q; }
    long g = qz_mdc(p, q);
    p /= g; q /= g;
    if(p == 0) q = 1;
    r.p = i128_from_i64(p);
    r.q = i128_from_i64(q);
    return r;
}
static QzP qz_prom_de(Qz a){
    QzP r;
    r.p = i128_from_i64((int64_t)a.p);
    r.q = i128_from_i64((int64_t)a.q);
    return r;
}
static int qz_prom_igual(QzP x, QzP y){
    if(!i128_fits_i64(x.p) || !i128_fits_i64(x.q) ||
       !i128_fits_i64(y.p) || !i128_fits_i64(y.q)) return 0;
    return i128_cmp(i128_smul(i128_to_i64(x.p), i128_to_i64(y.q)),
                      i128_smul(i128_to_i64(y.p), i128_to_i64(x.q))) == 0;
}
static Qz qz_prom_estreita(QzP w){ return qz(i128_to_i64(w.p), i128_to_i64(w.q)); }
static int qz_prom_estreito_bate(Qz r, QzP ex){
    if(qz_prom_sat(ex)) return 0;                /* saturado: qz(ex) também dá tecto */
    if(!i128_fits_i64(ex.p) || !i128_fits_i64(ex.q)) return 0;
    return qz_igual(r, qz(i128_to_i64(ex.p), i128_to_i64(ex.q)));
}
static QzP qz_prom_soma(Qz a, Qz b){
    long g = qz_mdc(a.q, b.q);
    long bq = b.q / g, aq = a.q / g;
    long n1 = (long)a.p * bq, n2 = (long)b.p * aq;
    long den = (long)a.q * bq;
    return qz_prom(n1 + n2, den);
}
static QzP qz_prom_soma_pp(QzP a, QzP b){
    I128 n1 = i128_smul_i128(a.p, i128_to_i64(b.q));
    I128 n2 = i128_smul_i128(b.p, i128_to_i64(a.q));
    I128 den = i128_smul_i128(a.q, i128_to_i64(b.q));
    return qz_prom(i128_to_i64(i128_add(n1, n2)), i128_to_i64(den));
}
static QzP qz_prom_mult(Qz a, Qz b){
    long g1 = qz_mdc(a.p, b.q), g2 = qz_mdc(b.p, a.q);
    long ap = a.p / g1, bq = b.q / g1;
    long bp = b.p / g2, aq = a.q / g2;
    return qz_prom(ap * bp, aq * bq);
}
static QzP qz_prom_mult_pp(QzP a, QzP b){
    return qz_prom(i128_to_i64(i128_smul_i128(a.p, i128_to_i64(b.p))),
                 i128_to_i64(i128_smul_i128(a.q, i128_to_i64(b.q))));
}

/* ── QzX: estreito + promovido juntos — caminho de migração (qz() legado intacto) ──
 * Quem precisa do exacto lê .promovido quando .saturo; quem ainda consome Qz usa
 * .estreito (comportamento actual, incluindo ±32767). */
typedef struct { Qz estreito; QzP promovido; int saturo; } QzX;

static QzX qz_x(long p, long q){
    QzX r;
    r.promovido = qz_prom(p, q);
    r.saturo = qz_prom_sat(r.promovido);
    r.estreito = qz(p, q);
    return r;
}
static QzX qz_x_de(Qz a){
    QzX r;
    r.promovido = qz_prom_de(a);
    r.saturo = qz_prom_sat(r.promovido);
    r.estreito = a;
    return r;
}
static QzX qz_x_soma(Qz a, Qz b){
    QzX r;
    r.promovido = qz_prom_soma(a, b);
    r.saturo = qz_prom_sat(r.promovido);
    if(!r.saturo)
        r.estreito = qz(i128_to_i64(r.promovido.p), i128_to_i64(r.promovido.q));
    else
        r.estreito = qz_soma(a, b);
    return r;
}
static QzX qz_x_mult(Qz a, Qz b){
    QzX r;
    r.promovido = qz_prom_mult(a, b);
    r.saturo = qz_prom_sat(r.promovido);
    if(!r.saturo)
        r.estreito = qz(i128_to_i64(r.promovido.p), i128_to_i64(r.promovido.q));
    else
        r.estreito = qz_mult(a, b);
    return r;
}
static int qz_x_divide(Qz a, Qz b, QzX *r){
    Qz i;
    if(!qz_inverso(b, &i)) return 0;
    if(r) *r = qz_x_mult(a, i);
    return 1;
}
static int qz_x_igual(QzX a, QzX b){
    return qz_prom_igual(a.promovido, b.promovido);
}
#endif
