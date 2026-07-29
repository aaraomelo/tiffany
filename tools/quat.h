/* quat.h — A PEÇA: os quaternions ℍ = M_2(ℤ_p), a matriz 2×2 [[a,b],[c,d]] (split, por Wedderburn).
 *
 * UMA implementação de ℍ, reusada — a mesma multiplicação não-comutativa que salta (saltos)
 * e o gato/esquilo que reverte e completa o corpo (esquilo). Cada tool inclui este header e
 * seta p,m no main. Nada de matemática nova aqui — só a peça, uma vez, em vez de copiada.
 */
#ifndef QUAT_H
#define QUAT_H
#include <stdlib.h>

static int p, m;                                     /* ℍ=M_2(ℤ_p); x²−mx−1 dá o gato G=[[m,1],[1,0]] */
typedef struct { int a,b,c,d; } M;                   /* [[a,b],[c,d]] ∈ M_2(ℤ_p) = ℍ               */
static int md(long x){ return (int)((x%p+p)%p); }
static M mmul(M X,M Y){ return (M){ md((long)X.a*Y.a+(long)X.b*Y.c), md((long)X.a*Y.b+(long)X.b*Y.d),
                                    md((long)X.c*Y.a+(long)X.d*Y.c), md((long)X.c*Y.b+(long)X.d*Y.d) }; }
static M madj(M X){ return (M){ X.d, md(-X.b), md(-X.c), X.a }; }   /* conjugado = tr·I − M = adj    */
static int mtr(M X){ return md(X.a+X.d); }
static int mdet(M X){ return md((long)X.a*X.d-(long)X.b*X.c); }
static int meq(M X,M Y){ return X.a==Y.a&&X.b==Y.b&&X.c==Y.c&&X.d==Y.d; }
static M mscal(long k,M X){ return (M){ md(k*X.a),md(k*X.b),md(k*X.c),md(k*X.d) }; }
static long pw(long b,long e){ long r=1; b%=p; while(e){ if(e&1) r=r*b%p; b=b*b%p; e>>=1; } return r; }

#endif
