#define _POSIX_C_SOURCE 200809L
/* assistente.c — a assistente pelo CICLO VALIDADO (queda pela convolução ∘ três batidas).
 *
 * O mecanismo é EXATAMENTE o medido em ciclo.c / ciclo_analog.c (resíduo 0, digital e analógico):
 *   a fala CAI na órbita de mínimo D=|f−o|² (f·o = a convolução = a mult do corpo);
 *   a resposta = essa órbita RECONSTRUÍDA pelas três batidas ℱ³=ℱ⁻¹ (exata, Parseval).
 * Aqui ele apenas RESPONDE. A voz é a OBRA que se põe — o corpus é o espelho, nada se inventa.
 *
 *   ./assistente lexico.txt obra.txt ["a fala"]      (sem fala: modo conversa)
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* vértices + hash */
static char **vocab; static int NV=0,cap=0; static int *H,HS;
static unsigned hsf(const char*s){ unsigned h=5381; for(;*s;s++) h=((h<<5)+h)^(unsigned char)*s; return h; }
static int lookup(const char*s){ unsigned h=hsf(s)&(HS-1); while(H[h]){ if(!strcmp(vocab[H[h]-1],s)) return H[h]-1; h=(h+1)&(HS-1);} return -1; }
static void hins(int id){ unsigned h=hsf(vocab[id])&(HS-1); while(H[h]) h=(h+1)&(HS-1); H[h]=id+1; }
static int wbyte(int c){ return (c>='a'&&c<='z')||(c>='A'&&c<='Z')||(c&0x80); }
static int lowa(int c){ return (c>='A'&&c<='Z')?c+32:c; }

/* órbitas + índice */
typedef struct { int k,cnt; } Post;
static Post **post; static int *pc,*pcap; static long *Eo; static long NS=0,caps=0; static char **orb;
static void liga(int v,int k,int cnt){ if(pc[v]==pcap[v]){pcap[v]=pcap[v]?pcap[v]*2:2; post[v]=realloc(post[v],pcap[v]*sizeof(Post));} post[v][pc[v]].k=k; post[v][pc[v]].cnt=cnt; pc[v]++; }
static int toks(const char*fr,int*wv,int*wc,int maxw){ int n=0; char w[128]; int wl=0;
    for(const char*p=fr;;p++){ int c=(unsigned char)*p;
        if(c&&wbyte(c)){ if(wl<127) w[wl++]=(char)lowa(c); }
        else { if(wl){ w[wl]=0; wl=0; int id=lookup(w); if(id>=0){ int j; for(j=0;j<n;j++) if(wv[j]==id){wc[j]++;break;} if(j==n&&n<maxw){wv[n]=id;wc[n]=1;n++;} } } if(!c) break; } }
    return n; }

/* a QUEDA: a órbita de mínimo D=|f−o|² (f·o convolução + energias, Kirchhoff) */
static long *touched,*score;
static long cai(int*wv,int*wc,int nf,long Ef){
    long nt=0;
    for(int i=0;i<nf;i++){ int v=wv[i]; for(int j=0;j<pc[v];j++){ int k=post[v][j].k; if(score[k]==0) touched[nt++]=k; score[k]+=(long)wc[i]*post[v][j].cnt; } }
    long bk=-1; double bD=1e300;
    for(long i=0;i<nt;i++){ long k=touched[i]; double D=(double)Ef+Eo[k]-2.0*score[k]; if(D<bD){bD=D;bk=k;} score[k]=0; }
    return bk;
}

/* as TRÊS BATIDAS ℱ (normalizada, ℱ⁴=id) */
typedef long long i64; static const i64 P=40961;
static i64 md(i64 x){ x%=P; return x<0?x+P:x; }
static i64 mul(i64 a,i64 b){ return md(a*b); }
static i64 pot(i64 b,i64 e){ i64 r=1;b=md(b);while(e>0){if(e&1)r=mul(r,b);b=mul(b,b);e>>=1;}return r; }
#define N 256
static i64 W,RN,wp[N];
static void Fa(const i64*x,i64*X){ for(int k=0;k<N;k++){ i64 a=0; for(int j=0;j<N;j++) a=md(a+mul(x[j],wp[(int)((i64)j*k%N)])); X[k]=mul(a,RN);} }

static void responde(const char*fala){
    int wv[256],wc[256]; int n=toks(fala,wv,wc,256);
    if(!n){ printf("tiffany: (não reconheço nenhuma palavra da fala no léxico)\n"); return; }
    long Ef=0; for(int i=0;i<n;i++) Ef+=(long)wc[i]*wc[i];
    long k=cai(wv,wc,n,Ef);                            /* a fala cai na órbita de mínimo D */
    if(k<0){ printf("tiffany: (a fala não toca nenhuma órbita da obra)\n"); return; }
    const char*fr=orb[k]; int L=(int)strlen(fr); if(L>N)L=N;
    i64 b[N]={0}; for(int i=0;i<L;i++) b[i]=(unsigned char)fr[i];
    i64 X[N],t1[N],t2[N],t3[N]; Fa(b,X); Fa(X,t1); Fa(t1,t2); Fa(t2,t3);   /* ℱ³=ℱ⁻¹ reconstrói a órbita */
    int err=0; for(int i=0;i<N;i++) if(t3[i]!=b[i]) err++;
    printf("tiffany: "); for(int i=0;i<L;i++){ int c=(int)t3[i]; putchar(c>=32&&c<256?c:' '); }
    printf("   [ℱ³=ℱ⁻¹, resíduo %d]\n", err);
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
    touched=malloc(NS*sizeof(long)); score=calloc(NS,sizeof(long));
    W=pot(3,(P-1)/N); RN=pot(16,P-2); wp[0]=1; for(int t=1;t<N;t++) wp[t]=mul(wp[t-1],W);
    printf("VÉRTICES %d · ÓRBITAS %ld  (obra: %s)\n\n", NV, NS, obra);

    if(argc>3){ printf("você: %s\n\n", argv[3]); responde(argv[3]); }
    else { char l[2048]; printf("você: "); fflush(stdout);
           while(fgets(l,sizeof l,stdin)){ l[strcspn(l,"\n")]=0; if(l[0]){ putchar('\n'); responde(l); putchar('\n'); } printf("você: "); fflush(stdout); } }
    return 0;
}
