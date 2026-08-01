/* homogeneo.c — os DOIS léxicos livres: Σg(w_en) − λ·Σh(w_pt) = 0.
 *
 * O dente (dente.c) mostrou que a inconsistência não vinha da ordem (1%), e sim do ALVO FIXO: o
 * hash arbitrário do português dava pontos distintos a paráfrases (39%) e, mesmo sanado isso,
 * faltava liberdade por um fator de 14 (E/V=14,4). O conserto que os números pedem é soltar os dois
 * lados. O sistema fica HOMOGÊNEO, e três coisas mudam de natureza:
 *
 *   · um homogêneo SEMPRE tem solução (a trivial, tudo 0). A pergunta deixa de ser "existe?" e
 *     passa a ser "o núcleo é degenerado?".
 *   · a paráfrase deixa de ser contradição: duas frases PT com o mesmo EN passam a impor
 *     Σh(pt₁)=Σh(pt₂) — restrição satisfazível que AFIRMA que as duas são sinônimas.
 *   · λ some. Pondo h'=λh a equação é Σg=Σh': com os dois lados livres a rotação é INOBSERVÁVEL,
 *     pura reparametrização. Aqui o λ fica explícito de propósito, e o programa aceita o seu
 *     expoente por argumento — trocá-lo tem de dar exatamente os mesmos números. É a medida da
 *     inobservabilidade, não a sua afirmação.
 *
 * A COLHEITA (como em ancora.c, sem matriz e sem memória): quando numa equação falta uma só
 * incógnita, ela cai exata. Nada cai do nada — um homogêneo precisa de SEMENTE, senão o peeling
 * desce à solução trivial. Então: cascata de determinações até esgotar; quando trava, gasta-se UMA
 * semente (uma palavra fixada a um valor arbitrário — um grau de liberdade queimado) numa equação de
 * duas incógnitas, e a cascata segue. As sementes gastas medem o quanto o corpus NÃO determina.
 *
 * As medidas: sementes gastas · palavras determinadas · equações que fecham na verificação (com o
 * sistema homogêneo, uma solução legítima fecharia TODAS) · a degeneração (quantas palavras zeradas;
 * o modo `dump` joga o léxico no stdout para `sort -u` contar os valores distintos, em disco).
 *
 *   cc -O2 -std=c99 -I../tools homogeneo.c -o homogeneo
 *   ./homogeneo pares.tsv [lim] [exp_lambda]
 *   ./homogeneo dump                      # os pontos colhidos, um por linha
 */
#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include "unidade.h"
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <unistd.h>
#include <fcntl.h>
#include "gp2.h"

#define NSLOT   (1L<<20)
#define LEXFILE "lexico_hom.bin"
#define MAXROD  8                  /* rodadas semente→cascata                                     */
#define MAXCAS  6                  /* passadas de cascata por rodada                              */

typedef struct { uint64_t fp; int32_t a, b; } Slot;

static int fdlex;
static E lam, lam_inv;

static uint64_t marca(const char *s, size_t n, uint64_t sal){
    uint64_t h = 1469598103934665603UL ^ sal;
    for(size_t i=0;i<n;i++){ h ^= (unsigned char)s[i]; h *= 1099511628211UL; }
    return h ? h : 1;
}
#define SAL_EN 0x1111111111111111UL
#define SAL_PT 0x2222222222222222UL

/* o valor arbitrário de uma semente — um grau de liberdade queimado, não um alvo */
static E semente(uint64_t fp){
    uint64_t h2 = (fp ^ 0x9e3779b97f4a7c15UL) * 1099511628211UL;
    E z = { (int)(fp % (uint64_t)p), (int)(h2 % (uint64_t)p) };
    if(!z.a && !z.b) z.a = 1;
    return z;
}
static int busca(uint64_t fp, E *g, long *slot){
    long i = (long)(fp % (uint64_t)NSLOT);
    for(long t=0;t<NSLOT;t++){
        Slot s;
        if(pread(fdlex,&s,sizeof s,i*(long)sizeof(Slot)) != (long)sizeof(Slot)) return -1;
        if(s.fp == 0){ *slot = i; return 0; }
        if(s.fp == fp){ g->a=s.a; g->b=s.b; *slot=i; return 1; }
        i = (i+1) % NSLOT;
    }
    return -1;
}
static void grava(long slot, uint64_t fp, E g){
    Slot s; s.fp=fp; s.a=g.a; s.b=g.b;
    pwrite(fdlex,&s,sizeof s,slot*(long)sizeof(Slot));
}
static long invp(long k){
    long r=1,b=((k%p)+p)%p,e=p-2;
    while(e>0){ if(e&1) r=(r*b)%p; b=(b*b)%p; e>>=1; }
    return r;
}
static int primo(long n){ if(n<2)return 0; for(long d=2;d*d<=n;d++) if(n%d==0) return 0; return 1; }

/* uma incógnita da equação */
typedef struct { uint64_t fp; long slot, k; int lado; } Inc;   /* lado: 0=EN, 1=PT */

/* varre a equação: soma os conhecidos (Gs no EN, Hs no PT) e junta até 3 incógnitas distintas */
static void varre(char *en, char *pt, E *Gs, E *Hs, Inc *u, int *nu){
    Gs->a=Gs->b=0; Hs->a=Hs->b=0; *nu=0;
    for(int lado=0; lado<2; lado++){
        for(char *w = lado?pt:en; *w; ){
            while(*w==' ') w++;
            char *e=w; while(*e && *e!=' ') e++;
            if(e>w){
                uint64_t fp = marca(w, e-w, lado?SAL_PT:SAL_EN);
                E g; long sl;
                if(busca(fp,&g,&sl) == 1){ if(lado) *Hs = add(*Hs,g); else *Gs = add(*Gs,g); }
                else {
                    int achou = 0;
                    for(int i=0;i<*nu && i<3;i++) if(u[i].fp == fp){ u[i].k++; achou=1; break; }
                    if(!achou && *nu < 3){ u[*nu].fp=fp; u[*nu].slot=sl; u[*nu].k=1; u[*nu].lado=lado; (*nu)++; }
                    else if(!achou) (*nu)++;                       /* ≥4: só conta                */
                }
            }
            w = e;
        }
    }
}
/* determina a única incógnita: EN → k·g = λHs − Gs ; PT → λk·h = Gs − λHs */
static void resolve(Inc *x, E Gs, E Hs){
    E v;
    if(x->lado == 0) v = scal(invp(x->k), sub(mul(lam,Hs), Gs));
    else             v = mul(scal(invp(x->k), sub(Gs, mul(lam,Hs))), lam_inv);
    grava(x->slot, x->fp, v);
}

static char line[8192];


/* O CORPUS NÃO ESTÁ NO REPOSITÓRIO — está no .gitignore, e com razão: são 12 MB. A consequência
 * é que este medidor depende de um ficheiro que um disco limpo não tem, e num disco limpo ele
 * não mede. Isso não se resolve escondendo: procura-se onde o corpus costuma estar, e se não
 * houver diz-se e sai-se com 2. Um medidor sem o objeto a medir não passa nem falha: não mediu.
 * (Foi assim que ele passou meses verde aqui e teria falhado no CI.) */
static const char *corpus_procura(const char *pedido){
    /* O PEDIDO É UMA PREFERÊNCIA, NÃO UMA OBRIGAÇÃO — e devolvê-lo sem o abrir era o defeito:
     * a bateria passa "pares.tsv" por argumento, o ficheiro não está aí, e o medidor desistia
     * em vez de procurar. Se o pedido abre, é ele; se não abre, procura-se. */
    if(pedido){
        FILE *f = fopen(pedido, "r");
        if(f){ fclose(f); return pedido; }
    }
    const char *e = getenv("TATOEBA_CORPUS");
    if(e && *e) return e;
    static const char *cands[] = { "pares.tsv", "tatoeba/pares.tsv", "../tatoeba/pares.tsv",
                                   "/tmp/pares.tsv", NULL };
    for(int i = 0; cands[i]; i++){
        FILE *f = fopen(cands[i], "r");
        if(f){ fclose(f); return cands[i]; }
    }
    return "pares.tsv";
}

int main(int argc, char **argv){
    m = 1;
    for(p = 40009; ; p++) if(primo(p) && irred_gp2()) break;

    if(argc>1 && !strcmp(argv[1],"dump")){                 /* o léxico colhido, para sort -u       */
        fdlex = open(LEXFILE, O_RDONLY);
        if(fdlex<0){ fprintf(stderr,"sem %s\n", LEXFILE); return 2; }
        for(long i=0;i<NSLOT;i++){
            Slot s;
            if(pread(fdlex,&s,sizeof s,i*(long)sizeof(Slot))!=(long)sizeof(Slot)) break;
            if(s.fp) printf("%05d,%05d\n", s.a, s.b);
        }
        close(fdlex); return 0;
    }

    const char *path = corpus_procura(argc>1 ? argv[1] : NULL);
    const long LIM   = argc>2 ? atol(argv[2]) : 0;
    const long EXPL  = argc>3 ? atol(argv[3]) : 0;         /* λ = σ^(p−1+EXPL): trocar não muda nada */
    lam = pw(SIG, (long)p - 1 + EXPL);
    lam_inv = pw(lam, (long)p*p - 2);

    printf("HOMOGÊNEO — GF(%d²) ; Σg(w_en) − λ·Σh(w_pt) = 0 ; λ=σ^(p−1%+ld)\n", p, EXPL);
    printf("os dois léxicos livres: nenhum lado sorteado, nenhum alvo fixo\n");
    printf("=================================================================\n");

    unlink(LEXFILE);
    fdlex = open(LEXFILE, O_RDWR|O_CREAT|O_TRUNC, 0644);
    if(fdlex<0 || ftruncate(fdlex, NSLOT*(long)sizeof(Slot))!=0){ printf("léxico falhou\n"); return 2; }
    FILE *f = fopen(path,"r");
    if(!f){ printf("NAO MEDIU — sem corpus (%s). Ponha TATOEBA_CORPUS=<caminho>.\n", path); return 2; }

    long sementes=0, determinadas=0;

    for(int rodada=1; rodada<=MAXROD; rodada++){
        /* --- cascata: só determina quem está sozinho --- */
        long caiu_total = 0;
        for(int c=0;c<MAXCAS;c++){
            rewind(f);
            long linha=0, caiu=0;
            while(fgets(line,sizeof line,f)){
                linha++; if(LIM && linha>LIM) break;
                char *tab=strchr(line,'\t'); if(!tab) continue;
                *tab=0; char *en=line, *pt=tab+1;
                char *nl=strchr(pt,'\n'); if(nl)*nl=0;
                if(!*en||!*pt) continue;
                E Gs,Hs; Inc u[3]; int nu;
                varre(en,pt,&Gs,&Hs,u,&nu);
                if(nu==1){ resolve(&u[0],Gs,Hs); caiu++; }
            }
            caiu_total += caiu;
            if(!caiu) break;
        }
        determinadas += caiu_total;

        /* --- semeadura: uma semente por equação travada, e a parceira cai --- */
        rewind(f);
        long linha=0, sem=0;
        while(fgets(line,sizeof line,f)){
            linha++; if(LIM && linha>LIM) break;
            char *tab=strchr(line,'\t'); if(!tab) continue;
            *tab=0; char *en=line, *pt=tab+1;
            char *nl=strchr(pt,'\n'); if(nl)*nl=0;
            if(!*en||!*pt) continue;
            E Gs,Hs; Inc u[3]; int nu;
            varre(en,pt,&Gs,&Hs,u,&nu);
            if(nu!=2) continue;
            E s = semente(u[0].fp);                        /* queima um grau de liberdade          */
            grava(u[0].slot, u[0].fp, s);
            if(u[0].lado) Hs = add(Hs, scal(u[0].k, s)); else Gs = add(Gs, scal(u[0].k, s));
            resolve(&u[1], Gs, Hs);                        /* a parceira cai exata                 */
            sem++; determinadas++;
            if(sem >= 20000) break;                        /* teto por rodada (não varrer à toa)   */
        }
        sementes += sem;
        printf("  rodada %d: cascata %7ld caem · sementes %6ld\n", rodada, caiu_total, sem);
        if(!sem && !caiu_total) break;
    }

    /* --- verificação: no homogêneo, uma solução legítima fecha TODAS as equações --- */
    rewind(f);
    long linha=0, tot=0, cob=0, fecha=0;
    while(fgets(line,sizeof line,f)){
        linha++; if(LIM && linha>LIM) break;
        char *tab=strchr(line,'\t'); if(!tab) continue;
        *tab=0; char *en=line, *pt=tab+1;
        char *nl=strchr(pt,'\n'); if(nl)*nl=0;
        if(!*en||!*pt) continue;
        tot++;
        E Gs,Hs; Inc u[3]; int nu;
        varre(en,pt,&Gs,&Hs,u,&nu);
        if(nu) continue;                                   /* alguma palavra ficou indeterminada   */
        cob++;
        if(eq(Gs, mul(lam,Hs))) fecha++;
    }
    fclose(f);

    long ocup=0, zeros=0;
    for(long i=0;i<NSLOT;i++){
        Slot s;
        if(pread(fdlex,&s,sizeof s,i*(long)sizeof(Slot))!=(long)sizeof(Slot)) break;
        if(s.fp){ ocup++; if(!s.a && !s.b) zeros++; }
    }
    close(fdlex);

    printf("\npalavras no léxico (EN+PT)          : %ld\n", ocup);
    printf("  sementes (graus de liberdade)     : %ld  (%.1f%% das palavras)\n",
           sementes, ocup?100.0*sementes/ocup:0);
    printf("  colhidas por dedução              : %ld\n", ocup-sementes);
    printf("  zeradas (degeneração)             : %ld  (%.2f%%)\n", zeros, ocup?100.0*zeros/ocup:0);
    printf("\nequações                            : %ld\n", tot);
    printf("  cobertas (todas as palavras vivas): %ld  (%.1f%%)\n", cob, tot?100.0*cob/tot:0);
    printf("  FECHAM  Σg = λ·Σh                 : %ld/%ld  (%.2f%%)\n", fecha, cob,
           cob?100.0*fecha/cob:0);

    int res0 = cob>0 && fecha==cob;
    double taxa = cob ? 100.0*fecha/cob : 0;
    printf("\n-----------------------------------------------------------------\n");

    /* Ver a nota longa no `ancora.c`: a asserção estava ao contrário e a bateria tinha uma lista
     * à mão para a desculpar. Dita positivamente, o teorema negativo passa a poder falhar — e é
     * isso que faz dele uma medida. */
    ok("o corpus tem pelo menos 50 000 equações — o negativo é sobre DADOS",
       tot >= 50000);
    ok("a taxa de fecho fica ABAIXO de 30% mesmo com os dois lados livres — o que falha é o Σ",
       cob > 0 && taxa < 30.0);
    printf("      (a taxa medida: %.2f%% de %ld equações cobertas, %ld no total)\n", taxa, cob, tot);
    if(res0)
        printf("RESÍDUO 0 — com os dois léxicos livres o corpus é CONSISTENTE: existe um\n"
               "embedding em que a soma da frase portuguesa e a da inglesa são o mesmo ponto\n"
               "(a menos de λ, que é inobservável). %ld sementes: o corpus não determina o\n"
               "significado, ele o VINCULA — o resto é dedução.\n", sementes);
    else
        printf("O DENTE PERMANECE — mesmo homogêneo, só %.2f%% das equações fecham. Soltar os\n"
               "dois lados não basta: com E/V ainda >1 a soma-de-palavras é sobredeterminada, e\n"
               "o que falha não é o léxico nem a rotação — é o Σ.\n", cob?100.0*fecha/cob:0);
    return falhas ? 1 : 0;
}
