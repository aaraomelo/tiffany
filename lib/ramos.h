/* ramos.h — OS DOIS RAMOS DE |det| = 1: a leitura MÉTRICA do par gato/esquilo.
 *
 * ── E UMA CORRECÇÃO DE NOME, QUE FOI UM ERRO MEU ──────────────────────────────
 * Escrevi isto primeiro como `esquilo.h`, e ao fazê-lo escrevi POR CIMA de
 * `tests/esquilo.c`, que já existia e media outra coisa — a coisa certa. O esquilo desta
 * casa não é a família elíptica: é a AÇÃO À DIREITA, a álgebra oposta ℍ^op, o espelho que
 * inverte a ordem e faz o corpo comutar. Está medido lá, e agora é o Teorema do Esquilo.
 *
 * O que este ficheiro mede é outra coisa, complementar e menor: a leitura MÉTRICA do
 * mesmo par — os dois ramos de |det| = 1. Vale por si, e não toma o nome do outro.
 *
 * O Aarão: «o esquilo está sobrando na teoria e a Lei 8 também — é lei central
 * operacional, precisa disso. Formaliza o teorema.»
 *
 * ── O ESQUILO NÃO É REMENDO: É DUAL, E OS DOIS SÃO UMA UNIDADE ────────────────
 * Escrevi primeiro que o esquilo era «o escape quando o gato bate no tecto», e o Aarão
 * corrigiu: «o esquilo não é remendo, é DUAL. Um lado é o corpo universal, do outro a
 * realização — Peano, por exemplo. Nenhum é melhor que o outro, os dois são uma unidade.»
 *
 * A correcção muda o teorema e não só as palavras. Um escape é assimétrico: há um caminho
 * bom e um remédio. Um dual é simétrico: há uma lei e duas faces, e a lei é o que elas têm
 * em comum.
 *
 *      |det| = 1  ⟸ A LEI: conserva a medida. É esta a unidade.
 *          ├── det = −1 :  o GATO     hiperbólico, σσ′ = −1, próprios REAIS
 *          └── det = +1 :  o ESQUILO  elíptico,   J² = −1,   próprios ±i
 *
 * O esquilo é a Lei 2 desta casa — «rodar: T† = −T, T² = −1, e a norma a² + b² não se
 * move» (Corpo estelar, o oscilador é o relógio). Estava escrito, e nunca tinha sido posto
 * ao lado do gato como o segundo ramo do MESMO teorema. Quando corrigi o enunciado do Gato
 * — σσ′ = −1 ⟺ |det| = 1 tem o recíproco falso — escrevi «e o +1 é a identidade, que não é
 * gato nenhum». Estava certo e era PARCIAL: o +1 não é a identidade sozinha, é esta
 * família inteira.
 *
 * ── E NENHUM É MELHOR: cada um tem o que falta ao outro ───────────────────────
 *      o GATO tem direcções próprias REAIS: estica por σ numa e encolhe por 1/σ na outra.
 *      É essa assimetria que dá uma RÉGUA — ele conserva MEDINDO. E o preço é que a órbita
 *      cresce como σᵏ: em qualquer representação finita, ele bate no tecto.
 *
 *      o ESQUILO não tem direcção própria real nenhuma: só roda. A órbita é PERIÓDICA,
 *      logo limitada, e ele não bate em tecto nenhum. E o preço é o simétrico: sem eixo,
 *      não há régua — ele conserva SEM MEDIR.
 *
 * Nenhum sozinho é o teorema. A conservação é o que ambos fazem; medir e rodar é como
 * cada um o faz.
 *
 * ── A LEI 8, E É AQUI QUE A UNIDADE SE VÊ ─────────────────────────────────────
 * No anel da Lei 8 (ℤ_65537, o primo de Fermat 2¹⁶+1) o grupo é finito, logo TODA órbita
 * fecha — inclusive a do gato. Ele, que sobre ℤ cresce sem parar, no anel ganha PERÍODO.
 *
 * E a leitura não é «o anel salva o gato». É que ser hiperbólico ou elíptico é uma
 * propriedade da REALIZAÇÃO — de se estar sobre ℤ, sobre ℝ ou sobre ℤ_p —, enquanto
 * |det| = 1 é a mesma em todas. A lei é universal; a face é da instância. É a doutrina
 * desta casa dita em matrizes: o Universal é dono da lei, cada instância é dona da sua
 * face.
 *
 * Precisa de `racionais.h`, `linear.h`, `dual32.h`. */
#ifndef ESQUILO_H
#define ESQUILO_H

static long sq_estouros = 0;

/* ── OS DOIS RAMOS, decididos pelo TRAÇO e pelo DETERMINANTE ───────────────────
 * Para 2×2, o polinómio característico é λ² − t·λ + d. O discriminante t² − 4d decide:
 *   > 0  próprios reais distintos  → HIPERBÓLICO (o gato)
 *   < 0  próprios complexos ±      → ELÍPTICO   (o esquilo)
 *   = 0  próprio duplo             → PARABÓLICO (a borda entre os dois)
 * Nada disto avalia raiz nenhuma: sai dos COEFICIENTES, que são inteiros. */
#define SQ_GATO      1
#define SQ_ESQUILO  -1
#define SQ_BORDA     0
#define SQ_NEM       9    /* |det| ≠ 1: nem conserva a medida */

static int sq_ramo(long traco, long det){
    if(det != 1 && det != -1) return SQ_NEM;
    long disc = traco*traco - 4*det;
    if(disc > 0) return SQ_GATO;
    if(disc < 0) return SQ_ESQUILO;
    return SQ_BORDA;
}
/* ── O ROTOR: a matriz do esquilo, e ela é INTEIRA ─────────────────────────────
 * [[a, b],[c, d]] com det = 1 e |traço| < 2. Nos inteiros isso restringe o traço a
 * {−1, 0, 1} — e cada um dá uma ordem finita: 3, 4 e 6. É a mesma restrição
 * cristalográfica que limita as simetrias de um reticulado, e ela aparece aqui sem se a
 * ter procurado: é o que «rodar e ser inteiro» ao mesmo tempo permite. */
typedef struct { long a, b, c, d; } Sq2;

static Sq2 sq_id(void){ Sq2 r = {1,0,0,1}; return r; }
static Sq2 sq_mult(Sq2 x, Sq2 y){
    Sq2 r;
    r.a = x.a*y.a + x.b*y.c;  r.b = x.a*y.b + x.b*y.d;
    r.c = x.c*y.a + x.d*y.c;  r.d = x.c*y.b + x.d*y.d;
    return r;
}
static int sq_igual(Sq2 x, Sq2 y){
    return x.a==y.a && x.b==y.b && x.c==y.c && x.d==y.d;
}
static long sq_traco(Sq2 x){ return x.a + x.d; }
static long sq_det(Sq2 x){ return x.a*x.d - x.b*x.c; }

/* o rotor J: traço 0, det 1 — período 4, e é a Lei 2 desta casa */
static Sq2 sq_rotor(void){ Sq2 r = {0,-1,1,0}; return r; }
/* o gato A_m: traço m, det −1 — hiperbólico para todo m ≥ 1 */
static Sq2 sq_gato(long m){ Sq2 r = {m,1,1,0}; return r; }

/* ── A ORDEM: quantos passos até voltar à identidade ───────────────────────────
 * Devolve o período, ou 0 se não fechar dentro do tecto — e é ESSA a diferença entre os
 * dois ramos, medida em vez de afirmada. O tecto é pequeno de propósito: o esquilo fecha
 * em 3, 4 ou 6, e o gato não fecha nunca. */
static long sq_periodo(Sq2 x, long tecto){
    Sq2 p = x;
    for(long k = 1; k <= tecto; k++){
        if(sq_igual(p, sq_id())) return k;
        /* e o guarda, porque o gato CRESCE: se as entradas saem do que se pode
         * multiplicar, pára-se e conta-se — a saturação não é um período */
        long m = 0;
        if(p.a > m) m = p.a; if(-p.a > m) m = -p.a;
        if(p.b > m) m = p.b; if(-p.b > m) m = -p.b;
        if(p.c > m) m = p.c; if(-p.c > m) m = -p.c;
        if(p.d > m) m = p.d; if(-p.d > m) m = -p.d;
        if(m > 1000000000L){ sq_estouros++; return 0; }
        p = sq_mult(p, x);
    }
    return 0;
}
/* ── A LEI 8: o mesmo, mas no ANEL — e aí o gato passa a fechar ────────────────
 * Em ℤ_p o grupo é finito, logo toda órbita de um elemento invertível é periódica. O
 * gato, que em ℤ cresce sem parar, no anel tem período — e as entradas nunca crescem,
 * porque vivem numa caixa fixa. É a segunda realização, e é o outro ramo. */
#define SQ_LEI8 65537L                 /* o primo de Fermat 2¹⁶ + 1 */

static Sq2 sq_mod(Sq2 x, long P){
    Sq2 r;
    r.a = ((x.a % P) + P) % P;  r.b = ((x.b % P) + P) % P;
    r.c = ((x.c % P) + P) % P;  r.d = ((x.d % P) + P) % P;
    return r;
}
static Sq2 sq_mult_mod(Sq2 x, Sq2 y, long P){
    Sq2 r;
    r.a = (x.a*y.a + x.b*y.c) % P;  r.b = (x.a*y.b + x.b*y.d) % P;
    r.c = (x.c*y.a + x.d*y.c) % P;  r.d = (x.c*y.b + x.d*y.d) % P;
    return r;
}
static long sq_periodo_anel(Sq2 x, long P, long tecto){
    Sq2 a = sq_mod(x, P), p = a, id = sq_mod(sq_id(), P);
    for(long k = 1; k <= tecto; k++){
        if(sq_igual(p, id)) return k;
        p = sq_mult_mod(p, a, P);
    }
    return 0;
}
/* ── E O CRESCIMENTO, medido: em que passo o gato sai do que se pode guardar ───*/
static long sq_passo_que_satura(Sq2 x, long tecto_entrada){
    Sq2 p = x;
    for(long k = 1; k <= 200; k++){
        long m = 0;
        if(p.a > m) m = p.a; if(-p.a > m) m = -p.a;
        if(p.b > m) m = p.b; if(-p.b > m) m = -p.b;
        if(p.c > m) m = p.c; if(-p.c > m) m = -p.c;
        if(p.d > m) m = p.d; if(-p.d > m) m = -p.d;
        if(m > tecto_entrada) return k;
        p = sq_mult(p, x);
    }
    return 0;                                   /* não saturou: é esquilo */
}
#endif
