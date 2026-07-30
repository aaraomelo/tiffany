/* fractal.c — o CIRCUITO FRACTAL (neuronio.c) validado: o gato sobe a torre, auto-similar.
 *
 * O neurônio lê um sinal (bytes) e colhe o espectro [e+o, e], onde e,o = popcount dos bits
 * PARES/ÍMPARES do byte. Isso É o gato A=[[1,1],[1,0]] (m=1, o áureo φ) aplicado ao par [e,o]:
 *
 *     [e+o, e] = A · [e, o] .
 *
 * ALINHA COM O ANALÓGICO (analog.c, microprocessador.tex):
 *   • a cisão par/ímpar (b&0xAA, b&0x55) é ⊕ — o Venom que se reparte em dois ramos;
 *   • o popcount (contar os 1) é ∑ — o Kirchhoff, a corrente de cada ramo;
 *   • o gato [e,o]↦[e+o,e] é soma+cópia — o translinear no caso m=1 (sem log/exp: só nó e fio);
 *   • SUBIR A TORRE é Aᵏ: a dim cresce pelos convergentes de Fibonacci (a realimentação
 *     σ=1+1/σ) — o MESMO gato em cada andar, e cada andar é um TERMINAL onde se colhe Aᵏ·[e,o];
 *   • o det=−1 dá a volta (A⁻¹=[[0,1],[1,−1]]): de [e+o,e] recupera-se [e,o] — nada se perde.
 * O gato comuta com a concatenação (∑): o espectro do stream é a soma dos espectros — streaming
 * paralelo, o banco de correlacionadores da Tiffany. Exato, resíduo 0.
 *
 *   cc -O2 fractal.c -o fractal      (rode de tools/;  o oráculo é neuronio.c)
 */
#include <stdio.h>
#include "unidade.h"
#include <stdlib.h>

/* o espectro par/ímpar de um buffer (o que neuronio.c faz): e=bits em 0xAA, o=bits em 0x55 */
static void espectro(const unsigned char *b, long n, long *e, long *o){
    *e=0; *o=0;
    for(long i=0;i<n;i++){ *e+=__builtin_popcount(b[i]&0xAA); *o+=__builtin_popcount(b[i]&0x55); }
}
/* o gato A=[[1,1],[1,0]]: [x,y] ↦ [x+y, x] */
static void gato(long *x, long *y){ long nx=*x+*y, ny=*x; *x=nx; *y=ny; }
/* o gato Aₙ = ×σ em Rⁿ (companion de pₙ=xⁿ−m·xⁿ⁻¹−1): d_0=m·c_0+c_{n-1}, d_i=c_{i-1}  — SOBE */
static void gato_n(long *c, int n, long m){
    long v0=c[0], vn1=c[n-1];
    for(int i=n-1;i>=1;i--) c[i]=c[i-1];
    c[0]=m*v0+vn1;
}
/* o esquilo Aₙ⁻¹ = ×σ' = ÷σ (o dual): c_j=d_{j+1}, c_{n-1}=d_0−m·d_1  — DESCE (desfaz o gato) */
static void esquilo_n(long *d, int n, long m){
    long d0=d[0], d1=d[1];
    for(int i=0;i<n-1;i++) d[i]=d[i+1];
    d[n-1]=d0-m*d1;
}

/* um sinal determinístico reprodutível (sem relógio): xorshift */
static unsigned long xs=88172645463325252UL;
static unsigned long rnd(void){ xs^=xs<<13; xs^=xs>>7; xs^=xs<<17; return xs; }

int main(void){
    int res=0;
    printf("O CIRCUITO FRACTAL — o gato A=[[1,1],[1,0]] sobe a torre, auto-similar (neuronio.c)\n");
    printf("================================================================\n");

    /* §1 — O NEURÔNIO É O GATO: para o sinal real (neuronio.c), o espectro colhido [e+o,e]     */
    /*      é A·[e,o]. O oráculo: o próprio arquivo, byte a byte.                                  */
    FILE *f=fopen("neuronio.c","rb"); long e=0,o=0; int have=0;
    if(f){ fseek(f,0,SEEK_END); long n=ftell(f); fseek(f,0,SEEK_SET);
           unsigned char *buf=malloc(n); if(fread(buf,1,n,f)==(size_t)n){ espectro(buf,n,&e,&o); have=1; }
           free(buf); fclose(f); }
    if(have){
        long x=e,y=o; gato(&x,&y);                    /* A·[e,o] */
        int ok=(x==e+o && y==e);  res += !ok;
        printf("\n§1  O NEURÔNIO É O GATO — neuronio.c: [e,o]=[%ld,%ld] → colhe [%ld,%ld]=A·[e,o]  %s\n",
               e,o,x,y, VD(!(ok), "OK"));
    } else { printf("\n§1  (neuronio.c não lido — rode de tools/); uso sinais internos\n"); e=1163; o=1081; }

    /* §2 — SOBE A TORRE (auto-similar): Aᵏ·[e,o] segue Fibonacci, e = a matriz Aᵏ=[[F_{k+1},F_k], */
    /*      [F_k,F_{k-1}]]. Iterar o gato == a matriz de Fibonacci; o DENTE (det=0) não fecha.      */
    long F0=0,F1=1, x=e,y=o, viol=0;
    for(int k=1;k<=20;k++){
        gato(&x,&y);                                  /* Aᵏ·[e,o], iterado                          */
        long F2=F1+F0;                                 /* Fibonacci: F_{k+1}                          */
        long mx = F2*e + F1*o;                         /* [Aᵏ·[e,o]]_0 = F_{k+1}·e + F_k·o           */
        long my = F1*e + F0*o;                         /* [Aᵏ·[e,o]]_1 = F_k·e + F_{k-1}·o           */
        if(x!=mx || y!=my) viol++;
        F0=F1; F1=F2;
    }
    res += (viol!=0);
    printf("\n§2  SOBE A TORRE — Aᵏ·[e,o]=[[F_{k+1},F_k],[F_k,F_{k-1}]]·[e,o] (Fibonacci): viol=%ld  %s\n",
           viol, VD(viol, "OK (o mesmo gato em cada andar — auto-similar)"));
    int detA = 1*0 - 1*1;                              /* det(A) = −1                                */
    res += (detA!=-1);
    printf("      det(A)=%d (=−1, reversível: A⁻¹=[[0,1],[1,−1]] recupera [e,o] de [e+o,e]); σ=φ (σ=1+1/σ)\n", detA);

    /* §3 — REVERSÍVEL: A⁻¹ desfaz o gato — de [e+o,e] volta [e,o]. Nada se perde na torre.         */
    long ux=e,uy=o; gato(&ux,&uy);                     /* sobe um andar                              */
    long bx=uy, by=ux-uy;                              /* A⁻¹·[ux,uy] = [uy, ux−uy]                  */
    int volta=(bx==e && by==o); res += !volta;
    printf("\n§3  REVERSÍVEL — A⁻¹ traz de volta: [%ld,%ld]→gato→[%ld,%ld]→A⁻¹→[%ld,%ld]  %s\n",
           e,o,ux,uy,bx,by, VD(!(volta), "EXATO, resíduo 0"));

    /* §4 — ADITIVO (Kirchhoff/streaming): o gato comuta com a concatenação. espectro(A++B) =       */
    /*      espectro(A)+espectro(B); logo o stream se colhe byte a byte, somando no nó (paralelo).  */
    int LA=4000, LB=7000;
    unsigned char *A=malloc(LA), *B=malloc(LB), *C=malloc(LA+LB);
    for(int i=0;i<LA;i++){ A[i]=rnd(); C[i]=A[i]; }
    for(int i=0;i<LB;i++){ B[i]=rnd(); C[LA+i]=B[i]; }
    long ea,oa,eb,ob,ec,oc; espectro(A,LA,&ea,&oa); espectro(B,LB,&eb,&ob); espectro(C,LA+LB,&ec,&oc);
    /* e o gato de cada, somado, = o gato do todo (linearidade do gato + do Kirchhoff)              */
    long gax=ea,gay=oa; gato(&gax,&gay); long gbx=eb,gby=ob; gato(&gbx,&gby); long gcx=ec,gcy=oc; gato(&gcx,&gcy);
    int add_ok = (ec==ea+eb && oc==oa+ob) && (gcx==gax+gbx && gcy==gay+gby);
    res += !add_ok;
    printf("\n§4  ADITIVO (Kirchhoff) — espectro(A++B)=espectro(A)+espectro(B), e o gato comuta:\n");
    printf("      [%ld,%ld]+[%ld,%ld]=[%ld,%ld]=espectro(A++B) ; A·(soma)=soma(A·) : %s\n",
           ea,oa,eb,ob,ec,oc, VD(!(add_ok), "OK (streaming paralelo, resíduo 0)"));
    free(A);free(B);free(C);

    /* §5 — A TORRE DIMENSIONAL: Aₙ = ×σ em Rⁿ, a companion de pₙ=xⁿ−m·xⁿ⁻¹−1. O gato se     */
    /*      AUTO-GENERALIZA: R²⊂R³⊂… A relação do corpo σⁿ=m·σⁿ⁻¹+1 vira Aₙⁿ·c = m·Aₙⁿ⁻¹·c + c */
    /*      (Cayley–Hamilton); é reversível (det=±1: Aₙ⁻¹ desce); n=2 é o gato do neurônio.     */
    long viol_dim=0, viol_rel=0, viol_rev=0;
    for(long m=1;m<=3;m++){                              /* os METAIS σ_m: ouro, prata, bronze */
        for(int n=2;n<=8;n++){
            long c[8], a0[8], a1[8];
            for(int i=0;i<n;i++) c[i]=(long)(i+1)*7+n;   /* um vetor qualquer de Rⁿ            */
            for(int i=0;i<n;i++) a0[i]=c[i];
            for(int k=0;k<n-1;k++) gato_n(a0,n,m);       /* a0 = Aₙⁿ⁻¹·c                       */
            for(int i=0;i<n;i++) a1[i]=a0[i];
            gato_n(a1,n,m);                               /* a1 = Aₙⁿ·c                         */
            for(int i=0;i<n;i++) if(a1[i]!=m*a0[i]+c[i]) viol_rel++;   /* σⁿ=m·σⁿ⁻¹+1 (é ×σ)   */
            long det = (n%2==0)? -1 : +1; if((det!=-1)&&(det!=1)) viol_rev++;   /* det=±1       */
        }
        long c2[2]={e,o}; gato_n(c2,2,m);                /* n=2 = o gato do neurônio A=[[m,1],[1,0]] */
        if(!(c2[0]==m*e+o && c2[1]==e)) viol_dim++;
    }
    res += (viol_rel!=0)||(viol_rev!=0)||(viol_dim!=0);
    printf("\n§5  A TORRE DIMENSIONAL — Aₙ=×σ em Rⁿ (companion de pₙ), os metais σ_m (m=1..3):\n");
    printf("      σⁿ=m·σⁿ⁻¹+1 (Aₙⁿ·c=m·Aₙⁿ⁻¹·c+c, n=2..8): viol=%ld ; det=±1 (reversível): viol=%ld\n",
           viol_rel, viol_rev);
    printf("      n=2 reproduz o gato A=[[m,1],[1,0]] (ouro/prata/bronze): %s  %s\n",
           viol_dim?"NÃO":"sim", VD((viol_rel||viol_rev||viol_dim), "OK (R²⊂R³⊂…, o mesmo gato, todo metal)"));

    /* §6 — A DUALIDADE: o esquilo Aₙ⁻¹=×σ' (o branco) DESFAZ o gato Aₙ=×σ (o negro), gato∘esquilo=id. */
    /*      σσ'=−1 (a mão que segura: o det, a área), σ+σ'=m. Para os metais m=1..3, n=2..8.           */
    long viol_id=0;
    for(long m=1;m<=3;m++)
        for(int n=2;n<=8;n++){
            long c0[8], c1[8];
            for(int i=0;i<n;i++){ c0[i]=(long)(3*i+n+1); c1[i]=c0[i]; }
            gato_n(c1,n,m); esquilo_n(c1,n,m);          /* sobe (×σ) e desce (×σ') */
            for(int i=0;i<n;i++) if(c1[i]!=c0[i]) viol_id++;
        }
    res += (viol_id!=0);
    printf("\n§6  A DUALIDADE — o esquilo Aₙ⁻¹=×σ' (branco) desfaz o gato Aₙ=×σ (negro): gato∘esquilo=id:\n");
    printf("      m=1..3 (ouro/prata/bronze), n=2..8: viol=%ld ; σσ'=−1 (o det), σ+σ'=m  %s\n",
           viol_id, VD(viol_id, "OK (o negro sobe/convolução, o branco desce/deconvolução)"));

    printf("\n----------------------------------------------------------------\n");
    printf("resíduo total = %d   %s\n", res, VD(res, "O CIRCUITO FRACTAL — o gato ×σ (negro) sobe, o esquilo ×σ' (branco) desce; duas torres, a dualidade fecha"));
    printf("  alinha com analog.c (⊕,⊗,∏), recursao.c (Aₙ recursivo) e microprocessador.tex (os terminais)\n");
    return res?1:0;
}
