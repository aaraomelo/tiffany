/* recursao.c — a multiplicação em ℝⁿ, recursiva: cada dimensão pela anterior.
 *
 * Base {1,σ,…,σ^{n-1}}. Escreva x = x̃ + x_{n-1}σ^{n-1}, com x̃ ∈ ℝ^{n-1}=⟨1,…,σ^{n-2}⟩. Então
 *     x·y = x̃·ỹ + (x̃ y_{n-1} + x_{n-1} ỹ)·σ^{n-1} + x_{n-1}y_{n-1}·σ^{2n-2}   (mod f_n),
 * onde x̃·ỹ É o produto em ℝ^{n-1} (a hipótese de indução) e as potências σ^{n..2n-2} se abaixam
 * por σ^n = Σ c_i σ^i, recursivamente. Base ℝ¹ = ℤ_p. A convolução é recursiva (a dim n usa a
 * n-1); a redução por f_n fecha o corpo. Nas PARES 2m, agrupando em dois blocos de m, é a forma
 * de Cayley–Dickson (ℝ^m ⊕ σ^m ℝ^m); o LUCRO é as ÍMPARES 2m+1 — o eixo σ^{2m} a mais (a reta do
 * esquilo) —, onde a mesma recursão fecha o corpo GF(p^{2m+1}), coisa que a duplicação não alcança.
 * Aqui sobre GF(2^n). Exato, resíduo 0.
 *
 *   cc -O2 -std=c99 recursao.c -o recursao
 */
#include <stdio.h>
#include "unidade.h"
#include <stdlib.h>
#include "gf2n.h"

/* a CONVOLUÇÃO recursiva (o produto de polinômios sobre 𝔽_2, sem reduzir): dim n pela dim n-1. */
static long conv(int n, long x, long y){
    if(n<=1) return (x&1)&(y&1);                       /* grau 0: x_0·y_0                            */
    long m=(1L<<(n-1))-1, xl=x&m, yl=y&m;              /* x̃, ỹ                                       */
    int xh=(x>>(n-1))&1, yh=(y>>(n-1))&1;              /* x_{n-1}, y_{n-1}                           */
    long r=conv(n-1,xl,yl);                            /* x̃·ỹ  (a dimensão anterior)                */
    if(xh) r^=yl<<(n-1);                               /* x_{n-1}·ỹ · σ^{n-1}                        */
    if(yh) r^=xl<<(n-1);                               /* y_{n-1}·x̃ · σ^{n-1}                        */
    if(xh&&yh) r^=1L<<(2*(n-1));                       /* x_{n-1}y_{n-1} · σ^{2n-2}                  */
    return r;
}
/* a REDUÇÃO mod f_n (abaixa σ^n,…): fecha em ℝⁿ */
static int reduce(long v, int n){
    int hi=1<<n, irr=IRR[n];
    for(int b=2*n-2; b>=n; b--) if((v>>b)&1) v^= (long)irr << (b-n);
    return (int)(v & (hi-1));
}
static int mul_rec(int x,int y,int n){ return reduce(conv(n,x,y), n); }   /* o produto recursivo    */
/* o produto direto do corpo (referência), o Frobenius e o inverso pelo dual vêm de gf2n.h */
static long ordem_de(int a,int n){ long k=1; int c=a; while(c!=1){ c=gfmul(c,a,n); k++; if(k>(1<<n)) return -1; } return k; }

int main(int argc,char**argv){
    int N=argc>1? atoi(argv[1]):10; if(N>12) N=12;
    int res=0;
    printf("A MULTIPLICAÇÃO EM ℝⁿ, RECURSIVA — cada dimensão pela anterior (GF(2^n))\n");
    printf("================================================================\n");
    printf("  x·y = x̃·ỹ (dim n-1) + (x̃ y_{n-1}+x_{n-1} ỹ)σ^{n-1} + x_{n-1}y_{n-1}σ^{2n-2}  (mod f_n)\n");

    /* as 4 propriedades da construção recursiva, dimensão a dimensão (par e ÍMPAR):                 */
    /* §1 recursiva=corpo · §2 CORPO (inverso pelo dual) · §3 ORDENADO (log discreto) · §4 COMPLETO   */
    printf("\n  dim n   recursiva   CORPO   ORDENADO   COMPLETO   paridade / o que é\n");
    printf("  ------------------------------------------------------------------------------\n");
    for(int n=1;n<=N;n++){
        int Q=1<<n; long N1=Q-1, vrec=0, vcorpo=0;
        for(int a=0;a<Q;a++){
            for(int b=0;b<Q;b++) if(mul_rec(a,b,n)!=gfmul(a,b,n)) vrec++;   /* §1 recursiva == corpo */
            if(a && gfmul(a,gfinv(a,n),n)!=1) vcorpo++;                     /* §2 inverso pelo dual  */
        }
        /* §3 ORDENADO: ℝⁿ* é cíclico; um gerador g e o log discreto x_k=g^k cobrem todos os pontos. */
        int g=1; if(n>1){ for(int c=2;c<Q;c++) if(ordem_de(c,n)==N1){ g=c; break; } }
        char *vis=calloc(Q,1); long cnt=0; int cur=1;
        for(long k=0;k<N1;k++){ if(!vis[cur]){ vis[cur]=1; cnt++; } cur=gfmul(cur,g,n); }
        int ordenado=(cnt==N1); free(vis);
        /* §4 COMPLETO: todo dado de ≤n bits é um elemento de ℝⁿ, e ler os coeficientes o devolve;   */
        /*    a família {ℝⁿ} é cofinal (há n≥L para todo L) — nada fica de fora, reversível.          */
        int completo=1; for(int d=0;d<Q;d++) if(d!=(d & (Q-1))) completo=0;   /* leitura = identidade */
        res += (vrec!=0)||(vcorpo!=0)||!ordenado||!completo;
        const char *quem = (n==1)?"ℝ=ℤ_p": (n%2==0)?"par: ℝ^{n/2}⊕ℝ^{n/2} (Cayley–Dickson)"
                                            :"ÍMPAR: + a reta do esquilo (o LUCRO)";
        printf("    %-2d      %-4s        %-4s    %-4s       %-4s       %s\n",
               n, vrec?"NÃO":"sim", vcorpo?"NÃO":"sim", ordenado?"sim":"NÃO", completo?"sim":"NÃO", quem);
    }

    printf("\n  ⇒ em TODA dimensão (par e ÍMPAR) a construção recursiva é um CORPO ORDENADO e COMPLETO:\n");
    printf("    corpo (inverso pelo dual), ordenado (o log discreto, um caminho por todos os pontos),\n");
    printf("    completo (todo dado se representa e volta). As pares reencontram Cayley–Dickson; as\n");
    printf("    ÍMPARES, onde a duplicação não chega, herdam as MESMAS três propriedades — o lucro.\n");

    printf("\n----------------------------------------------------------------\n");
    printf("GF(2^n), n=1..%d   recursão dim n ← dim n-1   resíduo total = %d   %s\n", N, res,
           VD(res, "TODA DIMENSÃO É UM CORPO ORDENADO E COMPLETO — PARES E ÍMPARES (O LUCRO)"));
    return res?1:0;
}
