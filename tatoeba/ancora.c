/* ancora.c — o EMBEDDING ANCORADO: o inglês não é sorteado, é COLHIDO do português.
 *
 * O gap do §P3 (embedding.c) era artificial: dois hashes independentes não têm por que cair na
 * mesma rotação. Aqui o EN não tem hash nenhum. Só o PT tem ponto próprio (z_pt = Σ h(w)); o EN
 * nasce da própria exigência da tradução ser a rotação linear ×λ:
 *
 *        Σ_{w ∈ frase EN}  g(w)  =  λ · z_pt        (uma equação por par do corpus)
 *
 * Cada palavra inglesa é uma incógnita do corpo GF(p²); cada par de frases é uma equação. Não se
 * resolve o sistema (isso seria matriz, seria memória): ele se COLHE. Quando numa frase falta
 * exatamente UMA palavra, ela cai exata —
 *
 *        k·g(w) = λ z_pt − Σ(as já colhidas)   ⟹   g(w) = (λ z_pt − S) · k⁻¹
 *
 * — e a próxima frase que a contém tem uma incógnita menos. A colheita propaga sozinha, em
 * passadas, até nada mais cair. É a queda do §2 aplicada ao léxico: a palavra cai quando só ela
 * falta. Aritmética exata, sem aproximação, sem métrica, sem parâmetro inventado.
 *
 * SEM MEMÓRIA (as duas faces):
 *   · zero malloc/calloc, zero array de dados, estado em RAM O(1) (algumas variáveis + a linha).
 *   · o léxico colhido é a TOPOLOGIA — no circuito cada palavra é um nó, e um nó não é RAM: é
 *     fiação. Aqui ele vive num arquivo (`lexico_en.bin`), lido e escrito slot a slot por
 *     pread/pwrite. O programa nunca tem o léxico dentro de si.
 *
 * A MEDIDA (honesta, não tautológica): a colheita usa só as linhas ÍMPARES do corpus; as PARES
 * nunca ancoram nada e são o teste. Numa frase de teste inteiramente coberta, ou Σ g(w_en) é
 * exatamente λ z_pt — resíduo 0 — ou não é, e então o veredito é o dente: a soma-de-palavras não
 * admite uma rotação global, e a fração diz o quanto.
 *
 *   cc -O2 -std=c99 -I../tools ancora.c -o ancora && ./ancora pares.tsv
 */
#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <unistd.h>
#include <fcntl.h>
#include "gp2.h"

#define NSLOT   (1L<<19)          /* slots do léxico em disco (folga p/ ~10⁵ palavras)            */
#define MAXPASS 8                 /* passadas de colheita (para antes, quando nada mais cai)      */
#define LEXFILE "lexico_en.bin"

typedef struct { uint64_t fp; int32_t a, b; } Slot;   /* 16 bytes: a impressão da palavra + o ponto */

static int fdlex;

/* a impressão digital da palavra (nunca 0 — 0 é o slot vago) */
static uint64_t marca(const char *s, size_t n){
    uint64_t h = 1469598103934665603UL;
    for(size_t i=0;i<n;i++){ h ^= (unsigned char)s[i]; h *= 1099511628211UL; }
    return h ? h : 1;
}
/* o hash do PORTUGUÊS — a única língua com ponto próprio (o EN é colhido) */
static E hpt(const char *s, size_t n){
    uint64_t h1 = marca(s,n), h2 = (h1 ^ 0x9e3779b97f4a7c15UL) * 1099511628211UL;
    E z = { (int)(h1 % (uint64_t)p), (int)(h2 % (uint64_t)p) };
    if(!z.a && !z.b) z.a = 1;
    return z;
}
/* o léxico em disco: acha a palavra (1) ou o slot vago onde ela cabe (0); -1 se lotou */
static int busca(uint64_t fp, E *g, long *slot){
    long i = (long)(fp % (uint64_t)NSLOT);
    for(long t=0; t<NSLOT; t++){
        Slot s;
        if(pread(fdlex, &s, sizeof s, i*(long)sizeof(Slot)) != (long)sizeof(Slot)) return -1;
        if(s.fp == 0){ *slot = i; return 0; }
        if(s.fp == fp){ g->a = s.a; g->b = s.b; *slot = i; return 1; }
        i = (i+1) % NSLOT;
    }
    return -1;
}
static void grava(long slot, uint64_t fp, E g){
    Slot s; s.fp = fp; s.a = g.a; s.b = g.b;
    pwrite(fdlex, &s, sizeof s, slot*(long)sizeof(Slot));
}
static E z_frase_pt(const char *f){                  /* z_pt = Σ h(palavra) — o embedding aditivo */
    E z = {0,0};
    for(const char *w=f; *w; ){
        while(*w==' ') w++;
        const char *e=w; while(*e && *e!=' ') e++;
        if(e>w) z = add(z, hpt(w, e-w));
        w = e;
    }
    return z;
}
static long invp(long k){                            /* k⁻¹ em ℤ_p (a multiplicidade da palavra)  */
    long r=1, b=((k%p)+p)%p, e=p-2;
    while(e>0){ if(e&1) r=(r*b)%p; b=(b*b)%p; e>>=1; }
    return r;
}
static int primo(long n){ if(n<2)return 0; for(long d=2;d*d<=n;d++) if(n%d==0) return 0; return 1; }

int main(int argc, char **argv){
    const char *path = argc>1 ? argv[1] : "pares.tsv";
    const long LIM = argc>2 ? atol(argv[2]) : 0;     /* 0 = tudo; senão, só as LIM primeiras linhas */
    m = 1;
    for(p = 40009; ; p++) if(primo(p) && irred_gp2()) break;
    E lam = pw(SIG, p-1);                            /* λ = σ^(p−1): N(λ)=1, a rotação (a borda)   */

    printf("ANCORADO — GF(%d²), σ²=%dσ+1 ; λ=σ^(p−1) (|λ|=1)\n", p, m);
    printf("o inglês não tem hash: cada palavra é incógnita, cada par é equação Σg(w_en)=λ·z_pt\n");
    printf("=================================================================\n");

    unlink(LEXFILE);                                 /* léxico novo, zerado (sparse) */
    fdlex = open(LEXFILE, O_RDWR|O_CREAT|O_TRUNC, 0644);
    if(fdlex < 0){ printf("não pude abrir %s\n", LEXFILE); return 2; }
    if(ftruncate(fdlex, NSLOT*(long)sizeof(Slot)) != 0){ printf("ftruncate falhou\n"); return 2; }

    FILE *f = fopen(path, "r");
    if(!f){ printf("sem %s\n", path); return 2; }

    static char line[8192];                          /* o único buffer — tamanho fixo, O(1)        */
    long colhidas = 0;

    /* ---------- a COLHEITA: passadas nas linhas ímpares, até nada mais cair ---------- */
    for(int passe=1; passe<=MAXPASS; passe++){
        rewind(f);
        long novas = 0, linha = 0;
        while(fgets(line, sizeof line, f)){
            linha++;
            if(LIM && linha > LIM) break;
            if((linha & 1) == 0) continue;                       /* pares = teste, intocadas       */
            char *tab = strchr(line, '\t'); if(!tab) continue;
            *tab = 0;
            char *en = line, *pt = tab+1;
            char *nl = strchr(pt, '\n'); if(nl) *nl = 0;
            if(!*en || !*pt) continue;

            E alvo = mul(lam, z_frase_pt(pt));                   /* λ·z_pt — o que o EN deve somar  */

            E S = {0,0};                                         /* soma das já colhidas            */
            uint64_t falta = 0; long slot = 0, k = 0; int misturado = 0;
            for(char *w=en; *w; ){
                while(*w==' ') w++;
                char *e=w; while(*e && *e!=' ') e++;
                if(e>w){
                    uint64_t fp = marca(w, e-w);
                    E g; long sl;
                    int r = busca(fp, &g, &sl);
                    if(r < 0){ printf("léxico lotou — aumente NSLOT\n"); return 2; }
                    if(r == 1) S = add(S, g);
                    else if(!falta){ falta = fp; slot = sl; k = 1; }
                    else if(falta == fp) k++;                    /* a mesma palavra: coeficiente k  */
                    else misturado = 1;                          /* ≥2 incógnitas: fica p/ depois   */
                }
                w = e;
            }
            if(!falta || misturado) continue;
            grava(slot, falta, scal(invp(k), sub(alvo, S)));      /* g(w) = (λz_pt − S)·k⁻¹ — cai    */
            novas++;
        }
        colhidas += novas;
        printf("  passe %2d: %7ld palavras caíram   (total %ld)\n", passe, novas, colhidas);
        if(!novas) break;
    }

    /* ---------- a VERIFICAÇÃO: as pares (inéditas) e as ímpares (a contradição interna) ----------
     * nas ímpares o léxico foi colhido, mas cada palavra caiu de UMA equação só; as demais equações
     * de treino não foram usadas. Se alguma delas, coberta, falha, então não existe g nenhum que
     * satisfaça o sistema — a inconsistência é provada por contradição, não estimada.            */
    rewind(f);
    long linha=0, tst=0, cob=0, exato=0, norma=0, cru=0;
    long tr_cob=0, tr_exato=0;
    while(fgets(line, sizeof line, f)){
        linha++;
        if(LIM && linha > LIM) break;
        if(linha & 1){                                            /* treino: a contradição interna */
            char *tb = strchr(line, '\t'); if(!tb) continue;
            *tb = 0;
            char *e_en = line, *e_pt = tb+1;
            char *n2 = strchr(e_pt, '\n'); if(n2) *n2 = 0;
            if(!*e_en || !*e_pt) continue;
            E alv = mul(lam, z_frase_pt(e_pt));
            E St = {0,0}; int compl_tr = 1;
            for(char *w=e_en; *w; ){
                while(*w==' ') w++;
                char *e=w; while(*e && *e!=' ') e++;
                if(e>w){ E g; long sl;
                    if(busca(marca(w,e-w), &g, &sl) == 1) St = add(St, g); else compl_tr = 0; }
                w = e;
            }
            if(compl_tr){ tr_cob++; if(eq(St, alv)) tr_exato++; }
            continue;
        }
        char *tab = strchr(line, '\t'); if(!tab) continue;
        *tab = 0;
        char *en = line, *pt = tab+1;
        char *nl = strchr(pt, '\n'); if(nl) *nl = 0;
        if(!*en || !*pt) continue;
        tst++;

        E alvo = mul(lam, z_frase_pt(pt));
        E S = {0,0}, Sind = {0,0}; int completa = 1;
        for(char *w=en; *w; ){
            while(*w==' ') w++;
            char *e=w; while(*e && *e!=' ') e++;
            if(e>w){
                uint64_t fp = marca(w, e-w);
                E g; long sl;
                if(busca(fp, &g, &sl) == 1) S = add(S, g); else completa = 0;
                Sind = add(Sind, hpt(w, e-w));                   /* o velho §P3: hash independente  */
            }
            w = e;
        }
        if(eq(Sind, alvo)) cru++;
        if(!completa) continue;
        cob++;
        if(eq(S, alvo)) exato++;
        if(eq(mul(S,frob(S)), mul(alvo,frob(alvo)))) norma++;     /* a norma (o significado)        */
    }
    fclose(f);

    /* palavras no léxico: varredura em streaming do próprio arquivo (nada em RAM) */
    long ocupados = 0;
    for(long i=0;i<NSLOT;i++){
        Slot s;
        if(pread(fdlex,&s,sizeof s,i*(long)sizeof(Slot))!=(long)sizeof(Slot)) break;
        if(s.fp) ocupados++;
    }
    close(fdlex);

    printf("\npalavras inglesas colhidas (nós do léxico): %ld\n", ocupados);
    printf("\n§A1  as frases de TESTE (pares — nunca usadas na colheita): %ld\n", tst);
    printf("      cobertas (todo o EN colhido)     : %ld/%ld  (%.1f%%)\n", cob, tst, tst?100.0*cob/tst:0);
    printf("      EXATO   Σg(w_en) = λ·z_pt        : %ld/%ld  (%.2f%%)\n", exato, cob, cob?100.0*exato/cob:0);
    printf("      norma   N(Σg) = N(λ z_pt)        : %ld/%ld  (%.2f%%)\n", norma, cob, cob?100.0*norma/cob:0);
    printf("\n§A2  a CONTRADIÇÃO INTERNA — as próprias equações da colheita (ímpares, cobertas):\n");
    printf("      exatas  Σg(w_en) = λ·z_pt        : %ld/%ld  (%.2f%%)\n", tr_exato, tr_cob,
           tr_cob?100.0*tr_exato/tr_cob:0);
    printf("      %s\n", tr_exato==tr_cob ?
        "consistente — nenhuma equação de treino se contradiz" :
        "INCONSISTENTE — equações incompatíveis: NÃO EXISTE g que feche o sistema");
    printf("\n§A3  o DENTE — o hash independente (o §P3 antigo), nas mesmas frases inéditas:\n");
    printf("      Σh(w_en) = λ·z_pt                : %ld/%ld\n", cru, tst);

    int res0 = cob > 0 && exato == cob;
    printf("\n-----------------------------------------------------------------\n");
    if(res0)
        printf("RESÍDUO 0 — ancorado, uma única rotação λ leva o português no inglês em\n"
               "frases inéditas: o embedding do EN não é sorteado, é colhido, e a soma\n"
               "das palavras basta. O gap do §P3 era do hash, não do corpo.\n");
    else
        printf("O DENTE — ancorada, a colheita fecha as frases que a determinaram, mas\n"
               "%.2f%% das frases INÉDITAS cobertas fecham exatas. O sistema\n"
               "Σg(w_en)=λ·z_pt é sobredeterminado e INCONSISTENTE no corpus real: uma\n"
               "rotação global sobre a soma-de-palavras não é a tradução. O que sobra da\n"
               "soma é o que a fala tem de caminho — a ordem, não o saco de palavras.\n",
               cob?100.0*exato/cob:0);
    return res0 ? 0 : 1;
}
