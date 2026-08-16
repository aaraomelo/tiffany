/* norma.c — a conservação com a NORMA DE CORPO: os k conjugados se somam e se multiplicam.
 *
 * Um objeto completo z em ℝ^k = GF(p^k) tem k CONJUGADOS de Galois — as imagens pelo Frobenius
 * φ:x↦x^p (o esquilo iterado): z, z^p, z^{p²}, …, z^{p^{k-1}}. São as k "projeções" de z, uma por
 * dimensão. Nenhuma sozinha é z (se z é completo/primitivo); só uma parte. Mas:
 *   • a SOMA das k projeções é o TRAÇO   Tr(z)=Σ_{j=0}^{k-1} φ^j(z) ∈ ℤ_p  (o corpo base, dim 1);
 *   • o PRODUTO das k projeções é a NORMA N(z)=∏_{j=0}^{k-1} φ^j(z) ∈ ℤ_p.
 * Ambos DESCEM ao fundo ℤ_p — conservados, invariantes de Galois —, e são os coeficientes do
 * polinômio de z (Tr = −c_{k-1}, N = (−1)^k c_0): as funções simétricas dos conjugados. A soma
 * vai de 1 até k, conservando. É a lei do gato generalizada: em dim 2, Tr(σ)=m e N(σ)=−1 (o metal).
 * Exato, resíduo 0.
 *
 *   cc -O2 -std=c99 norma.c -o norma
 */
#include <stdio.h>
#include "unidade.h"
#include <stdlib.h>
#include <string.h>

static int p;
static int md(long x){ return (int)((x%p+p)%p); }
static int invp(int a){ int t=0,nt=1,r=p,nr=((a%p)+p)%p; while(nr){ int q=r/nr,x; x=t-q*nt;t=nt;nt=x; x=r-q*nr;r=nr;nr=x; } return r>1?0:md(t); }
static int pdeg(const int*c,int n){ for(int i=n;i>=0;i--) if(c[i]) return i; return -1; }

static void pmul(const int*a,int da,const int*b,int db,int*out){
    for(int i=0;i<=da+db;i++) out[i]=0;
    for(int i=0;i<=da;i++) if(a[i]) for(int j=0;j<=db;j++) out[i+j]=md(out[i+j]+(long)a[i]*b[j]);
}
static void pmod_monic(int*a,int da,const int*f,int df){       /* f mônico: reduz a mod f in place  */
    for(int i=da;i>=df;i--) if(a[i]){ int c=a[i]; for(int j=0;j<=df;j++) a[i-df+j]=md(a[i-df+j]-(long)c*f[j]); }
}
static void pmulmod(const int*a,const int*b,const int*f,int k,int*out){
    int t[64]; pmul(a,k-1,b,k-1,t); pmod_monic(t,2*k-2,f,k); for(int i=0;i<k;i++) out[i]=t[i];
}
static void ppow(const int*a,long e,const int*f,int k,int*out){
    int r[32],b[32]; for(int i=0;i<k;i++){ r[i]=0; b[i]=a[i]; } r[0]=1;
    while(e){ if(e&1){ int t[32]; pmulmod(r,b,f,k,t); memcpy(r,t,k*sizeof(int)); } int t[32]; pmulmod(b,b,f,k,t); memcpy(b,t,k*sizeof(int)); e>>=1; }
    memcpy(out,r,k*sizeof(int));
}
static int pgcd_deg(const int*A,int da,const int*B,int db){     /* grau do mdc de A,B (0 = coprimos) */
    int a[32]={0},b[32]={0},na,nb;
    for(int i=0;i<=da;i++) a[i]=A[i];
    for(int i=0;i<=db;i++) b[i]=B[i];
    na=pdeg(a,da); nb=pdeg(b,db);
    while(nb>=0){
        int ib=invp(b[nb]);
        for(int i=na;i>=nb;i--) if(a[i]){ int c=md((long)a[i]*ib); for(int j=0;j<=nb;j++) a[i-nb+j]=md(a[i-nb+j]-(long)c*b[j]); }
        na=pdeg(a,na);
        int t[32]; memcpy(t,a,sizeof t); memcpy(a,b,sizeof t); memcpy(b,t,sizeof t);
        int tn=na; na=nb; nb=tn;
    }
    return na<0? 0 : na;
}
static int frob_iter(const int*a,int m,const int*f,int k,int*out){ /* φ^m(a) = a^{p^m}                */
    int g[32]; memcpy(g,a,k*sizeof(int));
    for(int i=0;i<m;i++){ int t[32]; ppow(g,p,f,k,t); memcpy(g,t,k*sizeof(int)); }
    memcpy(out,g,k*sizeof(int)); return 0;
}
static int is_irred(const int*f,int k){
    int x[32]={0}; x[1]=1;
    int g[32]; frob_iter(x,k,f,k,g);                            /* x^{p^k} =? x                      */
    int isx=(k>1)&&(g[1]==1); for(int i=0;i<k;i++) if(i!=1 && g[i]) isx=0;
    if(k==1) isx=1;
    if(!isx) return 0;
    for(int q=2;q<=k;q++){ if(k%q) continue; int pr=1; for(int d=2;d<q;d++) if(q%d==0) pr=0; if(!pr) continue;
        int h[32]; frob_iter(x,k/q,f,k,h); h[1]=md(h[1]-1);     /* x^{p^{k/q}} − x                   */
        int dh=pdeg(h,k-1); if(dh<0) return 0;
        if(pgcd_deg(f,k,h,dh)>0) return 0;
    }
    return 1;
}
static int find_f(int k,int*f){
    long total=1; for(int i=0;i<k;i++) total*=p;
    for(long code=0;code<total;code++){ f[k]=1; long c=code; for(int i=0;i<k;i++){ f[i]=(int)(c%p); c/=p; }
        if(is_irred(f,k)) return 1; }
    return 0;
}

int main(int argc,char**argv){
    p = argc>1? atoi(argv[1]) : 5;
    int K = argc>2? atoi(argv[2]) : 6; if(K>6) K=6;
    int res=0;
    printf("A CONSERVAÇÃO COM A NORMA DE CORPO — os k conjugados se somam (traço) e multiplicam (norma)\n");
    printf("================================================================\n");
    printf("  z em GF(%d^k): conjugados z,z^p,…,z^{p^{k-1}} (o Frobenius, o esquilo iterado)\n", p);
    printf("  Tr(z)=Σ conjugados ∈ ℤ_%d ; N(z)=∏ conjugados ∈ ℤ_%d — descem ao corpo base (dim 1)\n", p, p);
    printf("\n  dim k   Tr(z)=Σ (soma)   N(z)=∏ (produto)   ∈ ℤ_p (conservado)?   z num subcorpo?\n");
    printf("  --------------------------------------------------------------------------------\n");
    for(int k=1;k<=K;k++){
        int f[16]; if(!find_f(k,f)){ printf("    %-2d   (sem irredutível achado)\n",k); continue; }
        /* um objeto z genérico e completo (primitivo): z = x + 1 (grau 1, gera o corpo se f irred). */
        int z[32]={0}; z[0]=1; if(k>1) z[1]=1;                  /* z = 1 + σ                          */
        int soma[32]={0}, prod[32]={0}; prod[0]=1;
        for(int j=0;j<k;j++){ int c[32]; frob_iter(z,j,f,k,c);
            for(int i=0;i<k;i++) soma[i]=md(soma[i]+c[i]);       /* Σ conjugados                      */
            int t[32]; pmulmod(prod,c,f,k,t); memcpy(prod,t,k*sizeof(int)); } /* ∏ conjugados         */
        /* Tr e N devem ser escalares (só componente 0) — descem a ℤ_p                                */
        int tr_esc=1, n_esc=1; for(int i=1;i<k;i++){ if(soma[i]) tr_esc=0; if(prod[i]) n_esc=0; }
        /* z está num subcorpo próprio? (algum conjugado φ^d(z)=z com d|k, d<k) — não, z primitivo    */
        int subcorpo=0; for(int d=1;d<k;d++){ if(k%d==0){ int c[32]; frob_iter(z,d,f,k,c); int eq=1; for(int i=0;i<k;i++) if(c[i]!=z[i]) eq=0; if(eq) subcorpo=1; } }
        /* E AQUI APLICA-SE O GERADOR (thm:gerador-andar da teoria), que estava CALCULADO
         * e NÃO MEDIDO: a coluna «z num subcorpo?» era impressa e o `res` só olhava para
         * tr_esc e n_esc. Mas ela é a condição do resultado, e não um adorno.
         *
         * O traço e a norma são a soma e o produto dos k conjugados — e só são os do corpo
         * INTEIRO se z gerar esse corpo. Se z vivesse num subcorpo próprio, os conjugados
         * repetir-se-iam e o que se estaria a medir seria o traço de OUTRO andar, com o
         * mesmo aspecto no ecrã.
         *
         * Duas metades, e agora contam as duas: f é irredutível de grau k (verificado no
         * find_f, que é o que faz do quociente um corpo), e z não está em subcorpo próprio
         * (verificado aqui, e é o que faz de z um gerador). São exactamente (i) e (ii) do
         * teorema: o gerador define o andar, e sem ele o zero desce sem se dar por isso. */
        int f_irred = is_irred(f, k);
        res += !(tr_esc && n_esc);
        res += !!subcorpo;                    /* z TEM de gerar: senão é outro andar */
        res += !f_irred;                      /* e o quociente TEM de ser corpo */
        if(subcorpo || !f_irred)
            printf("      ↑ e este NÃO conta: f irredutível %s, z gera %s\n",
                   f_irred ? "sim" : "NÃO", subcorpo ? "NÃO" : "sim");
        printf("    %-2d      %-14d   %-16d   %-19s   %s\n",
               k, soma[0], prod[0], (tr_esc&&n_esc)?"sim (em ℤ_p)":"NÃO",
               subcorpo?"sim (parte)":"NÃO — z é completo");
    }

    /* §transitividade: a norma decompõe-se pela torre — N_{k/1}=N_{d/1}∘N_{k/d} (a conservação por  */
    /* níveis, de 1 até k passando pelas dimensões d que dividem k).                                 */
    printf("\n  §  A TORRE — a norma passa pelos níveis intermediários d|k: N_{k/1} = N_{d/1}∘N_{k/d}\n");
    long vtrans=0;
    for(int k=4;k<=K;k++){
        int f[16]; if(!find_f(k,f)) continue;
        int z[32]={0}; z[0]=1; z[1]=1;
        int Nabs[32]={0}; Nabs[0]=1;
        for(int j=0;j<k;j++){ int c[32]; frob_iter(z,j,f,k,c); int t[32]; pmulmod(Nabs,c,f,k,t); memcpy(Nabs,t,k*sizeof(int)); }
        for(int d=2;d<k;d++){ if(k%d) continue;
            int rel[32]={0}; rel[0]=1;                          /* N_{k/d}(z) = ∏_{b} φ^{db}(z)       */
            for(int b=0;b<k/d;b++){ int c[32]; frob_iter(z,d*b,f,k,c); int t[32]; pmulmod(rel,c,f,k,t); memcpy(rel,t,k*sizeof(int)); }
            int via[32]={0}; via[0]=1;                          /* N_{d/1}(rel) = ∏_{a<d} φ^a(rel)    */
            for(int a=0;a<d;a++){ int c[32]; frob_iter(rel,a,f,k,c); int t[32]; pmulmod(via,c,f,k,t); memcpy(via,t,k*sizeof(int)); }
            int eq=1; for(int i=0;i<k;i++) if(via[i]!=Nabs[i]) eq=0;
            if(!eq) vtrans++;
            printf("     k=%d, via a dimensão d=%d: N_{%d/%d} sobe a N_{%d/1} = N absoluta (=%d): %s\n",
                   k, d, k, d, d, Nabs[0], VD(!(eq), "OK"));
        }
    }
    res += (vtrans!=0);

    /* a conexão com o gato/Fermat: em dim 2 (o gato x²−mx−1), Tr(σ)=m e N(σ)=−1 — o metal.          */
    printf("\n  §  A LEI DO GATO GENERALIZADA — Tr e N são os coeficientes do polinômio (funções simétricas):\n");
    printf("     dim 2, o gato x²−mx−1: Tr(σ)=σ+σ'=m (a soma dos 2 conjugados), N(σ)=σσ'=−1 (o produto).\n");
    printf("     em dim k: Tr = a soma dos k conjugados, N = o produto — ambos em ℤ_p, os invariantes de z.\n");
    printf("     nenhuma dimensão anterior comporta z; a soma/produto das k projeções desce ao fundo, conservado.\n");

    printf("\n----------------------------------------------------------------\n");
    printf("     E AS DUAS CONDIÇÕES CONTAM: f irredutível de grau k faz do quociente um\n");
    printf("     CORPO, e z fora de todo subcorpo próprio faz dele um GERADOR. Sem a\n");
    printf("     segunda, os k conjugados repetir-se-iam e o traço medido seria o de\n");
    printf("     OUTRO andar, com o mesmo aspecto no ecrã. (thm:gerador-andar)\n");
    printf("GF(%d^k): Tr=Σ e N=∏ dos k conjugados descem a ℤ_%d (conservados)   resíduo total = %d   %s\n",
           p, p, res, VD(res, "A NORMA DE CORPO CONSERVA — OS k CONJUGADOS SE SOMAM (TRAÇO) E MULTIPLICAM (NORMA) NO FUNDO"));
    return res?1:0;
}
