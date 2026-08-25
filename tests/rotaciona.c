/* DEPENDE-DE: teoria.tex
 * O que este medidor LÊ entra na assinatura da bateria — sem isto, mudar um
 * destes ficheiros não reabre a semente, e o verde é sobre um estado que já
 * não existe. Mesma razão dos headers, um andar acima. */
/* rotaciona.c — ROTACIONA um polinômio pelo GATO, DESROTACIONA pelo ESQUILO. Ida e volta.
 *
 * Um polinômio é um dado: δ = Σ dᵢ σⁱ em Rⁿ = ℤ_p[x]/(x^n − m x^{n−1} − 1). Rotacioná-lo é o gato,
 * ×σ. Trazê-lo de volta é o esquilo. E o esquilo não se constrói — colhe-se da própria borda:
 *
 *      σⁿ = m·σⁿ⁻¹ + 1   ⟹   1 = σ·(σⁿ⁻¹ − m·σⁿ⁻²)   ⟹   σ⁻¹ = σⁿ⁻¹ − m·σⁿ⁻²
 *
 * dois termos, tirados da realimentação. Em n=2: σ⁻¹ = σ − m. Nada de Fermat, nada de Frobenius,
 * nada de eliminação.
 *
 * E há DOIS esquilos, que é a dualidade (§3), e a diferença entre eles é o sinal:
 *
 *   · o esquilo INVERSO, ×σ⁻¹ : gato ∘ esquilo = id — a volta EXATA, o dado de novo.
 *   · o esquilo CONJUGADO, ×σ' (𝒥σ = σ', o outro ponto fixo) : σσ' = −1, logo gato ∘ esquilo = −id.
 *     A ida e a volta devolve o dado com o sinal trocado: a FOLHA DESCOLOU. Duas idas-e-voltas
 *     colam, (σσ')² = +1 — o período 4.
 *
 * Mede-se em dado arbitrário e na prosa crua (bytes de teoria.tex), em toda dimensão: resíduo 0.
 *
 *   cc -O2 -std=c99 rotaciona.c -o rotaciona && ./rotaciona [n]
 */
#include <stdio.h>
#include "unidade.h"
#include <stdlib.h>

#define NMAX 8
static long p = 40009;
static int  m = 1, n = 2;

typedef struct { long c[NMAX]; } P;

static long md(long x){ x %= p; return x<0 ? x+p : x; }
static P pz(void){ P r; for(int i=0;i<NMAX;i++) r.c[i]=0; return r; }
static P p1(void){ P r=pz(); r.c[0]=1; return r; }
static int peq(P a, P b){ for(int i=0;i<n;i++) if(md(a.c[i])!=md(b.c[i])) return 0; return 1; }
static P pneg(P a){ P r=pz(); for(int i=0;i<n;i++) r.c[i]=md(-a.c[i]); return r; }
static int pzero(P a){ for(int i=0;i<n;i++) if(md(a.c[i])) return 0; return 1; }

/* o produto em Rⁿ: as potências excedentes BAIXAM pela borda σ^k = m σ^{k−1} + σ^{k−n} */
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
static P SIG(void){ P r=pz(); if(n>1) r.c[1]=1; else r.c[0]=md(m); return r; }
/* o ESQUILO, colhido da borda: σ⁻¹ = σ^{n−1} − m·σ^{n−2} */
static P SIG_INV(void){
    P r=pz();
    r.c[n-1] = 1;
    if(n>=2) r.c[n-2] = md(r.c[n-2] - m);
    return r;
}
/* o esquilo CONJUGADO (n=2): σ' = m − σ, o outro ponto fixo — 𝒥σ */
static P SIG_CONJ(void){ P r=pz(); r.c[0]=md(m); if(n>1) r.c[1]=md(-1); return r; }

static P gato(P a){ return pmul(a, SIG()); }
static P esquilo(P a){ return pmul(a, SIG_INV()); }
static P esquilo_conj(P a){ return pmul(a, SIG_CONJ()); }

static P arbitrario(long s){
    P r=pz();
    for(int i=0;i<n;i++){ s = s*6364136223846793005L + 1442695040888963407L; r.c[i]=md(s>>33); }
    if(pzero(r)) r.c[0]=1;
    return r;
}
static int primo(long q){ if(q<2)return 0; for(long d=2;d*d<=q;d++) if(q%d==0) return 0; return 1; }
static void mostra(const char *rot, P a){
    printf("%s", rot);
    for(int i=0;i<n;i++) printf("%s%ld", i?" ":"", a.c[i]);
    printf("\n");
}

int main(int argc, char **argv){
    if(argc>1) n = atoi(argv[1]);
    if(n<2 || n>NMAX){ printf("n entre 2 e %d\n", NMAX); return 2; }
    while(!primo(p)) p++;
    int ok = 1;

    printf("ROTACIONA — Rⁿ = ℤ_%ld[x]/(x^%d − %d·x^%d − 1)\n", p, n, m, n-1);
    printf("o gato rotaciona (×σ) ; o esquilo desrotaciona (×σ⁻¹ = σ^%d − %d·σ^%d, da borda)\n",
           n-1, m, n-2);
    printf("=================================================================\n");

    /* §R0 — o esquilo é de fato o inverso, e saiu da borda sem algoritmo */
    {
        P v = pmul(SIG(), SIG_INV());
        printf("§R0  σ ⊛ σ⁻¹ = 1 : %s   (σ⁻¹ colhido da realimentação, sem Fermat)\n",
               peq(v,p1())?"resíduo 0":"FALHA");
        if(!peq(v,p1())) ok=0;
    }

    /* §R1 — um polinômio, rotacionado e desrotacionado: volta o mesmo */
    {
        P A = arbitrario(12345);
        P B = gato(A);
        P V = esquilo(B);
        printf("\n§R1  um exemplo, coordenada a coordenada:\n");
        mostra("     A            = ", A);
        mostra("     B = gato(A)  = ", B);
        mostra("     esquilo(B)   = ", V);
        printf("     esquilo(gato(A)) == A : %s\n", peq(V,A)?"resíduo 0":"FALHA");
        if(!peq(V,A)) ok=0;
    }

    /* §R2 — em massa: dado arbitrário, ida e volta, e a ordem inversa também */
    {
        long tot=0, ida=0, volta=0;
        for(long k=1;k<=20000;k++){
            P A = arbitrario(k*2654435761L);
            tot++;
            if(peq(esquilo(gato(A)), A)) ida++;
            if(peq(gato(esquilo(A)), A)) volta++;
        }
        printf("\n§R2  gato∘esquilo = id em %ld/%ld ; esquilo∘gato = id em %ld/%ld  %s\n",
               ida, tot, volta, tot, VD(!((ida==tot&&volta==tot)), "resíduo 0"));
        if(ida!=tot||volta!=tot) ok=0;
    }

    /* §R3 — k rotações, k desrotações: a órbita fecha para qualquer k */
    {
        int falhas=0;
        for(int k=1;k<=64;k++){
            P A = arbitrario(k*7919+3), X = A;
            for(int i=0;i<k;i++) X = gato(X);
            for(int i=0;i<k;i++) X = esquilo(X);
            if(!peq(X,A)) falhas++;
        }
        printf("§R3  k rotações + k desrotações (k=1..64) : %s\n",
               VD(falhas, "resíduo 0 — a órbita fecha em qualquer k"));
        if(falhas) ok=0;
    }

    /* §R4 — o OUTRO esquilo, o conjugado: σσ' = −1, a folha descola na ida e volta */
    if(n == 2){
        P v = pmul(SIG(), SIG_CONJ());
        long tot=0, neg=0, duplo=0;
        for(long k=1;k<=20000;k++){
            P A = arbitrario(k*40503+11);
            tot++;
            if(peq(esquilo_conj(gato(A)), pneg(A))) neg++;                     /* dá −A            */
            P d = esquilo_conj(gato(esquilo_conj(gato(A))));
            if(peq(d, A)) duplo++;                                             /* duas voltas: +A  */
        }
        printf("\n§R4  o esquilo CONJUGADO (×σ', o dual): σ ⊛ σ' = %ld+%ldσ (=−1)\n",
               v.c[0]==p-1?-1L:v.c[0], v.c[1]);
        printf("     esquilo'(gato(A)) == −A : %ld/%ld  %s   ← a folha DESCOLOU\n", neg, tot,
               neg==tot?"resíduo 0":"FALHA");
        printf("     duas idas-e-voltas == +A : %ld/%ld  %s   ← COLOU (o período 4)\n", duplo, tot,
               duplo==tot?"resíduo 0":"FALHA");
        if(neg!=tot||duplo!=tot) ok=0;
        printf("     ⟹ os dois esquilos diferem por UM SINAL: σ⁻¹ = −σ'. Desrotacionar pelo\n");
        printf("        inverso devolve o dado; pelo conjugado devolve a outra folha.\n");
    }

    /* §R5 — a PROSA crua: rotaciona e desrotaciona bytes de teoria.tex */
    {
        FILE *f = fopen("../teoria.tex","rb");
        if(!f) f = fopen("teoria.tex","rb");
        if(!f) printf("\n§R5  (não achei teoria.tex — pulando a prosa)\n");
        else {
            unsigned char raw[8192];
            size_t lidos = fread(raw,1,sizeof raw,f);
            fclose(f);
            long tot=0, bom=0;
            for(size_t off=0; off+n<=lidos; off+=n){
                P A=pz();
                for(int i=0;i<n;i++) A.c[i]=raw[off+i];
                tot++;
                if(peq(esquilo(gato(A)), A)) bom++;
            }
            printf("\n§R5  a PROSA crua (bytes de teoria.tex), %ld blocos de %d: %ld/%ld  %s\n",
                   tot, n, bom, tot, VD(!((tot&&bom==tot)), "resíduo 0"));
            if(!tot||bom!=tot) ok=0;
        }
    }

    printf("\n-----------------------------------------------------------------\n");
    printf("%s\n", ok ?
      "RESÍDUO 0 — rotacionar é o gato, desrotacionar é o esquilo, e o esquilo saiu da\n"
      "BORDA: σ⁻¹ = σ^{n−1} − m·σ^{n−2}, dois termos da realimentação. A ida e volta é\n"
      "exata em qualquer dimensão, para qualquer dado — inclusive a prosa crua. E o\n"
      "outro esquilo, o conjugado σ', devolve −A: σ⁻¹ = −σ', e a diferença entre os dois\n"
      "duais é exatamente o sinal que descola a folha."
      : "FALHOU — rever");
    return !ok;
}
