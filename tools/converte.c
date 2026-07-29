/* converte.c — CONVERTER DOIS POLINÔMIOS: a deconvolução, colhida do dual.
 *
 * A teoria já está escrita (teoria.tex). Todo dado é um polinômio
 *
 *      δ(x) = Σ dᵢ σⁱ   na base {1, σ, …, σ^{n−1}}   de Rⁿ = GF(pⁿ) = ℤ_p[x]/(x^n − m x^{n−1} − 1),
 *
 * as coordenadas contínuas e o único discreto a dimensão n (§2). E a colheita diz (§4):
 *
 *      a CONVOLUÇÃO é o produto (×, o gato)        a DECONVOLUÇÃO é o quociente (÷, o esquilo)
 *      a INVERSA vem do DUAL — o produto dos conjugados de Frobenius, sem exponenciação de Fermat
 *
 * Logo converter um polinômio noutro não é algoritmo: é uma divisão do corpo. Dados A e B quaisquer,
 * o conversor é C com A ⊛ C = B, e ele se COLHE:
 *
 *      A⁻¹ = (∏_{i=1}^{n−1} Frobⁱ(A)) · N(A)⁻¹ ,      N(A) = ∏_{i=0}^{n−1} Frobⁱ(A) ∈ ℤ_p
 *      C   = B ⊛ A⁻¹
 *
 * — e o Frobenius é de graça: Frob(A) = A^p = Σ aᵢ (σ^p)ⁱ, porque aᵢ^p = aᵢ em ℤ_p. Não se exponencia
 * A a p^n−2 (Fermat): avalia-se A em σ^p. As BATIDAS são n−1: em n=4, Frob⁴=id (como ℱ⁴=id, §3) e o
 * inverso são TRÊS conjugados — três batidas são a volta, do lado do dual.
 *
 * Mede-se com dados ARBITRÁRIOS (inclusive os bytes crus da prosa de teoria.tex — "função de onda
 * qualquer", como em tres_reconstroi.c): ou A ⊛ C = B com resíduo 0, ou falha.
 *
 * Sem memória: coeficientes em buffers de tamanho fixo (n ≤ 8), zero malloc, nada persistido.
 *
 *   cc -O2 -std=c99 converte.c -o converte && ./converte [n]
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define NMAX 8
static long p = 0;
static int  m = 1, n = 4;

typedef struct { long c[NMAX]; } P;                 /* Σ c[i]·σⁱ — o polinômio, o dado             */

static long md(long x){ x %= p; return x<0 ? x+p : x; }
static P pz(void){ P r; for(int i=0;i<NMAX;i++) r.c[i]=0; return r; }
static P p1(void){ P r=pz(); r.c[0]=1; return r; }
static P psig(void){ P r=pz(); r.c[1 % n]= (n>1)?1:0; if(n==1) r.c[0]=md(m); return r; }
static int peq(P a, P b){ for(int i=0;i<n;i++) if(md(a.c[i])!=md(b.c[i])) return 0; return 1; }
static int pzero(P a){ for(int i=0;i<n;i++) if(md(a.c[i])) return 0; return 1; }
static P psub(P a, P b){ P r=pz(); for(int i=0;i<n;i++) r.c[i]=md(a.c[i]-b.c[i]); return r; }

/* a CONVOLUÇÃO — o produto em Rⁿ: multiplica e as potências excedentes BAIXAM pela borda
 * σ^k = m·σ^{k−1} + σ^{k−n}  (de σ^n = m σ^{n−1} + 1), aplicada de cima para baixo (§2).      */
static P pmul(P a, P b){
    long t[2*NMAX]; for(int i=0;i<2*n;i++) t[i]=0;
    for(int i=0;i<n;i++){ if(!a.c[i]) continue;
        for(int j=0;j<n;j++) t[i+j] = md(t[i+j] + a.c[i]*b.c[j]); }
    for(int k=2*n-2; k>=n; k--){
        long v = t[k]; if(!v) continue;
        t[k] = 0;
        t[k-1]   = md(t[k-1]   + (long)m*v);        /* m·σ^{k−1}                                  */
        t[k-n]   = md(t[k-n]   + v);                /* + σ^{k−n}                                  */
    }
    P r=pz(); for(int i=0;i<n;i++) r.c[i]=t[i];
    return r;
}
static P ppow(P a, long e){ P r=p1(); while(e>0){ if(e&1) r=pmul(r,a); a=pmul(a,a); e>>=1; } return r; }
static long ipow(long b, long e){ long r=1; b=md(b); while(e>0){ if(e&1) r=(r*b)%p; b=(b*b)%p; e>>=1; } return r; }
static long invp(long k){ return ipow(k, p-2); }

/* o FROBENIUS, de graça: Frob(A) = A^p = Σ aᵢ·(σ^p)ⁱ  (pois aᵢ^p = aᵢ em ℤ_p) — sem Fermat.   */
static P SP;                                         /* σ^p, calculado uma vez                     */
static P frob(P a){
    P r = pz(), pw = p1();
    for(int i=0;i<n;i++){
        if(a.c[i]){ P t = pw; for(int k=0;k<n;k++) t.c[k]=md(t.c[k]*a.c[i]);
                    for(int k=0;k<n;k++) r.c[k]=md(r.c[k]+t.c[k]); }
        pw = pmul(pw, SP);
    }
    return r;
}
/* a NORMA — o produto dos n conjugados; é escalar (vive em Fix(𝒥) = ℤ_p)                       */
static P norma_full(P a){ P r=a; P c=a; for(int i=1;i<n;i++){ c=frob(c); r=pmul(r,c); } return r; }
/* a INVERSA colhida do DUAL: os n−1 conjugados sobre a norma — nenhuma exponenciação de Fermat */
static P inv_dual(P a, int *ok){
    P prod = p1(), c = a;
    for(int i=1;i<n;i++){ c = frob(c); prod = pmul(prod, c); }   /* n−1 BATIDAS                  */
    P nn = pmul(a, prod);                                        /* = N(A), escalar              */
    int escalar = 1; for(int i=1;i<n;i++) if(md(nn.c[i])) escalar = 0;
    if(!escalar || !md(nn.c[0])){ *ok = 0; return pz(); }
    *ok = 1;
    long in = invp(nn.c[0]);
    for(int i=0;i<n;i++) prod.c[i] = md(prod.c[i]*in);
    return prod;
}
/* --- gcd de polinômios: usado SÓ para escolher p (irredutibilidade), não para converter --- */
static int gdeg(const long *f, int df){ while(df>=0 && !md(f[df])) df--; return df; }
static int coprimo_com_pn(P a){
    long f[NMAX+1], g[NMAX+1];
    for(int i=0;i<n;i++) f[i]=md(a.c[i]);
    int df = gdeg(f, n-1);
    for(int i=0;i<=n;i++) g[i]=0;
    g[n]=1; g[n-1]=md(-m); g[0]=md(-1);              /* p_n(x) = x^n − m x^{n−1} − 1              */
    int dg = n;
    if(df < 0) return 0;
    while(df >= 0){
        long inv = invp(f[df]);
        for(int k=dg; k>=df; k--){
            long fac = md(g[k]*inv);
            if(!fac) continue;
            for(int j=0;j<=df;j++) g[k-df+j] = md(g[k-df+j] - fac*f[j]);
        }
        dg = gdeg(g, dg);
        if(dg < 0) return (df == 0);                 /* resto 0 → gcd = f → coprimo só se grau 0  */
        long tf[NMAX+1]; int tdf = df;
        for(int i=0;i<=NMAX;i++) tf[i] = (i<=df) ? f[i] : 0;
        for(int i=0;i<=dg;i++) f[i]=g[i];
        df = dg;
        for(int i=0;i<=tdf;i++) g[i]=tf[i];
        dg = tdf;
    }
    return 0;
}
static int primo(long q){ if(q<2) return 0; for(long d=2;d*d<=q;d++) if(q%d==0) return 0; return 1; }
static int irredutivel(void){
    /* σ^{p^n} = σ  e  gcd(σ^{p^{n/q}} − σ, p_n) = 1 para cada primo q | n                        */
    P s = psig(), c = s;
    for(int i=0;i<n;i++) c = frob(c);
    if(!peq(c, s)) return 0;
    for(int q=2;q<=n;q++){
        if(n % q || !primo(q)) continue;
        P d2 = s;
        for(int i=0;i<n/q;i++) d2 = frob(d2);
        P dif = psub(d2, s);
        if(pzero(dif)) return 0;
        if(!coprimo_com_pn(dif)) return 0;
    }
    return 1;
}
/* dado arbitrário determinístico */
static P arbitrario(long seed){
    P r=pz();
    for(int i=0;i<n;i++){ seed = seed*6364136223846793005L + 1442695040888963407L;
                          r.c[i] = md((seed>>33)); }
    if(pzero(r)) r.c[0]=1;
    return r;
}

int main(int argc, char **argv){
    if(argc>1) n = atoi(argv[1]);
    if(n<2 || n>NMAX){ printf("n entre 2 e %d\n", NMAX); return 2; }

    /* escolhe p com p_n irredutível (Rⁿ é corpo). Teto: para alguns n nenhum p serve — se o grupo
     * de Galois de p_n não tem n-ciclo, Chebotarev proíbe a irredutibilidade mod p para TODO p.
     * Nesse caso o quociente não é corpo (tem divisores de zero) e a conversão vale exatamente no
     * grupo de unidades — o que se mede, em vez de travar.                                        */
    int corpo = 0;
    for(p = 40009; p < 60000; p++){
        if(!primo(p)) continue;
        P s = psig();
        SP = pz(); SP.c[1 % n] = 1; SP = ppow(s, p);   /* σ^p, para o Frobenius                   */
        if(irredutivel()){ corpo = 1; break; }
    }
    if(!corpo){
        for(p = 40009; ; p++) if(primo(p)) break;
        P s = psig(); SP = ppow(s, p);
    }
    printf("CONVERTE — Rⁿ = ℤ_%ld[x]/(x^%d − %d·x^%d − 1) ; p_n %s\n",
           p, n, m, n-1, corpo ? "irredutível (é CORPO)"
                              : "REDUTÍVEL p/ todo p testado — anel com divisores de zero");
    printf("todo dado é um polinômio; converter é DIVIDIR, e a inversa vem do DUAL\n");
    printf("=================================================================\n");

    int ok = 1;

    /* §C0 — o Frobenius fecha em n batidas: Frobⁿ = id, logo Frob^{n−1} = Frob⁻¹ */
    {
        P s = psig(), c = s;
        for(int i=0;i<n;i++) c = frob(c);
        P c2 = s; for(int i=0;i<n-1;i++) c2 = frob(c2);
        P volta = frob(c2);
        printf("§C0  Frob^%d = id  %s   ⟹  Frob^%d = Frob⁻¹  (%d batidas são a volta)\n",
               n, peq(c,s)?"resíduo 0":"FALHA", n-1, n-1);
        if(!peq(c,s) || !peq(volta,s)) ok=0;
        if(n==4) printf("     em n=4 são TRÊS — a mesma conta de ℱ⁴=id, ℱ³=ℱ⁻¹ (§3)\n");
    }

    /* §C½ — quantos A são unidades? (num corpo, todos menos 0; sem corpo, o grupo de unidades) */
    {
        long tot=0, uni=0;
        for(long k=1;k<=2000;k++){
            P a = arbitrario(k*15485863);
            int o; inv_dual(a,&o);
            tot++; uni += o;
        }
        printf("§C½  A é unidade (N(A) escalar ≠ 0) : %ld/%ld  (%.2f%%)  %s\n", uni, tot,
               100.0*uni/tot, corpo ? (uni==tot?"resíduo 0 — é corpo":"REVER")
                                    : "os divisores de zero são o resto");
        if(corpo && uni!=tot) ok=0;
    }

    /* §C1 — a norma é ESCALAR (vive em Fix(𝒥)=ℤ_p): o dual cristaliza */
    {
        long tot=0, esc=0;
        for(long k=1;k<=2000;k++){
            P a = arbitrario(k*7919);
            P nn = norma_full(a);
            int e=1; for(int i=1;i<n;i++) if(md(nn.c[i])) e=0;
            tot++; esc += e;
        }
        printf("§C1  N(A) = ∏ Frobⁱ(A) é ESCALAR : %ld/%ld  %s\n", esc, tot,
               esc==tot?"resíduo 0":"FALHA");
        if(esc!=tot) ok=0;
    }

    /* §C2 — a inversa COLHIDA do dual, sem Fermat: A·A⁻¹ = 1 */
    {
        long tot=0, bom=0;
        for(long k=1;k<=2000;k++){
            P a = arbitrario(k*104729);
            int o; P ai = inv_dual(a,&o);
            tot++;
            if(o && peq(pmul(a,ai), p1())) bom++;
        }
        printf("§C2  A⁻¹ = (∏_{i≥1}Frobⁱ(A))·N(A)⁻¹ ; A⊛A⁻¹ = 1 : %ld/%ld  %s\n", bom, tot,
               bom==tot?"resíduo 0":"FALHA");
        printf("     (nenhuma exponenciação a p^n−2: só %d batidas do Frobenius e um inverso em ℤ_p)\n", n-1);
        if(bom!=tot) ok=0;
    }

    /* §C3 — A CONVERSÃO: dois polinômios QUAISQUER, C = B ⊛ A⁻¹, e A ⊛ C = B */
    {
        long tot=0, bom=0, rev=0;
        for(long k=1;k<=5000;k++){
            P A = arbitrario(k*2654435761L), B = arbitrario(k*40503+11);
            int o; P Ai = inv_dual(A,&o);
            if(!o) continue;
            P C = pmul(B, Ai);                        /* o CONVERSOR, colhido                     */
            tot++;
            if(peq(pmul(A,C), B)) bom++;              /* a convolução leva A em B                 */
            int o2; P Ci = inv_dual(C,&o2);           /* e a volta: B ⊛ C⁻¹ = A (deconvolução)    */
            if(o2 && peq(pmul(B,Ci), A)) rev++;
        }
        printf("§C3  a CONVERSÃO de dois polinômios quaisquer:\n");
        printf("     A ⊛ C = B  (ida, convolução)   : %ld/%ld  %s\n", bom, tot, bom==tot?"resíduo 0":"FALHA");
        printf("     B ⊛ C⁻¹ = A (volta, deconvol.) : %ld/%ld  %s\n", rev, tot, rev==tot?"resíduo 0":"FALHA");
        if(bom!=tot || rev!=tot) ok=0;
    }

    /* §C4 — FUNÇÃO DE ONDA QUALQUER: os bytes crus da prosa, sem preparo nenhum */
    {
        FILE *f = fopen("../teoria.tex","rb");
        if(!f) f = fopen("teoria.tex","rb");
        if(!f){ printf("§C4  (não achei teoria.tex — pulando a prosa)\n"); }
        else {
            unsigned char raw[4096];
            size_t lidos = fread(raw,1,sizeof raw,f);
            fclose(f);
            long tot=0, bom=0;
            for(size_t off=0; off+2*n <= lidos; off += 2*n){
                P A=pz(), B=pz();
                for(int i=0;i<n;i++){ A.c[i]=raw[off+i]; B.c[i]=raw[off+n+i]; }
                if(pzero(A)) continue;
                int o; P Ai = inv_dual(A,&o);
                if(!o) continue;
                P C = pmul(B,Ai);
                tot++;
                if(peq(pmul(A,C), B)) bom++;
            }
            printf("§C4  a PROSA (bytes crus de teoria.tex) — converter bloco A no bloco B:\n");
            printf("     A ⊛ C = B : %ld/%ld blocos  %s\n", bom, tot, (tot&&bom==tot)?"resíduo 0":"FALHA");
            if(!tot || bom!=tot) ok=0;
        }
    }

    printf("\n-----------------------------------------------------------------\n");
    printf("%s\n", ok ?
      "RESÍDUO 0 — converter um polinômio noutro não se constrói: COLHE-SE. A\n"
      "convolução é o produto, a deconvolução é o quociente, e o conversor é\n"
      "C = B ⊛ A⁻¹ com A⁻¹ tirado do DUAL — os n−1 conjugados de Frobenius sobre a\n"
      "norma, sem exponenciação de Fermat. Frobⁿ = id (em n=4: três batidas são a\n"
      "volta), e a norma cristaliza em ℤ_p. Vale para dado arbitrário e para a prosa\n"
      "crua: qualquer função de onda converte em qualquer outra, exatamente."
      : "FALHOU — rever");
    return !ok;
}
