/* tools/_probe_duomorf_prof.c — experiência mínima III (redes.tex).
 *
 * Pergunta:  prof(Da,Db)  ?==  prof(a,b)
 * prof: fis:def:arvore (primeira divergência, MSB). Não se usa u, Hamming, lexMax.
 *
 * Mapas já existentes — nenhuma definição nova:
 *   D_can  i ↦ (N-1)-i     fis:thm:dual(2) = R†∘R^{-1} no índice
 *   T_rev  reversão de p bits   fis:prop:travessia (controlo: não é D)
 *   reloc  wasm_erg relocAddr   o mapa de endereços que o pipe realmente aplica
 *
 *   cc -O2 -std=c99 tools/_probe_duomorf_prof.c -o tools/_probe_duomorf_prof
 *   ./tools/_probe_duomorf_prof
 */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

static int prof(unsigned a, unsigned b, int p)
{
    for (int q = 0; q < p; q++)
        if (((a >> (p - 1 - q)) & 1u) != ((b >> (p - 1 - q)) & 1u)) return q;
    return p;
}

static unsigned d_can(unsigned i, unsigned mask)
{
    return mask - i;
}

static unsigned t_rev(unsigned x, int p)
{
    unsigned r = 0;
    for (int i = 0; i < p; i++)
        if (x & (1u << i)) r |= 1u << (p - 1 - i);
    return r;
}

/* lib/wasm_erg.mjs relocAddr */
static unsigned reloc(unsigned k)
{
    return k >= 65536u ? 65400u + (k - 65536u) : k;
}

struct placar {
    unsigned long pares, batem, dif;
};

static void conta(unsigned *A, unsigned n, int p,
                  unsigned (*f)(unsigned, int), int f_p,
                  struct placar *out)
{
    out->pares = out->batem = out->dif = 0;
    for (unsigned i = 0; i < n; i++)
        for (unsigned j = 0; j < n; j++) {
            unsigned a = A[i], b = A[j];
            int q = prof(a, b, p);
            int qD = prof(f(a, f_p), f(b, f_p), p);
            out->pares++;
            if (q == qD) out->batem++;
            else out->dif++;
        }
}

static unsigned wrap_can(unsigned x, int p)
{
    unsigned N = 1u << p;
    return d_can(x, N - 1);
}

static unsigned wrap_rev(unsigned x, int p)
{
    return t_rev(x, p);
}

static unsigned wrap_reloc(unsigned x, int p)
{
    (void)p;
    return reloc(x);
}

static void reporta(const char *nome, struct placar s)
{
    printf("  %-28s  %lu/%lu  %s\n",
           nome, s.batem, s.pares,
           s.dif == 0 ? "preserva" : "NAO preserva");
}

int main(void)
{
    printf("III  prof(Da,Db) ?== prof(a,b)\n");
    printf("prof = primeira divergencia MSB  (fis:def:arvore)\n\n");

    /* ── 1. D_can no dominio da arvore p=8 e p=12 ── */
    for (int p = 8; p <= 12; p += 4) {
        unsigned N = 1u << p;
        unsigned *A = malloc((size_t)N * sizeof *A);
        if (!A) return 1;
        for (unsigned i = 0; i < N; i++) A[i] = i;
        struct placar c, r;
        conta(A, N, p, wrap_can, p, &c);
        conta(A, N, p, wrap_rev, p, &r);
        printf("arvore p=%d  N=%u  pares=%lu  (inclui a=b)\n", p, N, c.pares);
        reporta("D_can  (N-1)-i", c);
        reporta("T_rev  bits invertidos", r);
        free(A);
        printf("\n");
    }

    /* ── 2. relocAddr do pipe, tres subdominios, prof 32 ── */
    {
        enum { M = 256, P = 32 };
        unsigned low[M], high[M], mix[M * 2];
        for (unsigned i = 0; i < M; i++) {
            low[i] = i;
            high[i] = 65536u + i;
            mix[i] = low[i];
            mix[M + i] = high[i];
        }
        struct placar L, H, X;
        conta(low, M, P, wrap_reloc, P, &L);
        conta(high, M, P, wrap_reloc, P, &H);
        conta(mix, M * 2, P, wrap_reloc, P, &X);
        printf("relocAddr  wasm_erg  (prof 32 bits)\n");
        reporta("subdom [0,256)", L);
        reporta("subdom [65536,65536+256)", H);
        reporta("mistura low+high", X);
        printf("\n");
    }

    /* ── 3. so a!=b em p=8 para D_can e T_rev ── */
    {
        int p = 8;
        unsigned N = 1u << p;
        unsigned long pares = 0, can = 0, rev = 0;
        unsigned mask = N - 1;
        for (unsigned a = 0; a < N; a++)
            for (unsigned b = 0; b < N; b++) {
                if (a == b) continue;
                pares++;
                if (prof(d_can(a, mask), d_can(b, mask), p) == prof(a, b, p)) can++;
                if (prof(t_rev(a, p), t_rev(b, p), p) == prof(a, b, p)) rev++;
            }
        printf("p=8  so a!=b  pares=%lu\n", pares);
        printf("  D_can  %lu/%lu\n", can, pares);
        printf("  T_rev  %lu/%lu\n", rev, pares);
    }
    return 0;
}
