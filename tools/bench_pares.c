#define _POSIX_C_SOURCE 199309L
/* bench_pares.c — O DUAL SORT EM PARES, que e' onde ele vive.
 *
 * O bench anterior media ESCALARES, e escalar e' o caso onde a cruz nao tem segunda
 * coordenada para projectar — a orbita degenera em dois. Aqui ordenam-se PARES, que e' a
 * torre em R², e onde as duas involucoes sao independentes.
 *
 * Cada elemento e' (a,b) e a ordem e' a do par. Comparam-se:
 *
 *   Dual Sort   desce pela estaca na PRIMEIRA coordenada; os iguais caem na fronteira e
 *               descem pela SEGUNDA — e' a cruz a lancar a projeccao no dual
 *   qsort       da libc, com comparador de par
 *   quicksort   mediana de tres, comparador de par
 *   mergesort   estavel
 *
 * A distribuicao das primeiras coordenadas e' que decide, e por isso ela varre de MUITA
 * repeticao a nenhuma — sem escolher o terreno.
 *
 *   cc -O2 -std=c99 -Wall bench_pares.c -o /tmp/bp && /tmp/bp
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <stdint.h>
#include <inttypes.h>

typedef struct { long a, b; } Par;

static long estaca(long x, long c){ return 2*c - x; }

/* ordem do par: primeiro a, depois b */
static int menor(Par x, Par y){ return x.a < y.a || (x.a == y.a && x.b < y.b); }

/* ── O DUAL SORT EM PARES. Desce pela estaca na coordenada `dim`; quem cai na FRONTEIRA
 * (o ponto fixo) desce pela coordenada seguinte — a passagem da dimensao. */
static void dual_pares(Par *v, long n, int dim){
    while(n > 1){
        long lo, hi;
        lo = hi = dim ? v[0].b : v[0].a;
        for(long i = 1; i < n; i++){
            long x = dim ? v[i].b : v[i].a;
            if(x < lo) lo = x;
            if(x > hi) hi = x;
        }
        if(lo == hi){                       /* esta coordenada saturou: PASSA A DIMENSAO */
            if(dim == 0) dual_pares(v, n, 1);
            return;
        }
        long c = lo + (hi - lo)/2;
        long i = 0, j = 0, k = n;
        while(j < k){
            long x = dim ? v[j].b : v[j].a;
            long d = estaca(x, c);
            if(d > x){ Par t=v[i]; v[i]=v[j]; v[j]=t; i++; j++; }
            else if(d < x){ k--; Par t=v[j]; v[j]=v[k]; v[k]=t; }
            else j++;
        }
        /* a FRONTEIRA (os iguais a c) desce pela dimensao seguinte */
        if(dim == 0 && k > i) dual_pares(v + i, k - i, 1);
        if(i < n - k){ dual_pares(v, i, dim); v += k; n -= k; }
        else         { dual_pares(v + k, n - k, dim); n = i; }
    }
}

static void quick(Par *v, long n){
    while(n > 12){
        long m = n/2;
        if(menor(v[m],v[0])){ Par t=v[0]; v[0]=v[m]; v[m]=t; }
        if(menor(v[n-1],v[0])){ Par t=v[0]; v[0]=v[n-1]; v[n-1]=t; }
        if(menor(v[n-1],v[m])){ Par t=v[m]; v[m]=v[n-1]; v[n-1]=t; }
        Par p = v[m]; long i = 0, j = n-1;
        while(i <= j){
            while(menor(v[i],p)) i++;
            while(menor(p,v[j])) j--;
            if(i <= j){ Par t=v[i]; v[i]=v[j]; v[j]=t; i++; j--; }
        }
        if(j < n-1-i){ quick(v, j+1); v += i; n -= i; }
        else         { quick(v+i, n-i); n = j+1; }
    }
    for(long i = 1; i < n; i++){ Par k=v[i]; long j=i-1;
        while(j>=0 && menor(k,v[j])){ v[j+1]=v[j]; j--; } v[j+1]=k; }
}

static void mrg(Par *v, Par *t, long n){
    if(n < 2) return;
    long m = n/2;
    mrg(v, t, m); mrg(v+m, t, n-m);
    long i=0, j=m, k=0;
    while(i<m && j<n) t[k++] = menor(v[j],v[i]) ? v[j++] : v[i++];
    while(i<m) t[k++]=v[i++];
    while(j<n) t[k++]=v[j++];
    memcpy(v, t, n*sizeof(Par));
}

static int cmp(const void *x, const void *y){
    const Par *p = x, *q = y;
    if(p->a != q->a) return (p->a > q->a) - (p->a < q->a);
    return (p->b > q->b) - (p->b < q->b);
}

static unsigned long sem = 88172645463325252UL;
static unsigned long rnd(void){ sem ^= sem<<13; sem ^= sem>>7; sem ^= sem<<17; return sem; }

static int64_t agora_ns(void){
    struct timespec t; clock_gettime(CLOCK_MONOTONIC, &t);
    return (int64_t)t.tv_sec * 1000000000 + (int64_t)t.tv_nsec;
}
static void ms10_print(int64_t ns){
    int64_t v = (ns + 50000) / 100000;
    printf("%10" PRId64 ".%" PRId64, v / 10, v % 10);
}

int main(void){
    const long N = 1000000;
    Par *orig = malloc(N*sizeof(Par)), *v = malloc(N*sizeof(Par)), *t = malloc(N*sizeof(Par));
    if(!orig || !v || !t){ puts("sem memoria"); return 1; }

    printf("\n  O DUAL SORT EM PARES — n = %ld, ms (menor e' melhor)\n", N);
    printf("  a primeira coordenada varre de MUITA repeticao a NENHUMA\n\n");
    printf("  %-22s %10s %10s %10s %10s\n", "1a coord. distinta", "DualSort", "qsort", "quick", "merge");
    printf("  %-22s %10s %10s %10s %10s\n", "----------------------",
           "--------","--------","--------","--------");

    long chaves[6] = { 4, 64, 1024, 16384, 262144, 1000000 };
    for(int c = 0; c < 6; c++){
        for(long i = 0; i < N; i++){
            orig[i].a = (long)(rnd() % (unsigned long)chaves[c]);
            orig[i].b = (long)(rnd() % 1000000);
        }
        int64_t d[4];
        memcpy(v, orig, N*sizeof(Par)); int64_t t0=agora_ns(); dual_pares(v, N, 0); d[0]=agora_ns()-t0;
        long mau = 0; for(long i=1;i<N;i++) if(menor(v[i],v[i-1])) mau++;
        memcpy(v, orig, N*sizeof(Par)); t0=agora_ns(); qsort(v,N,sizeof(Par),cmp); d[1]=agora_ns()-t0;
        memcpy(v, orig, N*sizeof(Par)); t0=agora_ns(); quick(v, N);                d[2]=agora_ns()-t0;
        memcpy(v, orig, N*sizeof(Par)); t0=agora_ns(); mrg(v, t, N);               d[3]=agora_ns()-t0;
        printf("  %-22ld", chaves[c]);
        for(int k = 0; k < 4; k++) ms10_print(d[k]);
        printf("%s\n", mau?"  <- NAO ORDENOU":"");
    }
    puts("");
    free(orig); free(v); free(t);
    return 0;
}
