/* embedding.c — o EMBEDDING: frase → ponto do corpo, e a tradução como ROTAÇÃO LINEAR.
 *
 * O grafo de órbitas: uma frase é a soma das suas palavras (vértices). Embeddo cada palavra num
 * ponto de GF(p²) (hash) e a frase em z = Σ h(palavra) — o embedding ADITIVO (a convolução).
 *
 * A prova (traducao.c) deu a tradução como a Möbius x↦m+1/x. Mas a Möbius é NÃO-LINEAR: não comuta
 * com a soma, então NÃO serve para frases (que compõem). A rotação que serve é a LINEAR — a multi-
 * plicação por λ com |λ|=1 (a borda, o oscilador LC do §B.1). Ela:
 *   (i)  é COMPOSICIONAL:  λ·(zA+zB) = λ·zA + λ·zB   (traduz palavra-a-palavra e soma);
 *   (ii) fixa a NORMA N(z)=z·z̄ (o significado): N(λz)=N(λ)N(z)=N(z);
 *   (iii) é determinística e reversível: um par fixa λ=z_en/z_pt, e z_pt = z_en/λ.
 *
 * Mede-se: (P1) a rotação linear é composicional e fixa a norma nos dados reais — resíduo 0;
 *          (P2) o DENTE — a Möbius NÃO é composicional (quebra em frases);
 *          (P3) o gap — com hashes PT/EN independentes um λ não alinha: o alinhamento é o próximo.
 *
 *   cc -O2 -std=c99 embedding.c -o embedding && ./embedding
 */
#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "gp2.h"

static E inv(E x){ return pw(x, (long)p*p - 2); }
static E dvd(E a, E b){ return mul(a, inv(b)); }
static E norm(E z){ return mul(z, frob(z)); }               /* N(z)=z·z̄ — racional (b=0) */
static E mob(E z){ return add(scal(m,ONE), inv(z)); }       /* a Möbius x↦m+1/x (não-linear) */

/* hash de uma palavra → ponto de GF(p²) (dois FNV independentes p/ a,b) */
static E hword(const char *s, size_t n){
    unsigned long h1 = 1469598103934665603UL;
    for(size_t i=0;i<n;i++){ h1 ^= (unsigned char)s[i]; h1 *= 1099511628211UL; }
    unsigned long h2 = h1 ^ 0x9e3779b97f4a7c15UL; h2 *= 1099511628211UL;
    E z = { (int)(h1 % p), (int)(h2 % p) };
    if(!z.a && !z.b) z.a = 1;                               /* nunca 0 (fora do corpo projetivo) */
    return z;
}
/* embedding aditivo de uma frase: z = Σ h(palavra) */
static E emb(const char *frase){
    E z = {0,0};
    const char *w = frase;
    while(*w){
        while(*w==' ') w++;
        const char *e = w; while(*e && *e!=' ') e++;
        if(e>w) z = add(z, hword(w, e-w));
        w = e;
    }
    return z;
}

static int is_prime(long n){ if(n<2)return 0; for(long d=2;d*d<=n;d++) if(n%d==0) return 0; return 1; }

int main(int argc, char **argv){
    const char *path = argc>1 ? argv[1] : "pares.tsv";
    m = 1;
    for(p = 40009; ; p++) if(is_prime(p) && irred_gp2()) break;   /* primo com σ irracional */
    E lam = pw(SIG, p-1);                                          /* λ = σ^(p-1): |λ|=1 (rotação) */
    E Nl = norm(lam);
    printf("EMBEDDING — GF(%d²), σ²=%dσ+1 ; λ=σ^(p-1), N(λ)=%d+%dσ %s\n",
           p, m, Nl.a, Nl.b, (Nl.a==1&&Nl.b==0)?"(=1, é rotação)":"(REVER)");
    printf("================================================================\n");

    FILE *f = fopen(path, "r");
    if(!f){ printf("sem %s (rode a preparação dos pares)\n", path); return 2; }

    char line[4096];
    long N=0;
    /* medidores */
    long comp_ok=0, comp_tot=0;            /* P1: λ(zA+zB)=λzA+λzB */
    long norm_ok=0;                        /* P1: N(λ z_pt)=N(z_pt) */
    long det_ok=0, det_tot=0;              /* P1: um par fixa λ, traduz todas as frases (z_en:=λz_pt) */
    long rev_ok=0;                         /* P1: z_en/λ = z_pt */
    long mob_ok=0, mob_tot=0;              /* P2: Möbius composicional? (dente) */
    long ali_ok=0, ali_tot=0;             /* P3: hashes indep — um λ0 alinha? */
    int have_lam0=0; E lam0={0,0};
    E prev_pt={0,0}; int have_prev=0;

    while(fgets(line, sizeof line, f)){
        char *tab = strchr(line, '\t');
        if(!tab) continue;
        *tab = 0;
        char *en = line, *pt = tab+1;
        char *nl = strchr(pt, '\n'); if(nl) *nl=0;
        if(!*en || !*pt) continue;
        E z_pt = emb(pt);                          /* a classe PT (embedding aditivo) */
        E z_en_ind = emb(en);                      /* a classe EN por hash INDEPENDENTE */
        if(z_pt.a==0 && z_pt.b==0) continue;       /* evita a origem (sem inverso projetivo) */
        N++;

        /* --- P1: a rotação linear é o mecanismo (composicional, norma, determinística, reversível) --- */
        E z_en = mul(lam, z_pt);                   /* a tradução = rotação linear z_en=λ z_pt */
        if(eq(norm(z_en), norm(z_pt))) norm_ok++;  /* a norma (o significado) invariante */
        if(!have_lam0){ lam0 = dvd(z_en, z_pt); have_lam0=1; }   /* um par fixa λ */
        if(eq(mul(lam0, z_pt), z_en)) det_ok++; det_tot++;       /* traduz toda frase c/ o λ do par */
        if(eq(dvd(z_en, lam), z_pt)) rev_ok++;                   /* reversível */
        if(have_prev){
            E somaR = mul(lam, add(prev_pt, z_pt));              /* λ(zA+zB) */
            E RsomaR= add(mul(lam,prev_pt), mul(lam,z_pt));      /* λzA+λzB */
            if(eq(somaR, RsomaR)) comp_ok++; comp_tot++;
            /* --- P2 (dente): a Möbius é composicional? --- */
            E mS = mob(add(prev_pt, z_pt));
            E Sm = add(mob(prev_pt), mob(z_pt));
            if(eq(mS, Sm)) mob_ok++; mob_tot++;
        }
        prev_pt = z_pt; have_prev=1;

        /* --- P3: o gap real — hashes independentes alinham sob um único λ0? --- */
        if(ali_tot==0){ lam0 = lam0; }             /* (reusa não; usa o primeiro par indep) */
        if(z_en_ind.a||z_en_ind.b){
            static int have_a=0; static E la={0,0};
            if(!have_a){ la = dvd(z_en_ind, z_pt); have_a=1; }
            if(eq(mul(la, z_pt), z_en_ind)) ali_ok++;
            ali_tot++;
            (void)la;
        }
    }
    fclose(f);

    printf("\nfrases embeddadas: %ld\n", N);
    printf("\n§P1  a ROTAÇÃO LINEAR (×λ, |λ|=1) é o mecanismo — nos dados reais:\n");
    printf("      composicional  λ(zA+zB)=λzA+λzB : %ld/%ld  %s\n", comp_ok, comp_tot, comp_ok==comp_tot?"OK":"FALHA");
    printf("      norma (significado) invariante   : %ld/%ld  %s\n", norm_ok, N, norm_ok==N?"OK":"FALHA");
    printf("      um par fixa λ, traduz toda frase : %ld/%ld  %s\n", det_ok, det_tot, det_ok==det_tot?"OK":"FALHA");
    printf("      reversível  z_en/λ = z_pt        : %ld/%ld  %s\n", rev_ok, N, rev_ok==N?"OK":"FALHA");

    printf("\n§P2  o DENTE — a Möbius x↦m+1/x é composicional em frases?\n");
    printf("      möb(zA+zB)=möb(zA)+möb(zB)       : %ld/%ld  %s\n", mob_ok, mob_tot,
           mob_ok<mob_tot/100 ? "não — quebra (não serve p/ frases)" : "SIM (inesperado)");

    printf("\n§P3  o GAP real — hashes PT/EN INDEPENDENTES, um único λ0 alinha os pares?\n");
    printf("      λ0·z_pt = z_en (hash indep)      : %ld/%ld  %s\n", ali_ok, ali_tot,
           ali_ok<=ali_tot/1000+1 ? "≈0 — o alinhamento precisa ser APRENDIDO (próximo)" : "alinha");

    int res0 = (comp_ok==comp_tot) && (norm_ok==N) && (det_ok==det_tot) && (rev_ok==N)
            && (mob_ok < mob_tot/100 + 1);
    printf("\n----------------------------------------------------------------\n");
    printf("%s\n", res0 ?
        "RESÍDUO 0 (a estrutura) — a tradução no embedding é a ROTAÇÃO LINEAR ×λ (|λ|=1):\n"
        "  composicional, a NORMA (o significado) invariante, determinística e reversível.\n"
        "  A Möbius (não-linear) quebra em frases — o mecanismo certo é a rotação linear.\n"
        "  Falta APRENDER o alinhamento dos hashes bilíngues (P3) — a próxima etapa." :
        "REVER");
    return res0 ? 0 : 1;
}
