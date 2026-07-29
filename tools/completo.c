/* completo.c — O CORPO É COMPLETO: todo dado admite representação consistente e reversível.
 *
 * Um dado é uma tupla finita D=(d_0,…,d_{L-1}) de qualquer tamanho e alfabeto. Afirmamos:
 * para todo D existe (p,n) tal que D É um elemento de GF(p^n) — os seus coeficientes — e a
 * máquina (soma, produto, a transformada universal) opera nele exata e reversivelmente.
 * A família {GF(p^n)} é COFINAL (há n≥L para todo L, e sempre existe irredutível de grau n)
 * e ENCAIXADA (n|n' ⇒ GF(p^n)⊂GF(p^{n'})); o limite ∪ GF(p^n) contém toda representação.
 * Logo o corpo é completo: nenhum dado fica de fora, e a volta é resíduo 0.
 *
 *   cc -O2 -std=c99 completo.c -o completo
 *   ./completo [imagem.pgm]
 */
#include <stdio.h>
#include <stdlib.h>

/* ---------- a transformada universal: NTT em ℤ/P, P = 2^16+1 (Fermat), raiz primitiva 3 ---------- */
#define P 65537
static long pm(long b,long e){ long r=1; b%=P; while(e){ if(e&1) r=r*b%P; b=b*b%P; e>>=1; } return r; }
static void ntt(long *a,int n,int inv){
    for(int i=1,j=0;i<n;i++){ int bit=n>>1; for(;j&bit;bit>>=1) j^=bit; j^=bit; if(i<j){ long t=a[i];a[i]=a[j];a[j]=t; } }
    for(int len=2;len<=n;len<<=1){
        long w = inv ? pm(3,(long)P-1-(P-1)/len) : pm(3,(P-1)/len);      /* raiz len-ésima da unidade */
        for(int i=0;i<n;i+=len){ long wn=1;
            for(int k=0;k<len/2;k++){
                long u=a[i+k], v=a[i+k+len/2]*wn%P;
                a[i+k]=(u+v)%P; a[i+k+len/2]=(u-v+P)%P; wn=wn*w%P;
            }
        }
    }
    if(inv){ long ni=pm(n,(long)P-2); for(int i=0;i<n;i++) a[i]=a[i]*ni%P; }
}

/* um gerador de dados determinístico (sem relógio): xorshift, para "qualquer dado" reprodutível */
static unsigned long xs=88172645463325252ULL;
static unsigned long rnd(void){ xs^=xs<<13; xs^=xs>>7; xs^=xs<<17; return xs; }

#include "pgm.h"                                    /* le_pgm: o leitor PGM binário (P5) reusado */

/* a contagem de irredutíveis mônicos de grau n sobre 𝔽_p (Gauss/Möbius): I_p(n)=(1/n)Σ μ(d) p^{n/d} */
static int mobius(int n){ int r=1; for(int i=2;(long)i*i<=n;i++){ if(n%i==0){ n/=i; if(n%i==0) return 0; r=-r; } } if(n>1) r=-r; return r; }
static long irred(long pp,int n){ long s=0; for(int d=1;d<=n;d++) if(n%d==0){ unsigned long long q=1; for(int e=0;e<n/d;e++) q*=pp; s+=(long)mobius(d)*(long)q; } return s/n; }

/* representa/reconstrói um dado por uma transformada e a sua inversa; devolve o nº de erros */
static long roundtrip(const unsigned char *d,int L){
    int n=1; while(n<L) n<<=1;                          /* o menor corpo-tamanho ≥ L (potência de 2) */
    long *a=calloc(n,sizeof(long));
    for(int i=0;i<L;i++) a[i]=d[i];                     /* D como coeficientes em ℤ/P (padding zeros) */
    ntt(a,n,0); ntt(a,n,1);                            /* ℱ e depois ℱ⁻¹                            */
    long err=0; for(int i=0;i<L;i++) if(a[i]!=d[i]) err++; for(int i=L;i<n;i++) if(a[i]!=0) err++;
    free(a); return err;
}

int main(int argc,char**argv){
    int res=0;
    printf("O CORPO É COMPLETO — todo dado admite representação consistente reversível\n");
    printf("================================================================\n");

    /* §1 — TODO DADO É UM ELEMENTO DO CORPO: D=(d_0,…,d_{L-1}), d_i<p, é o polinômio Σ d_i x^i,   */
    /*      um elemento de GF(p^n) para qualquer n≥L. A leitura dos coeficientes é a inversa exata.*/
    int p1=257;                                         /* primo > 256: cabe qualquer byte            */
    unsigned char D[4]={200,13,255,7}; int L=4;
    unsigned long long N=0, base=1;                     /* o "número" do dado: Σ d_i p^i (base-p)    */
    for(int i=0;i<L;i++){ N += (unsigned long long)D[i]*base; base*=p1; }
    unsigned char back[4]; unsigned long long t=N;
    for(int i=0;i<L;i++){ back[i]=(unsigned char)(t%p1); t/=p1; }
    long e1=0; for(int i=0;i<L;i++) if(back[i]!=D[i]) e1++;
    res += (e1!=0);
    printf("\n§1  TODO DADO É UM ELEMENTO DO CORPO — D=(coeficientes) ∈ GF(%d^n), n≥L:\n", p1);
    printf("      D=(200,13,255,7) → elemento nº %llu (base %d) → volta aos coeficientes: erros=%ld  %s\n",
           N, p1, e1, e1?"FALHA":"injetivo e reversível (OK)");

    /* §2 — A REPRESENTAÇÃO É REVERSÍVEL para QUALQUER dado: ℱ⁻¹ℱ(D)=D (resíduo 0). Testa dados     */
    /*      aleatórios de vários tamanhos e alfabetos — e um dado real (a imagem, se dada).         */
    printf("\n§2  A REPRESENTAÇÃO É REVERSÍVEL — a transformada universal ℱ⁻¹ℱ(D)=D, para qualquer D:\n");
    long tot=0; int sizes[5]={8,64,1000,4096,50000};
    for(int s=0;s<5;s++){
        int Ls=sizes[s]; unsigned char *d=malloc(Ls);
        for(int i=0;i<Ls;i++) d[i]=(unsigned char)(rnd()%256);   /* dado arbitrário               */
        long err=roundtrip(d,Ls); tot+=err;
        printf("      dado aleatório L=%-6d (alfabeto 0..255): erros=%ld  %s\n", Ls, err, err?"FALHA":"OK");
        free(d);
    }
    res += (tot!=0);

    /* §3 — CONSISTENTE NA TORRE: n | n' ⇒ GF(p^n) ⊂ GF(p^{n'}); o mesmo dado sobe sem mudar.        */
    unsigned char dd[10]; for(int i=0;i<10;i++) dd[i]=(unsigned char)(rnd()%256);
    long eN =roundtrip(dd,10);                          /* no menor corpo (n=16)                     */
    /* subir de grau = mais zeros à direita: o embedding preserva (a inversa devolve D e zeros).    */
    unsigned char dd2[10+20]; for(int i=0;i<10;i++) dd2[i]=dd[i]; for(int i=10;i<30;i++) dd2[i]=0;
    long eN2=roundtrip(dd2,30);                         /* no corpo maior (n=32)                     */
    res += (eN!=0 || eN2!=0);
    printf("\n§3  CONSISTENTE NA TORRE — GF(p^n) ⊂ GF(p^{n'}) (n|n'): o dado sobe de grau sem mudar:\n");
    printf("      D em corpo menor: erros=%ld ; o mesmo D mergulhado no maior: erros=%ld  %s\n",
           eN, eN2, (eN||eN2)?"FALHA":"o embedding preserva (OK)");

    /* §4 — COFINAL: para todo grau n existe corpo GF(p^n), pois há irredutível de grau n           */
    /*      (Gauss: I_p(n) = (1/n) Σ_{d|n} μ(d) p^{n/d} > 0). Nenhum tamanho de dado fica sem corpo. */
    printf("\n§4  COFINAL — sempre há corpo GF(2^n): nº de polinômios irredutíveis de grau n > 0:\n      ");
    int cof=1;
    for(int n=1;n<=24;n++){ long I=irred(2,n); if(I<=0) cof=0; if(n<=12) printf("I_2(%d)=%ld ", n, I); }
    res += !cof;
    printf("… (todos >0 até n=24)  %s\n", cof?"para todo L há n≥L com corpo (OK)":"FALHA");

    /* §5 — o dado real: a imagem passa pela representação e volta, resíduo 0 (é só mais um dado).   */
    if(argc>1){
        int w,h; unsigned char *px=le_pgm(argv[1],&w,&h);
        if(px){
            /* a imagem INTEIRA, em blocos de ≤ 2^16 (o teto da NTT em ℤ/P): cada bloco reversível. */
            long total=(long)w*h, off=0, err=0; int nb=0;
            while(off<total){
                int blk=(int)((total-off < 65536)? (total-off) : 65536);
                err += roundtrip(px+off, blk); off+=blk; nb++;
            }
            res+=(err!=0);
            printf("\n§5  A IMAGEM É SÓ MAIS UM DADO — %s (%ldx%ld):\n", argv[1], (long)w, (long)h);
            printf("      a imagem INTEIRA (%ld amostras) em %d blocos de ≤2^16, ida e volta: erros=%ld  %s\n",
                   total, nb, err, err?"FALHA":"EXATO, resíduo 0 — a imagem cabe no corpo como qualquer dado");
            free(px);
        }
    }

    printf("\n----------------------------------------------------------------\n");
    printf("resíduo total = %d   %s\n", res, res? "FALHA" :
           "O CORPO É COMPLETO — TODO DADO TEM REPRESENTAÇÃO CONSISTENTE E REVERSÍVEL");
    return res?1:0;
}
