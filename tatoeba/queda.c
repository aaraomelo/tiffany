#define _POSIX_C_SOURCE 200809L
/* queda.c — a QUEDA pela convolução no grafo de órbitas, VALIDADA pela teoria.
 *
 *   VÉRTICES — as palavras (o léxico).  ÓRBITAS — as frases (cada frase é um sinal o[v] =
 *   ocorrências da palavra v). A fala é f[v]. A fala CAI na órbita de mínimo
 *
 *        D = |f − o|² = |f|² + |o|² − 2·(f·o)          (f·o = a convolução = a mult do corpo)
 *
 *   é a MESMA queda do tiffany.c (o mínimo de D), sobre os vértices em vez dos bytes.
 *
 * O JUIZ é a medição:
 *   §1  AUTO-QUEDA — se a fala É uma órbita do corpus, a queda cai NELA (D=0, o mínimo). resíduo 0.
 *   §2  O DENTE   — a convolução crua max(f·o), SEM as energias |o|², erra: cai na frase que repete
 *                   a palavra (|o|² maior). Se o dente não quebrasse, D estaria errado.
 *
 *   cc -O2 -std=c99 queda.c -o queda && ./queda lexico.txt obra.txt
 */
#include <stdio.h>
#include "unidade.h"
#include <stdlib.h>
#include <string.h>

/* vértices + hash */
static char **vocab; static int NV=0,cap=0;
static int *H, HS;
static unsigned hsf(const char*s){ unsigned h=5381; for(;*s;s++) h=((h<<5)+h)^(unsigned char)*s; return h; }
static int lookup(const char*s){ unsigned h=hsf(s)&(HS-1); while(H[h]){ if(!strcmp(vocab[H[h]-1],s)) return H[h]-1; h=(h+1)&(HS-1);} return -1; }
static void hins(int id){ unsigned h=hsf(vocab[id])&(HS-1); while(H[h]) h=(h+1)&(HS-1); H[h]=id+1; }
static int wbyte(int c){ return (c>='a'&&c<='z')||(c>='A'&&c<='Z')||(c&0x80); }
static int lowa(int c){ return (c>='A'&&c<='Z')?c+32:c; }

/* órbitas: por órbita a energia |o|²; o índice post[v] = lista de (órbita k, contagem de v em k) */
typedef struct { int k, cnt; } Post;
static Post **post; static int *pc,*pcap;
static long *Eo;                         /* |o_k|² por órbita */
static long NS=0, caps=0;
static char **orb;

static void liga(int v,int k,int cnt){
    if(pc[v]==pcap[v]){ pcap[v]=pcap[v]?pcap[v]*2:2; post[v]=realloc(post[v],pcap[v]*sizeof(Post)); }
    post[v][pc[v]].k=k; post[v][pc[v]].cnt=cnt; pc[v]++;
}

/* tokeniza uma frase em (ids, contagens): retorna nº de palavras distintas em wv/wc */
static int toks(const char*fr, int*wv, int*wc, int maxw){
    int n=0; char w[128]; int wl=0;
    for(const char*p=fr; ; p++){ int c=(unsigned char)*p;
        if(c&&wbyte(c)){ if(wl<127) w[wl++]=(char)lowa(c); }
        else { if(wl){ w[wl]=0; wl=0; int id=lookup(w);
                   if(id>=0){ int j; for(j=0;j<n;j++) if(wv[j]==id){ wc[j]++; break; }
                              if(j==n && n<maxw){ wv[n]=id; wc[n]=1; n++; } } }
               if(!c) break; } }
    return n;
}

/* a queda: dado (fala como wv/wc, energia Ef), a órbita de mínimo D — e a de máximo f·o cru (o dente) */
static long *score, *touched;
static long cai(int*wv,int*wc,int nf,long Ef,int excl,long*outMinD,long*outMaxC_k){
    long nt=0;
    for(int i=0;i<nf;i++){ int v=wv[i]; for(int j=0;j<pc[v];j++){ int k=post[v][j].k; if(score[k]==0) touched[nt++]=k;
                                                                  score[k]+=(long)wc[i]*post[v][j].cnt; } }
    long bestk=-1; double bestD=1e300; long cruk=-1, cruc=-1;
    for(long i=0;i<nt;i++){ long k=touched[i]; if(k==excl){ score[k]=0; continue; }
        long fo=score[k]; double D=(double)Ef+Eo[k]-2.0*fo;
        if(D<bestD){ bestD=D; bestk=k; }
        if(fo>cruc){ cruc=fo; cruk=k; }                 /* o dente: max(f·o) cru */
        score[k]=0; }
    for(long i=0;i<nt;i++) score[touched[i]]=0;
    if(outMinD) *outMinD=(long)bestD; if(outMaxC_k) *outMaxC_k=cruk;
    return bestk;
}

int main(int argc,char**argv){
    const char*lex=argc>1?argv[1]:"lexico.txt";
    const char*obra=argc>2?argv[2]:"por.tsv";
    /* vértices */
    FILE*f=fopen(lex,"rb"); if(!f){fprintf(stderr,"não abri %s\n",lex);return 2;}
    char line[4096];
    while(fgets(line,sizeof line,f)){ line[strcspn(line,"\r\n")]=0; if(!line[0])continue;
        if(NV==cap){cap=cap?cap*2:1024; vocab=realloc(vocab,cap*sizeof(char*));} vocab[NV++]=strdup(line); }
    fclose(f);
    HS=1; while(HS<NV*2)HS<<=1; H=calloc(HS,sizeof(int)); for(int i=0;i<NV;i++)hins(i);
    post=calloc(NV,sizeof(Post*)); pc=calloc(NV,sizeof(int)); pcap=calloc(NV,sizeof(int));
    /* órbitas + índice + energias */
    f=fopen(obra,"rb"); if(!f){fprintf(stderr,"não abri %s\n",obra);return 2;}
    int wv[256],wc[256];
    while(fgets(line,sizeof line,f)){ line[strcspn(line,"\r\n")]=0; char*fr=line; int tb=0;
        for(char*p=line;*p;p++) if(*p=='\t')tb++; if(tb>=2){char*p=line;int t=0;while(*p&&t<2){if(*p=='\t')t++;p++;}fr=p;}
        if(!fr[0])continue;
        if(NS==caps){caps=caps?caps*2:4096; orb=realloc(orb,caps*sizeof(char*)); Eo=realloc(Eo,caps*sizeof(long));}
        long k=NS; orb[NS]=strdup(fr);
        int n=toks(fr,wv,wc,256); long E=0; for(int i=0;i<n;i++){ liga(wv[i],k,wc[i]); E+=(long)wc[i]*wc[i]; }
        Eo[NS]=E; NS++;
    }
    fclose(f);
    score=calloc(NS,sizeof(long)); touched=malloc(NS*sizeof(long));
    printf("VÉRTICES %d · ÓRBITAS %ld  (obra: %s)\n\n", NV, NS, obra);

    /* §1 AUTO-QUEDA + §2 O DENTE: cada órbita como fala deve cair nela (D=0); o dente (max c cru) erra */
    long amostra = NS<20000 ? NS : 20000;              /* mede uma amostra grande */
    long falhaD=0, denteQuebra=0, testes=0;
    for(long k=0;k<amostra;k++){
        int n=toks(orb[k],wv,wc,256); if(n==0) continue; long Ef=0; for(int i=0;i<n;i++) Ef+=(long)wc[i]*wc[i];
        long minD, cru_k;
        long qk=cai(wv,wc,n,Ef,-1,&minD,&cru_k);
        testes++;
        /* a queda por D deve dar D=0 (cai na própria órbita ou numa de mesma bag) */
        if(minD!=0) falhaD++;
        /* o dente: a queda por max(f·o) cru é DIFERENTE da queda por D (erra) em ao menos alguns casos */
        if(cru_k!=qk) denteQuebra++;
        (void)qk;
    }
    printf("§1  AUTO-QUEDA (a fala É uma órbita → cai nela, D=0): %ld testes, %ld com D≠0.  %s\n",
           testes, falhaD, VD(falhaD, "resíduo 0"));
    printf("§2  O DENTE (max f·o cru ≠ min D): quebra em %ld/%ld casos.  %s\n",
           denteQuebra, testes, denteQuebra? "o dente quebra (D, com as energias, é o certo)":"o dente NÃO quebra — D seria supérfluo");
    printf("\n    a queda é a convolução f·o (a mult do corpo, recursao.c) dentro de D=|f−o|²;\n");
    printf("    no analógico: f·o = o banco de correlacionadores (§B.4/B.5), o mínimo = winner-take-all.\n");
    printf("\n%s\n", (!falhaD && denteQuebra) ? "RESÍDUO 0 — a queda pela convolução é a teoria, medida." : "REVER");
    return (falhaD || !denteQuebra) ? 1 : 0;
}
