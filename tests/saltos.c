/* saltos.c — SALTAR sobre as órbitas: a multiplicação quaterniônica, não-comutativa.
 *
 * CAMINHAR era ×σ, dentro de UM plano ℂ=ℤ_p[gato] (comutativo, um par de atratores branco/negro).
 * SALTAR entre órbitas de atratores QUAISQUER sai desse ℂ: precisa da álgebra que contém todos os
 * ℂ — os QUATERNIONS ℍ = M_2(ℤ_p) (split, por Wedderburn). Um salto é ×(matriz 2×2). A parte real
 * é tr/2, os três VÉRTICES são a parte de traço-0, o conjugado é M*=tr·I−M (inverte os vértices),
 * a norma é det. O salto CONVERGE se mira o atrator NEGRO (sorvedouro) e DIVERGE se mira o BRANCO
 * (fonte) — o sinal da temperatura λ=ln σ. Como os atratores têm cores diferentes, a multiplicação
 * NÃO COMUTA (MN≠NM): não se volta pelo mesmo salto. Mas invertendo os vértices (o conjugado M*),
 * M*·M=det·I, e volta-se. Tudo exato, resíduo 0.
 *
 *   cc -O2 -std=c99 saltos.c -o saltos -lm
 *   ./saltos [p] [m]
 */
#include <stdio.h>
#include "naturais.h"      /* nt_primo: o p tem de ser primo */
#include "unidade.h"
#include <math.h>
#include "quat.h"

static M msub(M X,M Y){ return (M){ md(X.a-Y.a),md(X.b-Y.b),md(X.c-Y.c),md(X.d-Y.d) }; }
static M I1={1,0,0,1};

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
    printf("SALTAR SOBRE AS ÓRBITAS — a multiplicação quaterniônica ℍ=M_2(ℤ_%d), não-comutativa\n", p);
    printf("================================================================\n");
    if(!irred){ printf("  x²−%dx−1 cinde mod %d — o gato não dá um ℂ; escolha outro p,m\n",m,p); return 2; }

    M G={m,1,1,0};                                    /* o gato A_m = [[m,1],[1,0]]                 */

    /* §1 — OS SALTOS SÃO QUATERNIONS: ℍ=M_2(ℤ_p). real=tr/2, conjugado M*=tr·I−M, norma=det.       */
    /*      Base de Hamilton i,j,k (i²=j²=k²=−1, ij=k=−ji) quando −1 é quadrado (p≡1 mod4).          */
    M conjG=madj(G); M GGc=mmul(G,conjG);
    int q_ok = meq(GGc, mscal(mdet(G),I1)) && meq(msub(mscal(mtr(G),I1),G), conjG);
    res += !q_ok;
    printf("\n§1  OS SALTOS SÃO QUATERNIONS — ℍ=M_2(ℤ_%d): real=tr/2, conjugado M*=tr·I−M, norma=det\n", p);
    printf("      o gato G: tr=%d, det=%d ; G·G*=det·I e M*=tr·I−M: %s\n", mtr(G), mdet(G), VD(!(q_ok), "OK"));
    long s=-1; for(long t=0;t<p;t++) if(t*t%p==(long)(p-1)){ s=t; break; }   /* s²=−1 ?               */
    if(s>=0){
        M i={ (int)s,0,0,md(-s) }, j={0,1,md(-1),0}, k=mmul(i,j);
        int ham = meq(mmul(i,i),mscal(p-1,I1)) && meq(mmul(j,j),mscal(p-1,I1)) &&
                  meq(mmul(k,k),mscal(p-1,I1)) && meq(k, mscal(p-1,mmul(j,i)));   /* ij=k=−ji        */
        res += !ham;
        printf("      base de Hamilton (s=√−1=%ld): i²=j²=k²=−1 e ij=k=−ji: %s\n", s, VD(!(ham), "OK"));
    } else printf("      (−1 não é quadrado mod %d; a base i,j,k vive em GF(p²) — a álgebra é a mesma)\n", p);

    /* §2 — os muitos ℂ dentro de ℍ: cada gato/atrator gera um ℂ=ℤ_p[M] comutativo; ℍ os contém.    */
    /*      Saltar entre órbitas do MESMO ℂ comuta; entre atratores DIFERENTES, não (sai do ℂ).     */
    M Pm={1,1,1,2}, cP=mmul(G,Pm), pC=mmul(Pm,G);     /* G e outra matriz Pm (outro atrator/cor)    */
    int mesmoC = meq(mmul(G,mmul(G,G)), mmul(mmul(G,G),G));  /* dentro de ℤ_p[G]: comuta            */
    int outroC = !meq(cP,pC);                          /* G e Pm (cores diferentes): não comuta     */
    res += !(mesmoC && outroC);
    printf("\n§2  MUITOS ℂ DENTRO DE ℍ — cada atrator gera um ℂ=ℤ_p[gato]; saltar entre cores sai dele:\n");
    printf("      no mesmo ℂ (potências de G) comuta: %s ; entre atratores G,P diferentes: G·P≠P·G: %s\n",
           mesmoC?"sim":"não", outroC?"sim":"não");

    /* §3 — CONVERGENTE (negro) / DIVERGENTE (branco) = a TEMPERATURA. No autobase o gato é          */
    /*      diag(σ,σ'): a componente NEGRA ×σ (|σ|>1, cresce/converge à direção negra), a BRANCA     */
    /*      ×σ' (|σ'|<1, encolhe/foge). λ=ln σ é o inverso da temperatura (frio→negro, quente→branco)*/
    double sq=sqrt((double)m*m+4), sig=(m+sq)/2, sil=(m-sq)/2, lam=log(sig);
    double un=1, ub=1;                                  /* componentes negra e branca (autobase)     */
    printf("\n§3  CONVERGENTE (NEGRO) / DIVERGENTE (BRANCO) = A TEMPERATURA — λ=ln σ=%.4f:\n", lam);
    printf("      itero o gato; razão branca/negra = (σ'/σ)^k → 0 (esfria para o negro):\n      ");
    int mono=1; double prev=1e18;
    for(int k=0;k<24;k++){ double r=fabs(ub/un); if(k<6) printf("%.4f ", r); if(r>=prev) mono=0; prev=r; un*=sig; ub*=sil; }
    int esfria = (long long)(fabs(ub/un) * 1e6) == 0 && mono;            /* monótona decrescente → 0 */
    res += !esfria;
    printf("… → %.1e  %s\n", fabs(ub/un), VD(!(esfria), "(monótona → 0: converge ao NEGRO; o BRANCO diverge — a cor é a temperatura)"));

    /* §4 — SALTAR ENTRE QUAISQUER DUAS ÓRBITAS: GL_2(ℤ_p) age transitivamente — para todo x,y≠0 há  */
    /*      um salto (quaternion invertível) M com M·x=y. Verifica TODOS os pares.                   */
    long falhas=0, pares=0;
    for(int x1=0;x1<p;x1++) for(int x2=0;x2<p;x2++){ if(!x1&&!x2) continue;
      for(int y1=0;y1<p;y1++) for(int y2=0;y2<p;y2++){ if(!y1&&!y2) continue; pares++;
        /* base {x,u}: acha u com det(x|u)≠0 */
        int uu[3][2]={{1,0},{0,1},{1,1}}, ok=0; M Bx={0,0,0,0},By={0,0,0,0};
        for(int t=0;t<3;t++){ if(md((long)x1*uu[t][1]-(long)x2*uu[t][0])){ Bx=(M){x1,uu[t][0],x2,uu[t][1]}; ok=1; break; } }
        int ok2=0; for(int t=0;t<3;t++){ if(md((long)y1*uu[t][1]-(long)y2*uu[t][0])){ By=(M){y1,uu[t][0],y2,uu[t][1]}; ok2=1; break; } }
        if(!ok||!ok2){ falhas++; continue; }
        long di=pw(mdet(Bx),p-2); M Bxinv=mscal(di,madj(Bx));   /* inv(Bx) = adj/det                */
        M Mj=mmul(By,Bxinv);                                     /* o salto x→e1→y                  */
        int rx=md((long)Mj.a*x1+(long)Mj.b*x2), ry=md((long)Mj.c*x1+(long)Mj.d*x2);
        if(rx!=y1||ry!=y2||mdet(Mj)==0) falhas++;
    }}
    res += (falhas!=0);
    printf("\n§4  SALTAR ENTRE QUAISQUER DUAS ÓRBITAS — GL_2 transitivo: ∀ x,y≠0 há salto M·x=y:\n");
    printf("      %ld pares (x,y) testados, saltos que falharam: %ld  %s\n",
           pares, falhas, VD(falhas, "há sempre um quaternion que salta (OK)"));

    /* §5 — NÃO COMUTA ⇒ não se volta pelo mesmo salto; INVERTER OS VÉRTICES (M*=tr·I−M) ⇒ volta.    */
    M A={2,1,1,1}, B={1,1,0,1};                        /* dois saltos                                */
    int naovolta = !meq(mmul(A,B), mmul(B,A));         /* A·B ≠ B·A                                  */
    M Astar=madj(A), volta=mmul(Astar,A);              /* inverter os vértices de A                  */
    int volta_ok = meq(volta, mscal(mdet(A),I1));      /* A*·A = det·I ⇒ desfaz (a menos de det)     */
    /* de fato: (1/det)A* leva A·x de volta a x */
    long dinv=pw(mdet(A),p-2); M Ainv=mscal(dinv,Astar);
    int desfaz = meq(mmul(Ainv,A), I1);
    res += !(naovolta && volta_ok && desfaz);
    printf("\n§5  NÃO COMUTA ⇒ NÃO VOLTA; INVERTER OS VÉRTICES ⇒ VOLTA:\n");
    printf("      A·B ≠ B·A (cores diferentes, não volta pelo mesmo): %s\n", naovolta?"sim":"não");
    printf("      o conjugado A*=tr·I−A (inverte os 3 vértices): A*·A=det·I: %s ; (1/det)A* desfaz A: %s  %s\n",
           volta_ok?"sim":"não", desfaz?"sim":"não", VD(!((naovolta&&volta_ok&&desfaz)), "OK"));

    printf("\n----------------------------------------------------------------\n");
    printf("p=%d m=%d   ℍ=M_2 não-comutativo ; a cor (branco/negro) é a temperatura   resíduo total = %d   %s\n",
           p, m, res, VD(res, "SALTA-SE POR QUATERNIONS ENTRE ATRATORES; A COR NÃO DEIXA VOLTAR, MAS O CONJUGADO SIM"));
    return res?1:0;
}
