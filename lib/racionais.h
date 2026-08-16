/* racionais.h — ℚ: A REVERSIBILIDADE DA MULTIPLICAÇÃO NÃO NULA.
 *
 * O `eval.txt` põe a escada e o que cada andar acrescenta:
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
/* ═══ O RACIONAL MIGRADO PARA O TEOREMA DO GATO ═══════════════════════════════
 * Três coisas mudam aqui, e as três saem do Gato:
 *
 *   1. O racional vive em 32 bits. O par (`dual32.h`) segura os intermédios exactos, e
 *      o 64 deixa de ser um tipo primitivo para ser o dual de dois 32.
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

#include "dual32.h"

typedef struct { int p, q; } Qz;               /* p/q em 32 bits, com q ≠ 0 */

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
/* cabe em 32 bits com sinal? — a pergunta do Gato, feita a cada valor que se guarda */
static int qz_cabe(long v){ return v <= 2147483647L && v >= -2147483647L; }

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
        r.p = (p < 0) ? -2147483647 : 2147483647;
        r.q = 1;
        return r;
    }
    qz_ve(p); qz_ve(q);
    r.p = (int)p; r.q = (int)q;
    return r;
}
/* ── A ORDEM E A IGUALDADE: PELO PAR, e nunca saturam ─────────────────────────
 * O produto cruzado de dois de 32 é exacto no par, sempre. Esta é a operação que a casa
 * mais usa, e é a que passa a não ter tecto nenhum. */
static int qz_igual(Qz a, Qz b){ return d32_cmp_prod(a.p, b.q, b.p, a.q) == 0; }
static int qz_menor(Qz a, Qz b){ return d32_cmp_prod(a.p, b.q, b.p, a.q) < 0; }

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
    Qz s = qz_soma(a, b);
    return qz(s.p, 2L * s.q);
}
/* o ARQUIMEDIANO: existe n natural com n > |q| — e o n exibe-se */
static long qz_arquimediano(Qz a){
    long n = a.p < 0 ? -(long)a.p : (long)a.p;
    return n / a.q + 1;                        /* ⌊|p|/q⌋ + 1 serve sempre */
}
/* ── COMPARAR |a − b| COM ε SEM FORMAR A DIFERENÇA ────────────────────────────
 * Este é o sítio que faltava migrar, e é o mesmo defeito que a aritmética natural já
 * tinha diagnosticado: para decidir |a − b| < ε formava-se a diferença, e formá-la
 * obriga a multiplicar os denominadores — o produto é o QUADRADO do que os números
 * precisam. Era isso que fazia o módulo de Cauchy saturar.
 *
 *      |a.p/a.q − b.p/b.q| < e.p/e.q   ⟺   |a.p·b.q − b.p·a.q| · e.q  <  e.p · a.q·b.q
 *
 * Os dois lados são produtos de TRÊS inteiros de 32 bits, e o par (`dual32.h`) segura-os
 * exactos. Nenhum racional novo é construído, nenhum denominador é reduzido, e não há
 * nada a saturar. É a regra desta casa: comparar em vez de construir. */
static int qz_dist_menor(Qz a, Qz b, Qz eps){
    if(eps.p <= 0) return 0;
    /* o numerador da diferença, em módulo, como PAR — sem o formar como racional */
    unsigned ap = d32_abs(a.p), bq = d32_abs(b.q);
    unsigned bp = d32_abs(b.p), aq = d32_abs(a.q);
    int s1 = (a.p < 0) ? -1 : 1, s2 = (b.p < 0) ? -1 : 1;   /* q > 0 sempre */
    D64 x = d64_mult(ap, bq), y = d64_mult(bp, aq), n;
    if(s1 == s2){
        n = (d64_cmp(x,y) >= 0) ? d64_menos(x,y) : d64_menos(y,x);
    } else {
        n = d64_soma(x, y);
    }
    D64 d = d64_mult((unsigned)a.q, (unsigned)b.q);
    D64 esq, dir;
    if(!d64_esc(n, d32_abs(eps.q), &esq)) return 0;         /* não coube: recusa */
    if(!d64_esc(d, d32_abs(eps.p), &dir)) return 0;
    return d64_cmp(esq, dir) < 0;
}
/* ── COMPARAR x² COM UM INTEIRO SEM FORMAR x² ─────────────────────────────────
 * O mesmo defeito noutro sítio: para saber se x² < c formava-se x², e x² tem o DOBRO dos
 * dígitos de x — logo uma bisseção honesta em 32 bits morria a meio. Pelo par, p² e c·q²
 * são exactos e a comparação decide sem construir nada. Devolve −1, 0, +1, e 0 com
 * `*bom = 0` quando nem o par chega (que não acontece para c pequeno). */
static int qz_cmp_quad(Qz x, long c, int *bom){
    if(bom) *bom = 1;
    if(c < 0) return 1;
    D64 e = d64_mult(d32_abs(x.p), d32_abs(x.p));
    D64 q2 = d64_mult((unsigned)x.q, (unsigned)x.q), d;
    if(!d64_esc(q2, (unsigned)c, &d)){ if(bom) *bom = 0; return 0; }
    return d64_cmp(e, d);
}
/* e o mesmo com a diferença dos DOIS lados de um encaixe: |a − b| ≤ ε */
static int qz_dist_menor_ig(Qz a, Qz b, Qz eps){
    return qz_dist_menor(a, b, eps) || qz_igual(a, b);
}
/* a inclusão ℤ ↪ ℚ: n ↦ n/1 */
static Qz qz_de_inteiro(long n){ Qz r; r.p = n; r.q = 1; return r; }
#endif
