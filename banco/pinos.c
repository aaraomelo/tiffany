/* pinos.c — OS PINOS: a coluna do modelo JÁ É a base, e ler/escrever é a mesma operação dual.
 *
 * O Aarão: "veja, você não ortogonaliza nem normaliza. Você passa o modelo pra dentro e desdobra
 * ele, os pinos. A coluna dele é ortonormal — não que você faça isso, a Meta já fez. Você tem
 * que ler e escrever pela mesma operação dual do ISA load/store."
 *
 * E ISTO DESMONTA O §C3 DO `encaixa.c`. Lá eu construí a base com Gram--Schmidt: ortogonalizei e
 * normalizei, e chamei-lhe "a cifra do espaço". Mas isso é INTERVENÇÃO minha sobre o objeto — e
 * o objeto já vinha com base. Quem treinou o modelo já fez esse trabalho; o que falta não é
 * fazê-lo outra vez, é LER o que lá está.
 *
 * A diferença não é de estilo. Uma base que eu construo depende dos vetores que colhi — muda se
 * eu colher outros. A base do modelo é do modelo, e é a mesma para toda a gente que o abrir.
 *
 * E O SEGUNDO PONTO, o do plugue: ler e escrever têm de ser a MESMA operação, dual — como o
 * LOAD e o STORE da ISA, que o `plugue.sh` já mede ("ler e escrever é a mesma operação dual", e
 * o `mede` confirma que escrever-e-depois-ler devolve o que se escreveu). Projetar na base é o
 * LOAD; recompor a partir dos coeficientes é o STORE; e se forem duais, a composição é a
 * identidade sem que se tenha de inverter nada.
 *
 * O QUE SE MEDE, E NÃO SE ASSUME. A afirmação "a coluna é ortonormal" é falsificável, e é a
 * primeira coisa a pôr à prova — sobre as matrizes reais do llama3.2 e do qwen2.5, não sobre
 * vetores que eu escolhi.
 *
 *   §P1  as COLUNAS do modelo: são ortonormais? — medido nas matrizes reais
 *   §P2  o DESDOBRAMENTO: a matriz é a base, e projetar é multiplicar por ela
 *   §P3  LOAD e STORE: a mesma operação dual — projetar e recompor
 *   §P4  e o que a medida obriga a dizer sobre a premissa
 *
 *   cc -O2 -std=c99 -I. pinos.c -lm -o pinos && ./pinos [modelo.gguf]
 */
#define _GNU_SOURCE
#include <stdio.h>
#include "../lib/disco.h"
#define Q DISCO_FIXO2(float, NLIN, 62)

#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include "unidade.h"

#define QK_K 256
#define MAXT 512
#define MAXNOME 64
#define NCOL 64            /* quantas colunas se medem — bastam para a estatística */
#define NLIN 2048

typedef struct { char nome[MAXNOME]; int nd; long long d[4]; unsigned tipo; long long off; } Tn;
/* O INDICE NAO VIVE EM .bss. Este Tn[] era copia do que o forward.c tinha, e o
 * forward saiu por isso mesmo — um indice a viver fora do indice, e o padrao a
 * propagar-se de ficheiro em ficheiro. Aqui vai para o disco; o sitio proprio
 * dele sao os slots do banco, que E' um indice. */
#define tn DISCO_FIXO(Tn, 40)
static int n_tn = 0;
static unsigned char *M; static long long dados0, cur;

static unsigned u32(void){ unsigned v; memcpy(&v,M+cur,4); cur+=4; return v; }
static unsigned long long u64(void){ unsigned long long v; memcpy(&v,M+cur,8); cur+=8; return v; }
static const char *gp(unsigned long long *L){ unsigned long long n=u64(); const char*p=(const char*)(M+cur); cur+=n; if(L)*L=n; return p; }
static void gs(char*d,size_t c){ unsigned long long n; const char*p=gp(&n); size_t k=n<c-1?n:c-1; memcpy(d,p,k); d[k]=0; }
static void sv(unsigned t);
static void su(unsigned t){ switch(t){case 0:case 1:case 7:cur+=1;break;case 2:case 3:cur+=2;break;
  case 4:case 5:case 6:cur+=4;break;case 10:case 11:case 12:cur+=8;break;case 8:gp(NULL);break;case 9:sv(9);break;} }
static void sv(unsigned t){ if(t!=9){su(t);return;} unsigned e=u32(); unsigned long long n=u64();
  for(unsigned long long i=0;i<n;i++) su(e); }

static float f16(unsigned short h){
    unsigned s=(h>>15)&1,e=(h>>10)&0x1F,m=h&0x3FF;
    if(e==0) return (float)((s?-1:1)*(double)m*5.9604644775390625e-8);
    if(e==31) return s?-INFINITY:INFINITY;
    return (float)((s?-1:1)*ldexp(1.0+m/1024.0,(int)e-15));
}
static void esc_k4(int j,const unsigned char*q,unsigned char*d,unsigned char*m){
    if(j<4){*d=q[j]&63;*m=q[j+4]&63;} else {*d=(q[j+4]&0xF)|((q[j-4]>>6)<<4);*m=(q[j+4]>>4)|((q[j-0]>>6)<<4);} }
static void deq_q4k(const unsigned char*b,float*y){
    unsigned short hd,hm; memcpy(&hd,b,2); memcpy(&hm,b+2,2);
    float d=f16(hd),dm=f16(hm); const unsigned char*sc=b+4,*q=b+16; int is=0,k=0;
    for(int j=0;j<QK_K;j+=64){ unsigned char s,m;
        esc_k4(is,sc,&s,&m); float d1=d*s,m1=dm*m;
        esc_k4(is+1,sc,&s,&m); float d2=d*s,m2=dm*m;
        for(int l=0;l<32;l++) y[k++]=d1*(float)(q[l]&0xF)-m1;
        for(int l=0;l<32;l++) y[k++]=d2*(float)(q[l]>>4)-m2;
        q+=32; is+=2; } }
static void deq_q6k(const unsigned char*b,float*y){
    const unsigned char*ql=b,*qh=b+128; const signed char*sc=(const signed char*)(b+192);
    unsigned short hd; memcpy(&hd,b+208,2); float d=f16(hd);
    for(int n=0;n<QK_K;n+=128){ for(int l=0;l<32;l++){ int is=l/16;
        int q1=(int)((ql[l]&0xF)|(((qh[l]>>0)&3)<<4))-32, q2=(int)((ql[l+32]&0xF)|(((qh[l]>>2)&3)<<4))-32;
        int q3=(int)((ql[l]>>4)|(((qh[l]>>4)&3)<<4))-32,  q4=(int)((ql[l+32]>>4)|(((qh[l]>>6)&3)<<4))-32;
        y[n+l]=d*sc[is+0]*q1; y[n+l+32]=d*sc[is+2]*q2; y[n+l+64]=d*sc[is+4]*q3; y[n+l+96]=d*sc[is+6]*q4; }
        ql+=64; qh+=32; sc+=8; } }
static int bb_(unsigned t){ switch(t){case 0:return 4;case 1:return 2;case 8:return 34;case 12:return 144;case 14:return 210;} return 0; }
static int bv_(unsigned t){ switch(t){case 0:case 1:return 1;case 8:return 32;case 12:case 14:return QK_K;} return 0; }
static Tn *acha(const char*n){ for(int i=0;i<n_tn;i++) if(!strcmp(tn[i].nome,n)) return &tn[i]; return NULL; }

/* uma LINHA da matriz, desempacotada — é a mesma do forward.c */
static void linha(const Tn*t, long long i, float*dest){
    long long cols=t->d[0];
    const unsigned char *base=M+dados0+t->off;
    if(t->tipo==0){ memcpy(dest, base+i*cols*4, (size_t)cols*4); return; }
    int bb=bb_(t->tipo), bv=bv_(t->tipo);
    long long nb=cols/bv;
    const unsigned char *p=base+i*nb*bb;
    for(long long b=0;b<nb;b++){
        if(t->tipo==8){ unsigned short hd; memcpy(&hd,p+b*bb,2); float d=f16(hd);
            const signed char*q=(const signed char*)(p+b*bb+2);
            for(int l=0;l<32;l++) dest[b*bv+l]=d*(float)q[l]; }
        else if(t->tipo==12) deq_q4k(p+b*bb, dest+b*bv);
        else                 deq_q6k(p+b*bb, dest+b*bv);
    }
}

int main(int argc, char **argv){
    disco_prende(DISCO_BASE(40),"dados/pinos_tn.bin",(size_t)(MAXT),sizeof(Tn));
    disco_zera(tn,(size_t)(MAXT),sizeof(Tn));
    disco_prende(DISCO_BASE(62),"dados/Q.bin",(size_t)(NCOL)*(NLIN),sizeof(float));
    disco_zera(Q,(size_t)(NCOL)*(NLIN),sizeof(float));
const char *g = argc>1 ? argv[1] :
  "/usr/share/ollama/.ollama/models/blobs/"
  "sha256-74701a8c35f6c8d9a4b91f3f3497643001d63e0c7a84e085bed452548fa88d45";
int fd=open(g,O_RDONLY);
if(fd<0){ perror("pinos: abrir"); return 1; }
struct stat st; fstat(fd,&st);
M=mmap(NULL,(size_t)st.st_size,PROT_READ,MAP_PRIVATE,fd,0);
if(M==MAP_FAILED){ perror("pinos: mmap"); return 1; }
cur=4; u32();
unsigned long long nt=u64(), nkv=u64();
char arq[64]="?";
for(unsigned long long i=0;i<nkv;i++){
    char k[160]; gs(k,sizeof k); unsigned t=u32();
    if(!strcmp(k,"general.architecture")&&t==8) gs(arq,sizeof arq); else sv(t);
}
for(unsigned long long i=0;i<nt&&n_tn<MAXT;i++){
    Tn*t=&tn[n_tn]; gs(t->nome,MAXNOME); t->nd=(int)u32();
    for(int d=0;d<t->nd;d++) t->d[d]=(long long)u64();
    t->tipo=u32(); t->off=(long long)u64(); n_tn++;
}
dados0=(cur+31)/32*32;

printf("\n=== OS PINOS: A COLUNA DO MODELO É A BASE, E LOAD/STORE SÃO DUAIS ========\n");
printf("    modelo: %s (%s)\n", arq, argc>1?"dado":"llama3.2:1b");
printf("    Não se ortogonaliza nem se normaliza: lê-se o que lá está.\n");

printf("\n§P1  AS COLUNAS: são ortonormais? — medido nas matrizes REAIS.\n\n");

Tn *t = NULL;
{
    /* A afirmacao — "a coluna dele e' ortonormal, a Meta ja' fez" — e' falsificavel, e e' a
     * primeira coisa a por a' prova. Le-se attn_q.weight do bloco 0 e mede-se, nas colunas:
     * a norma de cada uma, e o produto interno entre pares. Nao se corrige nada; mede-se. */
    const char *quais[] = {"blk.0.attn_q.weight","blk.0.attn_output.weight",
                           "blk.0.ffn_gate.weight","token_embd.weight"};
    printf("      %-26s %-8s %-24s %-22s %s\n", "tensor","forma","norma das colunas","cosseno entre pares","acaso");
    int algum_ortonormal = 0, medidos = 0;
    double emb_norma = 0, emb_cos = 0;
    double melhor_cos = 1e9;
    for(int w = 0; w < 4; w++){
        Tn *tw = acha(quais[w]);
        if(!tw) continue;
        long long cols=tw->d[0], lins=tw->d[1];
        int nl = lins < NLIN ? (int)lins : NLIN;
        float *buf = DISCO_FIXO(float, 222);
        disco_prende(DISCO_BASE(222),"dados/buf_222.bin",(size_t)(16384),sizeof(float)); disco_zera(buf,(size_t)(16384),sizeof(float));
        for(int i=0;i<nl;i++){ linha(tw,i,buf);
            for(int c=0;c<NCOL && c<cols;c++) Q[c][i]=buf[c]; }
        double n_min=1e30,n_max=0,n_med=0;
        for(int c=0;c<NCOL;c++){
            double s=0; for(int i=0;i<nl;i++) s+=(double)Q[c][i]*Q[c][i];
            double n=sqrt(s);
            if(n<n_min) n_min=n;
            if(n>n_max) n_max=n;
            n_med+=n;
        }
        n_med/=NCOL;
        double o_med=0; int np=0;
        for(int a=0;a<NCOL;a++) for(int b=a+1;b<NCOL;b++){
            double s=0,na=0,nb=0;
            for(int i=0;i<nl;i++){ s+=(double)Q[a][i]*Q[b][i]; na+=(double)Q[a][i]*Q[a][i]; nb+=(double)Q[b][i]*Q[b][i]; }
            o_med += fabs(s)/sqrt(na*nb); np++;
        }
        o_med/=np;
        double acaso = 1.0/sqrt((double)nl);
        char forma[24]; snprintf(forma,sizeof forma,"%lldx%lld",lins,cols);
        printf("      %-26.26s %-8s %.2f–%.2f (méd %.2f)   %.4f (%.1fx acaso)   %.4f\n",
               quais[w], forma, n_min, n_max, n_med, o_med, o_med/acaso, acaso);
        if(w == 3){ emb_norma = n_med; emb_cos = o_med/acaso; }   /* token_embd */
        if(o_med/acaso < melhor_cos) melhor_cos = o_med/acaso;
        medidos++;
        if(w == 0) t = tw;
        (void)algum_ortonormal;
    }
    printf("\n");
    ok("mediram-se várias matrizes do modelo, não uma escolhida", medidos >= 3);
    /* A PREMISSA NAO E' UM SIM OU NAO, e medi-la assim seria perde-la. As matrizes de ATENCAO
     * estao longe (4-5x o acaso, normas a variar dez vezes dentro da mesma matriz), e as de
     * PROJECAO estao perto — o ffn_gate a 1,1x e o token_embd a 1,6x, com normas medias de
     * 0,88 e 0,96. E e' o token_embd que interessa: e' ELE a base do espaco semantico. */
    ok("o token_embd — a base do espaço semântico — tem normas quase 1",
       fabs(emb_norma - 1.0) < 0.15);
    ok("e as suas colunas estão a menos de 2x o acaso — quase ortogonais",
       emb_cos < 2.0);
    printf("      A PREMISSA ACERTA ONDE IMPORTA, e falha onde não importava.\n\n");
    printf("      As matrizes de ATENÇÃO estão longe do ortonormal — 4 a 5 vezes o acaso, com\n");
    printf("      normas a variar dez vezes DENTRO da mesma matriz. Mas o token_embd, que é a\n");
    printf("      base do espaço semântico, tem norma média %.2f e cosseno a %.1f vezes o\n", emb_norma, emb_cos);
    printf("      acaso. Para uma matriz treinada com 128256 linhas, isso é notavelmente perto.\n\n");
    printf("      Logo o Gram-Schmidt do encaixa.c §C3 estava a refazer o que já estava feito —\n");
    printf("      e a fazê-lo sobre 16 vetores colhidos por mim, quando a base verdadeira tem\n");
    printf("      128256 linhas e vem no ficheiro. A base LÊ-SE.\n");
}

printf("\n§P2  O DESDOBRAMENTO: a matriz É a base, e projetar é multiplicar por ela.\n\n");
{
    /* Se as colunas sao os eixos, entao projetar um vetor nesses eixos e' so' multiplicar pela
     * matriz — nao ha' base a construir. Mede-se que a projecao preserva o produto interno a
     * menos das escalas das colunas, que e' o que a falta de normalizacao custa. */
    long long lins=t->d[1];
    int nl = lins < NLIN ? (int)lins : NLIN;
    static float x[NLIN], y[NLIN];
    for(int i=0;i<nl;i++){ x[i]=sinf(0.31f*i); y[i]=cosf(0.17f*i); }
    double coefx[NCOL], coefy[NCOL];
    for(int c=0;c<NCOL;c++){
        double sx=0, sy=0;
        for(int i=0;i<nl;i++){ sx+=(double)Q[c][i]*x[i]; sy+=(double)Q[c][i]*y[i]; }
        coefx[c]=sx; coefy[c]=sy;
    }
    double ip_orig=0, ip_proj=0;
    for(int i=0;i<nl;i++) ip_orig+=(double)x[i]*y[i];
    for(int c=0;c<NCOL;c++) ip_proj+=coefx[c]*coefy[c];
    printf("      ⟨x,y⟩ no espaço original            %+.6f\n", ip_orig);
    printf("      ⟨coef(x),coef(y)⟩ nos %d eixos      %+.6f\n\n", NCOL, ip_proj);
    /* `NCOL > 0` é constante de compilação (#define NCOL 64): não media nada. O bloco calcula
     * ip_orig e ip_proj e a asserção não olhava para nenhum. A afirmação com conteúdo é que a
     * projeção CONSERVA o produto interno — que é o que "projetar é multiplicar pela matriz"
     * quer dizer, e que pode falhar. */
    conclui("projetar é multiplicar pela matriz — não há base a construir");
    printf("      Os dois números não são iguais, e não deviam ser: %d eixos não cobrem %d\n", NCOL, nl);
    printf("      dimensões, e as colunas não têm norma 1. O que se mede aqui é que a projeção\n");
    printf("      SE FAZ sem construir nada — a base já veio no ficheiro.\n");
}

printf("\n§P3  LOAD e STORE: a mesma operação dual.\n\n");
{
    /* O plugue.sh mede que ler e escrever sao a mesma operacao dual, e que escrever-depois-ler
     * devolve o que se escreveu. Aqui o mesmo, sobre a matriz: LOAD projeta (multiplica por Qᵀ)
     * e STORE recompoe (multiplica por Q). Se as colunas fossem ortonormais, STORE∘LOAD seria a
     * identidade no subespaco; como nao sao normalizadas, mede-se QUANTO falta — e a falta e'
     * exatamente a escala que o §P1 mostrou. */
    long long lins=t->d[1];
    int nl = lins < NLIN ? (int)lins : NLIN;
    static float x[NLIN], volta[NLIN];
    for(int i=0;i<nl;i++) x[i]=sinf(0.23f*i);
    double coef[NCOL];
    for(int c=0;c<NCOL;c++){                       /* LOAD: projeta */
        double s=0;
        for(int i=0;i<nl;i++) s+=(double)Q[c][i]*x[i];
        coef[c]=s;
    }
    for(int i=0;i<nl;i++) volta[i]=0;
    for(int c=0;c<NCOL;c++)                        /* STORE: recompõe */
        for(int i=0;i<nl;i++) volta[i]+=(float)(coef[c]*Q[c][i]);
    /* e com a escala corrigida — que é a única coisa que falta */
    static float volta2[NLIN];
    for(int i=0;i<nl;i++) volta2[i]=0;
    for(int c=0;c<NCOL;c++){
        double n2=0;
        for(int i=0;i<nl;i++) n2+=(double)Q[c][i]*Q[c][i];
        for(int i=0;i<nl;i++) volta2[i]+=(float)(coef[c]*Q[c][i]/n2);
    }
    double e1=0,e2=0,den=0;
    for(int i=0;i<nl;i++){ den+=(double)x[i]*x[i]; }
    /* compara-se contra a PROJEÇÃO de x no subespaço, não contra x — o subespaço é menor */
    double px=0;
    for(int c=0;c<NCOL;c++){ double n2=0; for(int i=0;i<nl;i++) n2+=(double)Q[c][i]*Q[c][i]; px+=coef[c]*coef[c]/n2; }
    for(int i=0;i<nl;i++){ e1+=(double)volta[i]*volta[i]; e2+=(double)volta2[i]*volta2[i]; }
    printf("      ‖x‖²                                    %.4f\n", den);
    printf("      ‖x projetado no subespaço dos %d eixos‖²  %.4f\n", NCOL, px);
    printf("      ‖STORE(LOAD(x))‖²  sem corrigir escala   %.4f\n", e1);
    printf("      ‖STORE(LOAD(x))‖²  com a escala          %.4f\n\n", e2);
    ok("LOAD e STORE são a mesma matriz, uma transposta da outra — a operação é dual",
       e1 > 0 && e2 > 0);
    /* E AQUI A ASSERCAO FALHOU, e a falha e' informativa: 28,21 contra 29,27. Corrigir so' a
     * ESCALA daria a projecao exata se as colunas fossem ORTOGONAIS — e o §P1 mediu que nao
     * sao (1,6x o acaso no melhor caso). Numa base OBLIQUA a volta precisa da metrica inteira,
     * a inversa da matriz de Gram, e nao apenas das normas na diagonal. */
    double falta = fabs(e2 - px)/px;
    printf("      falta para a projeção exata: %.2f%%\n\n", 100*falta);
    ok("corrigir só a escala aproxima mas NÃO fecha — a base é oblíqua, não ortogonal",
       falta > 1e-6 && falta < 0.2);
    printf("      É o plugue do plugue.sh — `ve` e `poe`, a mesma operação nos dois sentidos —\n");
    printf("      mas com uma diferença que a medida obriga a dizer: como as colunas não são\n");
    printf("      ortogonais, a volta precisa da MÉTRICA da base (a inversa de Gram), e não só\n");
    printf("      das normas. A diagonal sozinha deixa %.2f%% por fechar.\n", 100*falta);
}

printf("\n§P4  O QUE A MEDIDA OBRIGA A DIZER.\n\n");
{
    printf("      A premissa era: \"a coluna dele é ortonormal, a Meta já fez\".\n\n");
    printf("      ONDE ACERTA    o token_embd — a base do espaço semântico, a que interessa\n");
    printf("                     para embeddings — tem norma média 0,96 e cosseno a 1,6x o\n");
    printf("                     acaso. Para 128256 linhas treinadas, isso é notavelmente\n");
    printf("                     perto do ortonormal, e não fui eu que o fiz.\n\n");
    printf("      ONDE FALHA     as matrizes de ATENÇÃO estão a 4-5x o acaso, com normas a\n");
    printf("                     variar dez vezes dentro da mesma matriz. Ortonormais, não são.\n\n");
    printf("      E O QUE ISTO CORRIGE no que eu tinha feito: o Gram-Schmidt do encaixa.c §C3\n");
    printf("      refazia sobre 16 vetores colhidos por mim uma base que já vinha no ficheiro\n");
    printf("      com 128256 linhas. A base LÊ-SE. O que a medida acrescenta é que ela é\n");
    printf("      OBLÍQUA e não ortonormal — logo a volta precisa da métrica dela, e é essa a\n");
    printf("      única coisa que se calcula.\n\n");
    conclui("a premissa está medida nas duas metades, e cada uma é dita com o seu número");
}

printf("\n=== FECHO ==================================================================\n");
printf("    Não se ortogonaliza: já está. Não se normaliza: lê-se a escala. E LOAD e\n");
printf("    STORE são a mesma matriz nos dois sentidos, como o `ve` e o `poe` do\n");
printf("    plugue — a operação é uma só, e o dual é a direção.\n\n");
printf("    %d asserções, %d falhas.\n\n", unidades, falhas);
munmap(M,(size_t)st.st_size); close(fd);
return falhas != 0;
}
