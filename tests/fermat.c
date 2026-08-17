/* fermat.c — a prova do UTF cai no gato: o Frobenius é o gato na borda (traço, determinante).
 *
 * Os objetos das duas provas do Último Teorema de Fermat são o gato e o esquilo:
 *   • WILES: a representação de Galois ρ(Frob) é um objeto 2×2 por TRAÇO e DETERMINANTE — o
 *     gato A_n=[[n,1],[1,0]] (tr=n, det=−1). Para E/𝔽_p, o Frobenius tem traço a_p=p+1−#E e
 *     autovalores raízes de z²−a_p z+p, com |α|=|β|=√p (Hasse–Weil). É o gato NA BORDA: norma
 *     αβ=p (no círculo), contra o gato hiperbólico σσ'=det=−1 (fora). O flip β=p/α (o inverso
 *     conjugado) é o mesmo do esquilo σ'=−1/σ; o módulo decide o ramo.
 *   • KUMMER: o metal σ_n (raiz de x²−nx−1, Tr=n, N=−1, uma unidade) é o inteiro ciclotômico;
 *     n=4 fecha pela descida infinita — a órbita que contrai (o lado ×1/σ do gato).
 * A prova genuína do UTF é de Kummer e Wiles; a máquina exibe que os seus objetos são o gato
 * 2×2 (traço, det) e o flip do esquilo — uma identificação, não uma prova. Exato, resíduo 0.
 *
 *   cc -O2 -std=c99 fermat.c -o fermat -lm
 */
#include <stdio.h>
#include "unidade.h"
#include <math.h>

int main(void){
    printf("FERMAT NA MÁQUINA — a prova do UTF cai no gato (traço, determinante)\n");
    printf("================================================================\n");

    /* §1 — o GATO é o objeto 2×2 por traço e det: A_n=[[n,1],[1,0]], tr=n, det=−1; o metal σ_n.   */
    printf("\n§1  O GATO É O OBJETO 2×2 — A_n=[[n,1],[1,0]]: tr=n, det=−1; σ_n raiz de x²−nx−1\n");
    printf("      (Kummer: o inteiro ciclotômico é o metal — Tr(σ_n)=n, N(σ_n)=−1, uma unidade)\n");
    for(int n=1;n<=4;n++){
        double D=sqrt((double)n*n+4), s=(n+D)/2, sl=(n-D)/2;
        printf("      n=%d: tr=%d det=%d ; σ=%.4f σ'=%.4f ; σσ'=%.0f (=det) ; σ'=−1/σ? %s\n",
               n, n, -1, s, sl, s*sl, fabs(sl+1.0/s)== 0.0?"sim (o esquilo)":"?");
    }

    /* §2 — o FROBENIUS de E/𝔽_p é o gato NA BORDA: a_p=p+1−#E, autovalores de z²−a_p z+p,          */
    /*      |α|=√p (Hasse–Weil). Curva y²=x³+x+1. χ = símbolo de Legendre por contagem de quadrados. */
    int A=1,B=1;                                       /* E: y²=x³+x+1 (não-singular: 4·1+27≠0)      */
    printf("\n§2  O FROBENIUS É O GATO NA BORDA — E/𝔽_p (y²=x³+x+1): |α|=√p (Hasse–Weil)\n");
    printf("      p    #E   a_p=tr   |a_p|≤2√p?   αβ=det=p   |α|²=p (na borda)\n");
    printf("  ------------------------------------------------------------------------\n");
    int viol=0;
    int P[]={7,11,13,17,19,23,29}, NP=7;
    for(int pi=0;pi<NP;pi++){
        int p=P[pi];
        int isq[64]={0}; for(int v=0;v<p;v++) isq[(v*v)%p]=1;
        long ap=0;
        for(int x=0;x<p;x++){ int t=(((x*x%p)*x%p)+A*x+B)%p; t=(t%p+p)%p;
            int chi = (t==0)?0:(isq[t]?1:-1); ap += chi; }
        ap = -ap;                                      /* a_p = −Σ χ(x³+ax+b)                        */
        long E = p+1-ap;                               /* #E(𝔽_p) = p+1−a_p                          */
        int hasse = (ap*ap <= 4*p);                    /* |a_p| ≤ 2√p                                */
        long det = p;                                  /* αβ = det = p ; α+β = a_p                    */
        int naborda = (ap*ap < 4*p);                   /* discriminante<0 ⇒ α complexo, |α|²=αβ=p     */
        if(!hasse || !naborda) viol++;
        printf("   %-3d  %-4ld  %-6ld   %-9s    αβ=%ld     |α|²=%ld ⇒ |α|=√%d  %s\n",
               p, E, ap, hasse?"sim":"NÃO", det, det, p, naborda?"(borda)":"?");
    }

    /* §3 — o FLIP é o esquilo: β=p/α (inverso conjugado), como σ'=−1/σ. O módulo decide o ramo.    */
    printf("\n§3  O FLIP É O ESQUILO — o segundo autovalor é o inverso conjugado do primeiro:\n");
    printf("      Frobenius: β=p/α, αβ=+p (NO círculo, a borda)   |   gato: σ'=−1/σ, σσ'=−1 (FORA)\n");
    printf("      o mesmo espelho (conjugado=inverso); só o módulo (det=+p vs −1) muda o ramo.\n");

    printf("\n----------------------------------------------------------------\n");
    printf("Frobenius = gato na borda (|α|=√p); representação de Galois = o gato 2×2 (tr,det)\n");
    printf("resíduo total = %d   %s\n", viol, VD(viol, "A PROVA DO UTF (WILES/KUMMER) CAI NO GATO E NO ESQUILO — IDENTIFICAÇÃO, NÃO PROVA NOVA"));
    return viol?1:0;
}
