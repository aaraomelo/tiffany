#define _POSIX_C_SOURCE 200809L
/* ciclo.c — a assistente completa: a QUEDA (convolução) ∘ as TRÊS BATIDAS (ℱ³=ℱ⁻¹), medida.
 *
 *   a fala CAI na órbita de mínimo D=|f−o|² (a convolução f·o = a mult do corpo);
 *   a resposta RECONSTRÓI essa órbita pelas três batidas ℱ³=ℱ⁻¹ (ℱ⁴=id), exata (Parseval).
 *
 * O ciclo, medido (o juiz):
 *   §1  se a fala É uma órbita → cai nela (D=0) e a reconstrução volta byte a byte → resíduo 0.
 *   §2  DENTE (reconstrução): uma batida (ℱ¹) não é a inversa — não reconstrói. quebra.
 *   §3  DENTE (queda): a convolução crua max(f·o), sem as energias, cai na órbita errada. quebra.
 *
 *   cc -O2 -std=c99 ciclo.c -o ciclo && ./ciclo lexico.txt obra.txt
 */
#include <stdio.h>
#include "unidade.h"
#include <stdlib.h>
#include <string.h>

/* ---------- vértices + hash ---------- */
static char **vocab; static int NV=0,cap=0; static int *H,HS;
static unsigned hsf(const char*s){ unsigned h=5381; for(;*s;s++) h=((h<<5)+h)^(unsigned char)*s; return h; }
static int lookup(const char*s){ unsigned h=hsf(s)&(HS-1); while(H[h]){ if(!strcmp(vocab[H[h]-1],s)) return H[h]-1; h=(h+1)&(HS-1);} return -1; }
static void hins(int id){ unsigned h=hsf(vocab[id])&(HS-1); while(H[h]) h=(h+1)&(HS-1); H[h]=id+1; }
static int wbyte(int c){ return (c>='a'&&c<='z')||(c>='A'&&c<='Z')||(c&0x80); }
static int lowa(int c){ return (c>='A'&&c<='Z')?c+32:c; }

/* ---------- órbitas: texto, energia |o|², índice post[v]=(k,cnt) ---------- */
typedef struct { int k,cnt; } Post;
static Post **post; static int *pc,*pcap; static long *Eo; static long NS=0,caps=0; static char **orb;
static void liga(int v,int k,int cnt){ if(pc[v]==pcap[v]){pcap[v]=pcap[v]?pcap[v]*2:2; post[v]=realloc(post[v],pcap[v]*sizeof(Post));} post[v][pc[v]].k=k; post[v][pc[v]].cnt=cnt; pc[v]++; }
static int toks(const char*fr,int*wv,int*wc,int maxw){ int n=0; char w[128]; int wl=0;
    for(const char*p=fr;;p++){ int c=(unsigned char)*p;
        if(c&&wbyte(c)){ if(wl<127) w[wl++]=(char)lowa(c); }
        else { if(wl){ w[wl]=0; wl=0; int id=lookup(w); if(id>=0){ int j; for(j=0;j<n;j++) if(wv[j]==id){wc[j]++;break;} if(j==n&&n<maxw){wv[n]=id;wc[n]=1;n++;} } } if(!c) break; } }
    return n; }

/* a QUEDA: mínimo de D (retorna k'); e a órbita de max(f·o) cru (o dente da queda) */
static long *score,*touched;
static long cai(int*wv,int*wc,int nf,long Ef,long*outD,long*outCru){
    long nt=0;
    for(int i=0;i<nf;i++){ int v=wv[i]; for(int j=0;j<pc[v];j++){ int k=post[v][j].k; if(score[k]==0) touched[nt++]=k; score[k]+=(long)wc[i]*post[v][j].cnt; } }
    long bk=-1; double bD=1e300; long ck=-1,cc=-1;
    for(long i=0;i<nt;i++){ long k=touched[i]; long fo=score[k]; double D=(double)Ef+Eo[k]-2.0*fo;
        if(D<bD){bD=D;bk=k;} if(fo>cc){cc=fo;ck=k;} score[k]=0; }
    if(outD)*outD=(long)bD; if(outCru)*outCru=ck; return bk;
}

/* ---------- as TRÊS BATIDAS: ℱ (normalizada, ℱ⁴=id) ---------- */
typedef long long i64;
static const i64 P=40961;
static i64 md(i64 x){ x%=P; return x<0?x+P:x; }
static i64 mul(i64 a,i64 b){ return md(a*b); }
static i64 pot(i64 b,i64 e){ i64 r=1;b=md(b);while(e>0){if(e&1)r=mul(r,b);b=mul(b,b);e>>=1;}return r; }
#define N 256
static i64 W,RN,wp[N];
static void Fa(const i64*x,i64*X){ for(int k=0;k<N;k++){ i64 a=0; for(int j=0;j<N;j++) a=md(a+mul(x[j],wp[(int)((i64)j*k%N)])); X[k]=mul(a,RN);} }

int main(int argc,char**argv){
    const char*lex=argc>1?argv[1]:"lexico.txt"; const char*obra=argc>2?argv[2]:"por.tsv";
    FILE*f=fopen(lex,"rb"); if(!f){fprintf(stderr,"não abri %s\n",lex);return 2;} char line[4096];
    while(fgets(line,sizeof line,f)){ line[strcspn(line,"\r\n")]=0; if(!line[0])continue; if(NV==cap){cap=cap?cap*2:1024;vocab=realloc(vocab,cap*sizeof(char*));} vocab[NV++]=strdup(line);} fclose(f);
    HS=1; while(HS<NV*2)HS<<=1; H=calloc(HS,sizeof(int)); for(int i=0;i<NV;i++)hins(i);
    post=calloc(NV,sizeof(Post*)); pc=calloc(NV,sizeof(int)); pcap=calloc(NV,sizeof(int));
    f=fopen(obra,"rb"); if(!f){fprintf(stderr,"não abri %s\n",obra);return 2;} int wv[256],wc[256];
    while(fgets(line,sizeof line,f)){ line[strcspn(line,"\r\n")]=0; char*fr=line; int tb=0; for(char*p=line;*p;p++)if(*p=='\t')tb++; if(tb>=2){char*p=line;int t=0;while(*p&&t<2){if(*p=='\t')t++;p++;}fr=p;} if(!fr[0])continue;
        if(NS==caps){caps=caps?caps*2:4096;orb=realloc(orb,caps*sizeof(char*));Eo=realloc(Eo,caps*sizeof(long));}
        orb[NS]=strdup(fr); int n=toks(fr,wv,wc,256); long E=0; for(int i=0;i<n;i++){liga(wv[i],(int)NS,wc[i]);E+=(long)wc[i]*wc[i];} Eo[NS]=E; NS++; }
    fclose(f);
    score=calloc(NS,sizeof(long)); touched=malloc(NS*sizeof(long));
    W=pot(3,(P-1)/N); RN=pot(16,P-2); wp[0]=1; for(int t=1;t<N;t++) wp[t]=mul(wp[t-1],W);
    printf("VÉRTICES %d · ÓRBITAS %ld  (obra: %s)\n\n", NV, NS, obra);

    long amostra = NS<20000?NS:20000;
    long falha_ciclo=0, dente_rec=0, dente_q=0, testes=0;
    for(long k=0;k<amostra;k++){
        int n=toks(orb[k],wv,wc,256); if(!n) continue; long Ef=0; for(int i=0;i<n;i++) Ef+=(long)wc[i]*wc[i];
        long minD, cruk; long kq=cai(wv,wc,n,Ef,&minD,&cruk); if(kq<0) continue;
        testes++;
        /* §1 o ciclo: a queda cai na órbita (D=0) e as 3 batidas reconstroem EXATO */
        const char*resp=orb[kq]; int L=(int)strlen(resp); if(L>N) L=N;
        i64 bloco[N]={0}; for(int i=0;i<L;i++) bloco[i]=(unsigned char)resp[i];
        i64 X[N],t1[N],t2[N],t3[N]; Fa(bloco,X); Fa(X,t1); Fa(t1,t2); Fa(t2,t3);
        int errR=0; for(int i=0;i<N;i++) if(t3[i]!=bloco[i]) errR++;
        if(minD!=0 || errR!=0) falha_ciclo++;
        /* §2 o dente da reconstrução: UMA batida (ℱ¹) não é a inversa */
        int e1=0; for(int i=0;i<N;i++) if(X[i]!=bloco[i]) e1++; if(e1==0) dente_rec++;  /* se ℱ¹==id, o dente NÃO quebra */
        /* §3 o dente da queda: a convolução crua max(f·o) cai numa órbita ≠ a de D mínimo */
        if(cruk==kq) dente_q++;                          /* se crua==D, o dente NÃO quebra */
    }
    printf("§1  O CICLO (fala=órbita → cai D=0 → 3 batidas reconstroem exato): %ld testes, %ld falhas.  %s\n",
           testes, falha_ciclo, VD(falha_ciclo, "resíduo 0"));
    printf("§2  DENTE reconstrução (ℱ¹ ≠ inversa): NÃO quebra em %ld/%ld.  %s\n",
           dente_rec, testes, dente_rec?"FALHA (ℱ¹ reconstruiu?!)":"o dente quebra (só 3 batidas reconstroem)");
    printf("§3  DENTE queda (max f·o cru ≠ min D): NÃO quebra em %ld/%ld.  %s\n",
           dente_q, testes, (dente_q>testes/2)?"FALHA (a crua bastaria?)":"o dente quebra (D, com as energias, é o certo)");
    printf("\n    a assistente = QUEDA (convolução, recursao.c) ∘ TRÊS BATIDAS (ℱ³=ℱ⁻¹, tres_reconstroi.c),\n");
    printf("    ambas já validadas; o ciclo fecha. Analógico: banco de correlacionadores + winner-take-all\n");
    printf("    (§B.4/B.5) e a transformada (§B) — a con/deconvolução do gabarito.\n");
    int ok = (!falha_ciclo && !dente_rec && dente_q<=testes/2);
    printf("\n%s\n", ok?"RESÍDUO 0 — o ciclo da assistente é a teoria, medido.":"REVER");
    return ok?0:1;
}
