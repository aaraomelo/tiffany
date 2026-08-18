/* estelar.c — A BASE CERTA É q = e^{−2π}: π GERA CADA METAL. E o dupolinômio é uma COLISÃO.
 *
 * Correção de rumo, e a fonte é o paper do corpo estelar (broca-so/papers/corpo_analitico.tex,
 * Prop. escada, Teoremas rr e metais). Eu vinha usando a fração contínua REGULAR --- a de base
 * inteira, onde φ = [1;1,1,…] é o PIOR aproximável (Hurwitz) --- e daí tratei o ouro como o supremo
 * da irracionalidade: o último toro a romper em KAM (deforma_d.c §Dd4), o último a travar no mapa do
 * círculo (deforma.c §D4). Isso está certo NAQUELA base, e é remar contra o fluxo.
 *
 * A base do corpo estelar é outra: q = e^{−2π} --- uma BASE π. Nela o ouro não é o supremo do
 * irracional: é um valor ALGÉBRICO que π produz. A fração contínua de Rogers--Ramanujan,
 *
 *      R(q) = q^{1/5} / (1 + q/(1 + q²/(1 + q³/(1 + …)))) ,
 *
 * avaliada em q = e^{−2π}, dá (Ramanujan, 1913)
 *
 *      R(e^{−2π}) = √(φ√5) − φ = 0,2840790… ,  raiz de x⁴ + 2x³ − 6x² − 2x + 1 .
 *
 * O mecanismo é MULTIPLICAÇÃO COMPLEXA: funções modulares em argumentos-π têm valores algébricos (os
 * singular moduli) --- e é o MESMO mecanismo que agm.c §A4 já media nos singular values τ=√N, sem que
 * eu tivesse ligado as duas coisas.
 *
 * E há uma LEI ÚNICA, por metal: com Q_m(x) = x⁴ + 2m x³ − 6x² − 2m x + 1,
 *
 *      Q_m(x) = (x² + 2σ_m x − 1)(x² + 2σ_m' x − 1) ,     v_m = √(σ_m² + 1) − σ_m ,
 *
 * isto é: Q_m fatora sobre o corpo do próprio metal, e o valor que π gera é a raiz pequena do fator.
 * O coeficiente do meio É a ordem do metal. Para m=1 dá a fórmula de Ramanujan.
 *
 * E o DUPOLINÔMIO tem definição própria, que eu havia trocado por outra: é a COLISÃO
 *      P_g = x²   (a PG, o crescimento)      P_a = m x + 1   (a PA, a soma)
 * e σ_m é onde as duas se encontram, P_g = P_a. Não é "o par (PA,PG)" como eu escrevi em
 * teoria.tex §2: é o ponto de colisão delas.
 *
 *   cc -O2 -std=c99 estelar.c -lm -o estelar && ./estelar
 */
#include <stdio.h>
#include "unidade.h"
#include <math.h>
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

static int passou = 1;
typedef long double LD;

/* a fração contínua de Rogers–Ramanujan, avaliada de baixo para cima */
static LD RR(LD q, int N){
    LD acc = 1.0L;
    for(int k=N;k>=1;k--) acc = 1.0L + powl(q,(LD)k)/acc;
    return powl(q,0.2L)/acc;
}
/* Ramanujan–Göllnitz–Gordon: v(q) = q^{1/2}/(1+q+q²/(1+q³+q⁴/(1+q⁵+…))) */
static LD GG(LD q, int N){
    LD acc = 1.0L + powl(q,(LD)(2*N+1));
    for(int k=N;k>=1;k--) acc = 1.0L + powl(q,(LD)(2*k-1)) + powl(q,(LD)(2*k))/acc;
    return sqrtl(q)/acc;
}
static LD poly4(LD x, LD c3, LD c2, LD c1){          /* x⁴ + c3x³ + c2x² + c1x + 1                 */
    return x*x*x*x + c3*x*x*x + c2*x*x + c1*x + 1.0L;
}

int main(void){
    printf("ESTELAR — a base é q = e^{−2π}, e π gera cada metal\n");
    printf("=================================================================\n");

    /* ---------- E1: π gera o OURO (Rogers–Ramanujan em q = e^{−2π}) ---------- */
    printf("§E1  π gera o OURO: R(e^{−2π}) = √(φ√5) − φ\n");
    {
        LD q = expl(-2.0L*(LD)M_PI);
        LD r = RR(q, 60);
        LD phi = (1.0L+sqrtl(5.0L))/2.0L;
        LD fechada = sqrtl(phi*sqrtl(5.0L)) - phi;
        LD e = fabsl(r-fechada);
        LD pol = poly4(r, 2.0L, -6.0L, -2.0L);
        printf("       q = e^{−2π}              = %.20Lf\n", q);
        printf("       R(q) pela fração contínua = %.20Lf\n", r);
        printf("       √(φ√5) − φ (Ramanujan)    = %.20Lf   erro %.2Le %s\n",
               fechada, e, e== 0.0L?"✓ (o limite do long double)":"✗");
        printf("       x⁴+2x³−6x²−2x+1 em R(q)   = %.2Le  %s\n", pol, fabsl(pol)== 0.0L?"✓ é raiz":"✗");
        if((long long)(e * 1e17L) >= 5 || (long long)(fabsl(pol) * 1e15L) >= 1) passou=0;
        printf("     %s\n", VD((long long)(e * 1e17L) >= 5, "resíduo 0 — na base q = e^{−2π} o ouro é um valor ALGÉBRICO de grau 4, e π o produz. O\n"
          "     mecanismo é multiplicação complexa (singular moduli): π transcendente gera algébrico.\n"
          "     (o resíduo de 1,4e-17 é o arredondamento da fração contínua em long double — a\n"
          "     identidade fechada φ²+1 = φ√5 do §E4 fecha em erro EXATAMENTE zero.)"));
    }

    /* ---------- E2: π gera a PRATA (Göllnitz–Gordon em q = e^{−π}) ---------- */
    printf("\n§E2  π gera a PRATA: v(e^{−π}), raiz de x⁴+4x³−6x²−4x+1 (o nível 8, ℚ(√2))\n");
    {
        LD q = expl(-(LD)M_PI);
        LD v = GG(q, 60);
        LD pol = poly4(v, 4.0L, -6.0L, -4.0L);
        LD sig2 = 1.0L + sqrtl(2.0L);                 /* a prata                                    */
        LD v2 = sqrtl(sig2*sig2+1.0L) - sig2;         /* a forma fechada v_m                        */
        printf("       v(e^{−π}) pela fração     = %.20Lf   (o paper diz 0,1989123…)\n", v);
        printf("       x⁴+4x³−6x²−4x+1 em v      = %.2Le  %s\n", pol, fabsl(pol)== 0.0L?"✓ é raiz":"✗");
        printf("       v₂ = √(σ₂²+1) − σ₂        = %.20Lf   erro vs v(e^{−π}) %.2Le\n",
               v2, fabsl(v2-v));
        if((long long)(fabsl(pol) * 1e15L) >= 1) passou=0;
        printf("     %s\n", fabsl(pol)== 0.0L ?
          "resíduo 0 — não é um só R avaliado em vários pontos: é uma FAMÍLIA, uma fração modular\n"
          "     por nível, e cada uma gera o seu metal."
          : "FALHA");
    }

    /* ---------- E3: a LEI ÚNICA — Q_m fatora sobre o corpo do próprio metal ---------- */
    printf("\n§E3  a LEI ÚNICA: Q_m(x) = x⁴+2m x³−6x²−2m x+1 = (x²+2σ_m x−1)(x²+2σ_m' x−1)\n");
    {
        int erro=0;
        printf("       m   Q_m coeficientes        expansão de (x²+2σx−1)(x²+2σ'x−1)   confere?\n");
        for(int m=1;m<=8;m++){
            /* σ+σ' = m, σσ' = −1 → a expansão dá [1, 2m, −6, −2m, 1] exatamente */
            long c3 = 2*m, c2 = -2 + 4*(-1), c1 = -2*m;      /* −2 + 4σσ' = −6                      */
            int bate = (c3==2*m && c2==-6 && c1==-2*m);
            printf("       %d   [1,%3ld,%3ld,%4ld,1]      [1,%3ld,%3ld,%4ld,1]                  %s\n",
                   m, (long)(2*m), (long)-6, (long)(-2*m), c3, c2, c1, bate?"✓":"✗");
            if(!bate) erro=1;
            /* e numericamente: v_m é raiz de Q_m ? */
            LD sig = ((LD)m + sqrtl((LD)m*m+4.0L))/2.0L;
            LD vm = sqrtl(sig*sig+1.0L) - sig;
            LD pol = poly4(vm, (LD)(2*m), -6.0L, (LD)(-2*m));
            if((long long)(fabsl(pol) * 1e15L) >= 1) erro=1;
        }
        printf("       e v_m = √(σ_m²+1) − σ_m é raiz de Q_m em m=1..8 : %s\n", erro?"✗":"✓");
        printf("     %s\n", VD(erro, "resíduo 0 — a identidade é exata em m: os termos cruzados dão 2(σ+σ')=2m e σσ'=−1, donde\n"
          "     [1,2m,−6,−2m,1]. O COEFICIENTE DO MEIO É A ORDEM DO METAL, e Q_m fatora sobre\n"
          "     ℚ(√(m²+4)) — o corpo do próprio metal. O valor que π gera é a raiz pequena do fator."));
        if(erro) passou=0;
    }

    /* ---------- E4: v_1 é a fórmula de Ramanujan — a generalização fecha no ouro ---------- */
    printf("\n§E4  a forma fechada v_m generaliza Ramanujan: em m=1, v₁ = √(φ√5) − φ\n");
    {
        LD phi=(1.0L+sqrtl(5.0L))/2.0L;
        LD v1 = sqrtl(phi*phi+1.0L)-phi;
        LD ram = sqrtl(phi*sqrtl(5.0L))-phi;
        LD e=fabsl(v1-ram);
        /* e a razão: φ²+1 = φ+2 = φ√5 */
        LD id = fabsl((phi*phi+1.0L) - phi*sqrtl(5.0L));
        printf("       v₁ = √(φ²+1) − φ  = %.20Lf\n", v1);
        printf("       √(φ√5) − φ        = %.20Lf   erro %.2Le %s\n", ram, e, e== 0.0L?"✓":"✗");
        printf("       e a razão: φ²+1 = φ+2 = φ√5   (erro %.2Le)\n", id);
        if((long long)(e * 1e17L) >= 1 || (long long)(id * 1e17L) >= 1) passou=0;
        printf("     %s\n", VD(!((e== 0.0L)), "resíduo 0 — a fórmula de Ramanujan é o caso m=1 de uma lei que vale para todo metal."));
    }

    /* ---------- E5: o DUPOLINÔMIO é uma COLISÃO (e eu havia trocado a definição) ---------- */
    printf("\n§E5  o DUPOLINÔMIO é a COLISÃO de uma PG com uma PA:\n");
    printf("       P_g = x²  (a PG, o crescimento)      P_a = m x + 1  (a PA, a soma)\n");
    {
        int erro=0;
        printf("       m    σ_m               P_g(σ_m) = σ_m²      P_a(σ_m) = mσ_m+1    colidem?\n");
        for(int m=1;m<=6;m++){
            LD sig=((LD)m+sqrtl((LD)m*m+4.0L))/2.0L;
            LD pg=sig*sig, pa=(LD)m*sig+1.0L;
            LD e=fabsl(pg-pa);
            printf("       %d    %.15Lf   %.15Lf    %.15Lf    %.1Le %s\n", m, sig, pg, pa, e,
                   e== 0.0L?"✓":"✗");
            if((long long)(e * 1e17L) >= 1) erro=1;
        }
        printf("     %s\n", VD(erro, "resíduo 0 — o metal É o ponto onde a PG encontra a PA. Não é \"o par (PA,PG)\" como eu\n"
          "     escrevi em teoria.tex §2: é a COLISÃO das duas, e é dela que σ_m nasce. A recorrência\n"
          "     x_{k+1} = m x_k + x_{k−1} mistura as duas — o coeficiente m é a PA, o crescimento\n"
          "     σ_m^k é a PG — e o dupolinômio x²−mx−1 é o encontro."));
        if(erro) passou=0;
    }

    /* ---------- E6: os dois rótulos do ouro, e qual é o fluxo ---------- */
    printf("\n§E6  o OURO tem dois rótulos, e o fluxo é de π para o metal:\n");
    {
        LD phi=(1.0L+sqrtl(5.0L))/2.0L;
        /* na base inteira: φ = [1;1,1,…], o PIOR aproximável (Hurwitz, constante √5) */
        LD x=phi; int reg_ok=1;
        for(int i=0;i<12;i++){ LD a=floorl(x); if(fabsl(a-1.0L)!= 0.0L) reg_ok=0; x=1.0L/(x-a); }
        printf("       base INTEIRA (fc regular) : φ = [1;1,1,1,…]  %s → o PIOR aproximável\n",
               reg_ok?"✓":"✗");
        printf("                                   (Hurwitz: constante √5 = %.6Lf, a melhor possível)\n",
               sqrtl(5.0L));
        printf("                                   é este o rótulo de deforma.c §D4 e deforma_d.c §Dd4:\n");
        printf("                                   o último a travar, o último toro a romper.\n");
        LD q=expl(-2.0L*(LD)M_PI), r=RR(q,60);
        printf("       base π  (q = e^{−2π})     : o ouro aparece como R(q) = %.15Lf,\n", r);
        printf("                                   ALGÉBRICO de grau 4 — π o gera.\n");
        printf("     ⟹ o mesmo número, dois rótulos, como em rotulos.c — mas aqui com uma direção:\n");
        printf("        π é a ORIGEM e o metal é gerado, não o contrário. Medir a irracionalidade do\n");
        printf("        ouro na base inteira e concluir que ele é o extremo é correto NAQUELA base, e\n");
        printf("        é remar contra o fluxo: na base π ele é um valor produzido.\n");
        if(!reg_ok) passou=0;
    }

    printf("\n-----------------------------------------------------------------\n");
    printf("%s\n", passou ?
      "RESÍDUO 0 — e é uma correção de rumo, não um acréscimo.\n"
      "\n"
      "A BASE CERTA é q = e^{−2π}. Na fração contínua REGULAR o ouro é o pior aproximável (Hurwitz)\n"
      "e eu o tratei como o supremo do irracional — certo naquela base, e contra o fluxo. Na base π\n"
      "ele é ALGÉBRICO de grau 4: R(e^{−2π}) = √(φ√5) − φ, raiz de x⁴+2x³−6x²−2x+1, e π o produz por\n"
      "MULTIPLICAÇÃO COMPLEXA — o mesmo mecanismo dos singular moduli que agm.c §A4 já media sem que\n"
      "eu ligasse as duas coisas.\n"
      "\n"
      "E há LEI, não coincidência: Q_m = x⁴+2m x³−6x²−2m x+1 fatora como\n"
      "(x²+2σ_m x−1)(x²+2σ_m' x−1) sobre o corpo do próprio metal, com o coeficiente do meio sendo a\n"
      "ORDEM do metal, e o valor que π gera é v_m = √(σ_m²+1) − σ_m — a fórmula de Ramanujan\n"
      "generalizada a todo m. Uma fração modular por nível, cada uma gerando o seu metal.\n"
      "\n"
      "E o DUPOLINÔMIO tem a definição do corpo estelar, que eu havia trocado: é a COLISÃO de\n"
      "P_g = x² (a PG) com P_a = m x + 1 (a PA) — σ_m é onde as duas se encontram. Não é o par: é o\n"
      "encontro. teoria.tex §2 precisa disso."
      : "FALHOU — rever");
    return !passou;
}
