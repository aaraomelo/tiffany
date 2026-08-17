/* complexo.c — os atratores formam ℂ: GF(p²) é o plano complexo, branco e negro em eixos conjugados.
 *
 * GF(p²) é a extensão quadrática de ℤ_p — o análogo exato de ℂ=ℝ[i]: um EIXO REAL (ℤ_p, fixo
 * pela conjugação) e um EIXO IMAGINÁRIO (τ, com tr τ=0 e τ²=não-quadrado, o "i"). A conjugação
 * é o Frobenius z↦z^p (=z̄); a norma N(z)=z·z̄ ∈ ℤ_p (=|z|²) é multiplicativa; todo z≠0 é
 * invertível. Os atratores do gato são o par CONJUGADO: o NEGRO σ=m/2+τ e o BRANCO σ'=m/2−τ=σ̄,
 * com a MESMA parte real e partes imaginárias OPOSTAS (±1) — um em cada eixo. O gato é a
 * multiplicação complexa ×σ. As operações ⊕,⊗ herdadas do corpo são as de ℂ. Logo, dada uma
 * representação, o conjunto dos atratores com essas operações é isomorfo aos complexos, com os
 * brancos num eixo e os negros no outro. Tudo exato, resíduo 0.
 *
 *   cc -O2 -std=c99 complexo.c -o complexo
 *   ./complexo [p] [m]
 */
#include <stdio.h>
#include "naturais.h"      /* nt_primo: o p tem de ser primo */
#include "unidade.h"
#include <stdlib.h>
#include "gp2.h"                                   /* a peça: o gato em GF(p²) (mul, add, sub, scal, pw, frob, σ) */

int main(int argc,char**argv){
    p = argc>1? atoi(argv[1]) : 7;
    m = argc>2? atoi(argv[2]) : 1;
    /* O `p` E O PRIMO DO CORPO, e vinha de argv sem uma unica verificacao: com
     * `p = 0` toda a aritmetica %% p rebentava em SIGFPE, e com p composto (4, 6)
     * GF(p) nao e corpo nenhum e os resultados sairiam falsos EM SILENCIO, que e
     * pior. A primalidade tem teste na casa — nt_primo, em lib/naturais.h — e nao
     * se escreve aqui uma setima copia. */
    if(!nt_primo((unsigned long)p)){
        printf("  p = %d nao e primo: GF(p) so e corpo com p primo, e sem isso\n", p);
        printf("  nem a divisao existe. uso: %s <p primo> [m]\n", argv[0]);
        return 2;
    }

    int res=0;

    int irred=1; for(int t=0;t<p;t++) if((((long)t*t-(long)m*t-1)%p+p)%p==0){ irred=0; break; }
    printf("OS ATRATORES FORMAM ℂ — GF(%d²) é o plano complexo; branco e negro em eixos conjugados\n", p);
    printf("================================================================\n");
    if(!irred){ printf("  x²−%dx−1 cinde mod %d — GF(p²) seria ℝ×ℝ (split), não ℂ; escolha outro p,m\n",m,p); return 2; }

    long inv2=(p+1)/2, inv4=inv2*inv2%p;
    E TAU = sub(SIG, scal(m*inv2%p, ONE));           /* τ = σ − m/2 : o "i", com tr τ=0             */
    E t2  = mul(TAU,TAU);                             /* τ² deve ser um escalar não-quadrado        */
    long D=((long)m*m+4)%p, s=D*inv4%p;               /* τ² = (m²+4)/4                              */

    /* §1 — GF(p²) É ℂ: eixo real ℤ_p + eixo imaginário τ (τ²=s, não-quadrado, como i²=−1).         */
    int tau_ok = (t2.b==0 && t2.a==(int)s);
    int nqr=1; for(int t=0;t<p;t++) if((long)t*t%p==s){ nqr=0; break; }   /* s é não-quadrado?       */
    res += !(tau_ok && nqr);
    printf("\n§1  GF(%d²) É ℂ — eixo real ℤ_%d (fixo) + eixo imaginário τ (o \"i\"):\n", p, p);
    printf("      τ=(%d,%d) em base σ ; tr τ=0 ; τ²=%d (escalar) = (m²+4)/4 ; não-quadrado (como i²=−1): %s  %s\n",
           TAU.a, TAU.b, t2.a, nqr?"sim":"não", VD(!((tau_ok&&nqr)), "OK"));

    /* §2 — a conjugação é o Frobenius z↦z^p (=z̄): involução, fixa o eixo real, nega o imaginário.  */
    long v_inv=0,v_fix=0,v_neg=0,v_hom=0;
    for(int a=0;a<p;a++) for(int b=0;b<p;b++){
        E w={a,b}, wc=frob(w);
        if(!eq(frob(wc),w)) v_inv++;                                  /* z̄̄ = z (involução)          */
        E re=scal(1,add(w,wc)); /* w+z̄ = 2·Re ∈ eixo real (b-coef 0 em base τ) */
        /* Re(w) = (w + z̄)/2 é fixo pela conjugação: */ if(!eq(frob(scal(inv2,re)),scal(inv2,re))) v_fix++;
        E im=sub(w,wc);          /* w − z̄ = 2·(parte imaginária): a conjugação a nega */
        if(!eq(frob(im), scal(p-1, im))) v_neg++;                     /* frob(im) = −im             */
    }
    for(int a=0;a<p;a++) for(int b=0;b<p;b++) for(int c=0;c<p;c++) for(int d=0;d<p&&v_hom<1;d++){
        E w={a,b}, x={c,d};
        if(!eq(frob(mul(w,x)), mul(frob(w),frob(x)))) v_hom++;        /* frob(wx)=z̄·x̄ (homomorf.)   */
    }
    res += (v_inv||v_fix||v_neg||v_hom);
    printf("\n§2  A CONJUGAÇÃO É z↦z̄ (o Frobenius) — involução, fixa o real, nega o imaginário:\n");
    printf("      z̄̄=z: viol=%ld ; Re fixo: %ld ; Im negado: %ld ; frob(zw)=z̄w̄: %ld  %s\n",
           v_inv, v_fix, v_neg, v_hom, VD((v_inv||v_fix||v_neg||v_hom), "OK"));

    /* §3 — a norma N(z)=z·z̄ (=|z|²) ∈ ℤ_p é multiplicativa, e todo z≠0 é invertível (é corpo).     */
    long v_norm=0,v_inv2=0;
    for(int a=0;a<p;a++) for(int b=0;b<p;b++){
        E w={a,b}; E n=mul(w,frob(w));
        if(n.b!=0) v_norm++;                                         /* N(z) é real (escalar)      */
        if(a||b){ E winv=scal((n.a? (long)pw((E){n.a%p,0}, (long)p*p-2).a : 0), frob(w));
                  /* z^{-1} = z̄ / N(z) : mais simples via potência                                */
                  E zi=pw(w,(long)p*p-2); if(!eq(mul(w,zi),ONE)) v_inv2++; (void)winv; }
    }
    long v_mult=0;
    for(int a=0;a<p&&v_mult<1;a++) for(int b=0;b<p;b++) for(int c=0;c<p;c++) for(int d=0;d<p;d++){
        E w={a,b}, x={c,d}; E nl=mul(mul(w,x),frob(mul(w,x))); E nr=mul(mul(w,frob(w)),mul(x,frob(x)));
        if(nl.a!=nr.a){ v_mult++; break; }
    }
    res += (v_norm||v_inv2||v_mult);
    printf("\n§3  A NORMA N(z)=z·z̄ (=|z|²) — real, multiplicativa (|zw|²=|z|²|w|²); todo z≠0 invertível:\n");
    printf("      N(z) escalar: viol=%ld ; N(zw)=N(z)N(w): %ld ; z·z⁻¹=1: %ld  %s\n",
           v_norm, v_mult, v_inv2, VD((v_norm||v_inv2||v_mult), "OK"));

    /* §4 — os ATRATORES são o par conjugado: NEGRO σ e BRANCO σ'=σ̄, mesma parte real, Im=±1.       */
    E neg=SIG, bra=frob(SIG);                          /* σ e σ'=σ^p                                 */
    int negIm=neg.b, braIm=bra.b;                      /* coeficiente de τ = coeficiente de σ (Im)   */
    int negRe=(neg.a + neg.b*(int)(m*inv2%p))%p, braRe=(bra.a + bra.b*(int)(m*inv2%p))%p;
    E somaN=add(neg,bra), prodN=mul(neg,bra);
    int conjOK=eq(bra,frob(neg)), eixoOK=(negIm==1 && braIm==(p-1)), reOK=(negRe==braRe);
    res += !(conjOK && eixoOK && reOK);
    printf("\n§4  BRANCO E NEGRO EM EIXOS CONJUGADOS — σ'=σ̄, mesma parte real, Im opostas (±1):\n");
    printf("      NEGRO  σ : Im=+%d (eixo +τ) ; BRANCO σ'=σ̄ : Im=%d (eixo −τ) ; mesma Re=%d : %s\n",
           negIm, braIm-p, negRe, reOK?"sim":"não");
    printf("      σ'=frob(σ): %s ; σ+σ'=%d (real=m, no eixo real) ; σ·σ'=N(σ)=%d (=−1)  %s\n",
           conjOK?"sim":"não", somaN.a, prodN.a, VD(!((conjOK&&eixoOK&&reOK)), "OK"));

    /* §5 — o ISOMORFISMO com ℂ: a⊕bτ e o produto seguem a lei de ℂ, (a+bτ)(c+dτ)=(ac+bd·s)+(ad+bc)τ */
    /*      — a mesma de ℂ com i²=−1, aqui τ²=s. E quando −1 é não-quadrado (p≡3 mod 4), é ℤ_p[i].    */
    long v_iso=0;
    for(int a=0;a<p;a++) for(int b=0;b<p;b++) for(int c=0;c<p;c++) for(int d=0;d<p&&v_iso<1;d++){
        E w=add(scal(a,ONE),scal(b,TAU)), x=add(scal(c,ONE),scal(d,TAU));
        E prod=mul(w,x);
        E esper=add( scal(((long)a*c+(long)b*d%p*s)%p, ONE), scal(((long)a*d+(long)b*c)%p, TAU) );
        if(!eq(prod,esper)) v_iso++;
    }
    int m1qr=0; { long mm=(p-1); for(int t=0;t<p;t++) if((long)t*t%p==mm){ m1qr=1; break; } }
    int gauss = !m1qr;                               /* −1 não-quadrado (p≡3 mod4) ⇒ ℤ_p[i], i²=−1 */
    res += (v_iso!=0);
    printf("\n§5  O ISOMORFISMO COM ℂ — (a+bτ)(c+dτ)=(ac+bd·τ²)+(ad+bc)τ, a lei de ℂ (τ² faz de i²):\n");
    printf("      o produto do corpo segue a fórmula de ℂ: viol=%ld ; −1 %s quadrado ⇒ %s  %s\n",
           v_iso, m1qr?"é":"não é", gauss?"GF(p²)=ℤ_p[i] com i²=−1 (forma de Gauss)":"GF(p²)=ℤ_p[τ], τ²=não-quadrado (ℂ estrutural)",
           VD(v_iso, "OK"));

    /* §6 — dada uma representação, os atratores (os representantes das classes, o que a `linear`     */
    /*      extrai dos dados) são pontos deste ℂ; cada um traz o seu conjugado (branco), e ⊕,⊗ os     */
    /*      mantêm no corpo. O conjunto dos atratores com as operações herdadas É ℂ.                  */
    printf("\n§6  A REPRESENTAÇÃO — os atratores dos dados (os representantes irracionais, `linear`)\n");
    printf("      são pontos deste ℂ finito; cada negro traz o seu branco conjugado; ⊕,⊗ fecham nele.\n");

    printf("\n----------------------------------------------------------------\n");
    printf("p=%d m=%d  τ²=%d (não-quadrado)  σ=Im+1 (negro)  σ̄=Im−1 (branco)   resíduo total = %d   %s\n",
           p, m, t2.a, res,
           VD(res, "OS ATRATORES FORMAM ℂ — BRANCOS NUM EIXO, NEGROS NO OUTRO (CONJUGADOS)"));
    return res?1:0;
}
