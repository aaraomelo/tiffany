/* defeito.c — O DEFEITO E_k, AVALIADO NUMA BANDA REAL.
 *
 * isserlis.c validou a maquinaria (a esperança gaussiana é soma finita de emparelhamentos, e as
 * constantes do capítulo são elas próprias contagens de Wick). Aqui ela é USADA: constrói-se uma
 * banda pela regra real de broca-so/neuronio/n_qubit.py e mede-se o defeito dela.
 *
 * A construção, fiel ao código: byte b -> (par, ímpar) = (popcount(b & 0xAA), popcount(b & 0x55))
 * -> (e,o) = A·(par,ímpar) pelo GATO -> ângulos θ=πe/(e+o), φ=πo/8 -> |ψ⟩ ∈ C² na esfera de
 * Bloch -> |Ψ⟩ = ⊗ |ψ_t⟩ ∈ C^{2^6}. (O passo espectro->bytes NÃO é reimplementado aqui: entram
 * bytes reais de um arquivo do próprio repositório, no lugar dos seis bins mais fortes. Está dito
 * porque muda o que se pode afirmar: a construção do TENSOR é fiel, a extração espectral não é.)
 *
 * E o defeito, com B = |Ψ⟩ lido como k-tensor (k=6, V=C², um índice livre e cinco contraídos):
 *
 *      E_k(B) = E[ (‖B(a_1,…,a_5)‖² − 1)² ],   a_t ~ N(0, I_2) independentes
 *
 * Para ‖B‖=1 vale E[‖B(a)‖²]=‖B‖²_F=1, logo E_k é a VARIÂNCIA de ‖B(a)‖² em torno de 1. Por
 * Cauchy–Schwarz E_k ≥ 0, com igualdade se e só se ‖B(a)‖ é CONSTANTE — isto é, se e só se a
 * contração preserva norma. É exatamente a condição de recuperação exata: defeito zero é a cifra
 * ser bijeção. O defeito mede quanto falta para isso.
 *
 *   §D1  a banda real: os seis fatores, e a norma
 *   §D2  E_k por Isserlis — e conferido de fora, contra quadratura exata
 *   §D3  produto contra emaranhado: qual sela e qual recupera
 *   §D4  o piso: E_k ≥ 0, e zero é norma constante
 *
 *   cc -O2 -std=c99 defeito.c -lm -o defeito && ./defeito [arquivo]
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

typedef long double LD;
typedef struct { LD re, im; } C;
static C cmul(C a, C b){ C r = { a.re*b.re - a.im*b.im, a.re*b.im + a.im*b.re }; return r; }
static LD cabs2(C a){ return a.re*a.re + a.im*a.im; }
static C conj_(C a){ C r = { a.re, -a.im }; return r; }

static int falhas = 0;
static void ok(const char *r, int c){
    printf("      %-56s %s\n", r, c ? "sim  ✓" : "NÃO  ✗");
    if(!c) falhas++;
}

#define NQ  6                       /* o hexagrama */
#define DIM 64                      /* 2^6 */
#define NCONTR (NQ-1)               /* cinco contraídos, um livre */
#define SUB 32                      /* 2^5 */
static const LD PI_L = 3.14159265358979323846264338327950288L;

/* --- a regra real: byte -> (e,o) pelo gato -> qubit na esfera de Bloch --- */
static void passo_gato_byte(int b, int *e, int *o){
    int par = __builtin_popcount(b & 0xAA), imp = __builtin_popcount(b & 0x55);
    *e = 1*par + 1*imp;             /* A_1 = [[1,1],[1,0]] aplicada a (par, ímpar) */
    *o = 1*par + 0*imp;
}
static void qubit_de_gato(int e, int o, C q[2]){
    int s = e + o;
    if(s == 0){ q[0].re = 1; q[0].im = 0; q[1].re = 0; q[1].im = 0; return; }
    LD th = PI_L * e / s, ph = PI_L * o / 8.0L;
    q[0].re = cosl(th/2); q[0].im = 0;
    q[1].re = sinl(th/2)*cosl(ph); q[1].im = sinl(th/2)*sinl(ph);
}

/* --- E[‖B(a)‖⁴] por Isserlis: momento 4 por vetor, três emparelhamentos --- */
/* M4[j][j'][l][l'] = δ_{jj'}δ_{ll'} + δ_{jl}δ_{j'l'} + δ_{jl'}δ_{j'l}  (covariância = I) */
static int M4(int j, int jl, int l, int ll){
    return (j==jl && l==ll) + (j==l && jl==ll) + (j==ll && jl==l);
}
/* B[i][J], i o índice livre (2), J os cinco contraídos (32) */
static LD esperanca_quarta(C B[2][SUB]){
    LD tot = 0;
    for(int J = 0; J < SUB; J++) for(int Jl = 0; Jl < SUB; Jl++)
    for(int L = 0; L < SUB; L++) for(int Ll = 0; Ll < SUB; Ll++){
        int fator = 1;
        for(int t = 0; t < NCONTR && fator; t++){
            int j  = (J  >> t) & 1, jl = (Jl >> t) & 1;
            int l  = (L  >> t) & 1, ll = (Ll >> t) & 1;
            fator *= M4(j, jl, l, ll);
        }
        if(!fator) continue;
        for(int i = 0; i < 2; i++) for(int ip = 0; ip < 2; ip++){
            C a = cmul(B[i][J],  conj_(B[i][Jl]));
            C b = cmul(B[ip][L], conj_(B[ip][Ll]));
            tot += fator * cmul(a, b).re;
        }
    }
    return tot;
}
static LD norma2(C B[2][SUB]){
    LD s = 0;
    for(int i = 0; i < 2; i++) for(int J = 0; J < SUB; J++) s += cabs2(B[i][J]);
    return s;
}
/* --- conferência de fora: Gauss–Hermite de 3 nós (exata até grau 5) em 10 dimensões --- */
static const LD G3X[3] = { -1.7320508075688772935L, 0.0L, 1.7320508075688772935L };
static const LD G3W[3] = { 0.16666666666666666667L, 0.66666666666666666667L, 0.16666666666666666667L };
static LD quadratura_quarta(C B[2][SUB]){
    int idx[NCONTR*2]; memset(idx, 0, sizeof idx);
    LD tot = 0;
    long total = 1; for(int i = 0; i < NCONTR*2; i++) total *= 3;
    for(long n = 0; n < total; n++){
        long r = n; LD w = 1, a[NCONTR][2];
        for(int t = 0; t < NCONTR; t++) for(int c = 0; c < 2; c++){
            int k = (int)(r % 3); r /= 3;
            a[t][c] = G3X[k]; w *= G3W[k];
        }
        LD s = 0;
        for(int i = 0; i < 2; i++){
            C X = {0,0};
            for(int J = 0; J < SUB; J++){
                LD coef = 1;
                for(int t = 0; t < NCONTR; t++) coef *= a[t][(J >> t) & 1];
                X.re += B[i][J].re * coef; X.im += B[i][J].im * coef;
            }
            s += cabs2(X);
        }
        tot += w * s * s;
    }
    return tot;
}

int main(int argc, char **argv){
    const char *arq = argc > 1 ? argv[1] : "../teoria.tex";

printf("\n=== O DEFEITO E_k NUMA BANDA REAL ==========================================\n");
printf("    E_k = 0 é a cifra ser bijeção. O defeito mede quanto falta para isso.\n");

/* ---------------------------------------------------------------- §D1 ------ */
printf("\n§D1  A banda: seis bytes reais, cada um virando um fator tensorial pelo gato.\n\n");
    unsigned char by[NQ];
    {
        FILE *f = fopen(arq, "rb");
        if(!f){ perror(arq); return 2; }
        unsigned char buf[4096];
        size_t n = fread(buf, 1, sizeof buf, f);
        fclose(f);
        if(n < NQ){ fprintf(stderr, "arquivo curto\n"); return 2; }
        /* seis bytes espalhados, no lugar dos seis bins mais fortes do espectro */
        for(int t = 0; t < NQ; t++) by[t] = buf[(t*577) % n];
        printf("      fonte: %s (%zu bytes lidos)\n\n", arq, n);
    }
    C q[NQ][2];
    printf("      t   byte   par ímpar   (e,o) pelo gato    θ/π      φ/π\n");
    for(int t = 0; t < NQ; t++){
        int e, o;
        int par = __builtin_popcount(by[t] & 0xAA), imp = __builtin_popcount(by[t] & 0x55);
        passo_gato_byte(by[t], &e, &o);
        qubit_de_gato(e, o, q[t]);
        printf("      %d   0x%02X   %3d %5d   %6d %6d    %7.4Lf  %7.4Lf\n",
               t, by[t], par, imp, e, o,
               (e+o) ? (LD)e/(e+o) : 0.0L, (LD)o/8.0L);
    }
    /* |Ψ⟩ = ⊗ |ψ_t⟩, e a leitura como k-tensor: índice livre = qubit 0 */
    C Bp[2][SUB];
    for(int i = 0; i < 2; i++) for(int J = 0; J < SUB; J++){
        C v = q[0][i];
        for(int t = 0; t < NCONTR; t++) v = cmul(v, q[t+1][(J >> t) & 1]);
        Bp[i][J] = v;
    }
    LD n2p = norma2(Bp);
    printf("\n      ‖Ψ‖² do estado produto ........................ %.15Lf\n", n2p);
    ok("a banda é um estado normalizado", fabsl(n2p - 1.0L) < 1e-15L);

/* ---------------------------------------------------------------- §D2 ------ */
printf("\n§D2  E_k por Isserlis, e conferido de FORA contra quadratura exata.\n");
printf("     (3 nós por dimensão, 10 dimensões: exata até grau 5, e o integrando é 4.)\n\n");
    LD q4_iss = esperanca_quarta(Bp);
    LD q4_qua = quadratura_quarta(Bp);
    LD Ek_iss = q4_iss - 2.0L*n2p + 1.0L;
    LD Ek_qua = q4_qua - 2.0L*n2p + 1.0L;
    printf("      E[‖B(a)‖⁴]  Isserlis .................... %.15Lf\n", q4_iss);
    printf("      E[‖B(a)‖⁴]  quadratura .................. %.15Lf\n", q4_qua);
    printf("      resíduo entre os dois métodos ............. %.2Le\n", fabsl(q4_iss - q4_qua));
    ok("Isserlis = quadratura, na banda real", fabsl(q4_iss - q4_qua) < 1e-12L);
    printf("\n      E_k(banda) = E[‖B(a)‖⁴] − 2‖B‖² + 1 ....... %.15Lf\n", Ek_iss);
    printf("      (o mesmo pela quadratura) ................. %.15Lf\n", Ek_qua);

/* ---------------------------------------------------------------- §D3 ------ */
printf("\n§D3  Produto contra emaranhado: a mesma banda, as duas chaves.\n\n");
    C Bg[2][SUB];
    {
        /* |GHZ⟩ = (|0…0⟩ + |1…1⟩)/√2, misturado como no código (ghz_misto) */
        C ghz[2][SUB]; memset(ghz, 0, sizeof ghz);
        ghz[0][0].re = 1.0L/sqrtl(2.0L);
        ghz[1][SUB-1].re = 1.0L/sqrtl(2.0L);
        /* alpha = ⟨Ψ|GHZ⟩ ; psi = alpha·GHZ + sqrt(1−|alpha|²)·Ψ  (n_qubit.py) */
        C al = {0,0};
        for(int i = 0; i < 2; i++) for(int J = 0; J < SUB; J++){
            C c = cmul(conj_(Bp[i][J]), ghz[i][J]); al.re += c.re; al.im += c.im;
        }
        LD be = sqrtl(fmaxl(0.0L, 1.0L - cabs2(al)));
        LD nn = 0;
        for(int i = 0; i < 2; i++) for(int J = 0; J < SUB; J++){
            C t = cmul(al, ghz[i][J]);
            Bg[i][J].re = t.re + be*Bp[i][J].re;
            Bg[i][J].im = t.im + be*Bp[i][J].im;
            nn += cabs2(Bg[i][J]);
        }
        nn = sqrtl(nn);
        for(int i = 0; i < 2; i++) for(int J = 0; J < SUB; J++){ Bg[i][J].re /= nn; Bg[i][J].im /= nn; }
    }
    LD n2g = norma2(Bg), q4g = esperanca_quarta(Bg);
    LD Ekg = q4g - 2.0L*n2g + 1.0L;
    printf("      chave            ‖Ψ‖²              E_k\n");
    printf("      dividida (⊗)     %.12Lf     %.12Lf\n", n2p, Ek_iss);
    printf("      do sistema (GHZ) %.12Lf     %.12Lf\n", n2g, Ekg);
    printf("\n      razão E_k(sistema) / E_k(dividida) ........ %.6Lf\n",
           Ek_iss > 0 ? Ekg/Ek_iss : 0.0L);
    ok("as duas chaves têm defeitos DIFERENTES (não é a mesma coisa)",
       fabsl(Ekg - Ek_iss) > 1e-9L);

/* ---------------------------------------------------------------- §D4 ------ */
printf("\n§D4  O piso: E_k ≥ 0, e o zero é norma constante — a recuperação exata.\n\n");
    printf("      Por Cauchy–Schwarz, E[‖B(a)‖⁴] ≥ (E[‖B(a)‖²])² = ‖B‖⁴, logo para ‖B‖=1\n");
    printf("      vale E_k = Var(‖B(a)‖²) ≥ 0, com igualdade se e só se ‖B(a)‖ NÃO varia.\n\n");
    ok("E_k da banda é não-negativo", Ek_iss >= -1e-15L);
    ok("E_k do sistema é não-negativo", Ekg >= -1e-15L);
    printf("\n      E o defeito da banda medida NÃO é zero: %.6Lf. Logo esta contração não\n", Ek_iss);
    printf("      preserva norma, e a recuperação por ela não é exata — o que é informação\n");
    printf("      dura, e não uma falha do medidor. O zero de E_k é privilégio das álgebras\n");
    printf("      de divisão normadas (Hurwitz: 1, 2, 4, 8), e é por isso que o módulo do\n");
    printf("      corpo local dá |x|^d com d nessas dimensões (antissimetrico.c §A5).\n");

/* ---------------------------------------------------------------- §D5 ------ */
printf("\n§D5  De onde vêm os 87: o defeito é MULTIPLICATIVO nos fatores.\n\n");
{
    /* No estado produto, X_i = ψ0[i]·∏_t z_t com z_t = Σ_j ψt[j]·a_t[j]. Logo
     * ‖X‖² = ∏|z_t|² e, por independência, E[‖X‖⁴] = ∏_t E[|z_t|⁴]. */
    LD prod = 1;
    printf("      t   E[|z_t|⁴]   (2 = fase equilibrada, 3 = amplitude real)\n");
    for(int t = 1; t < NQ; t++){
        /* E[|z|⁴] = 3σu⁴ + 2σu²σv² + 4ρ²σu²σv² + 3σv⁴, com u,v as partes de ⟨ψ,a⟩ */
        LD su2 = 0, sv2 = 0, cuv = 0;
        for(int j = 0; j < 2; j++){
            su2 += q[t][j].re * q[t][j].re;
            sv2 += q[t][j].im * q[t][j].im;
            cuv += q[t][j].re * q[t][j].im;
        }
        LD e4 = 3*su2*su2 + 3*sv2*sv2 + 2*su2*sv2 + 4*cuv*cuv;
        prod *= e4;
        printf("      %d   %9.6Lf\n", t, e4);
    }
    printf("\n      ∏_t E[|z_t|⁴] .............................. %.12Lf\n", prod);
    printf("      E[‖B(a)‖⁴] medido .......................... %.12Lf\n", q4_iss);
    ok("o defeito FATORIZA sobre os clientes", fabsl(prod - q4_iss) < 1e-9L);
    printf("\n      Cada fator contribui entre 2 e 3: 3 quando a amplitude é real, 2 quando a\n");
    printf("      fase reparte igual entre real e imaginária (ψ = (1, i)/√2). Logo o defeito\n");
    printf("      de k clientes fica entre 2^(k-1) e 3^(k-1) — ele CRESCE com o número de\n");
    printf("      partes, e não linearmente. Isso é instrução de projeto, não observação:\n");
    printf("      para uma chave com muitos clientes valer, cada fator tem de estar na fase\n");
    printf("      equilibrada, e mesmo assim o piso sobe. A recuperação exata (E_k = 0) não\n");
    printf("      é alcançável por produto de fatores independentes — só por estrutura, que\n");
    printf("      é onde Hurwitz entra e por que as dimensões são 1, 2, 4, 8.\n");
}

printf("\n=== O QUE ISSO DIZ =========================================================\n");
printf("  O defeito deixou de ser conceito e passou a ter número numa banda construída\n");
printf("  pela regra real. E o número não é zero — a chave, como está, não é isometria.\n");
printf("  Isso é o que se queria saber: E_k não decora o desenho, ele MEDE o desenho, e\n");
printf("  agora dá para comparar candidatas em vez de escolher por gosto.\n");
if(falhas){ printf("\n  FALHAS: %d\n\n", falhas); return 1; }
printf("\n  RESÍDUO 0 — Isserlis e quadratura concordam na banda real.\n\n");
return 0;
}
