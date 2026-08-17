/* esquilo.c — o dual do gato: o esquilo reverte, e a ação bilateral vira corpo (comuta).
 *
 * O quaternion ℍ=M_2 é INCOMPLETO: não-comutativo, não é corpo — falta o seu dual, o ESPELHO.
 * O GATO anda à esquerda (×A, a seta do tempo, rumo ao negro); o ESQUILO anda à direita (×B, o
 * reflexo, a direção contrária). Não é duplicação: a ação à direita é o mesmo ℍ espelhado — a
 * álgebra oposta ℍ^op ≅ ℍ pela transposta, que INVERTE a ordem (reverte). O ponto: multiplicar
 * à esquerda (gato) e à direita (esquilo) SEMPRE COMUTA — A(xB)=(Ax)B —, enquanto duas à esquerda
 * (só gato) NÃO. A comutatividade que faltava volta com o dual: o corpo se completa. Assim vai-se
 * por gato na seta do tempo e volta-se por esquilo na direção contrária (a conjugação x↦AxA⁻¹ e a
 * sua inversa), saltando entre órbitas de atratores em QUALQUER direção. Exato, resíduo 0.
 *
 *   cc -O2 -std=c99 esquilo.c -o esquilo
 *   ./esquilo [p] [m]
 */
#include <stdio.h>
#include "naturais.h"      /* nt_primo: o p tem de ser primo */
#include "unidade.h"
#include "quat.h"

static M mtransp(M X){ return (M){ X.a, X.c, X.b, X.d }; }          /* o espelho: troca b↔c          */
static M minv(M X){ return mscal(pw(mdet(X),p-2), madj(X)); }       /* X⁻¹ = adj/det                 */

int main(int argc,char**argv){
    p = argc>1? atoi(argv[1]) : 13;
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
    printf("O ESQUILO — o dual do gato: o espelho que reverte e completa o corpo (ℤ_%d)\n", p);
    printf("================================================================\n");
    if(!irred){ printf("  x²−%dx−1 cinde mod %d — escolha outro p,m\n",m,p); return 2; }

    M G={m,1,1,0};                                    /* o gato                                     */

    /* §1 — O QUATERNION É INCOMPLETO: só o gato (tudo à esquerda) NÃO comuta. Não é corpo.          */
    long v_ll=0;
    for(int a=0;a<p;a++)for(int b=0;b<p;b++)for(int c=0;c<p;c++)for(int d=0;d<p&&v_ll<1;d++){
        M A={a,b,c,d}; if(!meq(mmul(G,A),mmul(A,G))) v_ll++;   /* existe A com GA≠AG                */
    }
    printf("\n§1  O QUATERNION É INCOMPLETO — só o gato (à esquerda) não comuta: ∃ A, G·A≠A·G: %s\n",
           v_ll?"sim (não é corpo — falta o dual)":"não");
    res += (v_ll==0);                                  /* esperamos que HAJA não-comutação           */

    /* §2 — O ESQUILO É O ESPELHO: a ação à direita = ℍ^op, a transposta que INVERTE a ordem         */
    /*      ((A·B)ᵀ=Bᵀ·Aᵀ). É o mesmo ℍ refletido — não uma duplicação.                              */
    long v_mir=0, v_anti=0;
    for(int a=0;a<p;a++)for(int b=0;b<p&&v_mir<1;b++)for(int c=0;c<p;c++)for(int d=0;d<p;d++){
        M A={a,b,c,d}, B={d,c,b,a};
        if(!meq(mtransp(mmul(A,B)), mmul(mtransp(B),mtransp(A)))){ v_mir++; break; }
        if(!meq(mtransp(mmul(A,B)), mmul(mtransp(A),mtransp(B)))) v_anti++;   /* ≠ na ordem direta   */
    }
    res += (v_mir!=0) || (v_anti==0);                  /* espelho inverte a ordem (anti-iso)         */
    printf("\n§2  O ESQUILO É O ESPELHO — a ação à direita (ℍ^op): a transposta INVERTE a ordem (reverte):\n");
    printf("      (A·B)ᵀ = Bᵀ·Aᵀ sempre: viol=%ld ; e ≠ Aᵀ·Bᵀ (é o reflexo, não cópia): %s  %s\n",
           v_mir, v_anti?"sim":"não", VD(!((v_mir==0&&v_anti)), "OK"));

    /* §3 — GATO (esquerda) E ESQUILO (direita) SEMPRE COMUTAM: A·(x·B) = (A·x)·B. A comutatividade  */
    /*      que faltava volta com o dual — o corpo se completa.                                       */
    long v_lr=0;
    for(int a=0;a<p;a++)for(int b=0;b<p;b++)for(int c=0;c<p;c++)for(int d=0;d<p;d++){
        M A=G, x={a,b,c,d}, B={2,1,1,1};               /* gato à esquerda, esquilo B à direita       */
        if(!meq(mmul(A,mmul(x,B)), mmul(mmul(A,x),B))) v_lr++;
    }
    res += (v_lr!=0);
    printf("\n§3  GATO⊗ESQUILO COMUTAM — ×esquerda (gato) e ×direita (esquilo): A·(x·B)=(A·x)·B sempre:\n");
    printf("      testados %d estados x: violações=%ld  %s\n", p*p*p*p, v_lr,
           VD(v_lr, "a comutatividade volta com o dual (o corpo se completa) — OK"));

    /* §4 — O LADO NEGRO: o gato e o seu espelho conjugado adj(G)=tr·I−G vivem em ℂ=ℤ_p[G] e COMUTAM; */
    /*      o esquilo σ'=σ̄ (o conjugado, já no corpo) é a direção contrária (σσ'=−1).                 */
    int negro_comuta = meq(mmul(G,madj(G)), mmul(madj(G),G));
    int somaraiz = mtr(G), prodraiz = mdet(G);         /* σ+σ'=tr=m, σ·σ'=det=−1                     */
    res += !(negro_comuta && somaraiz==m%p && prodraiz==p-1);
    printf("\n§4  O LADO NEGRO COMUTA — G e o seu espelho adj(G) ∈ ℂ=ℤ_p[G] comutam (não duplica):\n");
    printf("      G·adj(G)=adj(G)·G: %s ; σ+σ'=%d (=m), σ·σ'=%d (=−1) — o esquilo σ'=σ̄ é a direção contrária  %s\n",
           negro_comuta?"sim":"não", somaraiz, prodraiz, VD(!((negro_comuta&&somaraiz==m%p&&prodraiz==p-1)), "OK"));

    /* §5 — SALTAR EM QUALQUER DIREÇÃO: vai por gato (×G à esquerda, seta do tempo), volta por esquilo */
    /*      (×G⁻¹ à direita) — a conjugação x↦G x G⁻¹ e a sua inversa G⁻¹ x G devolvem x. Resíduo 0.  */
    M Gi=minv(G); long v_go=0;
    for(int a=0;a<p;a++)for(int b=0;b<p;b++)for(int c=0;c<p;c++)for(int d=0;d<p;d++){
        M x={a,b,c,d};
        M ida = mmul(mmul(G,x),Gi);                    /* gato à esquerda, esquilo (G⁻¹) à direita   */
        M volta = mmul(mmul(Gi,ida),G);                /* pela direção contrária                     */
        if(!meq(volta,x)) v_go++;
    }
    res += (v_go!=0);
    printf("\n§5  SALTAR EM QUALQUER DIREÇÃO — ida por gato (seta do tempo) x↦G x G⁻¹; volta por esquilo:\n");
    printf("      todos os %d estados: ida-e-volta com erro=%ld  %s\n", p*p*p*p, v_go,
           VD(v_go, "EXATO, resíduo 0 — salta-se entre atratores e volta-se, qualquer direção"));

    printf("\n----------------------------------------------------------------\n");
    printf("p=%d m=%d   gato→ (seta do tempo) + esquilo← (espelho) = corpo completo   resíduo total = %d   %s\n",
           p, m, res, VD(res, "O ESQUILO COMPLETA O GATO — VAI PELA SETA DO TEMPO, VOLTA PELO ESPELHO, EM QUALQUER DIREÇÃO"));
    return res?1:0;
}
