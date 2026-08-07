#define _POSIX_C_SOURCE 199309L
/* bench_dualsort.c — O DUAL SORT CONTRA OS OUTROS, medido e sem escolher o terreno.
 *
 * Isto NAO e' um medidor da bateria: tempo nao e' determinista e nao entra la'. E' um
 * banco de ensaio, e corre-se a mao:
 *
 *   cc -O2 -std=c99 -Wall bench_dualsort.c -o /tmp/bench && /tmp/bench
 *
 * O QUE SE COMPARA. O Dual Sort desce pela estaca (particao in-place pelo sinal de
 * x† - x, com o ponto fixo a nao pertencer a nenhum lado — o trial). Ao lado correm:
 *
 *   qsort da libc   o que qualquer pessoa usaria
 *   quicksort       particao por PIVO (mediana de tres)
 *   mergesort       o estavel classico
 *   heapsort        o de pior caso garantido
 *
 * E MEDE-SE EM TERRENO QUE EU NAO ESCOLHI: uniforme, quase ordenado, invertido, poucos
 * valores distintos, e ENVIESADO (o caso mau para quem parte por valor, e portanto para
 * o Dual Sort). Escolher so' o terreno favoravel seria escrever a resposta.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

/* ── EM P.U., que e' o que a teoria manda ────────────────────────────────────────────
 * «Em p.u. sao a mesma coisa; em absoluto separam-se pelo Delta.» A versao anterior
 * trabalhava em ABSOLUTO: descia os 8 niveis dos 64 bits mesmo quando os dados so'
 * ocupavam 20. Cinco passagens a toa por lista.
 *
 * Em p.u. normaliza-se pela escala PROPRIA da sequencia — o primeiro nivel com informacao
 * — e desce-se so' o que existe. E' a mesma regua vestida a roupa dos dados. */
#define BASE 256
static long simb(long x, int d){ return (x >> (8*d)) & (BASE-1); }

/* quantos simbolos a sequencia ocupa DE FACTO: a sua escala propria */
static int niveis_pu(const long *a, long n){
    long hi = 0;
    for(long i = 0; i < n; i++) if(a[i] > hi) hi = a[i];
    int k = 0; while(hi){ k++; hi >>= 8; }
    return k ? k : 1;
}

/* O DUAL SORT em p.u.: desce do simbolo MENOS significativo para o mais, e so' os niveis
 * que a sequencia ocupa. Sem centro calculado, sem comparacao entre elementos. */
static void dual_sort(long *a, long n, long *tmp){
    /* NAO SE VARRE para saber se ja' esta ordenada: medir para decidir e' interferir.
     * «Todo ponto e' fixo em alguma dimensao» — desce-se, e ele aparece onde tem de. */
    int niv = niveis_pu(a, n);
    for(int d = 0; d < niv; d++){
        long cont[BASE] = {0}, pos[BASE], acc = 0;
        for(long k = 0; k < n; k++) cont[simb(a[k], d)]++;
        for(int i = 0; i < BASE; i++){ pos[i] = acc; acc += cont[i]; }
        for(long k = 0; k < n; k++) tmp[pos[simb(a[k], d)]++] = a[k];
        for(long k = 0; k < n; k++) a[k] = tmp[k];
    }
}

/* ── quicksort com mediana de tres ─────────────────────────────────────────────────── */
static void quick(long *a, long n){
    while(n > 12){
        long m = n/2;
        if(a[0] > a[m]){ long t=a[0]; a[0]=a[m]; a[m]=t; }
        if(a[0] > a[n-1]){ long t=a[0]; a[0]=a[n-1]; a[n-1]=t; }
        if(a[m] > a[n-1]){ long t=a[m]; a[m]=a[n-1]; a[n-1]=t; }
        long p = a[m], i = 0, j = n-1;
        while(i <= j){
            while(a[i] < p) i++;
            while(a[j] > p) j--;
            if(i <= j){ long t=a[i]; a[i]=a[j]; a[j]=t; i++; j--; }
        }
        if(j < n-1-i){ quick(a, j+1); a += i; n -= i; }
        else         { quick(a+i, n-i); n = j+1; }
    }
    for(long i = 1; i < n; i++){ long v=a[i], j=i-1;
        while(j>=0 && a[j]>v){ a[j+1]=a[j]; j--; } a[j+1]=v; }
}

/* ── mergesort ─────────────────────────────────────────────────────────────────────── */
static void mrg(long *a, long *t, long n){
    if(n < 2) return;
    long m = n/2;
    mrg(a, t, m); mrg(a+m, t, n-m);
    long i=0, j=m, k=0;
    while(i<m && j<n) t[k++] = (a[i]<=a[j]) ? a[i++] : a[j++];
    while(i<m) t[k++]=a[i++];
    while(j<n) t[k++]=a[j++];
    memcpy(a, t, n*sizeof(long));
}

/* ── heapsort ──────────────────────────────────────────────────────────────────────── */
static void desce(long *a, long i, long n){
    for(;;){ long m=i, l=2*i+1, r=2*i+2;
        if(l<n && a[l]>a[m]) m=l;
        if(r<n && a[r]>a[m]) m=r;
        if(m==i) return;
        long t=a[i]; a[i]=a[m]; a[m]=t; i=m; }
}
static void heap(long *a, long n){
    for(long i=n/2-1;i>=0;i--) desce(a,i,n);
    for(long i=n-1;i>0;i--){ long t=a[0]; a[0]=a[i]; a[i]=t; desce(a,0,i); }
}

static int cmp(const void *x, const void *y){
    long a = *(const long*)x, b = *(const long*)y;
    return (a>b) - (a<b);
}

/* ── as distribuicoes, e nenhuma foi escolhida para favorecer ──────────────────────── */
static unsigned long sem = 88172645463325252UL;
static unsigned long rnd(void){ sem ^= sem<<13; sem ^= sem>>7; sem ^= sem<<17; return sem; }

static void gera(long *a, long n, int caso){
    for(long i = 0; i < n; i++){
        switch(caso){
            case 0: a[i] = rnd() % 1000000; break;                 /* uniforme */
            case 1: a[i] = i + (rnd()%16);  break;                 /* quase ordenado */
            case 2: a[i] = n - i;           break;                 /* invertido */
            case 3: a[i] = rnd() % 8;       break;                 /* poucos distintos */
            case 4: a[i] = (rnd()%100 == 0) ? (long)(rnd()%1000000) : (long)(rnd()%100);
                    break;                                          /* ENVIESADO */
        }
    }
}
static const char *nome_caso[5] = {
    "uniforme", "quase ordenado", "invertido", "8 valores", "enviesado"
};

static double agora(void){
    struct timespec t; clock_gettime(CLOCK_MONOTONIC, &t);
    return t.tv_sec + t.tv_nsec/1e9;
}

int main(void){
    const long N = 1000000;
    long *orig = malloc(N*sizeof(long));
    long *a    = malloc(N*sizeof(long));
    long *tmp  = malloc(N*sizeof(long));
    if(!orig || !a || !tmp){ puts("sem memoria"); return 1; }

    printf("\n  O DUAL SORT CONTRA OS OUTROS — n = %ld, tempo em ms (menor e' melhor)\n\n", N);
    printf("  %-16s %10s %10s %10s %10s %10s\n",
           "caso", "DualSort", "qsort", "quick", "merge", "heap");
    printf("  %-16s %10s %10s %10s %10s %10s\n",
           "----------------","--------","--------","--------","--------","--------");

    for(int caso = 0; caso < 5; caso++){
        gera(orig, N, caso);
        double t[5];
        memcpy(a, orig, N*sizeof(long)); double t0=agora(); dual_sort(a, N, tmp);      t[0]=agora()-t0;
        /* e a confirmacao: saiu mesmo ordenado? */
        long mau = 0; for(long i=1;i<N;i++) if(a[i-1]>a[i]) mau++;
        memcpy(a, orig, N*sizeof(long)); t0=agora(); qsort(a,N,sizeof(long),cmp); t[1]=agora()-t0;
        memcpy(a, orig, N*sizeof(long)); t0=agora(); quick(a, N);                 t[2]=agora()-t0;
        memcpy(a, orig, N*sizeof(long)); t0=agora(); mrg(a, tmp, N);              t[3]=agora()-t0;
        memcpy(a, orig, N*sizeof(long)); t0=agora(); heap(a, N);                  t[4]=agora()-t0;
        printf("  %-16s %10.1f %10.1f %10.1f %10.1f %10.1f%s\n", nome_caso[caso],
               t[0]*1000, t[1]*1000, t[2]*1000, t[3]*1000, t[4]*1000,
               mau ? "   <- NAO ORDENOU" : "");
    }

    puts("");
    free(orig); free(a); free(tmp);
    return 0;
}
