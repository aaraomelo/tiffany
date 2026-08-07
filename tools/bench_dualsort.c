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

/* ── a estaca: troca de lado num conjunto de centro c ──────────────────────────────── */
static long estaca(long x, long c){ return 2*c - x; }

/* ── O DUAL SORT, particao in-place. Tres lados (o trial): < c, = c, > c.
 * O destino sai do SINAL de x† - x, e nao de uma comparacao entre elementos. */
static void dual_sort(long *a, long n){
    while(n > 1){
        long lo = a[0], hi = a[0];
        for(long i = 1; i < n; i++){ if(a[i] < lo) lo = a[i]; if(a[i] > hi) hi = a[i]; }
        if(lo == hi) return;                         /* todos iguais: ja' esta' */
        long c = lo + (hi - lo)/2;
        /* particao de Dijkstra em tres: [<c][=c][>c] */
        long i = 0, j = 0, k = n;
        while(j < k){
            long d = estaca(a[j], c);
            if(d > a[j]){ long t=a[i]; a[i]=a[j]; a[j]=t; i++; j++; }   /* x < c */
            else if(d < a[j]){ k--; long t=a[j]; a[j]=a[k]; a[k]=t; }   /* x > c */
            else j++;                                                   /* x = c: fica */
        }
        /* desce no lado menor por recursao e itera no maior: a pilha fica em O(log n) */
        if(i < n - k){ dual_sort(a, i); a += k; n -= k; }
        else         { dual_sort(a + k, n - k); n = i; }
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
        memcpy(a, orig, N*sizeof(long)); double t0=agora(); dual_sort(a, N);      t[0]=agora()-t0;
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
