/* bench_dissipa.c — O QUE CADA ORDENACAO APAGA. A regua e' Landauer, nao o relogio.
 *
 * O Aarao: «o sistema nao tem tempo porque e' tudo reversivel — o tempo e' so' latencia,
 * entao e' tempo 0, como sempre foi aqui.»
 *
 * E O BENCHMARK ANTERIOR MEDIA MILISSEGUNDOS. Era a regua errada: tempo e' latencia da
 * maquina, nao propriedade do algoritmo. A regua deste projecto e' o CAUDAL — bits
 * apagados por corrida, kT ln2 cada (dissipa.sh, dissipa.c).
 *
 * O QUE APAGA, e conta-se sem estimar:
 *
 *   uma COMPARACAO      le dois valores de 64 bits e devolve UM BIT. Os 128 bits de
 *                       entrada nao se recuperam da saida: apagam-se 127.
 *   uma ESCRITA         poe um valor onde estava outro. O que la' estava perde-se: 64
 *                       bits, salvo se a escrita for involutiva (troca, XOR).
 *   uma TROCA           a[i] <-> a[j] e' INVOLUTIVA — aplicada duas vezes devolve, e
 *                       portanto NAO apaga nada.
 *
 * E' por isso que os numeros nao sao proporcionais ao tempo: um algoritmo pode ser rapido
 * e dissipar muito, ou lento e nao apagar quase nada.
 *
 *   cc -O2 -std=c99 -Wall bench_dissipa.c -o /tmp/bd && /tmp/bd
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <inttypes.h>

#define BASE 256
#define BITS 64

/* os contadores: cada operacao declara o que apaga */
static long apagou_cmp, apagou_esc, trocas;
static void CMP(void){ apagou_cmp += 2*BITS - 1; }   /* 128 bits entram, 1 sai */
static void ESC(void){ apagou_esc += BITS; }          /* sobrescreve: perde-se o que la' estava */
static void TROCA(void){ trocas++; }                  /* involutiva: NAO apaga */

/* ── O DUAL SORT: le o simbolo e desce. Em p.u., e com o ponto fixo lido. ─────────── */
static long simb(long x, int d){ return (x >> (8*d)) & (BASE-1); }
static int niveis_pu(const long *a, long n){
    long hi = 0;
    for(long i = 0; i < n; i++) if(a[i] > hi) hi = a[i];
    int k = 0; while(hi){ k++; hi >>= 8; }
    return k ? k : 1;
}
static long *MK;
static void dual_sort(long *a, long n, long *out){
    long M = 0;
    for(long i = 0; i < n; i++) if(a[i] > M) M = a[i];
    M++;
    for(long v = 0; v < M; v++) MK[v] = 0;
    for(long i = 0; i < n; i++) MK[a[i]]++;               /* MARCA: nao apaga, conta */
    long k = 0;
    for(long v = 0; v < M; v++) for(long r = 0; r < MK[v]; r++){ ESC(); out[k++] = v; }
    for(long i = 0; i < n; i++){ ESC(); a[i] = out[i]; }
}

/* ── quicksort: compara e troca ───────────────────────────────────────────────────── */
static void quick(long *a, long n){
    while(n > 12){
        long m = n/2;
        CMP(); if(a[0] > a[m]){ TROCA(); long t=a[0]; a[0]=a[m]; a[m]=t; }
        CMP(); if(a[0] > a[n-1]){ TROCA(); long t=a[0]; a[0]=a[n-1]; a[n-1]=t; }
        CMP(); if(a[m] > a[n-1]){ TROCA(); long t=a[m]; a[m]=a[n-1]; a[n-1]=t; }
        long p = a[m], i = 0, j = n-1;
        while(i <= j){
            while(1){ CMP(); if(a[i] < p) i++; else break; }
            while(1){ CMP(); if(a[j] > p) j--; else break; }
            if(i <= j){ TROCA(); long t=a[i]; a[i]=a[j]; a[j]=t; i++; j--; }
        }
        if(j < n-1-i){ quick(a, j+1); a += i; n -= i; }
        else         { quick(a+i, n-i); n = j+1; }
    }
    for(long i = 1; i < n; i++){ long v=a[i], j=i-1;
        while(j>=0){ CMP(); if(a[j]>v){ ESC(); a[j+1]=a[j]; j--; } else break; }
        ESC(); a[j+1]=v; }
}

/* ── mergesort: compara e ESCREVE (nao troca) ─────────────────────────────────────── */
static void mrg(long *a, long *t, long n){
    if(n < 2) return;
    long m = n/2;
    mrg(a, t, m); mrg(a+m, t, n-m);
    long i=0, j=m, k=0;
    while(i<m && j<n){ CMP(); ESC(); t[k++] = (a[i]<=a[j]) ? a[i++] : a[j++]; }
    while(i<m){ ESC(); t[k++]=a[i++]; }
    while(j<n){ ESC(); t[k++]=a[j++]; }
    for(long q=0;q<n;q++){ ESC(); a[q]=t[q]; }
}

/* ── heapsort ─────────────────────────────────────────────────────────────────────── */
static void desce_h(long *a, long i, long n){
    for(;;){ long m=i, l=2*i+1, r=2*i+2;
        if(l<n){ CMP(); if(a[l]>a[m]) m=l; }
        if(r<n){ CMP(); if(a[r]>a[m]) m=r; }
        if(m==i) return;
        TROCA(); long t=a[i]; a[i]=a[m]; a[m]=t; i=m; }
}
static void heap(long *a, long n){
    for(long i=n/2-1;i>=0;i--) desce_h(a,i,n);
    for(long i=n-1;i>0;i--){ TROCA(); long t=a[0]; a[0]=a[i]; a[i]=t; desce_h(a,0,i); }
}

static unsigned long sem = 88172645463325252UL;
static unsigned long rnd(void){ sem ^= sem<<13; sem ^= sem>>7; sem ^= sem<<17; return sem; }

static void zera(void){ apagou_cmp = apagou_esc = trocas = 0; }
static void linha(const char *nome, long n){
    long total = apagou_cmp + apagou_esc;
    int64_t x10 = (int64_t)total * 10 / (int64_t)n;  /* bits/elem em décimos, sem float */
    int64_t whole = x10 / 10;
    int64_t frac = x10 % 10;
    printf("  %-12s %14ld %14ld %12ld %10" PRId64 ".%" PRId64 "\n",
           nome, apagou_cmp, apagou_esc, trocas, whole, frac);
}

int main(void){
    const long N = 200000;
    MK = malloc(2000001*sizeof(long));
    long *o = malloc(N*sizeof(long)), *a = malloc(N*sizeof(long)), *t = malloc(N*sizeof(long));
    for(long i=0;i<N;i++) o[i] = (long)(rnd() % 1000000);

    printf("\n  O QUE CADA ORDENACAO APAGA — n = %ld, uniforme\n", N);
    printf("  a regua e' Landauer (kT ln2 por bit), e nao o relogio\n\n");
    printf("  %-12s %14s %14s %12s %10s\n",
           "algoritmo", "bits/comparar", "bits/escrever", "trocas", "bits/elem");
    printf("  %-12s %14s %14s %12s %10s\n",
           "------------","--------------","--------------","------------","----------");

    memcpy(a,o,N*sizeof(long)); zera(); dual_sort(a, N, t); linha("Dual Sort", N);
    { /* O INVERTIDO CUSTA MAIS AO ALGORITMO? mede-se na regua dele, nao na da maquina */
      long *inv = malloc(N*sizeof(long)), *cre = malloc(N*sizeof(long));
      for(long i=0;i<N;i++){ inv[i] = N-i; cre[i] = i+1; }
      zera(); dual_sort(inv, N, t); long bi = apagou_cmp + apagou_esc;
      zera(); dual_sort(cre, N, t); long bc = apagou_cmp + apagou_esc;
      printf("\n  >>> invertido: %ld bits    crescente: %ld bits    %s\n",
             bi, bc, bi==bc ? "IGUAL — nao ha ordem privilegiada" : "DIFERE");
      free(inv); free(cre); }
    { long mau=0; for(long i=1;i<N;i++) if(a[i-1]>a[i]) mau++;
      printf("  >>> o Dual Sort ORDENOU? %s  (%ld pares fora de ordem)\n",
             mau?"NAO":"sim", mau); }
    memcpy(a,o,N*sizeof(long)); zera(); quick(a,N);       linha("quicksort", N);
    memcpy(a,o,N*sizeof(long)); zera(); mrg(a,t,N);       linha("mergesort", N);
    memcpy(a,o,N*sizeof(long)); zera(); heap(a,N);        linha("heapsort",  N);

    puts("");
    puts("  A TROCA NAO APAGA: e' involutiva — aplicada duas vezes devolve.");
    puts("  A COMPARACAO APAGA 127 bits de cada vez: 128 entram, um sai.");
    free(o); free(a); free(t);
    return 0;
}
