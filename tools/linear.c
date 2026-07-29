/* linear.c — A TRANSFORMADA UNIVERSAL NA IMAGEM: linearizar e ver o que ela gera.
 *
 * A imagem é uma matriz; lineariza-se "uma linha no rabo da outra" (row-major) numa
 * sequência, e aplica-se a transformada universal — a FRAÇÃO CONTÍNUA. Ela engole a
 * sequência e gera o REPRESENTANTE: o valor da fração contínua, o número que aquela
 * membrana vale. Sem oráculo, sem predição — a transformada é bijetiva, reconstrói com
 * resíduo 0. Cada segmento tem o seu representante; segmentos com o mesmo representante
 * são a mesma CLASSE. (Pixels lidos como +2, para 2..257 (termos ≥2 ⇒ fração contínua bijetiva), sem zeros na fração contínua.)
 *
 *   cc -O2 -std=c99 linear.c -o linear
 *   ./linear imagem.pgm [L]        L = tamanho do segmento (default 6)
 */
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

static long mdc(long a,long b){ a=a<0?-a:a; b=b<0?-b:b; while(b){ long t=a%b; a=b; b=t; } return a; }

#include "pgm.h"                                    /* le_pgm: o leitor PGM binário (P5) reusado */

int main(int argc, char **argv){
    if(argc<2){ fprintf(stderr,"uso: linear imagem.pgm [L]\n"); return 2; }
    int L = argc>2? atoi(argv[2]) : 6;
    int w,h; unsigned char *px=le_pgm(argv[1],&w,&h);
    if(!px){ fprintf(stderr,"não abri PGM: %s\n", argv[1]); return 2; }
    long N=(long)w*h;
    printf("A TRANSFORMADA UNIVERSAL NA IMAGEM — %s (%dx%d = %ld pixels, linearizada)\n", argv[1], w, h, N);
    printf("================================================================\n");

    /* §1 — o representante da imagem INTEIRA: a fração contínua da sequência (do fim ao começo). */
    double r = px[N-1]+2.0;
    for(long i=N-2;i>=0;i--) r = (px[i]+2.0) + 1.0/r;
    printf("  §1  a imagem inteira, engolida como UMA fração contínua, gera o representante:\n");
    printf("      r = %.15f   (o número que a membrana vale — a transformada da imagem)\n", r);

    /* §2 — por segmento: o representante EXATO (p/q) de cada trecho, e as classes.               */
    clock_t t0=clock();
    long nseg = N/L;
    long *pnum=malloc(nseg*sizeof(long)), *pden=malloc(nseg*sizeof(long));
    for(long s=0;s<nseg;s++){
        long p0=1,p1=px[s*L]+2, q0=0,q1=1;                  /* convergentes p_k/q_k          */
        for(int k=1;k<L;k++){ long c=px[s*L+k]+2; long p2=c*p1+p0,q2=c*q1+q0; p0=p1;p1=p2;q0=q1;q1=q2; }
        long g=mdc(p1,q1); if(g==0)g=1; pnum[s]=p1/g; pden[s]=q1/g;   /* o representante p/q     */
    }
    /* as classes: segmentos com o MESMO representante (mesma fração contínua).                    */
    long hsz=nseg*2+7; long *hk=malloc(hsz*sizeof(long)), *hv=malloc(hsz*sizeof(long));
    for(long i=0;i<hsz;i++) hv[i]=-1;
    long nclass=0;
    for(long s=0;s<nseg;s++){
        long key=pnum[s]*1000003L + pden[s], hh=((unsigned long)(key)*1099511628211UL)%hsz;
        while(hv[hh]>=0 && hk[hh]!=key) hh=(hh+1)%hsz;
        if(hv[hh]<0){ hk[hh]=key; hv[hh]=nclass++; }
    }
    clock_t t1=clock();
    /* §3 — lossless: do representante p/q, o algoritmo de Euclides devolve os pixels do segmento. */
    long viol=0;
    for(long s=0;s<nseg;s++){
        long a=pnum[s],b=pden[s];
        for(int k=0;k<L;k++){ long ai = b? a/b : 0; long rr = b? a%b : 0; a=b; b=rr;
            if((ai-2) != px[s*L+k]) viol++; }
    }
    printf("  §2  cada segmento de %d pixels → o seu representante (fração contínua p/q):\n", L);
    for(long s=0;s<3 && s<nseg;s++){
        printf("      seg %ld: [", s); for(int k=0;k<L;k++) printf("%s%d", k?",":"", px[s*L+k]);
        printf("] → %ld/%ld\n", pnum[s], pden[s]);
    }
    printf("  §3  %ld segmentos geram %ld classes (representantes distintos)\n", nseg, nclass);
    printf("      reconstrução pela transformada inversa (Euclides): pixels errados = %ld (%s)\n",
           viol, viol?"FALHA":"EXATO, resíduo 0");
    printf("      tempo = %.3f s (em C)\n", (double)(t1-t0)/CLOCKS_PER_SEC);
    free(px); free(pnum); free(pden); free(hk); free(hv);
    return viol?1:0;
}
