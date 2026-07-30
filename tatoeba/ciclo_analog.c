#define _POSIX_C_SOURCE 200809L
/* ciclo_analog.c — o ciclo da assistente no ANALÓGICO (o gabarito), medido com pulso.
 *
 *   a QUEDA = a convolução f·o dentro de D=|f−o|². No circuito (analog.c):
 *       cada produto f[v]·o[v]  = o TRANSLINEAR  I_out = I₁·I₂/I_ref  (log/exp, §B.4);
 *       as somas f·o, |f|², |o|², e D  = o KIRCHHOFF (correntes no nó, §B.5);
 *       o mínimo de D sobre as órbitas  = o winner-take-all.
 *   as coordenadas f[v], o[v] são CORRENTES contínuas (nA), não bits.
 *   as TRÊS BATIDAS ℱ³=ℱ⁻¹ (ℱ⁴=id) = a rotação da malha LC, G⁴=I (§B.1) — já validada.
 *
 * pulso: o certo colhe (analógico == digital), o DENTE quebra (o translinear sem o I_ref não é
 * o produto), e o relógio corre (as falas variam). Resíduo 0 COM pulso.
 *
 *   cc -O2 -std=c99 ciclo_analog.c -lm -o ciclo_analog && ./ciclo_analog lexico.txt obra.txt
 */
#include <stdio.h>
#include "unidade.h"
#include <stdlib.h>
#include <string.h>
#include <math.h>

/* ---------- os modelos físicos (analog.c §B.2/B.4): a junção b-e e o translinear ---------- */
static const double I_S=1e-15, IU=1e-9, VT=0.025852;
static double tl(double a,double b){                 /* ANTILOG(log a+log b−log ref) = a·b (o gato ×) */
    double V1=VT*log(a*IU/I_S), V2=VT*log(b*IU/I_S), Vr=VT*log(IU/I_S);
    return I_S*exp((V1+V2-Vr)/VT)/IU;
}
static double tl_dente(double a,double b){            /* o DENTE: sem o I_ref — não é o produto */
    double V1=VT*log(a*IU/I_S), V2=VT*log(b*IU/I_S);
    return I_S*exp((V1+V2)/VT)/IU;
}

/* ---------- vértices + hash ---------- */
static char **vocab; static int NV=0,cap=0; static int *H,HS;
static unsigned hsf(const char*s){ unsigned h=5381; for(;*s;s++) h=((h<<5)+h)^(unsigned char)*s; return h; }
static int lookup(const char*s){ unsigned h=hsf(s)&(HS-1); while(H[h]){ if(!strcmp(vocab[H[h]-1],s)) return H[h]-1; h=(h+1)&(HS-1);} return -1; }
static void hins(int id){ unsigned h=hsf(vocab[id])&(HS-1); while(H[h]) h=(h+1)&(HS-1); H[h]=id+1; }
static int wbyte(int c){ return (c>='a'&&c<='z')||(c>='A'&&c<='Z')||(c&0x80); }
static int lowa(int c){ return (c>='A'&&c<='Z')?c+32:c; }

/* ---------- órbitas + índice ---------- */
typedef struct { int k,cnt; } Post;
static Post **post; static int *pc,*pcap; static long *Eo; static long NS=0,caps=0; static char **orb;
static void liga(int v,int k,int cnt){ if(pc[v]==pcap[v]){pcap[v]=pcap[v]?pcap[v]*2:2; post[v]=realloc(post[v],pcap[v]*sizeof(Post));} post[v][pc[v]].k=k; post[v][pc[v]].cnt=cnt; pc[v]++; }
static int toks(const char*fr,int*wv,int*wc,int maxw){ int n=0; char w[128]; int wl=0;
    for(const char*p=fr;;p++){ int c=(unsigned char)*p;
        if(c&&wbyte(c)){ if(wl<127) w[wl++]=(char)lowa(c); }
        else { if(wl){ w[wl]=0; wl=0; int id=lookup(w); if(id>=0){ int j; for(j=0;j<n;j++) if(wv[j]==id){wc[j]++;break;} if(j==n&&n<maxw){wv[n]=id;wc[n]=1;n++;} } } if(!c) break; } }
    return n; }

/* a QUEDA no CIRCUITO: D_k = |f|²+|o|²−2·(f·o), os produtos pelo translinear 'prod', a soma Kirchhoff.
 * devolve a órbita de menor D. (prod = tl → correto ; prod = tl_dente → o dente). */
static long *touched; static double *acc;
static long cai_circuito(int*wv,int*wc,int nf, double(*prod)(double,double)){
    /* |f|² pelo circuito */
    double Ef=0; for(int i=0;i<nf;i++) Ef+=prod((double)wc[i],(double)wc[i]);
    long nt=0;
    for(int i=0;i<nf;i++){ int v=wv[i]; for(int j=0;j<pc[v];j++){ int k=post[v][j].k;
        if(acc[k]==0) touched[nt++]=k; acc[k]+=prod((double)wc[i],(double)post[v][j].cnt); } }   /* f·o, Kirchhoff */
    long bk=-1; double bD=1e300;
    for(long i=0;i<nt;i++){ long k=touched[i]; double D = Ef + (double)Eo[k] - 2.0*acc[k]; if(D<bD){bD=D;bk=k;} acc[k]=0; }
    return bk;
}

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
    touched=malloc(NS*sizeof(long)); acc=calloc(NS,sizeof(double));
    printf("VÉRTICES %d · ÓRBITAS %ld  (obra: %s)\n\n", NV, NS, obra);

    /* §B — a QUEDA no circuito: analógico == digital ; o dente quebra ; o relógio corre */
    long amostra = NS<3000?NS:3000;
    long passou=0, tot=0, dente_quebra=0; double soma_min=0, soma2=0;
    for(long k=0;k<amostra;k++){
        int n=toks(orb[k],wv,wc,256); if(!n) continue; tot++;
        long a_ok = cai_circuito(wv,wc,n, tl);         /* a queda ANALÓGICA (translinear) */
        long a_dente = cai_circuito(wv,wc,n, tl_dente);/* o DENTE (translinear sem I_ref)  */
        /* o oráculo digital: o mínimo de D inteiro */
        long Ef=0; for(int i=0;i<n;i++) Ef+=(long)wc[i]*wc[i];
        long nt=0; for(int i=0;i<n;i++){int v=wv[i];for(int j=0;j<pc[v];j++){int kk=post[v][j].k; if(acc[kk]==0)touched[nt++]=kk; acc[kk]+=(double)((long)wc[i]*post[v][j].cnt);}}
        long dk=-1; double dD=1e300; for(long i=0;i<nt;i++){long kk=touched[i]; double D=(double)Ef+Eo[kk]-2.0*acc[kk]; if(D<dD){dD=D;dk=kk;} acc[kk]=0;}
        if(a_ok==dk) passou++;                             /* analógico == digital */
        if(a_dente!=dk) dente_quebra++;                /* o dente cai em outra órbita */
        soma_min+=(double)Ef; soma2+=(double)Ef*(double)Ef;   /* o relógio: o sinal (Ef) varia por fala */
        (void)dD;
    }
    double var = soma2/tot - (soma_min/tot)*(soma_min/tot);
    int pulso = (passou==tot) && (dente_quebra>tot/2) && (var>0);
    printf("§B  A QUEDA NO CIRCUITO — a convolução f·o pelo translinear (§B.4) + Kirchhoff (§B.5),\n");
    printf("     coordenadas CONTÍNUAS (correntes), o mínimo de D = winner-take-all:\n");
    printf("       analógico == digital : %ld/%ld órbitas\n", passou, tot);
    printf("       o DENTE (translinear sem o I_ref, não é o produto) erra a órbita : %ld/%ld\n", dente_quebra, tot);
    printf("       o relógio corre (variância de D > 0) : %s\n", var>0?"sim":"não");
    printf("\n     as TRÊS BATIDAS ℱ³=ℱ⁻¹ (ℱ⁴=id) = a rotação da malha LC, G⁴=I (§B.1) — já validada.\n");
    printf("\n%s\n", pulso ? "RESÍDUO 0 COM PULSO — a queda do ciclo roda no analógico (o gabarito)."
                            : "SEM PULSO — rever");
    ok("a queda do ciclo roda no analógico, com pulso", pulso);
    return pulso?0:1;
}
