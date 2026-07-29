/* tiffany.c — a assistente pela convolução do corpo, reproduzível no analógico.
 *
 * CADA operação tem análogo no circuito (microprocessador.tex §B), nada de ordenação/busca digital:
 *   • a fala cai  = a CONVOLUÇÃO fala⊛livro: c[d]=Σ fala[i]·livro[d+i] — o banco de correlacionadores
 *                   (o translinear §B.4 para cada produto, o Kirchhoff §B.5 na soma), TODO d em paralelo;
 *   • onde cai    = o mínimo da distância D=E_fala+E_janela−2c — o WINNER-TAKE-ALL (o comparador);
 *   • o navegante = a REALIMENTAÇÃO: o fim do trecho vira contexto e convolve de novo, de atrator em
 *                   atrator, na seta do tempo (o gato); e PARA na convergência (o próximo = o atual, o
 *                   ponto fixo — o negro/sorvedouro). Só multiplicação, soma, comparação e realimentação.
 * Em C a convolução é O(N) por salto (a simulação varre d); no analógico é O(1) — os d somam juntos.
 *
 *   cc -O2 -std=c99 tiffany.c -o tiffany
 *   ./tiffany por.tsv "Eu preciso de" [potência2_do_livro]
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define K 8                                            /* o contexto do atrator: 8 bytes            */
#define L 40                                           /* o passo de leitura em cada atrator         */

static long NL; static char *B; static long *pref;
/* a CONVOLUÇÃO do contexto com o livro (o banco de correlacionadores + winner-take-all).            */
/* exato=0: onde a fala cai (o mínimo de D global). exato=1: o casamento adiante (D=0, a seta do     */
/* tempo), dando a volta. Tudo é multiplicação (o gato/translinear) e soma (o Kirchhoff), em paralelo.*/
static long cai(const unsigned char *ctx, int cl, long from, int exato){
    long Ef=0; for(int i=0;i<cl;i++) Ef += (long)ctx[i]*ctx[i];
    if(exato){
        for(long d=from; d+cl<=NL; d++){                            /* adiante (a seta do tempo)     */
            long c=0; for(int i=0;i<cl;i++) c += (long)ctx[i]*(unsigned char)B[d+i];
            if(Ef+(pref[d+cl]-pref[d])-2*c==0) return d;
        }
        for(long d=0; d<from && d+cl<=NL; d++){                     /* dá a volta                    */
            long c=0; for(int i=0;i<cl;i++) c += (long)ctx[i]*(unsigned char)B[d+i];
            if(Ef+(pref[d+cl]-pref[d])-2*c==0) return d;
        }
        return -1;
    }
    long bestD=-1, bp=0;                                            /* o mínimo global (a fala cai)  */
    for(long d=0; d+cl<=NL; d++){
        long c=0; for(int i=0;i<cl;i++) c += (long)ctx[i]*(unsigned char)B[d+i];
        long D=Ef+(pref[d+cl]-pref[d])-2*c;
        if(bestD<0 || D<bestD){ bestD=D; bp=d; }
    }
    return bp;
}

int main(int argc,char**argv){
    const char *path = argc>1? argv[1] : "por.tsv";
    const char *fala = argc>2? argv[2] : "Eu preciso de";
    int lg = argc>3? atoi(argv[3]) : 16;
    long SZ=1L<<lg;
    FILE *f=fopen(path,"rb"); if(!f){ fprintf(stderr,"não abri %s\n",path); return 2; }
    fseek(f,0,SEEK_END); long N=ftell(f); fseek(f,0,SEEK_SET);
    char *raw=malloc(N); if(fread(raw,1,N,f)!=(size_t)N) return 2; fclose(f);
    B=malloc(SZ); NL=0;                                 /* o livro: um sinal corrido (frases juntas) */
    for(long i=0;i<N && NL<SZ;){ int tab=0; while(i<N && tab<2){ if(raw[i]=='\t') tab++; i++; }
        while(i<N && raw[i]!='\n' && NL<SZ) B[NL++]=raw[i++];
        while(i<N && raw[i]!='\n') i++;
        if(i<N) i++;
        if(NL<SZ) B[NL++]=' '; }
    free(raw);
    while(NL<SZ) B[NL++]=' ';
    pref=malloc((SZ+1)*sizeof(long)); pref[0]=0;        /* a energia da janela (livro×livro somado)  */
    for(long i=0;i<SZ;i++) pref[i+1]=pref[i]+(long)(unsigned char)B[i]*(unsigned char)B[i];

    /* a fala cai (a convolução, o mínimo). O navegante caminha por realimentação e para na convergência. */
    int fl=(int)strlen(fala);
    long p=cai((const unsigned char*)fala, fl, 0, 0)+fl;
    printf("%s ", fala);
    long p_ant=-1, saltos=0;
    while(saltos < 100000 && p>=0 && p+L<=NL && p!=p_ant){
        for(long i=p;i<p+L;i++) putchar(B[i]);
        unsigned char ctx[K];
        for(int i=0;i<K;i++) ctx[i]=(unsigned char)B[p+L-K+i];      /* o fim do trecho → a órbita     */
        p_ant=p;
        long nx=cai(ctx, K, p+L, 1);                               /* o próximo casamento adiante     */
        p = (nx<0)? -1 : nx+K;                                     /* (a seta do tempo, o gato)       */
        saltos++;
    }
    printf("\n\n[a fala caiu pela convolução; o navegante caminhou %ld atratores por realimentação\n"
           " e parou na convergência. Tudo reproduzível no analógico (§B): convolução, winner-take-all,\n"
           " realimentação — sem ordenação nem busca digital.]\n", saltos);
    free(B); free(pref);
    return 0;
}
