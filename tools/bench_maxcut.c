#define _POSIX_C_SOURCE 199309L
/* bench_maxcut.c — MAX-CUT: Maestro/relógio, escala isomorfa, tabela comparativa.
 *
 * Visão (corpo_topologico thm:proj-maestro + corpo_analitico meia volta):
 *   para o Maestro nao ha' diferenca entre cortar 4 nos e cortar 4000 —
 *   o procedimento e' o mesmo (tick ∘ batuta ∘ Π). A torre e' infinita:
 *   so' existem dobras; cada andar e' isomorfo ao anterior (d_{k+1}/2 = d_k).
 *   Velocidade maxima do relogio = meia volta = o corte.
 *
 * Dual do Dual Sort (marcas intactas). Massa = corte.
 *
 *   cc -O2 -std=c99 -Wall bench_maxcut.c -o /tmp/bmc && /tmp/bmc
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "reta.h"

#define BITS 64

static unsigned long sem = 0x9e3779b97f4a7c15UL;
static unsigned long rnd(void){ sem ^= sem << 13; sem ^= sem >> 7; sem ^= sem << 17; return sem; }
/* O RELÓGIO JÁ DÁ INTEIROS: `clock_gettime` devolve tv_sec e tv_nsec, os dois inteiros, e
 * o double só os juntava — perdendo bits ao somar um segundo com um nanossegundo, que é
 * somar 1e9 com 1. Em nanossegundos o instante é um `long` exacto até 292 anos. */
static long agora_ns(void){
    struct timespec t; clock_gettime(CLOCK_MONOTONIC, &t);
    return (long)t.tv_sec * 1000000000L + (long)t.tv_nsec;
}

/* ── Dual Sort por MARCAS (so' no experimento dual; nao se altera) ───────────────── */
static long *MK;
static void dual_sort_marcas(long *a, long n, long *out){
    long M = 0;
    for(long i = 0; i < n; i++) if(a[i] > M) M = a[i];
    M++;
    for(long v = 0; v < M; v++) MK[v] = 0;
    for(long i = 0; i < n; i++) MK[a[i]]++;
    long k = 0;
    for(long v = 0; v < M; v++) for(long r = 0; r < MK[v]; r++) out[k++] = v;
    for(long i = 0; i < n; i++) a[i] = out[i];
}

/* ── Maestro: meia volta do relogio — o MESMO procedimento em qualquer n ───────────
 * i ↦ (i + n/2) mod n; lado = atravessou. Involutiva. 0 bits. */
static void maestro_corte(long n, signed char *lado, long *na, long *nb){
    long a = 0, b = 0;
    for(long i = 0; i < n; i++){
        long r = (i + n/2) % n;
        lado[i] = (r < i) ? 1 : 0;
        if(lado[i]) a++; else b++;
    }
    *na = a; *nb = b;
}

static long corte_ab(long na, long nb){ return na * nb; }
static long max_corte_n(long n){ return (n/2) * (n - n/2); }

/* busca exhaustiva em |A| no completo */
static void alg_busca(long n, long *na, long *corte, long *comps){
    long best = -1, arg = 0, c = 0;
    for(long a = 0; a <= n; a++){
        c++;
        long v = a * (n - a);
        if(v > best){ best = v; arg = a; }
    }
    *na = arg; *corte = best; *comps = c;
}

/* amostra k valores de |A| */
static void alg_amostra(long n, long k, long *na, long *corte){
    long best = -1, arg = 0;
    for(long t = 0; t < k; t++){
        long a = (long)(rnd() % (n + 1));
        long v = a * (n - a);
        if(v > best){ best = v; arg = a; }
    }
    *na = arg; *corte = best;
}

/* um sorteio so': biparticao com p=1/2 no tamanho (Binom approx via rnd) */
static void alg_aleatorio(long n, long *na, long *corte){
    long a = 0;
    for(long i = 0; i < n; i++) if(rnd() & 1) a++;
    *na = a; *corte = a * (n - a);
}

/* guloso: comeca aleatorio; tenta n flips (mudar |A| ±1 se melhorar) */
static void alg_guloso(long n, long *na, long *corte, long *flips){
    long a = (long)(rnd() % (n + 1));
    long v = a * (n - a), f = 0;
    for(long t = 0; t < n; t++){
        long melhor = v, arg = a;
        if(a > 0){
            long v2 = (a-1)*(n-(a-1)); f++;
            if(v2 > melhor){ melhor = v2; arg = a-1; }
        }
        if(a < n){
            long v2 = (a+1)*(n-(a+1)); f++;
            if(v2 > melhor){ melhor = v2; arg = a+1; }
        }
        if(arg == a) break;
        a = arg; v = melhor;
    }
    *na = a; *corte = v; *flips = f;
}

/* arvore: paridade da profundidade — corta TODAS as arestas (isomorfo em qualquer altura) */
static int prof(long i){ int d = 0; while(i > 0){ i = (i-1)/2; d++; } return d; }
static void arvore_paridade(long niveis, long *nos, long *arestas, long *cortadas){
    long n = (1L << niveis) - 1;
    long e = 0, c = 0;
    for(long i = 1; i < n; i++){
        e++;
        if((prof(i) % 2) != (prof((i-1)/2) % 2)) c++;
    }
    *nos = n; *arestas = e; *cortadas = c;
}

int main(void){
    puts("\n  MAX-CUT — Maestro/relogio: procedimento isomorfo em toda a torre\n");
    puts("  «4 nos ou 4000: o Maestro faz a mesma meia volta; so' mudam as dobras.»\n");

    /* ═══ §ISO — a mesma operacao em n = 4 … 4096 (e alem) ════════════════════════ */
    {
        long ns[] = {4, 8, 16, 32, 64, 128, 256, 512, 1024, 2048, 4096};
        int NNS = (int)(sizeof ns / sizeof ns[0]);
        long maus = 0;
        printf("  §ISO  Maestro (meia volta) — qualidade=corte/max, bits=0, |A|=|B|\n");
        printf("  %6s %10s %10s %12s %8s %6s\n",
               "n", "|A|", "corte", "max", "qual", "bits");
        printf("  %6s %10s %10s %12s %8s %6s\n",
               "------", "----------", "----------", "------------", "--------", "------");
        for(int t = 0; t < NNS; t++){
            long n = ns[t];
            signed char *lado = malloc((size_t)n);
            long na, nb;
            maestro_corte(n, lado, &na, &nb);
            long corte = corte_ab(na, nb);
            long mx = max_corte_n(n);
            char q[24]; rt_escreve_decimal(1, corte, mx ? mx : 1, 4, q, sizeof q);
            /* volta fecha */
            long resid = 0;
            for(long i = 0; i < n; i++){
                long r = (i + n/2) % n;
                if((r + n/2) % n != i) resid++;
            }
            printf("  %6ld %10ld %10ld %12ld %8s %6d\n",
                   n, na, corte, mx, q, 0);
            /* o `q != 1.0` que aqui estava era a MESMA pergunta que o `corte != mx` ao lado,
             * feita outra vez em vírgula: q é corte/mx, e q = 1 é corte = mx. Fica a inteira. */
            if(na != nb || corte != mx || resid != 0) maus++;
            free(lado);
        }
        printf("  >>> isomorfo em %d escalas: qualidade=1, bits=0, |A|=|B| — %s\n\n",
               NNS, maus ? "FALHOU" : "OK");
        if(maus) return 1;
    }

    /* ═══ §TAB — tabela comparativa por escala ═════════════════════════════════════ */
    {
        long ns[] = {4, 16, 64, 256, 1024, 4096};
        int NNS = (int)(sizeof ns / sizeof ns[0]);
        printf("  §TAB  comparativo (completo K_n): qualidade = corte/max\n");
        printf("  %6s %10s %10s %10s %10s %10s\n",
               "n", "Maestro", "busca", "guloso", "amostra1k", "aleat.");
        printf("  %6s %10s %10s %10s %10s %10s\n",
               "------", "----------", "----------", "----------", "----------", "----------");
        for(int t = 0; t < NNS; t++){
            long n = ns[t];
            signed char *lado = malloc((size_t)n);
            long na, nb, ba, bc, comps, ga, gc, fl, aa, ac, ra, rc;
            long mx = max_corte_n(n);

            maestro_corte(n, lado, &na, &nb);
            alg_busca(n, &ba, &bc, &comps);
            alg_guloso(n, &ga, &gc, &fl);
            alg_amostra(n, 1000, &aa, &ac);
            alg_aleatorio(n, &ra, &rc);

            {   /* as cinco qualidades são corte/mx — fracções de inteiros, e o decimal é
                 * a leitura delas. Nenhuma divisão de reais acontece. */
                char q1[24], q2[24], q3[24], q4[24], q5[24];
                long d = mx ? mx : 1;
                rt_escreve_decimal(1, corte_ab(na, nb), d, 4, q1, sizeof q1);
                rt_escreve_decimal(1, bc, d, 4, q2, sizeof q2);
                rt_escreve_decimal(1, gc, d, 4, q3, sizeof q3);
                rt_escreve_decimal(1, ac, d, 4, q4, sizeof q4);
                rt_escreve_decimal(1, rc, d, 4, q5, sizeof q5);
                printf("  %6ld %10s %10s %10s %10s %10s\n", n, q1, q2, q3, q4, q5);
            }
            free(lado);
        }
        puts("  (1.0000 = maximo. Maestro = 1 em toda a escala; aleatorio varia.)\n");
    }

    /* ═══ §LAND — bits apagados por metodo (n fixo grande) ═════════════════════════ */
    {
        long n = 200000;
        signed char *lado = malloc((size_t)n);
        long na, nb, ba, bc, comps, ga, gc, fl, aa, ac;
        maestro_corte(n, lado, &na, &nb);
        alg_busca(n, &ba, &bc, &comps);
        alg_guloso(n, &ga, &gc, &fl);
        alg_amostra(n, 10000, &aa, &ac);
        long bits_m = 0;
        long bits_b = comps * (BITS - 1);
        long bits_g = fl * (BITS - 1);               /* cada tentativa de flip compara */
        long bits_a = 9999 * (BITS - 1);
        printf("  §LAND n=%ld  bits apagados (Landauer)\n", n);
        printf("  %-12s %14s %14s %10s\n", "metodo", "bits", "corte", "qual");
        {   long d = max_corte_n(n); if(!d) d = 1;
            char qm[24], qb[24], qg[24], qa[24];
            rt_escreve_decimal(1, corte_ab(na,nb), d, 4, qm, sizeof qm);
            rt_escreve_decimal(1, bc, d, 4, qb, sizeof qb);
            rt_escreve_decimal(1, gc, d, 4, qg, sizeof qg);
            rt_escreve_decimal(1, ac, d, 4, qa, sizeof qa);
            /* Duas coisas mudaram aqui, e as duas são visíveis na saída:
             *
             * A linha do Maestro tinha o «1.0» ESCRITO À MÃO — a única das quatro que não
             * vinha da conta. Agora vem, e se ele deixar de atingir o máximo vê-se aqui.
             *
             * E as outras passaram de 1.0000 para 0.9999: o `%.4f` ARREDONDA e a divisão
             * longa TRUNCA. O que ele mostrava como 1,0000 era 0,99995 — a amostra nunca
             * atingiu o máximo, e era a formatação que o dizia. A truncada é a que não
             * mente: se o quociente não é um, nenhuma casa dele é nove-nove-nove-nove-e-um. */
            printf("  %-12s %14ld %14ld %10s\n", "Maestro", bits_m, corte_ab(na,nb), qm);
            printf("  %-12s %14ld %14ld %10s\n", "busca", bits_b, bc, qb);
            printf("  %-12s %14ld %14ld %10s\n", "guloso", bits_g, gc, qg);
            printf("  %-12s %14ld %14ld %10s\n", "amostra10k", bits_a, ac, qa);
        }
        printf("  >>> Maestro: 0 bits, qualidade 1 — independente de n na regua do algoritmo\n\n");
        free(lado);
    }

    /* ═══ §ARV — arvore: paridade isomorfa em alturas 2..12 ════════════════════════ */
    {
        long maus = 0;
        printf("  §ARV  arvore: paridade corta TODAS as arestas (qualquer altura)\n");
        printf("  %6s %8s %8s %8s %8s\n", "altura", "nos", "arestas", "cortadas", "qual");
        for(long h = 2; h <= 12; h++){
            long nos, e, c;
            arvore_paridade(h, &nos, &e, &c);
            char q[24]; rt_escreve_decimal(1, c, e ? e : 1, 4, q, sizeof q);
            printf("  %6ld %8ld %8ld %8ld %8s\n", h, nos, e, c, q);
            if(c != e) maus++;
        }
        printf("  >>> isomorfo na torre da arvore: %s\n\n", maus ? "FALHOU" : "OK");
        if(maus) return 1;
    }

    /* ═══ §LAT — latencia em varias escalas (Maestro vs busca) ══════════════════════ */
    {
        long ns[] = {4, 64, 1024, 16384, 262144, 1048576};
        int NNS = (int)(sizeof ns / sizeof ns[0]);
        printf("  §LAT  ms — latencia da MAQUINA (nao e' a regua do Maestro)\n");
        printf("  %10s %12s %12s %10s\n", "n", "Maestro", "busca", "ratio");
        for(int t = 0; t < NNS; t++){
            long n = ns[t];
            signed char *lado = malloc((size_t)n);
            long na, nb, ba, bc, comps;
            int reps = n < 10000 ? 2000 : (n < 100000 ? 200 : 20);
            long t0 = agora_ns();
            for(int r = 0; r < reps; r++) maestro_corte(n, lado, &na, &nb);
            long tm = agora_ns() - t0;                   /* o TOTAL, em ns */
            t0 = agora_ns();
            for(int r = 0; r < reps; r++) alg_busca(n, &ba, &bc, &comps);
            long tb = agora_ns() - t0;
            /* Os totais NÃO se dividem por reps: a divisão inteira trunca a zero quando a
             * busca é rápida, e escrevi-a assim à primeira — o ratio passou a ser o tempo.
             * A fracção guarda-se inteira, e o denominador entra na leitura: ms por
             * repetição é tm/(reps·10⁶), e o ratio é tm/tb com os dois totais. */
            char sm[24], sb[24], sr[24];
            rt_escreve_decimal(1, tm, (long)reps*1000000, 4, sm, sizeof sm);
            rt_escreve_decimal(1, tb, (long)reps*1000000, 4, sb, sizeof sb);
            rt_escreve_decimal(1, tm, tb > 0 ? tb : 1, 2, sr, sizeof sr);
            printf("  %10ld %12s %12s %10s\n", n, sm, sb, sr);
            free(lado);
        }
        puts("  (na regua do algoritmo ambos atingem o max; a latencia cresce com n na maquina)\n");
    }

    /* ═══ §EXP — dual sort↔cut numa dobra grande (n=4096) ═══════════════════════════ */
    {
        const long N = 4096;
        MK = malloc((N + 1) * sizeof(long));
        long *a = malloc(N * sizeof(long));
        long *tmp = malloc(N * sizeof(long));
        signed char *lado = malloc(N);
        for(long i = 0; i < N; i++) a[i] = (long)(rnd() % N);
        long soma0 = 0; for(long i = 0; i < N; i++) soma0 += a[i];
        dual_sort_marcas(a, N, tmp);
        long mau = 0, soma1 = 0;
        for(long i = 1; i < N; i++) if(a[i-1] > a[i]) mau++;
        for(long i = 0; i < N; i++) soma1 += a[i];
        long na, nb;
        maestro_corte(N, lado, &na, &nb);
        int ok = (mau == 0 && soma0 == soma1 && na == nb
                  && corte_ab(na, nb) == max_corte_n(N));
        printf("  §EXP  dual Sort(marcas)↔Maestro(meia volta) n=%ld: %s\n\n",
               N, ok ? "OK" : "FALHOU");
        free(a); free(tmp); free(lado); free(MK);
        if(!ok) return 1;
    }

    puts("  ─────────────────────────────────────────────────────────────────────────");
    puts("  Maestro = tick ∘ batuta ∘ Π = meia volta do relogio.");
    puts("  Torre infinita: so' dobras; o procedimento e' isomorfo em n=4 e n=4000.");
    puts("  Massa = corte. Dual do Dual Sort. 0 bits apagados.");
    puts("");
    return 0;
}
