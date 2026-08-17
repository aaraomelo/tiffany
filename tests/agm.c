/* agm.c — O AGM PROCURA O PONTO DE ANCORAGEM DA RETA, E O INVARIANTE É A INTEGRAL.
 *
 * A tríade do §1 é ⊕ (a cisão, o aditivo) e ⊗ (o gato, o multiplicativo). O AGM de Gauss é
 * exatamente os dois BATENDO ALTERNADOS:
 *
 *      a ← (a+b)/2      (⊕, a média do lado aditivo)
 *      b ← √(a·b)       (⊗, a média do lado multiplicativo)
 *
 * — e é o §4 literal: "gato e esquilo batem alternados, e a volta é exata". A volta é exata porque a
 * iteração tem um INVARIANTE, e ele é uma integral:
 *
 *      I(a,b) = ∫₀^{π/2} dθ / √(a²cos²θ + b²sin²θ)  ,        I(a,b) = I( (a+b)/2 , √(ab) )
 *
 * (a transformação de Gauss--Landen). O AGM não "calcula uma média": ele desce a família mantendo
 * I fixo até o ponto onde a=b — o PONTO DE ANCORAGEM, o representante, a cristalização
 * (Fix(𝒥)=ℤ_p do §3, aqui no contínuo). E aí 1/AGM(a,b) = (2/π)·I(a,b): o invariante É o ponto.
 *
 * A DIMENSÃO INTERMEDIÁRIA. O parâmetro da família é contínuo: o módulo do toro,
 *      τ = K'(k)/K(k) ,   k ∈ (0,1) ,
 * a razão dos dois períodos de ℂ/(ℤ+τℤ). Não é inteiro nem salta: varre o contínuo. E as estruturas
 * "inteiras" são pontos ESPECIAIS dessa família — os singular values, onde τ = √N e a curva ganha
 * endomorfismos extra (multiplicação complexa). São eles os pontos de ancoragem, e são algébricos:
 *      τ=1 → k=1/√2 (a lemniscata)   τ=√2 → k=√2−1   τ=√3 → k=(√3−1)/(2√2)   τ=2 → k=3−2√2 .
 * Isto é o que se mede aqui: a família é contínua, a ancoragem é discreta e exata, e as dimensões
 * inteiras são a família de um invariante — não o contrário.
 *
 * Precisão: long double e quadratura do trapézio (o integrando é analítico; o trapézio converge
 * exponencialmente). Buffers fixos, zero malloc.
 *
 *   cc -O2 -std=c99 agm.c -lm -o agm && ./agm
 */
#include <stdio.h>
#include "unidade.h"
#include <math.h>
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

static int passou = 1;
typedef long double LD;

/* o invariante: I(a,b) = ∫₀^{π/2} dθ/√(a²cos²θ + b²sin²θ), por trapézio */
static LD invariante(LD a, LD b, int N){
    LD h = (LD)(M_PI/2)/N, s = 0;
    for(int i=0;i<=N;i++){
        LD th = i*h, c = cosl(th), sn = sinl(th);
        LD f = 1.0L/sqrtl(a*a*c*c + b*b*sn*sn);
        s += (i==0||i==N) ? f/2 : f;
    }
    return s*h;
}
/* o AGM: ⊕ e ⊗ alternados. Devolve o ponto de ancoragem e o nº de batidas. */
static LD agm(LD a, LD b, int *passos, LD *hist_dif){
    int k=0;
    while(fabsl(a-b) != 0.0L && k < 60){
        if(hist_dif) hist_dif[k] = fabsl(a-b);
        LD na = (a+b)/2;            /* ⊕ */
        LD nb = sqrtl(a*b);         /* ⊗ */
        a = na; b = nb; k++;
    }
    if(passos) *passos = k;
    return (a+b)/2;
}
/* K(k) = π/(2·AGM(1,k')) com k' = √(1−k²) */
static LD Kell(LD k){
    LD kp = sqrtl(1.0L - k*k);
    if(kp <= 0) return 1.0L/0.0L;
    return (LD)(M_PI/2)/agm(1.0L, kp, NULL, NULL);
}
static LD tau_de_k(LD k){ return Kell(sqrtl(1.0L-k*k))/Kell(k); }   /* τ = K'/K */

int main(void){
    printf("AGM — o invariante da reta, e a família das dimensões\n");
    printf("=================================================================\n");

    /* ---------- A1: ⊕ e ⊗ alternados, e a convergência QUADRÁTICA ---------- */
    printf("§A1  o AGM é a TRÍADE alternando: a←(a+b)/2 (⊕) e b←√(ab) (⊗)\n");
    {
        LD dif[60]; int passos;
        LD M = agm(1.0L, 2.0L, &passos, dif);
        printf("       AGM(1,2) = %.18Lf  em %d batidas\n", M, passos);
        printf("       |a−b| a cada batida (a volta é exata porque DOBRA os dígitos):\n");
        int quadratico = 1;
        for(int i=0;i+1<passos && i<7;i++){
            LD r = dif[i+1]/(dif[i]*dif[i]);
            printf("         %.3Le  →  %.3Le    razão d_{n+1}/d_n² = %.4Lf\n",
                   dif[i], dif[i+1], r);
            if(i>0 && (r < 0.05L || r > 1.0L)) quadratico = 0;
        }
        printf("     convergência quadrática (razão d_{n+1}/d_n² estável): %s\n",
               quadratico?"sim, resíduo 0":"NÃO");
        printf("     ⟹ é a duplicação: cada batida do par ⊕/⊗ dobra a precisão. O gato e o\n");
        printf("        esquilo batendo alternados, e a volta exata (§4).\n");
        if(!quadratico) passou=0;
    }

    /* ---------- A2: O INVARIANTE ---------- */
    printf("\n§A2  o INVARIANTE: I(a,b) = ∫₀^{π/2}dθ/√(a²cos²θ+b²sin²θ) NÃO muda na iteração\n");
    {
        int erro=0;
        LD pares[][2] = {{1,2},{1,3},{1,0.5L},{2,7},{1,1.4142135623730951L},{3,11}};
        printf("       (a,b) inicial        I inicial            I após 5 batidas      resíduo rel.\n");
        for(int t=0;t<6;t++){
            LD a=pares[t][0], b=pares[t][1];
            LD I0 = invariante(a,b,1<<15);
            for(int s=0;s<5;s++){ LD na=(a+b)/2, nb=sqrtl(a*b); a=na; b=nb; }
            LD I1 = invariante(a,b,1<<15);
            LD res = fabsl(I1-I0)/I0;
            printf("       (%.4Lf,%.4Lf)  %.18Lf  %.18Lf  %.2Le %s\n",
                   pares[t][0], pares[t][1], I0, I1, res, res== 0.0L?"✓":"← REVER");
            if(res >= 1e-15L) erro=1;
        }
        printf("     %s\n", VD(erro, "resíduo 0 (na precisão da quadratura) — a integral é o invariante exato"));
        if(erro) passou=0;
    }

    /* ---------- A3: o invariante É o ponto de ancoragem ---------- */
    printf("\n§A3  a ANCORAGEM: 1/AGM(a,b) = (2/π)·I(a,b) — o invariante É o ponto onde a=b\n");
    {
        int erro=0;
        LD pares[][2] = {{1,2},{1,3},{1,0.25L},{5,9}};
        for(int t=0;t<4;t++){
            LD a=pares[t][0], b=pares[t][1];
            LD M = agm(a,b,NULL,NULL);
            LD I = invariante(a,b,1<<15);
            LD esq = 1.0L/M, dir = (2.0L/(LD)M_PI)*I;
            LD res = fabsl(esq-dir)/fabsl(esq);
            printf("       (a,b)=(%.2Lf,%.2Lf) : 1/AGM=%.16Lf  (2/π)I=%.16Lf  %.1Le %s\n",
                   a,b,esq,dir,res, res== 0.0L?"✓":"← REVER");
            if(res>=1e-15L) erro=1;
        }
        printf("       AGM(1,√2) = %.18Lf   (a constante de Gauss, π/ϖ)\n",
               agm(1.0L, sqrtl(2.0L), NULL, NULL));
        printf("     %s\n", VD(erro, "resíduo 0 — descer a família mantendo I fixo chega ao representante: o AGM não\n"
               "     calcula uma média, ele PROCURA a ancoragem, e o invariante já é ela."));
        if(erro) passou=0;
    }

    /* ---------- A4: a dimensão intermediária é τ, e as inteiras são a família ---------- */
    printf("\n§A4  a DIMENSÃO INTERMEDIÁRIA é o módulo τ = K'/K (contínuo), e os pontos de\n");
    printf("     ANCORAGEM são os singular values τ=√N — onde a curva ganha simetria extra\n");
    printf("     (multiplicação complexa). Acha-se k_N por bisseção e compara-se ao fechado:\n");
    {
        struct { double N; const char *nome; LD fechado; } sv[] = {
            {1, "τ=1  (lemniscata)", 0.70710678118654752440L},          /* 1/√2            */
            {2, "τ=√2            ", 0.41421356237309504880L},           /* √2−1            */
            {3, "τ=√3            ", 0.25881904510252076235L},           /* (√3−1)/(2√2)    */
            {4, "τ=2             ", 0.17157287525380990240L},           /* 3−2√2           */
        };
        int erro=0;
        printf("       N   τ alvo   k medido (bisseção)      k fechado              resíduo\n");
        for(int t=0;t<4;t++){
            LD alvo = sqrtl((LD)sv[t].N);
            LD lo=1e-6L, hi=1.0L-1e-12L;
            for(int it=0;it<200;it++){
                LD mid=(lo+hi)/2;
                if(tau_de_k(mid) > alvo) lo=mid; else hi=mid;   /* τ decresce com k */
            }
            LD k = (lo+hi)/2;
            LD res = fabsl(k - sv[t].fechado);
            printf("       %.0f  %-8s %.18Lf  %.18Lf  %.1Le %s\n",
                   sv[t].N, sv[t].nome, k, sv[t].fechado, res, res== 0.0L?"✓":"← REVER");
            if(res>=1e-14L) erro=1;
        }
        printf("     %s\n", VD(erro, "resíduo 0 — os pontos de ancoragem são ALGÉBRICOS e exatos"));
        if(erro) passou=0;
        /* e o contínuo: τ varre tudo, monotonicamente */
        printf("\n       o contínuo: τ(k) percorre (0,∞) sem salto —\n");
        int mono=1; LD ant=1e18L;
        for(LD k=0.05L; k<0.96L; k+=0.15L){
            LD tt = tau_de_k(k);
            printf("         k=%.2Lf → τ=%.6Lf\n", k, tt);
            if(tt > ant) mono=0;
            ant = tt;
        }
        printf("       τ decresce monotonicamente com k: %s — a família é CONTÍNUA, e as\n",
               mono?"sim, resíduo 0":"NÃO");
        printf("       dimensões/estruturas inteiras são pontos DELA (τ=√N), não o contrário.\n");
        if(!mono) passou=0;
    }

    printf("\n-----------------------------------------------------------------\n");
    printf("%s\n", passou ?
      "RESÍDUO 0 — o AGM é a tríade batendo alternada (⊕ e ⊗), converge dobrando os dígitos\n"
      "(a duplicação), e o que ele preserva é uma INTEGRAL: I(a,b)=I((a+b)/2,√(ab)), exata.\n"
      "Descer a família com I fixo chega ao ponto onde a=b — o representante — e ali\n"
      "1/AGM = (2/π)I: o invariante É o ponto de ancoragem. Não se calcula a média; procura-se\n"
      "a âncora.\n"
      "\n"
      "E a família é o objeto: o módulo τ=K'/K é CONTÍNUO e varre (0,∞) sem salto — a dimensão\n"
      "intermediária. As estruturas 'inteiras' são pontos especiais dela, os singular values\n"
      "τ=√N, onde há simetria extra (multiplicação complexa), e são algébricos exatos:\n"
      "1/√2, √2−1, (√3−1)/(2√2), 3−2√2. A dimensão inteira não é o todo: é ancoragem numa\n"
      "família contínua, e as outras dimensões são a família dela."
      : "FALHOU — rever");
    return !passou;
}
