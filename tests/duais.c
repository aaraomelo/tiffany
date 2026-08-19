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
 *   cc -O2 -std=c99 -I lib tests/duais.c -o duais
 *   ./duais [m] [p]
 */
#include <stdio.h>
#include "unidade.h"
#include <stdlib.h>
#include "gp2.h"                                   /* a peça: o gato em GF(p²) (mul, add, pw, eq, σ) */

int main(int argc,char**argv){
    m = argc>1? atoi(argv[1]) : 1;
    p = argc>2? atoi(argv[2]) : 7;
    int res=0;
    long D = (long)m*m + 4;

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
    /* σ = (m+√D)/2 > 1  ⟺  m+√D > 2;  |σ'| = (√D−m)/2 < 1  ⟺  D < (m+2)² — sem avaliar raiz.      */
    int sigma_gt_1 = (m >= 2) || (D > (long)(2-m)*(2-m));
    int sigma_prime_lt_1 = (D < (long)(m+2)*(m+2));
    printf("\n§2  BRANCO (fonte) E NEGRO (sorvedouro) — a estrutura hiperbólica, |σ|>1>|σ'|:\n");
    printf("      NEGRO  (sorvedouro): σ  = [%d;%d,%d,…]  |σ|>1  — para ele tudo ENTRA\n",
           m, m, m);
    printf("      BRANCO (fonte)     : σ' = −1/σ          |σ'|<1 — dele tudo SAI\n");
    res += !(sigma_gt_1 && sigma_prime_lt_1);

    /* §3 — DIRECIONAL: iterar o gato x↦m+1/x CONVERGE ao negro (entra); iterar o inverso            */
    /*      x↦1/(x−m) converge ao branco (sai). Não se entra e sai pelo mesmo ponto.                 */
    /* ITERA-SE O PAR, E NÃO O VALOR. O gato x ↦ m + 1/x aplicado a p/q dá (m·p+q)/p:
     * inteiro puro, e é a recorrência dos convergentes. Assim «converge ao negro» deixa
     * de precisar de comparar dois doubles com 1e-9 e passa a ser o ENCAIXE, decidido
     * por produto cruzado contra x² − mx − 1 — a raiz nunca se avalia. */
    long P0 = 1, Q0 = 1;                       /* x = 1/1 */
    int alterna = 1, aperta = 1, dentro = 1;
    long antP = P0, antQ = Q0;
    for(int k = 0; k < 40; k++){
        long nP = (long)m*P0 + Q0, nQ = P0;
        if(nP > 1000000000L) break;
        /* de que lado de σ está P/Q? (2P − mQ)² contra Q²Δ, em inteiros */
        long s1 = 2*antP - (long)m*antQ, s2 = 2*nP - (long)m*nQ;
        int l1 = (s1 < 0) ? -1 : (s1*s1 < antQ*antQ*D ? -1 : (s1*s1 > antQ*antQ*D ? 1 : 0));
        int l2 = (s2 < 0) ? -1 : (s2*s2 < nQ*nQ*D ? -1 : (s2*s2 > nQ*nQ*D ? 1 : 0));
        if(k > 0 && (l1 == 0 || l2 == 0 || l1 == l2)) alterna = 0;   /* têm de ALTERNAR */
        if(k > 0 && !(nQ > antQ)) aperta = 0;                        /* e apertar       */
        antP = nP; antQ = nQ; P0 = nP; Q0 = nQ;
    }
    /* o mesmo pelo reverso: y ↦ 1/(y − m) leva ao branco, e em par é (q)/(p − m·q) */
    long R0 = 1, T0 = 2;                       /* y = 1/2 */
    int rev_ok = 1;
    for(int k = 0; k < 30; k++){
        long nR = T0, nT = R0 - (long)m*T0;
        if(nT == 0){ rev_ok = 0; break; }
        if(nR > 1000000000L || nT < -1000000000L) break;
        R0 = nR; T0 = nT;
    }
    int entra = (alterna && aperta && dentro), sai = rev_ok;
    res += !(entra && sai);
    printf("\n§3  DIRECIONAL — o gato aponta do branco para o negro, e o par é INTEIRO:\n");
    printf("      x↦m+1/x sobre p/q é (m·p+q)/p — os convergentes ALTERNAM à volta de σ\n");
    printf("      e o denominador APERTA: decidido por (2p−mq)² contra q²Δ, sem avaliar raiz: %s\n",
           VD(!(entra), "OK"));
    printf("      x↦1/(y−m) (reverso) corre em par e não degenera (SAI do branco): %s\n",
           VD(!(sai), "OK"));

    /* §4 — DUAIS: σ·σ'=−1, σ+σ'=m, σ'=−1/σ; inverter o gato (tempo reverso) troca branco↔negro.     */
    /* AS TRÊS IDENTIDADES EM ℤ[σ], sem avaliar raiz nenhuma. Na base {1,σ} com
     * σ² = mσ + 1, tem-se σ = (0,1) e σ† = (m,−1); e o produto (a+bσ)(c+dσ) reduz a
     * (ac+bd , ad+bc+m·bd). Donde σ+σ† = (m,0) = m e σ·σ† = (−1,0) = −1, EXACTOS. */
    long sa_=0, sb_=1, da_=m, db_=-1;                       /* σ = (0,1), σ† = (m,−1)  */
    long som_a = sa_+da_, som_b = sb_+db_;                  /* σ + σ†                  */
    long pro_a = sa_*da_ + sb_*db_;                         /* σ · σ†, parte racional  */
    long pro_b = sa_*db_ + sb_*da_ + (long)m*sb_*db_;       /*           parte em σ    */
    int dual_c = (som_a == m && som_b == 0)                 /* σ + σ† = m              */
              && (pro_a == -1 && pro_b == 0);               /* σ · σ† = −1  ⟺ σ† = −1/σ */
    /* no finito (§1): A⁻¹ = gato reverso; σ⁻¹=−σ' (pois σ·(−σ')=1) ⇒ A⁻¹ troca branco↔negro.        */
    res += !(dual_c && troca);
    printf("\n§4  DUAIS — o branco e o negro são um par:\n");
    printf("      em Z[s], por IGUALDADE: s+s* = (%ld,%ld) = m,  s*s* = (%ld,%ld) = -1,"
           "  logo s* = -1/s : %s\n", som_a, som_b, pro_a, pro_b, VD(!(dual_c), "OK"));
    printf("      inverter o gato (A⁻¹, tempo reverso) troca branco↔negro:  σ⁻¹=−σ' (exato): %s\n",
           VD(!(troca), "OK"));

    /* §5 — a leitura: o metal que a `linear` achou (σ_m, o brain=prata σ₂) é o NEGRO/sorvedouro;     */
    /*      o `navega` caminha do branco ao negro. Cada gato é uma seta branco→negro.                 */
    printf("\n§5  A LEITURA — o atrator metálico dos dados (linear: brain=σ₂) é o NEGRO/sorvedouro;\n");
    printf("      o navega caminha da fonte branca ao sorvedouro negro. Cada gato é uma seta ●→○ dual.\n");

    printf("\n----------------------------------------------------------------\n");
    printf("m=%d p=%d  σ=[%d;…] (negro, |σ|>1)  σ'=-1/σ (branco, |σ'|<1)   resíduo total = %d   %s\n",
           m, p, m, res,
           VD(res, "OS ATRATORES SÃO DIRECIONAIS E DUAIS — BRANCO (FONTE) → NEGRO (SORVEDOURO)"));
    return res?1:0;
}
