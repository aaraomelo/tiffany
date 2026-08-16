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
#ifndef RACIONAIS_H
#define RACIONAIS_H

typedef struct { long p, q; } Qz;              /* p/q, com q ≠ 0 */

/* ── A PERGUNTA DO TEOREMA DO GATO, FEITA AO PRÓPRIO TIPO ──────────────────────
 * «Cabe no tipo?» é o teste mais barato que esta casa tem, e nunca o tinha feito ao Qz.
 * Com -DQZ_MEDE a casa regista a MAIOR magnitude que passa por um racional e o MAIOR
 * produto cruzado — que é o que estoura primeiro, porque a ordem compara p·q' com p'·q.
 * Não é uma opinião sobre o tipo: é o valor medido, e decide se `int` chega. */
#ifdef QZ_MEDE
#include <stdio.h>
#include <stdlib.h>   /* atexit */
static long qz_max_pq = 0, qz_max_prod = 0;
static int qz_reg = 0;
static void qz_rodape(void){ fprintf(stderr, "#QZMAX %ld %ld\n", qz_max_pq, qz_max_prod); }
static void qz_ve(long v){
    long a = v < 0 ? -v : v;
    if(!qz_reg){ qz_reg = 1; atexit(qz_rodape); }
    if(a > qz_max_pq) qz_max_pq = a;
    
}
static void qz_ve_prod(long a, long b){
    __int128 v = (__int128)a * b;
    if(v < 0) v = -v;
    if(v > (__int128)qz_max_prod) qz_max_prod = (long)(v > (__int128)9000000000000000000LL
                                                       ? 9000000000000000000LL : v);
}
#else
#define qz_ve(v)          ((void)0)
#define qz_ve_prod(a,b)   ((void)0)
#endif

static long qz_mdc(long a, long b){
    if(a < 0) a = -a;
    if(b < 0) b = -b;
    while(b){ long t = a % b; a = b; b = t; }
    return a ? a : 1;
}
/* a forma REDUZIDA, com o sinal no numerador — e é ela a única por classe */
static Qz qz(long p, long q){
    Qz r; r.p = p; r.q = q;
    if(q < 0){ r.p = -r.p; r.q = -r.q; }
    long g = qz_mdc(r.p, r.q);
    r.p /= g; r.q /= g;
    if(r.p == 0) r.q = 1;
    qz_ve(r.p); qz_ve(r.q);
    return r;
}
/* a EQUIVALÊNCIA: ad = bc — e é ela que define a igualdade, sem decimal nenhum */
static int qz_igual(Qz a, Qz b){ return a.p * b.q == b.p * a.q; }

static Qz qz_soma(Qz a, Qz b){
    qz_ve_prod(a.p,b.q); qz_ve_prod(b.p,a.q); qz_ve_prod(a.q,b.q);
    return qz(a.p*b.q + b.p*a.q, a.q*b.q);
}
static Qz qz_mult(Qz a, Qz b){
    qz_ve_prod(a.p,b.p); qz_ve_prod(a.q,b.q);
    return qz(a.p*b.p, a.q*b.q);
}
static Qz qz_oposto(Qz a){ return qz(-a.p, a.q); }

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
/* a ORDEM: com os denominadores positivos, a/b < c/d ⟺ ad < bc — produto cruzado,
 * e nunca um decimal */
static int qz_menor(Qz a, Qz b){
    qz_ve_prod(a.p,b.q); qz_ve_prod(b.p,a.q);
    return a.p * b.q < b.p * a.q;
}

/* a DENSIDADE: entre dois há sempre o ponto médio — e é ele a testemunha */
static Qz qz_medio(Qz a, Qz b){ return qz(a.p*b.q + b.p*a.q, 2*a.q*b.q); }

/* o ARQUIMEDIANO: existe n natural com n > |q| — e o n exibe-se */
static long qz_arquimediano(Qz a){
    long n = a.p < 0 ? -a.p : a.p;
    return n / a.q + 1;                        /* ⌊|p|/q⌋ + 1 serve sempre */
}
/* a inclusão ℤ ↪ ℚ: n ↦ n/1 */
static Qz qz_de_inteiro(long n){ Qz r; r.p = n; r.q = 1; return r; }
#endif
