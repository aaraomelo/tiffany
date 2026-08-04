/* duais.c — os atratores são DIRECIONAIS: brancos (fontes) e negros (sorvedouros).
 *
 * Cada gato tem DOIS pontos fixos — as duas direções próprias, as raízes de x²−mx−1:
 *   σ  = (m+√(m²+4))/2  = [m;m,m,…]   |σ|>1  domina  →  NEGRO  (sorvedouro): tudo ENTRA nele
 *   σ' = (m−√(m²+4))/2  = −1/σ         |σ'|<1          →  BRANCO (fonte):     tudo SAI dele
 * Não se entra e sai pelo mesmo: entra-se no negro, sai-se do branco — o gato aponta do branco
 * para o negro. Mas os dois são DUAIS:
 *   σ·σ' = −1 (det) ,  σ+σ' = m (tr) ,  σ' = −1/σ = σ^p (o conjugado de Frobenius).
 * Inverter o gato (o tempo reverso, A⁻¹) troca branco↔negro: o sorvedouro de A é a fonte de A⁻¹.
 * Verificado exato no finito (resíduo 0) e ilustrado no contínuo (a convergência que os separa).
 *
 *   cc -O2 -std=c99 duais.c -o duais -lm
 *   ./duais [m] [p]
 */
#include <stdio.h>
#include "unidade.h"
#include <stdlib.h>
#include <math.h>
#include "gp2.h"                                   /* a peça: o gato em GF(p²) (mul, add, pw, eq, σ) */

int main(int argc,char**argv){
    m = argc>1? atoi(argv[1]) : 1;
    p = argc>2? atoi(argv[2]) : 7;
    int res=0;

    double sq=sqrt((double)m*m+4.0), S=(m+sq)/2.0, Sl=(m-sq)/2.0;   /* σ e σ' no contínuo          */
    printf("OS ATRATORES SÃO DIRECIONAIS — brancos (fontes) e negros (sorvedouros)\n");
    printf("================================================================\n");

    /* §1 — os DOIS pontos fixos, exatos no finito: as raízes σ,σ' de x²−mx−1. Se irredutível,      */
    /*      vivem em GF(p²) e σ'=σ^p (Frobenius); se cinde, vivem em ℤ_p. Sempre σ+σ'=m, σ·σ'=−1.    */
    int r1=-1, r2=-1;
    for(int t=0;t<p;t++) if((((long)t*t-(long)m*t-1)%p+p)%p==0){ if(r1<0) r1=t; else if(t!=r1) r2=t; }
    int irred = (r1<0);
    int somaOK, prodOK, troca, sa=0, spv=0;
    if(irred){                                       /* σ irracional: GF(p²), σ'=σ^p                  */
        E s1=SIG, s2=pw(SIG,p), soma=add(s1,s2), prod=mul(s1,s2);
        somaOK=(soma.a==((m%p)+p)%p && soma.b==0);   prodOK=(prod.a==p-1 && prod.b==0);
        sa=soma.a; spv=prod.a;
        E sinv=pw(s1,(long)p*p-2), neg2={(p-s2.a)%p,(p-s2.b)%p};   troca=eq(sinv,neg2);  /* σ⁻¹=−σ' */
        printf("\n§1  DOIS PONTOS FIXOS (exato, GF(%d²), σ irracional) — raízes de x²−%dx−1:\n", p, m);
        printf("      σ=(%d,%d)  σ'=σ^p=(%d,%d) (Frobenius)", s1.a,s1.b, s2.a,s2.b);
    } else {                                         /* σ racional: as duas raízes em ℤ_p            */
        if(r2<0) r2=r1;
        sa=(r1+r2)%p; spv=(int)(((long)r1*r2)%p);
        somaOK=(sa==m%p);  prodOK=(spv==p-1);
        long inv=1,base=r1,e=p-2; while(e){ if(e&1) inv=inv*base%p; base=base*base%p; e>>=1; }
        troca=((int)inv==(p-r2)%p);                  /* σ⁻¹ = −σ' em ℤ_p                             */
        printf("\n§1  DOIS PONTOS FIXOS (exato, ℤ_%d, x²−%dx−1 cinde) — raízes racionais:\n", p, m);
        printf("      σ=%d  σ'=%d", r1, r2);
    }
    res += !(somaOK && prodOK);
    printf(" ;  σ+σ'=%d (=m): %s ;  σ·σ'=%d (=−1): %s\n",
           sa, VD(!(somaOK), "OK"), spv, VD(!(prodOK), "OK"));

    /* §2 — classificar por quem DOMINA (o módulo, no contínuo): σ (|·|>1) é o sorvedouro NEGRO;     */
    /*      σ' (|·|<1) é a fonte BRANCA. É a estrutura hiperbólica: um estica, o outro esmaga.        */
    printf("\n§2  BRANCO (fonte) E NEGRO (sorvedouro) — a estrutura hiperbólica, |σ|>1>|σ'|:\n");
    printf("      NEGRO  (sorvedouro): σ  = %.9f  = [%d;%d,%d,…]  |σ|=%.4f>1  — para ele tudo ENTRA\n",
           S, m,m,m, fabs(S));
    printf("      BRANCO (fonte)     : σ' = %.9f  = −1/σ          |σ'|=%.4f<1 — dele tudo SAI\n",
           Sl, fabs(Sl));
    res += !(fabs(S)>1.0 && fabs(Sl)<1.0);

    /* §3 — DIRECIONAL: iterar o gato x↦m+1/x CONVERGE ao negro (entra); iterar o inverso            */
    /*      x↦1/(x−m) converge ao branco (sai). Não se entra e sai pelo mesmo ponto.                 */
    double x=1.0;   for(int k=0;k<200;k++) x = m + 1.0/x;          /* o gato → o negro (sorvedouro) */
    double y=0.5;   for(int k=0;k<200;k++) y = 1.0/(y - m);        /* o inverso → o branco (fonte)  */
    int entra = fabs(x - S)  < 1e-9;
    int sai   = fabs(y - Sl) < 1e-9;
    res += !(entra && sai);
    printf("\n§3  DIRECIONAL — o gato aponta do branco para o negro:\n");
    printf("      x↦m+1/x  (o gato)   converge a %.9f = σ  (ENTRA no negro): %s\n", x, VD(!(entra), "OK"));
    printf("      x↦1/(x−m) (reverso) converge a %.9f = σ' (SAI do branco): %s\n", y, VD(!(sai), "OK"));
    printf("      são pontos distintos: entra-se num, sai-se do outro (Δ=%.6f)\n", fabs(S-Sl));

    /* §4 — DUAIS: σ·σ'=−1, σ+σ'=m, σ'=−1/σ; inverter o gato (tempo reverso) troca branco↔negro.     */
    int dual_c = (fabs(S*Sl + 1.0)<1e-9) && (fabs(S+Sl - m)<1e-9) && (fabs(Sl + 1.0/S)<1e-9);
    /* no finito (§1): A⁻¹ = gato reverso; σ⁻¹=−σ' (pois σ·(−σ')=1) ⇒ A⁻¹ troca branco↔negro.        */
    res += !(dual_c && troca);
    printf("\n§4  DUAIS — o branco e o negro são um par:\n");
    printf("      σ·σ'=%.6f (=−1), σ+σ'=%.6f (=m), σ'=−1/σ : %s\n", S*Sl, S+Sl, VD(!(dual_c), "OK"));
    printf("      inverter o gato (A⁻¹, tempo reverso) troca branco↔negro:  σ⁻¹=−σ' (exato): %s\n",
           VD(!(troca), "OK"));

    /* §5 — a leitura: o metal que a `linear` achou (σ_m, o brain=prata σ₂) é o NEGRO/sorvedouro;     */
    /*      o `navega` caminha do branco ao negro. Cada gato é uma seta branco→negro.                 */
    printf("\n§5  A LEITURA — o atrator metálico dos dados (linear: brain=σ₂) é o NEGRO/sorvedouro;\n");
    printf("      o navega caminha da fonte branca ao sorvedouro negro. Cada gato é uma seta ●→○ dual.\n");

    printf("\n----------------------------------------------------------------\n");
    printf("m=%d p=%d  σ=%.6f (negro) σ'=%.6f (branco)   resíduo total = %d   %s\n",
           m, p, S, Sl, res,
           VD(res, "OS ATRATORES SÃO DIRECIONAIS E DUAIS — BRANCO (FONTE) → NEGRO (SORVEDOURO)"));
    return res?1:0;
}
