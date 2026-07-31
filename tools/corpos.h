/* corpos.h — O TOOLKIT DOS CORPOS. A tríade ⊕ ⊗ ∏, uma por corpo.
 *
 * O mapa está em CORPOS_NA_ISA.md, trazido do catálogo: cada corpo tem a MESMA estrutura —
 * uma soma (Clifford), um produto (La Hire) e um operador (Pontryagin). Muda o que são, não
 * quantos são.
 *
 * Aqui ficam os que já estão MEDIDOS neste repositório. Um corpo só entra quando as três
 * operações dele estão implementadas e há medidor a fechá-las — pôr a assinatura sem a conta
 * seria catálogo, não toolkit.
 *
 *   ÁUREO ℤ[φ]   ⊕ componente a componente   ⊗ o gato (a,b)↦(ma+b,a)   ∏ o deslocamento
 *                coroa.c, familia_real.c, normal_circulo.c
 *   RACIONAL ℚ   ⊕ Clifford cruzado          ⊗ La Hire componente      ∏ a classe (reduzir)
 *                racional_pg.c, rastro.c, tudo_ouro.c
 *   MÓRFICO      ⊕ XOR (deflexão D₁)         ⊗ AND (a erosão)          ∏ a adjunção δ⊣ε
 *                morfico.py — 36/36 certificadas
 *   MECÂNICO     ⊕ soma de matrizes          ⊗ produto de matrizes     ∏ a palavra em S,T
 *                mecanica.c
 *
 * Os que faltam do mapa — fractal, criativo, eletromagnético, motor, telescópico, cristalino,
 * conforme, entrópico, espaço-temporal, óptico, celeste, econômico, evolutivo, expansivo,
 * somático, geométrico, técnico, rotor, cósmico — estão no CORPOS_NA_ISA.md com a tríade
 * descrita, e entram aqui à medida que forem medidos. Ficam DITOS, não implementados: o
 * toolkit não promete o que não fecha.
 */
#ifndef CORPOS_H
#define CORPOS_H

/* ---------------- o par que todos partilham: a Word da ISA ---------------- */
typedef struct { long a, b; } Par;

static long c_mdc(long x, long y){ if(x<0)x=-x; if(y<0)y=-y; while(y){ long t=x%y; x=y; y=t; } return x?x:1; }

/* ---------------- ÁUREO ℤ[φ]: a + bσ, com σ² = mσ + 1 ---------------- */
static Par au_soma(Par x, Par y){ Par r = { x.a+y.a, x.b+y.b }; return r; }
static Par au_prod(Par x, Par y, long m){            /* (a+bσ)(c+dσ) com a borda */
    Par r = { x.a*y.a + x.b*y.b, x.a*y.b + x.b*y.a + m*x.b*y.b }; return r; }
static Par au_op(Par x, long m){ Par r = { m*x.a + x.b, x.a }; return r; }   /* ×σ: o gato */
static long au_norma(Par x, long m){ return x.a*x.a + m*x.a*x.b - x.b*x.b; }

/* ---------------- RACIONAL ℚ: (num, den) ---------------- */
static Par ra_classe(Par x){
    if(x.b < 0){ x.a = -x.a; x.b = -x.b; }
    long g = c_mdc(x.a, x.b); x.a /= g; x.b /= g; return x; }
static Par ra_soma(Par x, Par y){ Par r = { x.a*y.b + x.b*y.a, x.b*y.b }; return ra_classe(r); }
static Par ra_prod(Par x, Par y){ Par r = { x.a*y.a, x.b*y.b }; return ra_classe(r); }
static int  ra_cmp(Par x, Par y){                    /* sem divisão: cruzado */
    x = ra_classe(x); y = ra_classe(y);
    long e = x.a*y.b, f = y.a*x.b; return (e>f) - (e<f); }

/* ---------------- MÓRFICO: conjuntos como máscaras de bits ---------------- */
static unsigned mo_soma(unsigned A, unsigned B){ return A ^ B; }   /* Clifford: a deflexão */
static unsigned mo_prod(unsigned A, unsigned B){ return A & B; }   /* La Hire: a EROSÃO   */
__attribute__((unused)) static unsigned mo_nao(unsigned A, unsigned topo){ return A ^ topo; }
/* a adjunção: dilatação por B (Minkowski em Z/n) e a erosão que lhe é adjunta */
static unsigned mo_dil(unsigned A, unsigned B, int n){
    unsigned r = 0;
    for(int i = 0; i < n; i++) if(A & (1u<<i))
        for(int j = 0; j < n; j++) if(B & (1u<<j)) r |= 1u << ((i+j) % n);
    return r; }
static unsigned mo_ero(unsigned A, unsigned B, int n){
    unsigned r = 0;
    for(int i = 0; i < n; i++){
        int cabe = 1;
        for(int j = 0; j < n; j++) if((B & (1u<<j)) && !(A & (1u << ((i+j)%n)))) cabe = 0;
        if(cabe) r |= 1u << i;
    }
    return r; }

/* ---------------- MECÂNICO: matrizes 2×2 sobre o par ---------------- */
typedef struct { long a, b, c, d; } Mat;
static Mat me_prod(Mat x, Mat y){
    Mat r = { x.a*y.a + x.b*y.c, x.a*y.b + x.b*y.d,
              x.c*y.a + x.d*y.c, x.c*y.b + x.d*y.d }; return r; }
static long me_det(Mat x){ return x.a*x.d - x.b*x.c; }
static Par me_ap(Mat m, Par v){ Par r = { m.a*v.a + m.b*v.b, m.c*v.a + m.d*v.b }; return r; }
__attribute__((unused)) static Mat me_rot(void){ Mat r = {0,-1,1,0}; return r; }          /* o esquilo    */
static Mat me_cis(long k){ Mat r = {1,k,0,1}; return r; }         /* cisalhamento */
static Mat me_gato(long m){ Mat r = {m,1,1,0}; return r; }        /* a cifra      */

#endif
