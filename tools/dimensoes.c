/* dimensoes.c — TODAS as dimensões: cada GF(p^n) é corpo, e a divisão vem do dual (o esquilo).
 *
 * Não há dimensões só pares nem álgebra que "perca o corpo": isso seria a torre de outra gente,
 * que aqui não vale. No navegante, CADA dimensão n é o corpo GF(p^n) — comutativo, associativo,
 * com divisão — para TODO n, as ímpares (3,5,7,…) inclusive. O DUAL de um elemento são os seus
 * CONJUGADOS pelo Frobenius (o esquilo) φ: x ↦ x^p; há n deles, e a DIVISÃO sai só deles, sem
 * Fermat:
 *     x⁻¹ = ∏_{k=1}^{n-1} φ^k(x) · N(x)⁻¹ ,   N(x)=∏_{k=0}^{n-1} φ^k(x) ∈ ℤ_p .
 * Sobre GF(2^n), N(x)=1 para x≠0, e o inverso É o produto dos conjugados — puro dual. Cada corpo
 * é o anterior com o seu dual, e a torre inteira é corpo, em toda dimensão. Exato, resíduo 0.
 *
 *   cc -O2 -std=c99 dimensoes.c -o dimensoes
 *   ./dimensoes [N]      (verifica GF(2^n) para n=1..N)
 */
#include <stdio.h>
#include <stdlib.h>
#include "gf2n.h"

/* o produto GF(2^n), o Frobenius φ:x↦x² e o inverso pelo dual vêm de gf2n.h */
/* as constantes de estrutura do gato: σ^t = x^t mod f (σ = x = 2), e o produto BILINEAR do paper */
static int sig(int t,int n){ int r=1; while(t--) r=gfmul(r,2,n); return r; }              /* σ^t       */
static int bilin(int a,int b,int n){                 /* (a·b)_k=Σ Γ^k_{ij} a_i b_j ; Γ_{ij}=σ^{i+j}   */
    int r=0; for(int i=0;i<n;i++) if(a>>i&1) for(int j=0;j<n;j++) if(b>>j&1) r^=sig(i+j,n); return r;
}

int main(int argc,char**argv){
    int N = argc>1? atoi(argv[1]) : 10;
    if(N>12) N=12;
    int res=0;
    printf("TODAS AS DIMENSÕES — cada GF(2^n) é corpo; a divisão vem do dual (os conjugados)\n");
    printf("================================================================\n");
    printf("  o dual = os n conjugados por φ:x↦x²;  x⁻¹ = produto dos conjugados (sem Fermat)\n");
    printf("\n  dim n   corpo      elems   comuta  associa  divide   φ ordem   paridade\n");
    printf("  --------------------------------------------------------------------------\n");
    for(int n=1;n<=N;n++){
        int Q=1<<n, comuta=1, associa=1, divide=1, fordem=1;
        /* comutatividade e divisão: exaustivo (Q ≤ 4096) */
        for(int a=0;a<Q;a++){
            for(int b=0;b<Q;b++) if(gfmul(a,b,n)!=gfmul(b,a,n)) comuta=0;
            if(a){ if(gfmul(a,gfinv(a,n),n)!=1) divide=0; }        /* x·x⁻¹=1 (o inverso pelo dual)  */
            if(frobk(a,n,n)!=a) fordem=0;                          /* φ^n = id                       */
        }
        /* associatividade: amostra ampla */
        for(int a=0;a<Q;a++) for(int b=0;b<Q;b+= (Q>64?7:1)) for(int c=0;c<Q;c+=(Q>64?11:1))
            if(gfmul(gfmul(a,b,n),c,n)!=gfmul(a,gfmul(b,c,n),n)) associa=0;
        /* a ordem de φ é EXATA n: existe elemento primitivo (não em subcorpo próprio) */
        int exata = (n==1);   /* GF(2): φ=id, ordem 1 (trivial) */
        for(int a=2;a<Q && !exata;a++){ int ok=1; for(int k=1;k<n;k++) if(frobk(a,k,n)==a){ ok=0; break; } if(ok) exata=1; }
        int corpo = comuta && associa && divide && fordem && exata;
        if(!corpo) res++;
        printf("    %-2d    GF(2^%-2d)  %5d    %-3s     %-3s      %-3s      %-3s       %s%s\n",
               n, n, Q, comuta?"sim":"NÃO", associa?"sim":"NÃO", divide?"sim":"NÃO",
               exata?"sim":"NÃO", (n&1)?"ÍMPAR":"par", corpo?"":"  <FALHA>");
    }
    /* a multiplicação EXPLÍCITA do paper — o produto bilinear via as constantes de estrutura Γ do  */
    /* gato — reproduz exatamente o produto do corpo, em toda dimensão.                              */
    long vbil=0;
    for(int n=1;n<=N;n++){ int Q=1<<n; for(int a=0;a<Q;a++) for(int b=0;b<Q;b++) if(bilin(a,b,n)!=gfmul(a,b,n)) vbil++; }
    res += (vbil!=0);
    printf("\n  a multiplicação explícita (a·b)_k=Σ Γ^k_{ij} a_i b_j, Γ=σ^{i+j} (o gato): = o produto do corpo? viol=%ld  %s\n",
           vbil, vbil?"FALHA":"sim (a fórmula do paper É a multiplicação do corpo)");

    printf("\n  ⇒ TODA dimensão é corpo — pares e ÍMPARES —, e o inverso é o produto dos conjugados (o dual)\n");
    printf("    dim 1 = ℤ_p (ℝ) ; dim 2 = GF(p²) (ℂ, o gato) ; dim n = GF(p^n): o anterior com o seu dual\n");
    printf("    nenhuma dimensão fica de fora e nenhuma deixa de ser corpo — é o navegante, multidimensional\n");

    printf("\n----------------------------------------------------------------\n");
    printf("GF(2^n), n=1..%d   resíduo total = %d   %s\n", N, res,
           res? "FALHA" : "TODA DIMENSÃO É CORPO — A DIVISÃO SAI DO DUAL (OS CONJUGADOS), SEM FERMAT");
    return res?1:0;
}
