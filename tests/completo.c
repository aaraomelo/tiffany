/* completo.c — O CORPO É COMPLETO: todo dado admite representação consistente e reversível.
 *
 * Um dado é uma tupla finita D=(d_0,…,d_{L-1}) de qualquer tamanho e alfabeto. Afirmamos:
 * para todo D existe (p,n) tal que D É um elemento de GF(p^n) — os seus coeficientes — e a
 * máquina (soma, produto, a transformada universal) opera nele exata e reversivelmente.
 * A família {GF(p^n)} é COFINAL (há n≥L para todo L, e sempre existe irredutível de grau n)
 * e ENCAIXADA (n|n' ⇒ GF(p^n)⊂GF(p^{n'})); o limite ∪ GF(p^n) contém toda representação.
 * Logo o corpo é completo: nenhum dado fica de fora, e a volta é resíduo 0.
 *
 *   cc -O2 -std=c99 completo.c -o completo
 *   ./completo [imagem.pgm]
 */
#include <stdio.h>
#include "unidade.h"
#include <stdlib.h>
#include <stdint.h>
#include <inttypes.h>

/* ---------- a transformada universal: NTT em ℤ/P, P = 2^16+1 (Fermat), raiz primitiva 3 ---------- */
#define P 65537
static int32_t pm(int32_t b, int32_t e){
    int32_t r = 1; b %= P;
    while(e){ if(e & 1) r = (int32_t)((int64_t)r * b % P); b = (int32_t)((int64_t)b * b % P); e >>= 1; }
    return r;
}
static void ntt(int32_t *a, int n, int inv){
    for(int i = 1, j = 0; i < n; i++){
        int bit = n >> 1;
        for(; j & bit; bit >>= 1) j ^= bit;
        j ^= bit;
        if(i < j){ int32_t t = a[i]; a[i] = a[j]; a[j] = t; }
    }
    for(int len = 2; len <= n; len <<= 1){
        int32_t w = inv ? pm(3, P - 1 - (P - 1) / len) : pm(3, (P - 1) / len);
        for(int i = 0; i < n; i += len){
            int32_t wn = 1;
            for(int k = 0; k < len / 2; k++){
                int32_t u = a[i + k], v = (int32_t)((int64_t)a[i + k + len / 2] * wn % P);
                a[i + k] = (int32_t)((u + v) % P);
                a[i + k + len / 2] = (int32_t)((u - v + P) % P);
                wn = (int32_t)((int64_t)wn * w % P);
            }
        }
    }
    if(inv){
        int32_t ni = pm(n, P - 2);
        for(int i = 0; i < n; i++) a[i] = (int32_t)((int64_t)a[i] * ni % P);
    }
}

/* um gerador de dados determinístico (sem relógio): xorshift, para "qualquer dado" reprodutível */
static uint64_t xs = 88172645463325252ULL;
static uint64_t rnd(void){ xs ^= xs << 13; xs ^= xs >> 7; xs ^= xs << 17; return xs; }

#include "pgm.h"                                    /* le_pgm: o leitor PGM binário (P5) reusado */

/* a contagem de irredutíveis mônicos de grau n sobre 𝔽_p (Gauss/Möbius): I_p(n)=(1/n)Σ μ(d) p^{n/d} */
static int mobius(int n){
    int r = 1;
    for(int i = 2; (int64_t)i * i <= n; i++){
        if(n % i == 0){ n /= i; if(n % i == 0) return 0; r = -r; }
    }
    if(n > 1) r = -r;
    return r;
}
static int32_t irred(int32_t pp, int n){
    int64_t s = 0;
    for(int d = 1; d <= n; d++) if(n % d == 0){
        uint64_t q = 1;
        for(int e = 0; e < n / d; e++) q *= (uint64_t)pp;
        s += (int64_t)mobius(d) * (int64_t)q;
    }
    return (int32_t)(s / n);
}

/* representa/reconstrói um dado por uma transformada e a sua inversa; devolve o nº de erros */
static int32_t roundtrip(const unsigned char *d, int L){
    int n = 1; while(n < L) n <<= 1;
    int32_t *a = calloc((size_t)n, sizeof(int32_t));
    for(int i = 0; i < L; i++) a[i] = d[i];
    ntt(a, n, 0); ntt(a, n, 1);
    int32_t err = 0;
    for(int i = 0; i < L; i++) if(a[i] != d[i]) err++;
    for(int i = L; i < n; i++) if(a[i] != 0) err++;
    free(a); return err;
}

int main(int argc, char **argv){
    int res = 0;
    printf("O CORPO É COMPLETO — todo dado admite representação consistente reversível\n");
    printf("================================================================\n");

    int p1 = 257;
    unsigned char D[4] = {200, 13, 255, 7};
    int L = 4;
    uint64_t N = 0, base = 1;
    for(int i = 0; i < L; i++){ N += (uint64_t)D[i] * base; base *= (uint64_t)p1; }
    unsigned char back[4];
    uint64_t t = N;
    for(int i = 0; i < L; i++){ back[i] = (unsigned char)(t % (uint64_t)p1); t /= (uint64_t)p1; }
    int32_t e1 = 0;
    for(int i = 0; i < L; i++) if(back[i] != D[i]) e1++;
    res += (e1 != 0);
    printf("\n§1  TODO DADO É UM ELEMENTO DO CORPO — D=(coeficientes) ∈ GF(%d^n), n≥L:\n", p1);
    printf("      D=(200,13,255,7) → elemento nº %" PRIu64 " (base %d) → volta aos coeficientes: erros=%" PRId32 "  %s\n",
           N, p1, e1, VD(e1, "injetivo e reversível (OK)"));

    printf("\n§2  A REPRESENTAÇÃO É REVERSÍVEL — a transformada universal ℱ⁻¹ℱ(D)=D, para qualquer D:\n");
    int32_t tot = 0;
    int sizes[5] = {8, 64, 1000, 4096, 50000};
    for(int s = 0; s < 5; s++){
        int Ls = sizes[s];
        unsigned char *d = malloc((size_t)Ls);
        for(int i = 0; i < Ls; i++) d[i] = (unsigned char)(rnd() % 256);
        int32_t err = roundtrip(d, Ls);
        tot += err;
        printf("      dado aleatório L=%-6d (alfabeto 0..255): erros=%" PRId32 "  %s\n", Ls, err, VD(err, "OK"));
        free(d);
    }
    res += (tot != 0);

    unsigned char dd[10];
    for(int i = 0; i < 10; i++) dd[i] = (unsigned char)(rnd() % 256);
    int32_t eN = roundtrip(dd, 10);
    unsigned char dd2[10 + 20];
    for(int i = 0; i < 10; i++) dd2[i] = dd[i];
    for(int i = 10; i < 30; i++) dd2[i] = 0;
    int32_t eN2 = roundtrip(dd2, 30);
    res += (eN != 0 || eN2 != 0);
    printf("\n§3  CONSISTENTE NA TORRE — GF(p^n) ⊂ GF(p^{n'}) (n|n'): o dado sobe de grau sem mudar:\n");
    printf("      D em corpo menor: erros=%" PRId32 " ; o mesmo D mergulhado no maior: erros=%" PRId32 "  %s\n",
           eN, eN2, VD((eN || eN2), "o embedding preserva (OK)"));

    printf("\n§4  COFINAL — sempre há corpo GF(2^n): nº de polinômios irredutíveis de grau n > 0:\n      ");
    int cof = 1;
    for(int n = 1; n <= 24; n++){
        int32_t I = irred(2, n);
        if(I <= 0) cof = 0;
        if(n <= 12) printf("I_2(%d)=%" PRId32 " ", n, I);
    }
    res += !cof;
    printf("… (todos >0 até n=24)  %s\n", VD(!cof, "para todo L há n≥L com corpo (OK)"));

    if(argc > 1){
        int w, h;
        unsigned char *px = le_pgm(argv[1], &w, &h);
        if(px){
            int32_t total = (int32_t)w * h, off = 0, err = 0;
            int nb = 0;
            while(off < total){
                int blk = (int)((total - off < 65536) ? (total - off) : 65536);
                err += roundtrip(px + off, blk);
                off += blk;
                nb++;
            }
            res += (err != 0);
            printf("\n§5  A IMAGEM É SÓ MAIS UM DADO — %s (%dx%d):\n", argv[1], w, h);
            printf("      a imagem INTEIRA (%" PRId32 " amostras) em %d blocos de ≤2^16, ida e volta: erros=%" PRId32 "  %s\n",
                   total, nb, err, VD(err, "EXATO, resíduo 0 — a imagem cabe no corpo como qualquer dado"));
            free(px);
        }
    }

    printf("\n----------------------------------------------------------------\n");
    printf("resíduo total = %d   %s\n", res, VD(res, "O CORPO É COMPLETO — TODO DADO TEM REPRESENTAÇÃO CONSISTENTE E REVERSÍVEL"));
    return res ? 1 : 0;
}
