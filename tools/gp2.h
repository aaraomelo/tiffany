/* gp2.h — A PEÇA: o gato em GF(p²)=ℤ_p[σ], σ²=mσ+1 (o plano ℂ finito).
 *
 * UMA implementação do gato, reusada — a auto-similaridade posta nos tools: o mesmo
 * ×σ que caminha (navega), converge (duais), ordena (ordem) e forma ℂ (complexo).
 * Cada tool inclui este header e seta p,m no main. Nada de matemática nova aqui —
 * só a peça, uma vez, em vez de copiada.
 */
#ifndef GP2_H
#define GP2_H
#include <stdlib.h>

static int p, m;                                   /* GF(p²)=ℤ_p[σ], σ²=mσ+1 (x²−mx−1 irredutível) */
typedef struct { int a, b; } E;                    /* o elemento a + bσ                             */
static E ONE = {1,0}, SIG = {0,1};                 /* 1 e σ                                          */

static int eq(E x, E y){ return x.a==y.a && x.b==y.b; }
static E add(E x, E y){ E r={(x.a+y.a)%p,(x.b+y.b)%p}; return r; }
static E sub(E x, E y){ E r={((x.a-y.a)%p+p)%p,((x.b-y.b)%p+p)%p}; return r; }
static E scal(long k, E w){ E r={(int)(((k*w.a)%p+p)%p),(int)(((k*w.b)%p+p)%p)}; return r; }
/* o produto: (a+bσ)(c+dσ) = (ac+bd) + (ad+bc+m·bd)σ   [usa σ²=mσ+1]                              */
static E mul(E x, E y){
    long ac=(long)x.a*y.a, bd=(long)x.b*y.b, ad=(long)x.a*y.b, bc=(long)x.b*y.a;
    E r; r.a=(int)(((ac+bd)%p+p)%p); r.b=(int)(((ad+bc+(long)m*bd)%p+p)%p); return r;
}
static E gato(E x){ return mul(x, SIG); }          /* o gato: ×σ (caminhar um passo na órbita)      */
static E pw(E x, long e){ E r=ONE; while(e>0){ if(e&1) r=mul(r,x); x=mul(x,x); e>>=1; } return r; }
static E frob(E w){ return pw(w, p); }             /* o Frobenius / conjugado z↦z^p = z̄            */
static long ordem(E x){ long k=1; E c=x; while(!eq(c,ONE)){ c=mul(c,x); k++; if(k>(long)p*p) return -1; } return k; }
/* x²−mx−1 é irredutível mod p (σ irracional)?  1 = sim (σ vive em GF(p²)), 0 = cinde em ℤ_p       */
static int irred_gp2(void){ for(int t=0;t<p;t++) if((((long)t*t-(long)m*t-1)%p+p)%p==0) return 0; return 1; }

#endif
