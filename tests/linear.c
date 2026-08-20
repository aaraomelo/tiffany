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
#include "unidade.h"
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

    /* §1 — o representante da imagem INTEIRA, e ONDE ELE DEIXA DE CABER.
     *
     * Estava assim:
     *
     *     long r = px[N-1]+2.0;
     *     for(long i=N-2;i>=0;i--) r = (px[i]+2.0) + 1.0/r;
     *
     * e não avaliava fração contínua nenhuma. Com `r` em long e `r >= 2` sempre,
     * `1.0/r` cai em (0, 1/2] e some inteira no truncamento: o laço reduzia-se a
     * `r = px[i]+2`, e o «representante da imagem inteira» era o PRIMEIRO PIXEL
     * mais dois. A linha de texto afirmava a imagem toda.
     *
     * A fração contínua não vive num escalar: vive no PAR p/q, que é o que o §2
     * já usa por segmento. E o par cresce como Fibonacci — a imagem inteira NÃO
     * cabe. Então é isso que se mede e se diz: até que termo o representante ainda
     * é o representante, e a partir de qual a máquina não o tem. */
    long p0=1, p1=px[N-1]+2, q0=0, q1=1, ate=1;
    const long TECTO = 4611686018427387903L;             /* 2⁶²−1: o produto seguinte ainda cabe */
    for(long i=N-2;i>=0;i--){
        long c = px[i]+2;
        if(p1 > TECTO/c || q1 > TECTO/c) break;          /* não cabe: PÁRA, e diz-se */
        long p2=c*p1+p0, q2=c*q1+q0;
        if(p2 < p1 || q2 < q1) break;                    /* rede: transbordo é paragem */
        p0=p1; p1=p2; q0=q1; q1=q2; ate++;
    }
    printf("  §1  a imagem inteira NÃO cabe num representante: a fração contínua dela é o par\n");
    printf("      p/q, e o par cresce como Fibonacci. Dos %ld pixels, os últimos %ld ainda\n", N, ate);
    printf("      cabem em 64 bits, e o convergente aí é\n");
    printf("      p/q = %ld/%ld   (o que a máquina TEM; o resto não foi medido)\n", p1, q1);
    /* E A PARAGEM TEM DE SER REAL, senão «não cabe» é uma frase e não uma medida.
     * Dois lados: (a) o convergente que se tem cumpre mesmo a recorrência — p/q é
     * primo entre si e p_k = a_k·p_{k-1} + p_{k-2} —, e (b) o passo SEGUINTE
     * transbordaria de facto. Sem (b), parar no primeiro pixel passava igual. */
    {
        long det = p1*q0 - p0*q1;                    /* |det| = 1 em todo convergente */
        long i_prox = N-1-ate;
        int parou_por_tecto = (i_prox >= 0) &&
            (p1 > TECTO/(px[i_prox]+2) || q1 > TECTO/(px[i_prox]+2));
        /* «consumiu tudo» só é aceitável onde CABE: com todos os a_k >= 2 o par cresce
         * pelo menos como Fibonacci, e passado o 90.º termo nenhum par de 64 bits o
         * segura. Sem esta condição, tirar o tecto fazia o laço percorrer a imagem
         * inteira a transbordar e a asserção aceitava-o pela porta do «tudo» — e o
         * |det| = ±1 não a fecha, porque a recorrência preserva o determinante MÓDULO
         * 2⁶⁴: o transbordo passa por ela sem deixar marca. */
        int tudo = (i_prox < 0) && (N <= 90);
        printf("      |p·q' − p'·q| = %ld (tem de ser 1) · a paragem é real: %s\n",
               det < 0 ? -det : det,
               tudo ? "consumiu tudo" : (parou_por_tecto ? "sim, o passo seguinte não cabia"
                                                         : "NÃO — parou sem motivo"));
        ok("O REPRESENTANTE DA IMAGEM É UM PAR, E O QUE NÃO CABE DIZ-SE: o convergente"
           " devolvido cumpre |p·q' − p'·q| = 1, que é a assinatura de que ele saiu mesmo"
           " da recorrência da fracção contínua, e a paragem é a de um passo que"
           " TRANSBORDARIA — verificado no pixel seguinte, não decretado. Antes disto o"
           " laço era `r = (px[i]+2.0) + 1.0/r` num long: o 1/r caía sempre em (0, 1/2],"
           " sumia no truncamento, e «a imagem inteira» valia o PRIMEIRO PIXEL mais dois",
           (det == 1 || det == -1) && ate >= 2 && (tudo || parou_por_tecto)
             && (ate <= 90 || N <= 90));
    }

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
           viol, VD(viol, "EXATO, resíduo 0"));
    printf("      tempo = %ld s (em C)\n", (long)(t1-t0)/CLOCKS_PER_SEC);
    free(px); free(pnum); free(pden); free(hk); free(hv);
    /* O EXIT TEM DE SEGUIR AS UNIDADES. Este ficheiro não tinha `ok()` nenhum e o
     * `return viol?1:0` bastava; com a asserção do §1 passou a haver `falhas`, e um
     * exit que a ignorasse era o verde falso que a bateria persegue — ela cruza os
     * dois caminhos e chamar-lhe-ia FALHA de qualquer modo, mas o defeito é aqui. */
    printf("\n=== %d asserções, %d falhas ===\n", unidades, falhas);
    return (viol || falhas) ? 1 : 0;
}
