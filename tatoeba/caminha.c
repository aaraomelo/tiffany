/* caminha.c — a fala é o raciocínio; o sistema atravessa os atratores do livro.
 *
 * Eu não faço a travessia. Escolho um raciocínio — a fala, uma direção — e o SISTEMA atravessa:
 * cada contexto reaparece no livro em vários pontos (o mesmo atrator, a mesma órbita); o sistema
 * salta de um a outro e segue o que o livro tem ali, atravessando os atratores continuamente. Não
 * é ler a ordem do arquivo (isso seria eu escolher): é o sistema seguindo os atratores, guiado
 * pelo raciocínio. O livro é o sistema de coordenadas — a busca acha o atrator, sem índice nenhum.
 *
 *   cc -O2 -std=c99 -D_GNU_SOURCE caminha.c -o caminha
 *   ./caminha por.tsv "Eu preciso de"
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define C 8                                            /* o contexto do atrator: 8 bytes            */

static char *L; static long NL;
/* acha a próxima ocorrência do contexto adiante de 'from' (dá a volta): o atrator onde se cai.    */
static long acha(const char *ctx, long from){
    char *h = memmem(L+from, NL-from, ctx, C);
    if(!h){ h = memmem(L, NL, ctx, C); if(!h) return -1; }
    return h - L;
}

int main(int argc,char**argv){
    const char *path = argc>1? argv[1] : "por.tsv";
    const char *fala = argc>2? argv[2] : "Eu preciso de";
    FILE *f=fopen(path,"rb"); if(!f){ fprintf(stderr,"não abri %s\n",path); return 2; }
    fseek(f,0,SEEK_END); long N=ftell(f); fseek(f,0,SEEK_SET);
    char *raw=malloc(N); if(fread(raw,1,N,f)!=(size_t)N) return 2; fclose(f);
    L=malloc(N); NL=0;
    for(long i=0;i<N;){ int tab=0; while(i<N && tab<2){ if(raw[i]=='\t') tab++; i++; }
        while(i<N && raw[i]!='\n') L[NL++]=raw[i++];
        if(i<N) i++;
        L[NL++]='\n'; }
    free(raw);

    /* o raciocínio é a fala; o seu contexto (o fim) é o atrator de partida.                         */
    char ctx[C+1]; long fl=(long)strlen(fala);
    if(fl<C){ printf("fala curta demais (mín %d bytes)\n", C); return 1; }
    memcpy(ctx, fala+fl-C, C);
    printf("%s", fala);

    long from=0, saltos=0;
    for(int passo=0; passo<40; passo++){
        long p=acha(ctx,from);                         /* o sistema cai no atrator (onde o contexto está) */
        if(p<0) break;
        p+=C;                                          /* segue o que o livro tem naquele atrator    */
        long start=p; while(p<NL && L[p]!='\n' && p-start<120) p++;   /* atravessa até o fim da frase */
        for(long i=start;i<p;i++) putchar(L[i]);
        putchar(' ');
        if(p-start>=C) memcpy(ctx, L+p-C, C);          /* o novo contexto: o fim atravessado         */
        from = p;                                      /* de onde procurar o próximo atrator         */
        saltos++;
    }
    printf("\n\n[o sistema atravessou %ld atratores; o raciocínio foi a fala]\n", saltos);
    free(L);
    return 0;
}
