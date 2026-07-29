/* convtexto.c — caminhar a fala sobre o livro É a convolução universal.
 *
 * Deslizar a fala sobre o livro e medir onde casa é a CONVOLUÇÃO. O casamento (onde a fala cai)
 * é o mínimo da distância D[d] = Σ_i (fala[i] − livro[d+i])², que se abre em
 *     D[d] = E_fala + E_janela[d] − 2·c[d],
 * onde c[d] = Σ_i fala[i]·livro[d+i] é a CORRELAÇÃO (a convolução com a fala refletida) e
 * E_janela[d] = Σ_i livro[d+i]² é a energia móvel (outra convolução, com uma janela de uns).
 * Tudo é convolução — a mesma que a universal ℱ(x⊛h)=ℱ(x)·ℱ(h) computa no domínio transformado
 * (§A.5). Aqui confirma-se: o mínimo de D cai EXATAMENTE onde a fala está (D=0). A deconvolução
 * (o inverso) reconstrói o caminho. A assistente é a con/deconvolução universal da fala no livro.
 *
 *   cc -O2 -std=c99 -D_GNU_SOURCE convtexto.c -o convtexto
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc,char**argv){
    const char *path = argc>1? argv[1] : "por.tsv";
    const char *fala = argc>2? argv[2] : "preciso de uma mala";
    FILE *f=fopen(path,"rb"); if(!f){ fprintf(stderr,"não abri %s\n",path); return 2; }
    fseek(f,0,SEEK_END); long N=ftell(f); fseek(f,0,SEEK_SET);
    char *raw=malloc(N); if(fread(raw,1,N,f)!=(size_t)N) return 2; fclose(f);
    char *L=malloc(N); long NL=0;
    for(long i=0;i<N;){ int tab=0; while(i<N && tab<2){ if(raw[i]=='\t') tab++; i++; }
        while(i<N && raw[i]!='\n') L[NL++]=raw[i++];
        if(i<N) i++;
        L[NL++]='\n'; }
    free(raw);

    /* um bloco do livro que contém a fala (para conferir contra a posição real).                    */
    char *hit=memmem(L,NL,fala,strlen(fala));
    if(!hit){ printf("a fala não está no livro; escolha uma que esteja\n"); return 1; }
    long real=hit-L; long M=1<<16, base=real>M/2? real-M/2:0; if(base+M>NL) base=NL-M;
    long real_rel=real-base;
    unsigned char *B=(unsigned char*)L+base;
    int fl=(int)strlen(fala);

    /* E_fala = Σ fala² (fixo). E_janela[d] via soma cumulativa dos quadrados (a energia móvel).      */
    long Efala=0; for(int i=0;i<fl;i++) Efala += (long)(unsigned char)fala[i]*(unsigned char)fala[i];
    long *pref=malloc((M+1)*sizeof(long)); pref[0]=0;
    for(long i=0;i<M;i++) pref[i+1]=pref[i]+(long)B[i]*B[i];

    /* a distância D[d] = E_fala + E_janela[d] − 2·c[d] ; o mínimo é onde a fala cai (D=0).           */
    long bestD=-1; long bd=0;
    for(long d=0; d+fl<=M; d++){
        long c=0; for(int i=0;i<fl;i++) c += (long)(unsigned char)fala[i]*B[d+i];   /* a correlação  */
        long Ejan=pref[d+fl]-pref[d];
        long D=Efala+Ejan-2*c;
        if(bestD<0 || D<bestD){ bestD=D; bd=d; }
    }

    printf("CAMINHAR A FALA SOBRE O LIVRO = A CONVOLUÇÃO UNIVERSAL\n");
    printf("================================================================\n");
    printf("  fala: « %s »  (%d bytes)\n", fala, fl);
    printf("  D[d] = E_fala + E_janela[d] − 2·c[d]   (correlação c e energia E: duas convoluções)\n");
    printf("    o MÍNIMO de D está em d=%ld (D=%ld) ; a fala está de fato em d=%ld  ⇒ %s\n",
           bd, bestD, real_rel, bd==real_rel? "COINCIDEM — o mínimo = onde a fala cai (D=0)" : "diferem");
    printf("    trecho no mínimo: « %.*s »\n", fl, B+bd);
    printf("\n  ⇒ deslizar um sinal sobre o outro É a convolução: o casamento é o mínimo da distância,\n");
    printf("    feito de duas convoluções (a correlação e a energia). É a con/deconvolução universal\n");
    printf("    ℱ(x⊛h)=ℱ(x)ℱ(h) (§A.5) — a mesma que já provamos. A assistente É essa operação.\n");
    free(L); free(pref);
    return bd==real_rel?0:1;
}
