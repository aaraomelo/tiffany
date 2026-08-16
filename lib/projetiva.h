/* projetiva.h — ZERO E INFINITO SÃO INVERSOS, e não há caso especial nenhum.
 *
 * O Aarão: «mostra um corolário do teorema universal, que é simplesmente zero e infinito
 * são inversos. Estamos a ter problemas com isso — é isso, não tem "não dá", especial.
 * Segue a simbologia igualmente via Möbius.»
 *
 * ── O PROBLEMA, E ELE É NOSSO ─────────────────────────────────────────────────
 * Esta casa repete, andar após andar, a mesma frase: «0⁻¹ NÃO EXISTE — a fibra é vazia»,
 * e trata-a como A excepção, a única, aquela que sobrevive a toda a escada. Está escrita
 * no `racionais.h`, no `corpos.h`, na escada aritmética inteira, e em cada teorema que
 * fala de divisão.
 *
 * E é um artefacto de ficarmos em ℚ. Na recta projectiva
 *
 *      ℙ¹ = ℚ ∪ {∞},        o ponto [p : q] com (p,q) ≠ (0,0), a menos de escala
 *
 * a inversão é
 *
 *      [p : q]  ↦  [q : p]
 *
 * — uma TROCA. Não há divisão, não há teste, não há ramo. E então
 *
 *      1/0 = [1:0] = ∞      e      1/∞ = [0:1] = 0,
 *
 * ou seja ZERO E INFINITO SÃO INVERSOS. A fibra nunca é vazia: a inversão é uma bijecção
 * de ℙ¹ em ℙ¹, total e involutiva. O «não dá» desaparece — e desaparece do CÓDIGO, que é
 * onde se vê que não era uma verdade sobre o objecto: era sobre a carta que usávamos.
 *
 * ── O QUE ISTO NÃO APAGA, E É PRECISO DIZER ───────────────────────────────────
 * ℙ¹ não é um corpo. A inversão torna-se total, e o preço é que a SOMA deixa de o ser:
 * ∞ + ∞ e 0 · ∞ não têm valor. A excepção não é abolida — é MUDADA DE SÍTIO, e o sítio
 * novo é o certo, porque é lá que ela é uma consequência da construção e não um remendo.
 * Dizer «não há excepção» sem dizer isto seria trocar um erro por outro.
 *
 * ── E O GATO JÁ PRECISAVA DO PONTO QUE A CASA DIZIA NÃO EXISTIR ───────────────
 * A_m = [[m,1],[1,0]] lida como Möbius é x ↦ (m·x + 1)/x. Aplicada a 0 dá ∞; aplicada a
 * ∞ dá m. O objecto central desta casa ATRAVESSA o infinito em dois passos, e nós
 * escrevíamos ao lado que ele não existia. A órbita do gato só fecha em ℙ¹.
 *
 * E a involução da casa, ν(x) = −1/x — «o bit é i» —, em coordenadas é [p:q] ↦ [−q:p], e
 * o período dela depende de onde se olha: QUATRO no par (o quarto de volta, ν² = −id) e
 * DOIS em ℙ¹, porque [−p:−q] é o mesmo ponto. A projectivização mata o sinal, que é a
 * mesma frase do Gato — «a medida não vê o sinal» — dita noutro sítio. Escrevi quatro nos
 * dois e a medição desmentiu-me; e nos dois casos o zero não é excepção.
 *
 * Precisa de `dual32.h`. Tudo em inteiros de 32 bits. */
#ifndef PROJETIVA_H
#define PROJETIVA_H

#include "dual32.h"

static long pj_estouros = 0;

/* o ponto de ℙ¹: [p : q], com (p,q) ≠ (0,0) e a menos de escala.
 * A forma canónica: reduzido pelo mdc, e com o PRIMEIRO não-nulo positivo. */
typedef struct { int p, q; } Pj;

static long pj_mdc(long a, long b){
    if(a < 0) a = -a;
    if(b < 0) b = -b;
    while(b){ long t = a % b; a = b; b = t; }
    return a ? a : 1;
}
/* devolve 0 se (0,0) — que NÃO é ponto de ℙ¹, e é a única coisa que aqui se recusa */
static int pj(long p, long q, Pj *r){
    if(p == 0 && q == 0) return 0;
    long g = pj_mdc(p, q);
    p /= g; q /= g;
    if(q < 0 || (q == 0 && p < 0)){ p = -p; q = -q; }
    if(p > 2147483647L || p < -2147483647L || q > 2147483647L){ pj_estouros++; return 0; }
    r->p = (int)p; r->q = (int)q;
    return 1;
}
static Pj pj_zero(void){ Pj r = { 0, 1 }; return r; }      /* 0   = [0:1] */
static Pj pj_infinito(void){ Pj r = { 1, 0 }; return r; }  /* ∞   = [1:0] */
static Pj pj_um(void){ Pj r = { 1, 1 }; return r; }
static int pj_igual(Pj a, Pj b){ return a.p == b.p && a.q == b.q; }
static int pj_e_infinito(Pj x){ return x.q == 0; }
static int pj_e_zero(Pj x){ return x.p == 0; }

/* ── A INVERSÃO: UMA TROCA, e o `if` que desaparece é o do ZERO ────────────────
 * É esta a linha inteira do corolário. Em ℚ era preciso perguntar «o denominador é
 * zero?»; aqui essa pergunta não tem onde ser feita, porque a operação não divide nada.
 *
 * Fica um ramo, e é preciso não o esconder: a forma canónica quer q ≥ 0, e a troca pode
 * pôr o sinal do lado errado. Mas esse é um ramo sobre o SINAL, não sobre o zero — é
 * escolha de representante, e o representante existe sempre. Escrevi primeiro que não
 * havia `if` nenhum e a medição deu 1600 divergências em 3200: a troca sem normalizar
 * devolve [2:−1], que é o mesmo ponto escrito ao contrário. */
static Pj pj_inverte(Pj x){
    Pj r; long p = x.q, q = x.p;
    if(q < 0 || (q == 0 && p < 0)){ p = -p; q = -q; }
    r.p = (int)p; r.q = (int)q;
    return r;
}

/* ── A INVOLUÇÃO DA CASA, ν(x) = −1/x, em coordenadas ──────────────────────────
 * [p:q] ↦ [−q:p]. E aqui há uma coisa que a medição corrigiu e que é conteúdo: no PAR
 * (p,q) este passo tem período QUATRO — é o quarto de volta, o «bit é i». Em ℙ¹ tem
 * período DOIS, porque [−p:−q] é o mesmo ponto que [p:q]: a projectivização mata o sinal.
 *
 * É a mesma frase do Teorema do Gato dita noutro sítio — «a medida não vê o sinal». Aqui
 * é ℙ¹ que não o vê, e por isso ι² = id enquanto no par ν⁴ = id. Eu tinha escrito quatro
 * nos dois, e a medição deu ν² = id em 6560 de 6560. */
static Pj pj_nu(Pj x){
    Pj r;
    long p = -(long)x.q, q = x.p;
    if(q < 0 || (q == 0 && p < 0)){ p = -p; q = -q; }
    r.p = (int)p; r.q = (int)q;
    return r;
}
/* o MESMO passo no par, SEM projectivizar — e é aqui que o período é quatro */
typedef struct { long p, q; } Par;
static Par pj_nu_par(Par x){ Par r; r.p = -x.q; r.q = x.p; return r; }
static int pj_par_igual(Par a, Par b){ return a.p == b.p && a.q == b.q; }

/* ── A ACÇÃO DE MÖBIUS: x ↦ (a x + b)/(c x + d), que em coordenadas é a MATRIZ ─
 * [p:q] ↦ [a p + b q : c p + d q]. Total em ℙ¹ exactamente quando det ≠ 0 — e é aqui
 * que o Teorema do Gato entra: com |det| = 1 ela é bijecção E preserva o reticulado. */
static int pj_mobius(long a, long b, long c, long d, Pj x, Pj *r){
    if(a*d - b*c == 0) return 0;                    /* degenerada: colapsa ℙ¹ num ponto */
    return pj((long)a*x.p + (long)b*x.q, (long)c*x.p + (long)d*x.q, r);
}
/* o GATO como Möbius: x ↦ (m x + 1)/x — e ele leva 0 em ∞, e ∞ em m */
static int pj_gato(long m, Pj x, Pj *r){ return pj_mobius(m, 1, 1, 0, x, r); }

/* ── E O QUE DEIXA DE SER TOTAL: a soma ────────────────────────────────────────
 * [p:q] + [r:s] = [ps + rq : qs] em ℚ; em ℙ¹ isso dá [0:0] quando ambos são ∞ — e
 * [0:0] não é ponto. Devolve 0 e diz-se: a excepção mudou de sítio, não desapareceu. */
static int pj_soma(Pj x, Pj y, Pj *r){
    return pj((long)x.p*y.q + (long)y.p*x.q, (long)x.q*y.q, r);
}
static int pj_mult(Pj x, Pj y, Pj *r){
    return pj((long)x.p*y.p, (long)x.q*y.q, r);     /* 0·∞ dá [0:0]: recusa-se */
}
#endif
