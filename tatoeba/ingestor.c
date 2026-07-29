/* ingestor.c — o corpus é o 0: põe-se inteiro na máquina e ele se reparte. Reversível.
 *
 * O texto não é especial: é um sinal como uma imagem. O corpus INTEIRO é o 0 (Venom) — não se
 * fragmenta em frases, não se tokeniza, não se escolhe nada. Põe-se inteiro e ele mesmo se
 * reparte, cindindo ao meio (a Haar, §1), e a árvore da cisão — o vértice e as memórias — É o
 * grafo de órbitas. Reversível, resíduo 0: a junta devolve o corpus. Tudo sai do mesmo 0.
 *
 *   cc -O2 -std=c99 ingestor.c -o ingestor
 *   ./ingestor por.tsv
 */
#include <stdio.h>
#include <stdlib.h>

static void cinde(int *x, int n, int *tmp){
    for(int s=n; s>=2; s/=2){ int h=s/2;
        for(int i=0;i<h;i++){ int a=x[2*i], b=x[2*i+1]; int d=b-a; tmp[i]=a+(d>>1); tmp[h+i]=d; }
        for(int i=0;i<s;i++) x[i]=tmp[i]; }
}
static void junta(int *x, int n, int *tmp){
    for(int s=2; s<=n; s*=2){ int h=s/2;
        for(int i=0;i<h;i++){ int sv=x[i], d=x[h+i], a=sv-(d>>1); tmp[2*i]=a; tmp[2*i+1]=a+d; }
        for(int i=0;i<s;i++) x[i]=tmp[i]; }
}

int main(int argc,char**argv){
    const char *path = argc>1? argv[1] : "por.tsv";
    FILE *f=fopen(path,"rb"); if(!f){ fprintf(stderr,"não abri %s\n",path); return 2; }
    fseek(f,0,SEEK_END); long N=ftell(f); fseek(f,0,SEEK_SET);
    unsigned char *buf=malloc(N); if(fread(buf,1,N,f)!=(size_t)N){ return 2; } fclose(f);

    long P=1; while(P*2<=N) P*=2;                         /* o corpus inteiro: o maior bloco 2^k     */
    int *x=malloc(P*sizeof(int)), *orig=malloc(P*sizeof(int)), *tmp=malloc(P*sizeof(int));
    for(long i=0;i<P;i++){ x[i]=buf[i]; orig[i]=buf[i]; }

    cinde(x,P,tmp);                                       /* o 0 se reparte                          */
    int niveis=0; for(long s=P;s>=2;s/=2) niveis++;
    long z0=0,z1=0,zg=0; for(long i=1;i<P;i++){ int d=x[i]<0?-x[i]:x[i]; if(d==0)z0++; else if(d==1)z1++; else zg++; }
    long bic=0; for(long i=0;i<P;i++){ int d=x[i]<0?-x[i]:x[i]; int b=0; while(d){b++;d>>=1;} bic+=b+1; }

    printf("O CORPUS É O 0 — %s posto inteiro na máquina, e ele se reparte\n", path);
    printf("================================================================\n");
    printf("  corpus ............. %ld bytes ; cindido: %ld (2^%d), sozinho, sempre ao meio\n", N, P, niveis);
    printf("  resta UM vértice ... %d   (a média de tudo, a origem do corpus)\n", x[0]);
    printf("  memórias ........... %ld ; =0: %ld ; |d|=1: %ld ; maiores: %ld  ⇒ %.1f%% ~0\n",
           P-1, z0, z1, zg, 100.0*(z0+z1)/(P-1));
    printf("  compressão sem perda %ld bits → %ld bits = %.2f:1 (só os bits de cada valor)\n",
           P*8, bic, (double)(P*8)/bic);

    junta(x,P,tmp);                                       /* reversível: a junta devolve o corpus    */
    long viol=0; for(long i=0;i<P;i++) if(x[i]!=orig[i]) viol++;
    printf("  reversível ......... a junta devolve o corpus: erros = %ld (%s)\n",
           viol, viol?"FALHA":"EXATO, resíduo 0");
    printf("\n  a árvore da cisão É o grafo de órbitas: o vértice (a origem) e as memórias (as\n");
    printf("  diferenças em cada escala). Tudo saiu do mesmo 0, reversível. Nada foi escolhido.\n");
    free(buf); free(x); free(orig); free(tmp);
    return viol?1:0;
}
