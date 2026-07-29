/* gf2n.h — A PEÇA: o corpo binário GF(2ⁿ) = 𝔽_2[x]/(f_n), f_n mônico irredutível.
 *
 * UMA implementação do produto binário, reusada — no espírito do gp2.h: o mesmo
 * gfmul (produto mod f_n), o Frobenius φ:x↦x² e o inverso pelo dual (∏ dos
 * conjugados), postos uma vez em vez de copiados. Cada tool inclui este header e
 * passa n. Nada de matemática nova aqui — só a peça, uma vez.
 */
#ifndef GF2N_H
#define GF2N_H
#include <stdlib.h>

/* polinômios irredutíveis mônicos sobre 𝔽_2, grau n (o bit n marca o grau) */
static int IRR[13]={0, 3, 7, 11, 19, 37, 67, 137, 283, 529, 1033, 2053, 4179};

static int gfmul(int a,int b,int n){                 /* produto em GF(2^n), mod o irredutível       */
    int r=0, irr=IRR[n], hi=1<<n;
    while(b){ if(b&1) r^=a; b>>=1; a<<=1; if(a&hi) a^=irr; }
    return r;
}
static int frob(int a,int n){ return gfmul(a,a,n); }              /* o dual: φ(x)=x² (o Frobenius)   */
static int frobk(int a,int k,int n){ while(k--) a=frob(a,n); return a; }
static int gfinv(int a,int n){                       /* x⁻¹ = ∏_{k=1}^{n-1} φ^k(x)  (só os conjugados)*/
    int r=1; for(int k=1;k<n;k++) r=gfmul(r, frobk(a,k,n), n); return r;
}

#endif
