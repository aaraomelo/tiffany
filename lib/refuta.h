/* refuta.h — O REFUTADOR EXAUSTIVO: as identidades da casa passam pela face pequena.
 *
 * O `reducao.h` mostrou que 𝔽ₚ não prova nada sobre ℚ mas REFUTA. Este ficheiro põe isso
 * a trabalhar: as identidades que esta casa afirma — o centro, a membrana,
 * Cayley--Hamilton, Lagrange, a multiplicatividade do determinante — passam a ser
 * verificadas EXAUSTIVAMENTE nas variáveis, sobre vários primos, antes de serem afirmadas.
 *
 * ── POR QUE VÁRIOS PRIMOS, E NÃO O 127 ────────────────────────────────────────
 * Duas razões, e as duas importam.
 *
 *   1. EXAUSTÃO NAS VARIÁVEIS. Uma identidade em quatro entradas varre-se inteira em 𝔽₁₃
 *      (13⁴ = 28561) e não em 𝔽₁₂₇ (127⁴ = 260 milhões). Exaustão é o que se quer, e ela
 *      compra-se baixando o primo, não subindo o tempo.
 *
 *   2. ACIDENTES DE CARACTERÍSTICA. Uma identidade FALSA pode passar num primo por
 *      acidente — o mais óbvio é 2ab ≡ 0 em 𝔽₂, que faz (a+b)² = a² + b² parecer
 *      verdadeira. Um só primo é uma régua; vários primos são a mesma pergunta feita em
 *      corpos onde os acidentes são diferentes.
 *
 * ── E O QUE ELE PODE DIZER ────────────────────────────────────────────────────
 *      REFUTOU  → a identidade é FALSA em ℚ, e o contra-exemplo exibe-se
 *      VAZIO    → não se sabe. Não é «é verdadeira»; é «não caiu aqui».
 *
 * Um refutador que dissesse mais do que isto seria a insinuação que esta casa persegue.
 *
 * Não precisa de nada. Tudo em `int`, com o resto a fazer de tecto. */
#ifndef REFUTA_H
#define REFUTA_H

/* os primos pequenos: exaustivos em quatro variáveis, e com acidentes diferentes */
#define RF_NP 5
static const int RF_PRIMOS[RF_NP] = { 5, 7, 11, 13, 17 };

static int rf_red(long x, int p){ int r = (int)(x % p); return r < 0 ? r + p : r; }
static int rf_som(int a, int b, int p){ return (a + b) % p; }
static int rf_mul(int a, int b, int p){ return (a * b) % p; }
static int rf_sub(int a, int b, int p){ return ((a - b) % p + p) % p; }

/* uma identidade em QUATRO variáveis: devolve o lado esquerdo menos o direito, em 𝔽ₚ.
 * A identidade vale sse isto é 0 para todas as atribuições. */
typedef int (*RfId4)(int a, int b, int c, int d, int p);

/* o refutador: varre TODAS as atribuições em 𝔽ₚ e devolve o primeiro contra-exemplo.
 * Devolve 1 se refutou (e escreve o contra-exemplo), 0 se voltou vazio. */
static int rf_varre(RfId4 f, int p, int *ra, int *rb, int *rc, int *rd){
    for(int a = 0; a < p; a++) for(int b = 0; b < p; b++)
    for(int c = 0; c < p; c++) for(int d = 0; d < p; d++)
        if(f(a,b,c,d,p) != 0){
            if(ra){ *ra = a; *rb = b; *rc = c; *rd = d; }
            return 1;
        }
    return 0;
}
/* e sobre TODOS os primos: refuta se cair em algum, e diz em qual */
static int rf_refuta(RfId4 f, int *primo, int *ra, int *rb, int *rc, int *rd){
    for(int i = 0; i < RF_NP; i++)
        if(rf_varre(f, RF_PRIMOS[i], ra, rb, rc, rd)){
            if(primo) *primo = RF_PRIMOS[i];
            return 1;
        }
    return 0;
}
/* ── AS IDENTIDADES DA CASA, escritas como «esquerda − direita» ────────────────
 * A matriz é [[a,b],[c,d]]; adj = [[d,−b],[−c,a]]; tr = a+d; det = ad−bc. */

/* o CENTRO: M + adj(M) = tr(M)·I — a entrada (0,0) e a (0,1) */
static int id_centro(int a,int b,int c,int d,int p){
    (void)c;
    int e00 = rf_som(a, d, p);                    /* (M + adj M)₀₀ = a + d */
    int t   = rf_som(a, d, p);                    /* tr·I₀₀ = a + d */
    int e01 = rf_sub(b, b, p);                    /* (M + adj M)₀₁ = b − b = 0 */
    return rf_som(rf_sub(e00, t, p), e01, p);
}
/* a MEMBRANA: M·adj(M) = det(M)·I — a entrada (0,0) e a (0,1) */
static int id_membrana(int a,int b,int c,int d,int p){
    int e00 = rf_sub(rf_mul(a,d,p), rf_mul(b,c,p), p);          /* ad − bc */
    int det = rf_sub(rf_mul(a,d,p), rf_mul(b,c,p), p);
    int e01 = rf_sub(rf_mul(a,b,p), rf_mul(b,a,p), p);          /* −ab + ba = 0 */
    return rf_som(rf_sub(e00, det, p), e01, p);
}
/* CAYLEY--HAMILTON 2×2: M² − tr·M + det·I = 0, na entrada (0,0) */
static int id_cayley(int a,int b,int c,int d,int p){
    int m00 = rf_som(rf_mul(a,a,p), rf_mul(b,c,p), p);          /* (M²)₀₀ */
    int t = rf_som(a,d,p), det = rf_sub(rf_mul(a,d,p), rf_mul(b,c,p), p);
    return rf_som(rf_sub(m00, rf_mul(t,a,p), p), det, p);
}
/* LAGRANGE em 2D: (ac+bd)² + (ad−bc)² = (a²+b²)(c²+d²) — directo² + cruzado² = norma */
static int id_lagrange(int a,int b,int c,int d,int p){
    int dir = rf_som(rf_mul(a,c,p), rf_mul(b,d,p), p);
    int cru = rf_sub(rf_mul(a,d,p), rf_mul(b,c,p), p);
    int esq = rf_som(rf_mul(dir,dir,p), rf_mul(cru,cru,p), p);
    int na = rf_som(rf_mul(a,a,p), rf_mul(b,b,p), p);
    int nc = rf_som(rf_mul(c,c,p), rf_mul(d,d,p), p);
    return rf_sub(esq, rf_mul(na,nc,p), p);
}
/* det(A·B) = det A · det B, com A = [[a,b],[c,d]] e B = [[d,c],[b,a]] */
static int id_detmult(int a,int b,int c,int d,int p){
    int p00 = rf_som(rf_mul(a,d,p), rf_mul(b,b,p), p);
    int p01 = rf_som(rf_mul(a,c,p), rf_mul(b,a,p), p);
    int p10 = rf_som(rf_mul(c,d,p), rf_mul(d,b,p), p);
    int p11 = rf_som(rf_mul(c,c,p), rf_mul(d,a,p), p);
    int dp = rf_sub(rf_mul(p00,p11,p), rf_mul(p01,p10,p), p);
    int da = rf_sub(rf_mul(a,d,p), rf_mul(b,c,p), p);
    int db = rf_sub(rf_mul(d,a,p), rf_mul(c,b,p), p);
    return rf_sub(dp, rf_mul(da,db,p), p);
}
/* ── E AS MUTAÇÕES: a mesma identidade com UM defeito ─────────────────────────*/
static int mut_centro(int a,int b,int c,int d,int p){       /* tr trocado por a+a */
    (void)c;
    return rf_sub(rf_som(a,d,p), rf_som(a,a,p), p);
}
static int mut_membrana(int a,int b,int c,int d,int p){     /* det com + em vez de − */
    int e = rf_sub(rf_mul(a,d,p), rf_mul(b,c,p), p);
    int f = rf_som(rf_mul(a,d,p), rf_mul(b,c,p), p);
    return rf_sub(e, f, p);
}
static int mut_cayley(int a,int b,int c,int d,int p){       /* sem o termo det */
    int m00 = rf_som(rf_mul(a,a,p), rf_mul(b,c,p), p);
    int t = rf_som(a,d,p);
    return rf_sub(m00, rf_mul(t,a,p), p);
}
static int mut_lagrange(int a,int b,int c,int d,int p){     /* sem o cruzado */
    int dir = rf_som(rf_mul(a,c,p), rf_mul(b,d,p), p);
    int na = rf_som(rf_mul(a,a,p), rf_mul(b,b,p), p);
    int nc = rf_som(rf_mul(c,c,p), rf_mul(d,d,p), p);
    return rf_sub(rf_mul(dir,dir,p), rf_mul(na,nc,p), p);
}
static int mut_detmult(int a,int b,int c,int d,int p){      /* det(A)+det(B) */
    int p00 = rf_som(rf_mul(a,d,p), rf_mul(b,b,p), p);
    int p01 = rf_som(rf_mul(a,c,p), rf_mul(b,a,p), p);
    int p10 = rf_som(rf_mul(c,d,p), rf_mul(d,b,p), p);
    int p11 = rf_som(rf_mul(c,c,p), rf_mul(d,a,p), p);
    int dp = rf_sub(rf_mul(p00,p11,p), rf_mul(p01,p10,p), p);
    int da = rf_sub(rf_mul(a,d,p), rf_mul(b,c,p), p);
    int db = rf_sub(rf_mul(d,a,p), rf_mul(c,b,p), p);
    return rf_sub(dp, rf_som(da,db,p), p);
}
/* e a que só cai em primos ímpares: (a+b)² = a² + b², verdadeira em 𝔽₂ */
static int mut_quadrado(int a,int b,int c,int d,int p){
    (void)c; (void)d;
    int s = rf_som(a,b,p);
    return rf_sub(rf_mul(s,s,p), rf_som(rf_mul(a,a,p), rf_mul(b,b,p), p), p);
}

/* quantas atribuições foram varridas ao todo — o custo, dito */
static long rf_custo(void){
    long t = 0;
    for(int i = 0; i < RF_NP; i++){
        long p = RF_PRIMOS[i];
        t += p*p*p*p;
    }
    return t;
}
#endif
