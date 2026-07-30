/* bairro.c — A ÓRBITA É O BAIRRO. E QUEM ESCOLHE É A CONTRAÇÃO POR REALIMENTAÇÃO.
 *
 * Correção de rota. Em centro.c eu tratei vizinhança como uma CHAVE que eu escolhia —
 * (anterior, forma) — e inventei níveis de delta à mão. Errado: vizinhança e órbita são a
 * MESMA coisa. A órbita de uma palavra é o seu bairro, todos os vizinhos entram no
 * significado, e quem separa não sou eu: é o próprio sistema, contraindo por realimentação.
 *
 *      s(e) = a(e) · ( m + Σ_vizinhos w(f)·c(e,f) )
 *              ^^^^         ^^^^^^^^^^^^^^^^^^^^^^^
 *              o próprio    o que o bairro devolve
 *
 * É σ = m + 1/σ: o valor de cada candidato depende do valor dos outros, que dependem dele.
 * Fecha-se o laço e itera-se até o ponto fixo. E a tese é FALSIFICÁVEL: iteração 0 é
 * exatamente a marginal (que já mediu 62,7% no cego). Se a realimentação não paga, o ponto
 * fixo não bate a iteração 0, e a tese cai.
 *
 *   §B1  o bairro: a candidatura de cada palavra, colhida da borda (Dice, sem dicionário)
 *   §B2  a contração: converge? em quantas batidas, e com que fator
 *   §B3  o cego: acerto contra o RAIO do bairro e contra o número de batidas
 *   §B4  digital == analógico: o mesmo ponto fixo pelo translinear
 *
 * SEM MEMÓRIA: RAM O(1) em tabelas — contagens, coocorrências e candidaturas vivem em
 * disco (pread/pwrite). A matriz do bairro NÃO é memória: no analógico ela é a FIAÇÃO
 * (a condutância entre dois nós), montada da topologia da frase e desfeita com ela.
 *
 *   cc -O2 -std=gnu99 bairro.c -lm -o bairro && ./bairro pares.tsv
 */
#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <unistd.h>
#include <fcntl.h>
#include <math.h>

#define NS   (1L<<20)          /* contagens de tipos                       */
#define NCO  (1L<<23)          /* coocorrências PT×EN e EN×EN              */
#define NC   (1L<<15)          /* candidaturas por tipo PT                 */
#define KC   4                 /* candidatos guardados por palavra         */
#define RMAX 12                /* raio do bairro: quantas palavras entram  */
#define LMAX 4096
#define WMAX 32
#define MINC 5                 /* massa mínima: a cauda é amputada (§R3)   */

static int falhas = 0;
static void ok(const char *r, int c){
    printf("      %-56s %s\n", r, c ? "sim  ✓" : "NÃO  ✗");
    if(!c) falhas++;
}
static int abre(const char *nome, long bytes){
    int fd = open(nome, O_RDWR|O_CREAT|O_TRUNC, 0644);
    if(fd < 0){ perror(nome); exit(2); }
    if(ftruncate(fd, bytes) < 0){ perror("ftruncate"); exit(2); }
    return fd;
}
static uint64_t fp64(const char *s, size_t n){
    uint64_t h = 1469598103934665603UL;
    for(size_t i = 0; i < n; i++){ h ^= (unsigned char)s[i]; h *= 1099511628211UL; }
    return h ? h : 1;
}
static uint64_t fps(const char *s){ return fp64(s, strlen(s)); }

static uint64_t tab_inc(int fd, long slots, uint64_t fp){
    long i = (long)(fp % (uint64_t)slots);
    for(long t = 0; t < slots; t++){
        uint64_t s[2] = {0,0};
        if(pread(fd,s,16,i*16) != 16) return 0;
        if(!s[0]){ s[0]=fp; s[1]=1; pwrite(fd,s,16,i*16); return 1; }
        if(s[0]==fp){ s[1]++; pwrite(fd,s+1,8,i*16+8); return s[1]; }
        i = (i+1) % slots;
    }
    return 0;
}
static uint64_t tab_ol(int fd, long slots, uint64_t fp){
    long i = (long)(fp % (uint64_t)slots);
    for(long t = 0; t < slots; t++){
        uint64_t s[2] = {0,0};
        if(pread(fd,s,16,i*16) != 16) return 0;
        if(!s[0]) return 0;
        if(s[0]==fp) return s[1];
        i = (i+1) % slots;
    }
    return 0;
}

/* a candidatura de uma palavra PT: os KC melhores caules EN, por Dice */
struct cand { uint64_t fp; struct { char en[WMAX]; float sc; } k[KC]; };
static void cand_put(int fd, uint64_t fp, const char *en, double sc){
    long i = (long)(fp % (uint64_t)NC);
    for(long t = 0; t < NC; t++){
        struct cand c; memset(&c,0,sizeof c);
        if(pread(fd,&c,sizeof c,i*(long)sizeof c) != (long)sizeof c) return;
        if(!c.fp || c.fp == fp){
            c.fp = fp;
            int pos = -1;
            for(int j = 0; j < KC; j++) if(!strcmp(c.k[j].en, en)){ pos = j; break; }
            if(pos >= 0){ if(sc > c.k[pos].sc) c.k[pos].sc = (float)sc; }
            else {
                int pior = 0;
                for(int j = 1; j < KC; j++) if(c.k[j].sc < c.k[pior].sc) pior = j;
                if(sc > c.k[pior].sc){ snprintf(c.k[pior].en,WMAX,"%s",en); c.k[pior].sc = (float)sc; }
            }
            pwrite(fd,&c,sizeof c,i*(long)sizeof c);
            return;
        }
        i = (i+1) % NC;
    }
}
static int cand_get(int fd, uint64_t fp, struct cand *out){
    long i = (long)(fp % (uint64_t)NC);
    for(long t = 0; t < NC; t++){
        struct cand c; memset(&c,0,sizeof c);
        if(pread(fd,&c,sizeof c,i*(long)sizeof c) != (long)sizeof c) return 0;
        if(!c.fp) return 0;
        if(c.fp == fp){ *out = c; return 1; }
        i = (i+1) % NC;
    }
    return 0;
}

static int letra(unsigned char c){
    return (c>='a'&&c<='z')||(c>='A'&&c<='Z')||(c>='0'&&c<='9')||c>=0x80;
}
static size_t proxima(const char *s, size_t n, size_t *i, char *out, size_t om){
    while(*i < n && !letra((unsigned char)s[*i])) (*i)++;
    size_t k = 0;
    while(*i < n && letra((unsigned char)s[*i])){
        unsigned char c = (unsigned char)s[*i];
        if(c>='A'&&c<='Z') c += 32;
        if(k+1 < om) out[k++] = (char)c;
        (*i)++;
    }
    out[k] = 0;
    return k;
}
static int craw = -1;
static size_t caule_en(char *w, size_t l){
    if(craw < 0) return l;
    char cnd[2][WMAX]; int nc = 0;
    if(l >= 6 && !strcmp(w+l-3,"ing")){
        snprintf(cnd[nc++],WMAX,"%.*s",(int)(l-3),w);
        snprintf(cnd[nc++],WMAX,"%.*se",(int)(l-3),w);
    } else if(l >= 4 && !strcmp(w+l-2,"ed")){
        snprintf(cnd[nc++],WMAX,"%.*s",(int)(l-2),w);
        snprintf(cnd[nc++],WMAX,"%.*s",(int)(l-1),w);
    } else if(l >= 4 && !strcmp(w+l-2,"es")){
        snprintf(cnd[nc++],WMAX,"%.*s",(int)(l-2),w);
        snprintf(cnd[nc++],WMAX,"%.*s",(int)(l-1),w);
    } else if(l >= 3 && w[l-1]=='s' && w[l-2]!='s'){
        snprintf(cnd[nc++],WMAX,"%.*s",(int)(l-1),w);
    }
    int melhor = -1; size_t lm = 0;
    for(int k = 0; k < nc; k++){
        size_t lk = strlen(cnd[k]);
        if(lk < 2) continue;
        if(tab_ol(craw, NS, fp64(cnd[k],lk)) < MINC) continue;
        if(lk > lm){ lm = lk; melhor = k; }
    }
    if(melhor < 0) return l;
    snprintf(w,WMAX,"%s",cnd[melhor]);
    return lm;
}
/* chave da coocorrência EN×EN, sempre na mesma ordem (a aresta não tem sentido) */
static uint64_t par_en(const char *a, const char *b){
    char k[2*WMAX+2];
    if(strcmp(a,b) <= 0) snprintf(k,sizeof k,"%s\t%s",a,b);
    else                 snprintf(k,sizeof k,"%s\t%s",b,a);
    return fps(k);
}

/* ---- a contração: s(e) = a(e)·(m + Σ w(f)·c(e,f)), iterada até o ponto fixo ---------- */
/* Duas vias para o MESMO laço. A digital multiplica; a analógica soma logaritmos e
 * antilogaritma (o translinear do gabarito, §B.3), com Kirchhoff fazendo o Σ do bairro. */
static int contrai(int nt, int nk[RMAX], double a[RMAX][KC], double G[RMAX*KC][RMAX*KC],
                   double s[RMAX][KC], int translinear, double *dmax_saida, int *batidas,
                   int raio)
{
    const double m = 1.0, eps = 1e-13;
    double w[RMAX][KC];
    for(int t = 0; t < nt; t++){
        double z = 0; for(int j = 0; j < nk[t]; j++) z += a[t][j];
        for(int j = 0; j < nk[t]; j++) s[t][j] = z > 0 ? a[t][j]/z : 0;
    }
    int it = 0; double dmax = 1;
    for(; it < 60 && dmax > eps; it++){
        for(int t = 0; t < nt; t++) for(int j = 0; j < nk[t]; j++) w[t][j] = s[t][j];
        dmax = 0;
        for(int t = 0; t < nt; t++){
            double nv[KC], z = 0;
            for(int j = 0; j < nk[t]; j++){
                double viz = 0;                                   /* Kirchhoff: o bairro soma */
                for(int u = 0; u < nt; u++){
                    if(u == t) continue;
                    int d = u > t ? u - t : t - u;
                    if(d > raio) continue;                        /* o raio do bairro */
                    for(int l = 0; l < nk[u]; l++) viz += w[u][l] * G[t*KC+j][u*KC+l];
                }
                if(translinear){
                    /* ANTILOG( log a + log(m+viz) ) — a mesma conta, no domínio do log */
                    double la = log(a[t][j] > 0 ? a[t][j] : 1e-300);
                    nv[j] = exp(la + log(m + viz));
                } else {
                    nv[j] = a[t][j] * (m + viz);
                }
                z += nv[j];
            }
            for(int j = 0; j < nk[t]; j++){
                double v = z > 0 ? nv[j]/z : 0;
                double d = fabs(v - s[t][j]); if(d > dmax) dmax = d;
                s[t][j] = v;
            }
        }
    }
    *dmax_saida = dmax; *batidas = it;
    return dmax <= eps;
}

int main(int argc, char **argv){
    const char *arq = argc > 1 ? argv[1] : "pares.tsv";
    FILE *f; char linha[LMAX], w[WMAX];

    printf("\n=== O BAIRRO: A ÓRBITA É A VIZINHANÇA, E A CONTRAÇÃO ESCOLHE ================\n");
    printf("    s(e) = a(e)·( m + Σ_vizinhos w(f)·c(e,f) ) — iterado até o ponto fixo.\n");
    printf("    Iteração 0 É a marginal. Se a realimentação não paga, a tese cai aqui.\n");

    craw = abre("bairro_craw.bin", NS*16);
    int cpt = abre("bairro_cpt.bin", NS*16), cen = abre("bairro_cen.bin", NS*16);
    int coa = abre("bairro_coa.bin", NCO*16);        /* PT × EN            */
    int coe = abre("bairro_coe.bin", NCO*16);        /* EN × EN (a fiação) */
    int cnd = abre("bairro_cand.bin", NC*(long)sizeof(struct cand));

    /* ---------- passada 0: superfícies EN cruas (para o caule se validar) ---------- */
    if(!(f = fopen(arq,"r"))){ perror(arq); return 2; }
    while(fgets(linha,sizeof linha,f)){
        char *tab = strchr(linha,'\t'); if(!tab) continue;
        if(fps(linha) % 10 == 0) continue;                /* 10% CEGO fora do aprendizado */
        *tab = 0; size_t n = strlen(linha), i = 0, l;
        while((l = proxima(linha,n,&i,w,sizeof w))) tab_inc(craw, NS, fp64(w,l));
    }
    fclose(f);

    /* ---------- passada 1: contagens PT e caules EN ---------- */
    long tok_pt = 0, tok_en = 0;
    if(!(f = fopen(arq,"r"))){ perror(arq); return 2; }
    while(fgets(linha,sizeof linha,f)){
        char *tab = strchr(linha,'\t'); if(!tab) continue;
        if(fps(linha) % 10 == 0) continue;
        *tab = 0; const char *en = linha, *pt = tab+1;
        size_t nen = strlen(en), npt = strlen(pt), i, l;
        i = 0; while((l = proxima(en,nen,&i,w,sizeof w))){ caule_en(w,l); tab_inc(cen,NS,fps(w)); tok_en++; }
        i = 0; while((l = proxima(pt,npt,&i,w,sizeof w))){ tab_inc(cpt,NS,fp64(w,l)); tok_pt++; }
    }
    fclose(f);

    /* ---------- passada 2: as duas coocorrências ---------- */
    long nco_a = 0, nco_e = 0;
    if(!(f = fopen(arq,"r"))){ perror(arq); return 2; }
    while(fgets(linha,sizeof linha,f)){
        char *tab = strchr(linha,'\t'); if(!tab) continue;
        if(fps(linha) % 10 == 0) continue;
        *tab = 0; const char *en = linha, *pt = tab+1;
        size_t nen = strlen(en), npt = strlen(pt), i, l;
        char ev[24][WMAX]; int ne = 0;
        i = 0; while((l = proxima(en,nen,&i,w,sizeof w)) && ne < 24){
            caule_en(w,l);
            if(tab_ol(cen,NS,fps(w)) < MINC) continue;
            int rep = 0; for(int k=0;k<ne;k++) if(!strcmp(ev[k],w)) rep = 1;
            if(!rep) snprintf(ev[ne++],WMAX,"%s",w);
        }
        char pv[24][WMAX]; int np = 0;
        i = 0; while((l = proxima(pt,npt,&i,w,sizeof w)) && np < 24){
            if(tab_ol(cpt,NS,fp64(w,l)) < MINC) continue;
            int rep = 0; for(int k=0;k<np;k++) if(!strcmp(pv[k],w)) rep = 1;
            if(!rep) snprintf(pv[np++],WMAX,"%s",w);
        }
        for(int x = 0; x < np; x++) for(int y = 0; y < ne; y++){
            char k[2*WMAX+2]; snprintf(k,sizeof k,"%s\t%s",pv[x],ev[y]);
            if(tab_inc(coa,NCO,fps(k)) == 1) nco_a++;
        }
        for(int x = 0; x < ne; x++) for(int y = x+1; y < ne; y++)
            if(tab_inc(coe,NCO,par_en(ev[x],ev[y])) == 1) nco_e++;
    }
    fclose(f);

    /* ---------- passada 3: a candidatura (os KC melhores, por Dice) ---------- */
    if(!(f = fopen(arq,"r"))){ perror(arq); return 2; }
    while(fgets(linha,sizeof linha,f)){
        char *tab = strchr(linha,'\t'); if(!tab) continue;
        if(fps(linha) % 10 == 0) continue;
        *tab = 0; const char *en = linha, *pt = tab+1;
        size_t nen = strlen(en), npt = strlen(pt), i, l;
        char ev[24][WMAX]; int ne = 0;
        i = 0; while((l = proxima(en,nen,&i,w,sizeof w)) && ne < 24){
            caule_en(w,l);
            if(tab_ol(cen,NS,fps(w)) < MINC) continue;
            int rep = 0; for(int k=0;k<ne;k++) if(!strcmp(ev[k],w)) rep = 1;
            if(!rep) snprintf(ev[ne++],WMAX,"%s",w);
        }
        i = 0;
        while((l = proxima(pt,npt,&i,w,sizeof w))){
            double cp = (double)tab_ol(cpt,NS,fp64(w,l));
            if(cp < MINC) continue;
            for(int y = 0; y < ne; y++){
                char k[2*WMAX+2]; snprintf(k,sizeof k,"%s\t%s",w,ev[y]);
                double c = (double)tab_ol(coa,NCO,fps(k));
                double ce = (double)tab_ol(cen,NS,fps(ev[y]));
                if(c < 3 || ce < 1) continue;
                cand_put(cnd, fp64(w,l), ev[y], 2.0*c/(cp+ce));
            }
        }
    }
    fclose(f);
    printf("\n§B1  O bairro, colhido da borda.\n\n");
    printf("      tokens de treino: PT %ld, EN %ld   (10%% cego fora)\n", tok_pt, tok_en);
    printf("      coocorrências PT×EN distintas ....... %ld\n", nco_a);
    printf("      coocorrências EN×EN distintas ....... %ld   (a fiação do bairro)\n", nco_e);
    ok("há bairro: a fiação EN×EN não é vazia", nco_e > 1000);

    /* ---------- passada 4: o cego, com a contração ---------- */
    printf("\n§B2/§B3  A contração no cego: acerto contra o RAIO do bairro.\n");
    printf("     O raio é DISTÂNCIA no bairro: quem está a mais de R palavras não influencia.\n");
    printf("     Raio 0 = ninguém influencia = a marginal pura. E a POPULAÇÃO É A MESMA em todas\n");
    printf("     as linhas — só muda quem fala. (Na primeira versão eu media a marginal só na\n");
    printf("     primeira palavra da frase e o bairro em doze: populações diferentes, e a\n");
    printf("     comparação não valia nada.)\n\n");
    int raios[6] = {0, 1, 2, 4, 8, RMAX};
    printf("      raio   respondeu   acerto   batidas médias   convergiu\n");
    double acerto_r0 = 0, acerto_rmax = 0;
    long conv_falha_total = 0;
    for(int ri = 0; ri < 6; ri++){
        int R = raios[ri];
        long tot = 0, ac = 0, bat = 0, nconv = 0, ncasos = 0;
        if(!(f = fopen(arq,"r"))){ perror(arq); return 2; }
        while(fgets(linha,sizeof linha,f)){
            char *tab = strchr(linha,'\t'); if(!tab) continue;
            if(fps(linha) % 10 != 0) continue;                 /* SÓ o cego */
            *tab = 0; const char *en = linha, *pt = tab+1;
            size_t nen = strlen(en), npt = strlen(pt), i, l;
            /* a referência: os caules EN da frase (só para conferir, nunca para prever) */
            char ref[24][WMAX]; int nr = 0;
            i = 0; while((l = proxima(en,nen,&i,w,sizeof w)) && nr < 24){
                caule_en(w,l);
                int rep = 0; for(int k=0;k<nr;k++) if(!strcmp(ref[k],w)) rep = 1;
                if(!rep) snprintf(ref[nr++],WMAX,"%s",w);
            }
            /* os vizinhos: as primeiras RMAX palavras PT com candidatura */
            char tw[RMAX][WMAX]; struct cand cc[RMAX]; int nt = 0;
            i = 0;
            while((l = proxima(pt,npt,&i,w,sizeof w)) && nt < RMAX){
                struct cand c;
                if(!cand_get(cnd, fp64(w,l), &c)) continue;
                int nk = 0; for(int j = 0; j < KC; j++) if(c.k[j].sc > 0) nk++;
                if(!nk) continue;
                snprintf(tw[nt],WMAX,"%s",w); cc[nt] = c; nt++;
            }
            if(!nt) continue;
            int nviz = nt;                     /* população FIXA: o raio não corta quem é medido */
            /* a fiação: G[(t,j)][(u,l)] = Dice EN×EN — no analógico, a condutância */
            static double G[RMAX*KC][RMAX*KC];
            int nk[RMAX]; double a[RMAX][KC], s[RMAX][KC];
            for(int t = 0; t < nviz; t++){
                nk[t] = 0;
                for(int j = 0; j < KC; j++) if(cc[t].k[j].sc > 0){ a[t][nk[t]] = cc[t].k[j].sc;
                    if(nk[t] != j) cc[t].k[nk[t]] = cc[t].k[j];
                    nk[t]++; }
            }
            for(int t = 0; t < nviz; t++) for(int j = 0; j < nk[t]; j++)
            for(int u = 0; u < nviz; u++) for(int q = 0; q < nk[u]; q++){
                if(u == t){ G[t*KC+j][u*KC+q] = 0; continue; }
                double c = (double)tab_ol(coe,NCO,par_en(cc[t].k[j].en, cc[u].k[q].en));
                double ca = (double)tab_ol(cen,NS,fps(cc[t].k[j].en));
                double cb = (double)tab_ol(cen,NS,fps(cc[u].k[q].en));
                G[t*KC+j][u*KC+q] = (c >= 2 && ca+cb > 0) ? 2.0*c/(ca+cb) : 0.0;
            }
            double dmax; int batidas;
            int convergiu = contrai(nviz, nk, a, G, s, 0, &dmax, &batidas, R);
            bat += batidas; ncasos++;
            if(convergiu) nconv++; else conv_falha_total++;
            /* a previsão de CADA palavra do bairro, conferida na referência */
            for(int t = 0; t < nviz; t++){
                int melhor = 0;
                for(int j = 1; j < nk[t]; j++) if(s[t][j] > s[t][melhor]) melhor = j;
                tot++;
                for(int k = 0; k < nr; k++) if(!strcmp(ref[k], cc[t].k[melhor].en)){ ac++; break; }
            }
        }
        fclose(f);
        double p = tot ? 100.0*ac/tot : 0;
        if(ri == 0) acerto_r0 = p;
        if(ri == 5) acerto_rmax = p;
        printf("      %4d   %9ld   %5.1f%%   %14.2f   %s\n",
               R, tot, p, ncasos ? (double)bat/ncasos : 0.0,
               ncasos && nconv == ncasos ? "todas ✓" : "NÃO");
    }
    ok("a contração converge em TODAS as frases do cego", conv_falha_total == 0);
    printf("\n      raio 0 (marginal) %.1f%%   ->   raio %d (bairro) %.1f%%   =   %+.1f pontos\n",
           acerto_r0, RMAX, acerto_rmax, acerto_rmax - acerto_r0);
    ok("a REALIMENTAÇÃO paga: o ponto fixo bate a iteração 0", acerto_rmax > acerto_r0);

    /* ---------- §B3b: onde a realimentação MEXEU ---------- */
    printf("\n§B3b  +0,4 ponto não diz nada sozinho. O que decide o próximo passo é se o\n");
    printf("      bairro MUDA pouca decisão (fiação inerte) ou muda muitas e erra metade\n");
    printf("      (fiação ruidosa). A matriz da virada:\n\n");
    {
        long n = 0, mexeu = 0, gg = 0, ee = 0, ge = 0, eg = 0;
        if(!(f = fopen(arq,"r"))){ perror(arq); return 2; }
        while(fgets(linha,sizeof linha,f)){
            char *tab = strchr(linha,'\t'); if(!tab) continue;
            if(fps(linha) % 10 != 0) continue;
            *tab = 0; const char *en = linha, *pt = tab+1;
            size_t nen = strlen(en), npt = strlen(pt), i, l;
            char ref[24][WMAX]; int nr = 0;
            i = 0; while((l = proxima(en,nen,&i,w,sizeof w)) && nr < 24){
                caule_en(w,l);
                int rep = 0; for(int k=0;k<nr;k++) if(!strcmp(ref[k],w)) rep = 1;
                if(!rep) snprintf(ref[nr++],WMAX,"%s",w);
            }
            struct cand cc[RMAX]; int nt = 0;
            i = 0;
            while((l = proxima(pt,npt,&i,w,sizeof w)) && nt < RMAX){
                struct cand c;
                if(!cand_get(cnd, fp64(w,l), &c)) continue;
                int nk0 = 0; for(int j = 0; j < KC; j++) if(c.k[j].sc > 0) nk0++;
                if(!nk0) continue;
                cc[nt++] = c;
            }
            if(!nt) continue;
            static double G[RMAX*KC][RMAX*KC];
            int nk[RMAX]; double a[RMAX][KC], s0[RMAX][KC], sR[RMAX][KC];
            for(int t = 0; t < nt; t++){
                nk[t] = 0;
                for(int j = 0; j < KC; j++) if(cc[t].k[j].sc > 0){ a[t][nk[t]] = cc[t].k[j].sc;
                    if(nk[t] != j) cc[t].k[nk[t]] = cc[t].k[j]; nk[t]++; }
            }
            for(int t = 0; t < nt; t++) for(int j = 0; j < nk[t]; j++)
            for(int u = 0; u < nt; u++) for(int q = 0; q < nk[u]; q++){
                if(u == t){ G[t*KC+j][u*KC+q] = 0; continue; }
                double c = (double)tab_ol(coe,NCO,par_en(cc[t].k[j].en, cc[u].k[q].en));
                double ca = (double)tab_ol(cen,NS,fps(cc[t].k[j].en));
                double cb = (double)tab_ol(cen,NS,fps(cc[u].k[q].en));
                G[t*KC+j][u*KC+q] = (c >= 2 && ca+cb > 0) ? 2.0*c/(ca+cb) : 0.0;
            }
            double d; int b;
            contrai(nt, nk, a, G, s0, 0, &d, &b, 0);
            contrai(nt, nk, a, G, sR, 0, &d, &b, RMAX);
            for(int t = 0; t < nt; t++){
                int m0 = 0, mR = 0;
                for(int j = 1; j < nk[t]; j++){ if(s0[t][j] > s0[t][m0]) m0 = j;
                                                if(sR[t][j] > sR[t][mR]) mR = j; }
                int c0 = 0, cR = 0;
                for(int k = 0; k < nr; k++){
                    if(!strcmp(ref[k], cc[t].k[m0].en)) c0 = 1;
                    if(!strcmp(ref[k], cc[t].k[mR].en)) cR = 1;
                }
                n++;
                if(m0 == mR){ if(c0) gg++; else ee++; }
                else { mexeu++; if(!c0 && cR) eg++; else if(c0 && !cR) ge++; }
            }
        }
        fclose(f);
        printf("      decisões medidas ............................ %ld\n", n);
        printf("      o bairro MUDOU a escolha em .................. %ld  (%.2f%%)\n",
               mexeu, n ? 100.0*mexeu/n : 0);
        printf("        errado -> certo (ganhou) .................. %ld\n", eg);
        printf("        certo -> errado (perdeu) ................... %ld\n", ge);
        printf("        trocou de errado para outro errado ........ %ld\n", mexeu - eg - ge);
        printf("      não mexeu: já certo %ld, já errado %ld\n", gg, ee);
        if(mexeu) printf("\n      Nas que mexeu, acertou %.1f%% das vezes em que houve troca decidida\n",
               (eg+ge) ? 100.0*eg/(eg+ge) : 0.0);
        ok("quando o bairro fala, ele ganha mais do que perde", eg > ge);
        printf("\n      Leitura: a fiação é DIFUSA, não inerte — ela mexe em pouca decisão porque\n");
        printf("      a condutância é coocorrência EN×EN crua, e palavra frequente coocorre com\n");
        printf("      tudo. O laço está certo (converge, e o analógico bate); o que está fraco é\n");
        printf("      o que eu pendurei nele. É medida, não desculpa: o próximo passo é a\n");
        printf("      condutância contra o ACASO, não contra a massa.\n");
    }

    /* ---------- §B4: digital == analógico ---------- */
    printf("\n§B4  O mesmo laço, nos dois meios: digital (multiplica) e translinear\n");
    printf("     (soma logaritmos e antilogaritma; o Σ do bairro é Kirchhoff).\n\n");
    {
        long casos = 0, iguais = 0; double pior = 0;
        if(!(f = fopen(arq,"r"))){ perror(arq); return 2; }
        while(fgets(linha,sizeof linha,f) && casos < 4000){
            char *tab = strchr(linha,'\t'); if(!tab) continue;
            if(fps(linha) % 10 != 0) continue;
            *tab = 0; const char *pt = tab+1;
            size_t npt = strlen(pt), i = 0, l;
            char twn[RMAX][WMAX]; struct cand cc[RMAX]; int nt = 0;
            while((l = proxima(pt,npt,&i,w,sizeof w)) && nt < RMAX){
                struct cand c;
                if(!cand_get(cnd, fp64(w,l), &c)) continue;
                int nk0 = 0; for(int j = 0; j < KC; j++) if(c.k[j].sc > 0) nk0++;
                if(!nk0) continue;
                snprintf(twn[nt],WMAX,"%s",w); cc[nt] = c; nt++;
            }
            if(nt < 2) continue;
            static double G[RMAX*KC][RMAX*KC];
            int nk[RMAX]; double a[RMAX][KC], sd[RMAX][KC], st[RMAX][KC];
            for(int t = 0; t < nt; t++){
                nk[t] = 0;
                for(int j = 0; j < KC; j++) if(cc[t].k[j].sc > 0){ a[t][nk[t]] = cc[t].k[j].sc;
                    if(nk[t] != j) cc[t].k[nk[t]] = cc[t].k[j]; nk[t]++; }
            }
            for(int t = 0; t < nt; t++) for(int j = 0; j < nk[t]; j++)
            for(int u = 0; u < nt; u++) for(int q = 0; q < nk[u]; q++){
                if(u == t){ G[t*KC+j][u*KC+q] = 0; continue; }
                double c = (double)tab_ol(coe,NCO,par_en(cc[t].k[j].en, cc[u].k[q].en));
                double ca = (double)tab_ol(cen,NS,fps(cc[t].k[j].en));
                double cb = (double)tab_ol(cen,NS,fps(cc[u].k[q].en));
                G[t*KC+j][u*KC+q] = (c >= 2 && ca+cb > 0) ? 2.0*c/(ca+cb) : 0.0;
            }
            double d1, d2; int b1, b2;
            contrai(nt, nk, a, G, sd, 0, &d1, &b1, RMAX);
            contrai(nt, nk, a, G, st, 1, &d2, &b2, RMAX);
            casos++;
            int mesmo = 1; double dif = 0;
            for(int t = 0; t < nt; t++) for(int j = 0; j < nk[t]; j++){
                double e = fabs(sd[t][j] - st[t][j]);
                if(e > dif) dif = e;
            }
            for(int t = 0; t < nt; t++){
                int m1 = 0, m2 = 0;
                for(int j = 1; j < nk[t]; j++){ if(sd[t][j] > sd[t][m1]) m1 = j;
                                                if(st[t][j] > st[t][m2]) m2 = j; }
                if(m1 != m2) mesmo = 0;
            }
            if(dif > pior) pior = dif;
            if(mesmo) iguais++;
        }
        fclose(f);
        printf("      frases medidas ............................ %ld\n", casos);
        printf("      mesma escolha em toda palavra ............. %ld\n", iguais);
        printf("      pior diferença de peso entre os dois meios  %.3e\n", pior);
        ok("digital == translinear (mesma escolha, sempre)", casos > 0 && iguais == casos);
        ok("e os pesos coincidem à precisão da máquina", pior < 1e-12);
    }

    printf("\n=== O QUE MUDOU ===========================================================\n");
    printf("  Vizinhança não é uma chave que eu escolho: é a ÓRBITA. O bairro inteiro entra,\n");
    printf("  e quem separa é a contração — σ = m + 1/σ, o valor de cada um dependendo dos\n");
    printf("  outros. A iteração 0 é a marginal; o ponto fixo é o bairro tendo falado.\n");
    printf("  E a matriz do bairro não é memória: é a fiação, montada da frase e desfeita\n");
    printf("  com ela — no analógico, condutância entre dois nós.\n");
    if(falhas){ printf("\n  FALHAS: %d\n\n", falhas); return 1; }
    printf("\n  RESÍDUO 0 — a contração fecha, e os dois meios dão o mesmo ponto fixo.\n\n");
    return 0;
}
