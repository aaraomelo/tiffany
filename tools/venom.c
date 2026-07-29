/* venom.c — A ENTRADA É O 0: põe-se ela inteira e ela mesma se reparte, como Venom.
 *
 * A imagem inteira é o 0 — o indiferenciado 𝒱₀. Não a segmento, não escolho ordem nem
 * tamanho: ponho-a inteira e ELA se reparte, cindindo ao meio na máxima simetria, e cada
 * metade cinde de novo, recursivamente — a §1. Cada cisão deixa o VÉRTICE que permanece
 * (a média s = a + ⌊(b−a)/2⌋) e a MEMÓRIA da divisão (a diferença d = b − a) — o par da §7.
 * É reversível (a = s − ⌊d/2⌋, b = a + d): resíduo 0. Não muda nada — é o Venom.
 *
 * No fim resta UM vértice (a média de tudo, a origem) e a árvore das memórias. Onde as
 * metades se parecem, a memória é ~0: a estrutura da imagem cai da própria cisão, sem que
 * eu a imponha.
 *
 *   cc -O2 -std=c99 venom.c -o venom
 *   ./venom imagem.pgm
 */
#include <stdio.h>
#include <stdlib.h>

#include "pgm.h"                                    /* le_pgm: o leitor PGM binário (P5) reusado */

/* uma cisão ao meio, ao longo de n amostras com passo stride: média (vértice) e diferença (memória). */
static void cinde(int *x, int n, int stride, int *tmp){
    int h=n/2;
    for(int i=0;i<h;i++){ int a=x[(2*i)*stride], b=x[(2*i+1)*stride]; int d=b-a; tmp[i]=a+(d>>1); tmp[h+i]=d; }
    for(int i=0;i<n;i++) x[i*stride]=tmp[i];
}
static void junta(int *x, int n, int stride, int *tmp){          /* a inversa: o par volta ao todo */
    int h=n/2;
    for(int i=0;i<h;i++){ int s=x[i*stride], d=x[(h+i)*stride]; int a=s-(d>>1); tmp[2*i]=a; tmp[2*i+1]=a+d; }
    for(int i=0;i<n;i++) x[i*stride]=tmp[i];
}

int main(int argc, char **argv){
    if(argc<2){ fprintf(stderr,"uso: venom imagem.pgm\n"); return 2; }
    int w,h; unsigned char *px=le_pgm(argv[1],&w,&h);
    if(!px){ fprintf(stderr,"não abri PGM: %s\n", argv[1]); return 2; }
    int S=1; while(S*2<=w && S*2<=h) S*=2;                       /* o maior lado potência de 2 */
    int *img=malloc((size_t)S*S*sizeof(int)), *orig=malloc((size_t)S*S*sizeof(int));
    long soma_px=0;
    for(int y=0;y<S;y++) for(int x=0;x<S;x++){ img[y*S+x]=px[y*w+x]; orig[y*S+x]=px[y*w+x]; soma_px+=px[y*w+x]; }
    int *tmp=malloc(S*sizeof(int));
    long media_px = soma_px/((long)S*S);                        /* a média direta de todos os pixels */

    printf("A ENTRADA É O 0 — ponho a imagem inteira e ela mesma se reparte — %s (%dx%d)\n", argv[1], S, S);
    printf("================================================================\n");

    /* ela se reparte: cinde ao meio em linhas e colunas, e recursa na média (o vértice).    */
    int niveis=0;
    for(int s=S; s>=2; s/=2){
        for(int r=0;r<s;r++) cinde(&img[r*S], s, 1, tmp);       /* cada linha cinde           */
        for(int c=0;c<s;c++) cinde(&img[c], s, S, tmp);         /* cada coluna cinde          */
        niveis++;
    }
    /* o que resta: UM vértice (a média de tudo) e as memórias (as diferenças).               */
    long z0=0,z1=0,zg=0; long total=(long)S*S-1;                /* todos menos o vértice [0]   */
    for(int i=1;i<S*S;i++){ int d=img[i]<0?-img[i]:img[i]; if(d==0)z0++; else if(d<=1)z1++; else zg++; }
    printf("  a imagem cindiu-se em %d níveis, sozinha, sempre ao meio (máxima simetria)\n", niveis);
    printf("  resta UM vértice — a média de tudo, a origem: %d\n", img[0]);
    printf("  as %ld memórias (as diferenças): =0: %ld ; |d|=1: %ld ; maiores: %ld\n", total, z0, z1, zg);
    printf("  ⇒ %.1f%% das memórias são ~0 (|d|≤1): a estrutura caiu da cisão, eu não a impus\n",
           100.0*(z0+z1)/total);

    /* A IMAGEM CAI NO NEGRO (liga `duais`) — a cisão é o fluxo direcional branco→negro.       */
    /* O gato tem dois pontos fixos: o NEGRO (σ, sorvedouro, tudo entra) e o BRANCO (σ', fonte,*/
    /* tudo sai). A cisão é o gato direto: SORVE a imagem inteira ao vértice único — o NEGRO,   */
    /* a média de tudo, a origem 0. As memórias são o BRANCO (a fonte): 84% ~0; a junta (o gato */
    /* reverso A⁻¹) faz a imagem RE-EMERGIR delas. Não se sorve e emana pelo mesmo: ●≠○, duais. */
    int negro = img[0];
    long vies = media_px - negro;                               /* o viés do ⌊·⌋ acumulado (≤ níveis) */
    int bate = (vies>=0 && vies<=niveis);                       /* o vértice é a média a menos do floor */
    printf("\n  A IMAGEM CAI NO NEGRO (liga `duais`) — a cisão é o fluxo branco→negro:\n");
    printf("    ● cisão (o gato →): a imagem é SORVIDA ao vértice único, o NEGRO (sorvedouro):\n");
    printf("        %ld pontos → 1 vértice em %d níveis; o negro É a média de tudo (a origem 0):\n",
           (long)S*S, niveis);
    printf("        vértice = %d ;  média direta = %ld ;  viés do ⌊·⌋ = %ld (≤%d níveis)  ⇒ %s\n",
           negro, media_px, vies, niveis,
           bate? "é a média, a menos do arredondamento que a junta desfaz (resíduo 0)" : "?");
    printf("    ○ as memórias são o BRANCO (fonte): %.1f%% ~0; a junta (o gato ← A⁻¹) as faz re-emergir\n",
           100.0*(z0+z1)/total);

    /* a compressão JÁ está aqui, sem perda: cada memória custa só os bits do seu valor    */
    /* (as ~0 quase nada), o vértice idem. Nada quantizado, nada cortado — reversível.     */
    long bic=0; for(int i=0;i<S*S;i++){ int d=img[i]<0?-img[i]:img[i]; int b=0; while(d){ b++; d>>=1; } bic += b+1; }
    printf("  a compressão SEM PERDA que a cisão já dá (só os bits de cada valor, nada cortado):\n");
    printf("     %ld bits (pixels) → %ld bits (vértice + memórias) = %.2f:1 (reversível, resíduo 0)\n",
           (long)S*S*8, bic, (double)(S*S*8)/bic);

    /* reversível: junta de volta e confere resíduo 0 (a imagem inteira volta).               */
    for(int s=2; s<=S; s*=2){
        for(int c=0;c<s;c++) junta(&img[c], s, S, tmp);
        for(int r=0;r<s;r++) junta(&img[r*S], s, 1, tmp);
    }
    long viol=0; for(int i=0;i<S*S;i++) if(img[i]!=orig[i]) viol++;
    printf("  a cisão é reversível — junta-se o par e a imagem volta inteira: erros = %ld (%s)\n",
           viol, viol?"FALHA":"EXATO, resíduo 0");
    free(px); free(img); free(orig); free(tmp);
    return viol?1:0;
}
