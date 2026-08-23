/* ═══════════════════════════════════════════════════════════════════════════
 * lib/faces.h — o par de faces em estados extremos: o lógico e o simétrico
 *
 * A tabela das duas faces (aranha §dualidades) emparelha oposto/inverso,
 * aritmética/geométrica, reta/círculo, cartesiano/polar. Dois corpos mostram
 * o par nos seus extremos, e é isso que este header expõe:
 *
 *   LÓGICO      as faces COLAPSAM: ∂x = x, cada elemento é o seu oposto, e a
 *               multiplicativa é a mais pobre possível --- no AND só o topo
 *               tem inverso. É por isso que este andar é XOR.
 *
 *   SIMÉTRICO   fecha na SOMA e não no produto, com a condição exacta
 *                   AB é simétrica  ⟺  AB = BA
 *               --- o que fecharia este corpo é a comutação, e é isso que ele
 *               não tem.
 *
 * Medido em `tests/faces.c` e `tests/pgwire.c` §W148.
 * ═══════════════════════════════════════════════════════════════════════════ */
#ifndef FACES_H
#define FACES_H

/* ═══ O LÓGICO: bitstrings com ⊕ ═════════════════════════════════════════ */

static unsigned lg_soma(unsigned x, unsigned y){ return x ^ y; }
static unsigned lg_prod(unsigned x, unsigned y){ return x & y; }
static unsigned lg_op(unsigned x){ return x; }        /* ∂x = x: o seu próprio oposto */

/* as faces colapsam: verificável, e é a razão de o andar ser XOR */
static int lg_faces_colapsam(unsigned x){ return (x ^ lg_op(x)) == 0; }

/* o inverso no AND: só o topo o tem, e RECUSA para todo o resto */
static int lg_inverso(unsigned x, unsigned topo, unsigned *out){
    if(x != topo) return 0;
    *out = topo;
    return 1;
}
/* quantos elementos têm inverso --- a medida da pobreza da face multiplicativa */
static long lg_quantos_invertem(unsigned topo){ (void)topo; return 1; }

/* ═══ O SIMÉTRICO: A = Aᵀ ════════════════════════════════════════════════ */

/* 2×2 simétrica pelo terno (a,b,c) ↦ [[a,b],[b,c]] */
typedef struct { long a, b, c; } Sim;

static Sim sim(long a, long b, long c){ Sim r = {a,b,c}; return r; }
static Sim sim_soma(Sim x, Sim y){ Sim r = {x.a+y.a, x.b+y.b, x.c+y.c}; return r; }
static long sim_det(Sim x){ return x.a*x.c - x.b*x.b; }
static long sim_traco(Sim x){ return x.a + x.c; }

/* o produto de duas simétricas NÃO é simétrico em geral: devolve as quatro
 * entradas, e o cliente vê. */
static void sim_prod(Sim x, Sim y, long *p00, long *p01, long *p10, long *p11){
    *p00 = x.a*y.a + x.b*y.b;
    *p01 = x.a*y.b + x.b*y.c;
    *p10 = x.b*y.a + x.c*y.b;
    *p11 = x.b*y.b + x.c*y.c;
}
/* a condição EXACTA: AB é simétrica sse AB = BA */
static int sim_produto_simetrico(Sim x, Sim y){
    long a,b,c,d; sim_prod(x, y, &a, &b, &c, &d);
    (void)a; (void)d;
    return b == c;
}
static int sim_comutam(Sim x, Sim y){
    long a1,b1,c1,d1, a2,b2,c2,d2;
    sim_prod(x, y, &a1, &b1, &c1, &d1);
    sim_prod(y, x, &a2, &b2, &c2, &d2);
    return a1==a2 && b1==b2 && c1==c2 && d1==d2;
}
/* e as duas condições são a MESMA --- é a relação deste corpo */
static int sim_lei(Sim x, Sim y){ return sim_produto_simetrico(x,y) == sim_comutam(x,y); }

#endif /* FACES_H */
