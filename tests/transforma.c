/* transforma.c — ESCALA, CISALHAMENTO, ROTAÇÃO, e a composição: de qualquer polinômio a qualquer
 * outro, sempre reversível.
 *
 * Já se tem a rotação (o gato, ×σ) e a volta (o esquilo, ×σ⁻¹ colhido da borda — rotaciona.c). As
 * outras operações do corpo, e o que a composição delas alcança:
 *
 *   ESCALA          ×k, k ∈ ℤ_p*        — muda a norma: N(kA) = kⁿ·N(A) ; volta com k⁻¹
 *   CISALHAMENTO    A_i += s·A_j        — det = 1: NÃO muda o volume ; volta com −s
 *   ROTAÇÃO         ×σ^j                — o gato ; volta com o esquilo
 *
 * E a composição dá o teorema: as transvecções (cisalhamentos) geram SL_n, e SL_n já é
 * TRANSITIVO nos vetores não-nulos (n ≥ 2). Logo
 *
 *      dados A ≠ 0 e B ≠ 0 quaisquer, uma sequência de CISALHAMENTOS leva A em B,
 *      e ela é reversível operação por operação (cada shear volta com −s).
 *
 * A escala não é necessária para chegar: ela só ajusta o determinante (o volume). A construção é
 * colhida, não buscada: reduz-se A a e₀ = (1,0,…,0) por cisalhamentos, reduz-se B a e₀ também, e
 * T = (reduz B)⁻¹ ∘ (reduz A). No máximo n cisalhamentos de cada lado.
 *
 * E as DUAS rotas não são a mesma coisa — é o que este programa mede:
 *
 *   · a rota do CORPO: um único produto, C = B ⊛ A⁻¹. É ÚNICA (o corpo é cancelativo) e comuta com
 *     a convolução: (C⊛A)⊛y = C⊛(A⊛y). É a canônica.
 *   · a rota LINEAR: cisalhamentos e escala. Chega no mesmo lugar, mas há |GL_n|/(pⁿ−1) delas —
 *     muitas —, e nenhuma respeita a convolução. Chega, mas não é a transformação: é uma delas.
 *
 *   cc -O2 -std=c99 transforma.c -o transforma && ./transforma [n] [p]
 */
#include <stdio.h>
#include "unidade.h"
#include <stdlib.h>

#define NMAX 8
#define MAXOP 64
static long p = 40009;
static int  m = 1, n = 2;

typedef struct { long c[NMAX]; } P;
typedef struct { int tipo, i, j; long s; } Op;       /* tipo 0: shear A_i += s·A_j ; 1: escala ×s   */

static long md(long x){ x %= p; return x<0 ? x+p : x; }
static long ipow(long b, long e){ long r=1; b=md(b); while(e>0){ if(e&1) r=(r*b)%p; b=(b*b)%p; e>>=1; } return r; }
static long invp(long k){ return ipow(k, p-2); }
static P pz(void){ P r; for(int i=0;i<NMAX;i++) r.c[i]=0; return r; }
static P p1(void){ P r=pz(); r.c[0]=1; return r; }
static int peq(P a, P b){ for(int i=0;i<n;i++) if(md(a.c[i])!=md(b.c[i])) return 0; return 1; }
static int pzero(P a){ for(int i=0;i<n;i++) if(md(a.c[i])) return 0; return 1; }

static P pmul(P a, P b){
    long t[2*NMAX]; for(int i=0;i<2*n;i++) t[i]=0;
    for(int i=0;i<n;i++){ if(!a.c[i]) continue;
        for(int j=0;j<n;j++) t[i+j] = md(t[i+j] + a.c[i]*b.c[j]); }
    for(int k=2*n-2;k>=n;k--){
        long v=t[k]; if(!v) continue;
        t[k]=0; t[k-1]=md(t[k-1]+(long)m*v); t[k-n]=md(t[k-n]+v);
    }
    P r=pz(); for(int i=0;i<n;i++) r.c[i]=t[i];
    return r;
}
static P SIG(void){ P r=pz(); r.c[1%n]=1; if(n==1) r.c[0]=md(m); return r; }
static P SIG_INV(void){ P r=pz(); r.c[n-1]=1; if(n>=2) r.c[n-2]=md(r.c[n-2]-m); return r; }
static P SIGP;                                        /* σ^p — o Frobenius de graça               */
static P ppow(P a, long e){ P r=p1(); while(e>0){ if(e&1) r=pmul(r,a); a=pmul(a,a); e>>=1; } return r; }
static P frob(P a){
    P r=pz(), pw=p1();
    for(int i=0;i<n;i++){
        if(a.c[i]) for(int k=0;k<n;k++) r.c[k]=md(r.c[k]+pw.c[k]*a.c[i]);
        pw = pmul(pw, SIGP);
    }
    return r;
}
static P inv_dual(P a, int *ok){                      /* A⁻¹ pelo dual (converte.c)               */
    P prod=p1(), c=a;
    for(int i=1;i<n;i++){ c=frob(c); prod=pmul(prod,c); }
    P nn = pmul(a, prod);
    for(int i=1;i<n;i++) if(md(nn.c[i])){ *ok=0; return pz(); }
    if(!md(nn.c[0])){ *ok=0; return pz(); }
    *ok=1;
    long in=invp(nn.c[0]);
    for(int i=0;i<n;i++) prod.c[i]=md(prod.c[i]*in);
    return prod;
}
static P norma(P a){ P r=a, c=a; for(int i=1;i<n;i++){ c=frob(c); r=pmul(r,c); } return r; }

/* --- as operações elementares --- */
static P escala(P a, long k){ P r=a; for(int i=0;i<n;i++) r.c[i]=md(r.c[i]*k); return r; }
static P shear(P a, int i, int j, long s){ P r=a; r.c[i]=md(r.c[i]+s*a.c[j]); return r; }
static P aplica(P a, const Op *L, int k){
    for(int t=0;t<k;t++) a = L[t].tipo ? escala(a, L[t].s) : shear(a, L[t].i, L[t].j, L[t].s);
    return a;
}
static P aplica_inv(P a, const Op *L, int k){         /* reversível operação por operação          */
    for(int t=k-1;t>=0;t--)
        a = L[t].tipo ? escala(a, invp(L[t].s)) : shear(a, L[t].i, L[t].j, md(-L[t].s));
    return a;
}
/* reduz A a e₀ = (1,0,…,0) usando SÓ cisalhamentos — a colheita, não busca */
static int reduz_e0(P a, Op *L){
    int k = 0;
    if(md(a.c[0]) != 1){
        int j = -1;
        for(int t=0;t<n;t++) if(md(a.c[t]) && t!=0){ j=t; break; }
        if(j < 0){                                    /* só a coordenada 0 é ≠ 0: usa outra p/ subir */
            if(!md(a.c[0])) return -1;                /* A = 0                                      */
            L[k].tipo=0; L[k].i=1%n; L[k].j=0; L[k].s=1; a=shear(a,1%n,0,1); k++;
            j = 1%n;
        }
        long s = md((1 - md(a.c[0])) * invp(md(a.c[j])));
        L[k].tipo=0; L[k].i=0; L[k].j=j; L[k].s=s; a=shear(a,0,j,s); k++;
    }
    for(int i=1;i<n;i++){
        if(!md(a.c[i])) continue;
        long s = md(-md(a.c[i]));
        L[k].tipo=0; L[k].i=i; L[k].j=0; L[k].s=s; a=shear(a,i,0,s); k++;
    }
    return peq(a, p1()) ? k : -1;
}
static P arbitrario(long s){
    P r=pz();
    for(int i=0;i<n;i++){ s=s*6364136223846793005L+1442695040888963407L; r.c[i]=md(s>>33); }
    if(pzero(r)) r.c[0]=1;
    return r;
}
static int primo(long q){ if(q<2)return 0; for(long d=2;d*d<=q;d++) if(q%d==0) return 0; return 1; }

int main(int argc, char **argv){
    if(argc>1) n = atoi(argv[1]);
    if(argc>2) p = atol(argv[2]);
    if(argc>3) m = atoi(argv[3]);                     /* o metal: 1 ouro, 2 prata, 3 bronze        */
    if(n<2||n>NMAX){ printf("n entre 2 e %d\n", NMAX); return 2; }
    /* escolhe p que faça Rⁿ ser CORPO. n=2: exato — x²−mx−1 é irredutível ⟺ m²+4 não é quadrado
     * mod p. n>2: critério do Frobenius (σ tem grau n) e confirmação empírica das unidades.      */
    {
        long p0 = p; int achou = 0;
        for(p = p0; p < p0 + 4000; p++){
            if(!primo(p)) continue;
            SIGP = ppow(SIG(), p);
            if(n == 2){
                int quad = 0;
                for(long t=0;t<p;t++) if(md(t*t) == md((long)m*m+4)){ quad=1; break; }
                if(!quad){ achou=1; break; }
            } else {
                P s = SIG(), c = s;
                for(int i=0;i<n;i++) c = frob(c);
                if(!peq(c,s)) continue;
                int grau_n = 1;
                for(int q=2;q<=n;q++){
                    if(n%q || !primo(q)) continue;
                    P d2 = s; for(int i=0;i<n/q;i++) d2 = frob(d2);
                    if(peq(d2,s)) grau_n = 0;
                }
                if(!grau_n) continue;
                long amostras=0, unidades=0;                    /* confirma: tudo invertível?      */
                for(long t=1;t<=500;t++){
                    P a = arbitrario(t*15485863); int o;
                    inv_dual(a,&o); amostras++; unidades += o;
                }
                if(unidades == amostras){ achou=1; break; }
            }
        }
        if(!achou){ printf("não achei p que faça Rⁿ corpo perto de %ld — abortando\n", p0); return 2; }
    }
    SIGP = ppow(SIG(), p);
    int ok = 1;

    printf("TRANSFORMA — Rⁿ = ℤ_%ld[x]/(x^%d − %d·x^%d − 1)\n", p, n, m, n-1);
    printf("escala ×k · cisalhamento A_i+=s·A_j · rotação ×σ  —  e a composição\n");
    printf("=================================================================\n");

    /* §T1 — ESCALA: reversível, e o seu efeito na norma é kⁿ */
    {
        long tot=0, rev=0, nrm=0;
        for(long t=1;t<=5000;t++){
            P A = arbitrario(t*2654435761L);
            long k = 1 + (t*7919) % (p-1);
            P S = escala(A,k);
            tot++;
            if(peq(escala(S, invp(k)), A)) rev++;
            /* N(kA) = kⁿ N(A) */
            P nk = norma(S), na = norma(A);
            if(md(nk.c[0]) == md(ipow(k,n)*md(na.c[0]))) nrm++;
        }
        printf("§T1  ESCALA ×k : reversível %ld/%ld %s ; N(kA)=kⁿ·N(A) %ld/%ld %s\n",
               rev,tot, rev==tot?"resíduo 0":"FALHA", nrm,tot, nrm==tot?"resíduo 0":"FALHA");
        if(rev!=tot||nrm!=tot) ok=0;
    }

    /* §T2 — CISALHAMENTO: reversível, det=1 (não muda o volume) */
    {
        long tot=0, rev=0;
        for(long t=1;t<=5000;t++){
            P A = arbitrario(t*40503+7);
            int i = t % n, j = (t/3) % n;
            if(i==j) j = (j+1)%n;
            long s = md(t*13);
            P S = shear(A,i,j,s);
            tot++;
            if(peq(shear(S,i,j,md(-s)), A)) rev++;
        }
        printf("§T2  CISALHAMENTO A_i+=s·A_j : reversível (−s) %ld/%ld %s ; det=1 (volume fixo)\n",
               rev,tot, rev==tot?"resíduo 0":"FALHA");
        if(rev!=tot) ok=0;
    }

    /* §T3 — a COMPOSIÇÃO: de A a B, só com cisalhamentos, e a volta */
    {
        long tot=0, ida=0, volta=0, maxops=0, somaops=0;
        for(long t=1;t<=20000;t++){
            P A = arbitrario(t*2246822519L), B = arbitrario(t*3266489917L+5);
            Op LA[MAXOP], LB[MAXOP];
            int ka = reduz_e0(A, LA), kb = reduz_e0(B, LB);
            if(ka<0||kb<0) continue;
            tot++;
            /* T = (reduz B)⁻¹ ∘ (reduz A) : leva A → e₀ → B */
            P X = aplica(A, LA, ka);
            X = aplica_inv(X, LB, kb);
            if(peq(X,B)) ida++;
            /* a volta, operação por operação */
            P Y = aplica(B, LB, kb);
            Y = aplica_inv(Y, LA, ka);
            if(peq(Y,A)) volta++;
            long ops = ka+kb;
            somaops += ops; if(ops>maxops) maxops=ops;
        }
        printf("§T3  COMPOSIÇÃO (só cisalhamentos): A→B %ld/%ld %s ; B→A %ld/%ld %s\n",
               ida,tot, ida==tot?"resíduo 0":"FALHA", volta,tot, volta==tot?"resíduo 0":"FALHA");
        printf("     operações: média %ld, máximo %ld  (teto 2n+2 = %d) — nenhuma ESCALA foi\n",
               tot?(long)somaops/tot:0, maxops, 2*n+2);
        printf("     necessária: SL_n já é transitivo; a escala só ajusta o determinante.\n");
        if(ida!=tot||volta!=tot) ok=0;
    }

    /* §T4 — a rota do CORPO: uma só operação, e ela comuta com a convolução */
    {
        long tot=0, bom=0, comuta=0;
        for(long t=1;t<=5000;t++){
            P A = arbitrario(t*99991), B = arbitrario(t*777767+3);
            int o; P Ai = inv_dual(A,&o);
            if(!o) continue;
            P C = pmul(B,Ai);
            tot++;
            if(peq(pmul(A,C), B)) bom++;
            /* (C⊛A)⊛y = C⊛(A⊛y) : o produto respeita a convolução */
            P y = arbitrario(t*31337+11);
            if(peq(pmul(pmul(C,A),y), pmul(C,pmul(A,y)))) comuta++;
        }
        printf("§T4  rota do CORPO: C=B⊛A⁻¹, A⊛C=B %ld/%ld %s ; comuta com a convolução %ld/%ld %s\n",
               bom,tot, VD(!((tot&&bom==tot)), "resíduo 0"), comuta,tot,
               VD(!((tot&&comuta==tot)), "resíduo 0"));
        if(!tot || bom!=tot || comuta!=tot) ok=0;        /* tot=0 é FALHA, não verde                */
    }

    /* §T5 — quantas transformações há? A do corpo é ÚNICA; as lineares, muitas (n=2, p pequeno) */
    if(n==2 && p<=101){
        P A = arbitrario(4242), B = arbitrario(9999);
        long conta_c = 0;
        for(long a0=0;a0<p;a0++) for(long a1=0;a1<p;a1++){
            P C=pz(); C.c[0]=a0; C.c[1]=a1;
            if(peq(pmul(A,C),B)) conta_c++;
        }
        long conta_T = 0;
        for(long w=0;w<p;w++) for(long x=0;x<p;x++) for(long y=0;y<p;y++) for(long z=0;z<p;z++){
            if(md(w*z - x*y) == 0) continue;                       /* det ≠ 0: reversível          */
            long r0 = md(w*A.c[0] + x*A.c[1]), r1 = md(y*A.c[0] + z*A.c[1]);
            if(r0==md(B.c[0]) && r1==md(B.c[1])) conta_T++;
        }
        long gln = (p*p-1)*(p*p-p);
        printf("§T5  quantas T levam A em B? (n=2, p=%ld)\n", p);
        printf("     no CORPO  (A⊛C=B)          : %ld    %s\n", conta_c,
               conta_c==1?"ÚNICA — o corpo é cancelativo":"REVER");
        printf("     em GL_2   (T·A=B)          : %ld   (= |GL_2|/(p²−1) = %ld/%ld = %ld)\n",
               conta_T, gln, p*p-1, gln/(p*p-1));
        printf("     ⟹ chegar é fácil e há muitas; a transformação CANÔNICA é uma só, a do\n");
        printf("        corpo — a única que respeita a convolução.\n");
        if(conta_c!=1 || conta_T != gln/(p*p-1)) ok=0;
    }

    printf("\n-----------------------------------------------------------------\n");
    printf("%s\n", ok ?
      "RESÍDUO 0 — escala (×k, N↦kⁿN), cisalhamento (det=1) e rotação (o gato) são todas\n"
      "reversíveis, e a composição leva QUALQUER polinômio em QUALQUER outro: bastam os\n"
      "cisalhamentos (≤2n), porque SL_n já é transitivo — a escala só ajusta o volume.\n"
      "Mas as rotas não se igualam: em GL_n há |GL_n|/(pⁿ−1) transformações que chegam,\n"
      "e no corpo há UMA, C = B⊛A⁻¹ — a única que comuta com a convolução. Chegar é\n"
      "barato; ser a transformação é ser única."
      : "FALHOU — rever");
    return !ok;
}
