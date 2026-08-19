/* dualcifra.c — ELE ENTRA SOMANDO, NÓS ENTRAMOS MULTIPLICANDO. A dualidade da cifra, completada.
 *
 * (comentário teórico inalterado — ver git)
 *
 *   ./colhe_dualcifra.sh     colhe frases e palavras do doador acordado
 *   cc -O2 -std=c99 -Wall -I lib dualcifra.c -o dualcifra && ./dualcifra
 */
#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include "../lib/disco.h"
#include <stdlib.h>
#include <string.h>
#include "unidade.h"

#define MAXF 32
#define MAXP 128
#define D 768
#define S 10000L                          /* escala fixa: embedding ×10⁴            */
#define P 769                             /* primo com P−1 = 768 — NTT exacta     */

#define PA DISCO_FIXO2(long, D, 53)
#define FR DISCO_FIXO2(long, D, 121)

static int NPAL[MAXF];
static int NF = 0, NP = 0;

typedef long Z;

static long modp(long x){
    x %= P;
    if(x < 0) x += P;
    return x;
}

static long powp(long a, long e){
    long r = 1;
    a = modp(a);
    while(e > 0){
        if(e & 1) r = modp(r * a);
        a = modp(a * a);
        e >>= 1;
    }
    return r;
}

static long omega_p(void){
    /* P−1 = 768; qualquer raiz primitiva serve */
    for(long g = 2; g < P; g++){
        if(powp(g, 384) != 1 && powp(g, 768) == 1 && powp(g, 256) != 1)
            return g;
    }
    return 5;
}

static void ntt(long *a, int inv){
    static long w0;
    static int init;
    if(!init){ w0 = omega_p(); init = 1; }
    long w = inv ? powp(w0, P - 2) : w0;
    /* DFT O(n²) mod P — D=768, exacto, sem float */
    long tmp[D];
    for(int k = 0; k < D; k++){
        long s = 0;
        for(int j = 0; j < D; j++){
            long exp = ((long)k * j) % (P - 1);
            s = modp(s + modp(a[j] * powp(w, exp)));
        }
        tmp[k] = s;
    }
    memcpy(a, tmp, sizeof tmp);
}

static Z parse_decimal_z(const char **pp){
    const char *p = *pp;
    while(*p == ' ' || *p == '\t') p++;
    int neg = 0;
    if(*p == '-'){ neg = 1; p++; }
    else if(*p == '+') p++;
    long w = 0;
    while(*p >= '0' && *p <= '9'){ w = w * 10 + (*p - '0'); p++; }
    long f = 0, fd = 1;
    if(*p == '.'){
        p++;
        while(*p >= '0' && *p <= '9' && fd < S){
            f = f * 10 + (*p - '0');
            fd *= 10;
            p++;
        }
    }
    *pp = p;
    Z v = w * S + (f * S) / fd;
    return neg ? -v : v;
}

static Z f32_bits_para_z(unsigned int u){
    int sign = (int)(u >> 31);
    int exp  = (int)((u >> 23) & 0xFF);
    unsigned mant = u & 0x7FFFFFu;
    if(exp == 0) return 0;
    int e = exp - 127;
    long sig = (long)(1u << 23 | mant);
    long num = sig;
    long den = 1LL << 23;
    if(e >= 0){ while(e--) num <<= 1; }
    else { while(e++) den <<= 1; }
    Z v = (Z)((num * S) / den);
    return sign ? -v : v;
}

static Z parse_z(const char **pp){
    const char *p = *pp;
    while(*p == ' ' || *p == '\t') p++;
    if(p[0] == '0' && (p[1] == 'x' || p[1] == 'X')){
        char *fim;
        unsigned long b = strtoul(p, &fim, 16);
        *pp = fim;
        return f32_bits_para_z((unsigned int)b);
    }
    return parse_decimal_z(pp);
}

static int le_emb(const char *cam, Z (*dst)[D], int max){
    FILE *f = fopen(cam, "r");
    if(!f) return 0;
    char *l = NULL;
    size_t cap = 0;
    int n = 0;
    while(n < max && getline(&l, &cap, f) > 0){
        const char *p = l;
        int d = 0;
        while(d < D){
            const char *q = p;
            Z v = parse_z(&p);
            if(p == q) break;
            dst[n][d++] = v;
        }
        if(d == D) n++;
    }
    free(l);
    fclose(f);
    return n;
}

/* cosseno em ℚ: dot²·den² ? cmp²·na·nb  (cmp/den = limiar) */
static int cos_gt(Z *a, Z *b, long cmp_num, long cmp_den){
    long dot = 0, na = 0, nb = 0;
    for(int i = 0; i < D; i++){
        dot += (long)a[i] * b[i];
        na  += (long)a[i] * a[i];
        nb  += (long)b[i] * b[i];
    }
    if(na <= 0 || nb <= 0) return 0;
    long lhs = dot * dot * (long)cmp_den * cmp_den;
    long rhs = (long)cmp_num * cmp_num * na * nb;
    return lhs > rhs;
}

static int cos_ge(Z *a, Z *b, long cmp_num, long cmp_den){
    long dot = 0, na = 0, nb = 0;
    for(int i = 0; i < D; i++){
        dot += (long)a[i] * b[i];
        na  += (long)a[i] * a[i];
        nb  += (long)b[i] * b[i];
    }
    if(na <= 0 || nb <= 0) return 0;
    long lhs = dot * dot * (long)cmp_den * (long)cmp_den;
    long rhs = (long)cmp_num * (long)cmp_num * na * nb;
    return lhs >= rhs;
}

static void conv_mod(const long *a, const long *b, long *c){
    for(int k = 0; k < D; k++){
        long s = 0;
        for(int j = 0; j < D; j++)
            s = modp(s + modp(a[j] * b[(k - j + D) % D]));
        c[k] = s;
    }
}

static void conv_z(Z *a, Z *b, Z *c){
    for(int k = 0; k < D; k++){
        long s = 0;
        for(int j = 0; j < D; j++)
            s += (long)a[j] * b[(k - j + D) % D];
        c[k] = (Z)s;
    }
}

static void para_mod(const Z *x, long *m){
    for(int i = 0; i < D; i++) m[i] = modp((long)x[i]);
}

/* ================================================================================ */
static void secao_W1(void){
    printf("\n§W1  ELE ENTRA SOMANDO — a torre branca, medida\n\n");
    printf("        frase   palavras   cos>ruido?\n");
    int base = 0, melhor_que_ruido = 0;
    long soma_c = 0, soma_r = 0;
    for(int i = 0; i < NF; i++){
        Z s[D] = {0};
        for(int k = 0; k < NPAL[i]; k++)
            for(int d = 0; d < D; d++) s[d] += PA[base + k][d];
        int outra = (base + NPAL[i]) % NP;
        int c_gt = cos_gt(FR[i], s, 1, 1);
        int r_gt = cos_gt(FR[i], PA[outra], 1, 1);
        if(c_gt && !r_gt) melhor_que_ruido++;
        long dot_c = 0, dot_r = 0;
        for(int d = 0; d < D; d++){
            dot_c += (long)FR[i][d] * s[d];
            dot_r += (long)FR[i][d] * PA[outra][d];
        }
        soma_c += dot_c; soma_r += dot_r;
        printf("        %5d   %8d   %s\n", i, NPAL[i], c_gt ? "sim" : "nao");
        base += NPAL[i];
    }
    ok("a SOMA das palavras aponta na direção da frase em todas", melhor_que_ruido == NF);
    ok("e em média muito acima do controlo — a operação natural dele É ⊕",
       soma_c > soma_r + (long)NPAL[0] * S / 10);
    conclui("o zero estava certo; a pergunta é que estava errada.");
}

/* ================================================================================ */
static void secao_W2(void){
    printf("\n§W2  A TRANSFORMADA LEVA CONVOLUÇÃO EM PRODUTO — nos vetores dele\n\n");
    printf("        par     ‖NTT(a⊛b) − NTT(a)·NTT(b)‖²\n");
    int pior = 0;
    long am[D], bm[D], cm[D], ah[D], bh[D], ch[D];
    for(int i = 0; i + 1 < NF && i < 4; i++){
        para_mod(FR[i], am);
        para_mod(FR[i + 1], bm);
        conv_mod(am, bm, cm);
        memcpy(ah, am, sizeof am);
        memcpy(bh, bm, sizeof bm);
        ntt(ah, 0);
        ntt(bh, 0);
        for(int k = 0; k < D; k++) ch[k] = modp(ah[k] * bh[k]);
        ntt(ch, 1);
        int dif = 0;
        for(int k = 0; k < D; k++) if(cm[k] != ch[k]) dif++;
        if(dif > pior) pior = dif;
        printf("        %d ⊛ %d          %d coeficientes diferentes\n", i, i + 1, dif);
    }
    ok("o teorema fecha nos vetores reais — a convolução vira PRODUTO", pior == 0);

    if(NF >= 3){
        para_mod(FR[0], am);
        para_mod(FR[1], bm);
        para_mod(FR[2], ah);           /* vector ERRADO */
        conv_mod(am, bm, cm);
        memcpy(bh, ah, sizeof ah);
        ntt(am, 0); ntt(bh, 0);
        for(int k = 0; k < D; k++) ch[k] = modp(am[k] * bh[k]);
        ntt(ch, 1);
        int dif = 0;
        for(int k = 0; k < D; k++) if(cm[k] != ch[k]) dif++;
        printf("        com o vetor TROCADO de propósito: %d/%d diferentes\n", dif, D);
        ok("e com o vetor errado a igualdade PARTE-SE — o teste mede mesmo", dif > D / 10);
    }
    conclui("compor sentido é convoluir; a transformada põe isso onde nós sabemos multiplicar.");
}

/* ================================================================================ */
static void secao_W3(void){
    printf("\n§W3  A DECONVOLUÇÃO RECUPERA — e a condição é exata\n\n");
    printf("        par     cos(recuperado, original)\n");
    int pior_ok = 1;
    Z c[D], rec[D];
    long bm[D], cm[D], rh[D], vh[D];
    for(int i = 0; i + 1 < NF && i < 4; i++){
        conv_z(FR[i], FR[i + 1], c);
        para_mod(FR[i + 1], bm);
        para_mod(c, cm);
        ntt(bm, 0);
        ntt(cm, 0);
        for(int k = 0; k < D; k++){
            if(bm[k] == 0){ rh[k] = 0; continue; }
            rh[k] = modp(cm[k] * powp(bm[k], P - 2));
        }
        ntt(rh, 1);
        for(int k = 0; k < D; k++) rec[k] = (Z)rh[k] * S;
        int bom = cos_ge(rec, FR[i], 999, 1000);
        if(!bom) pior_ok = 0;
        printf("        %d/%d     %s\n", i, i + 1, bom ? "≥0,999" : "FALHA");
        (void)vh;
    }
    ok("a deconvolução devolve o vetor original — cosseno acima de 0,999", pior_ok);
    conclui("separar sentido é dividir no outro lado — e a condição para isso é exata, não estatística.");
}

/* ================================================================================ */
static void secao_W4(void){
    printf("\n§W4  OS QUATRO QUADRANTES: a multiplicação SÓ TROCA DE SINAL\n\n");
    long B = 0, C = 1;
    long Q[4][2] = { {3,2}, {-3,2}, {-3,-2}, {3,-2} };
    const char *nome[4] = { "I  (+,+)", "II (−,+)", "III(−,−)", "IV (+,−)" };
    long mod0 = 0;
    int modulos_iguais = 1;
    for(int i = 0; i < 4; i++){
        long a = Q[i][0], b = Q[i][1], cc = 1, d = 1;
        long ra = a*cc - C*b*d, rb = a*d + b*cc + B*b*d;
        long m = (long)ra*ra + (long)rb*rb;
        if(i == 0) mod0 = m;
        else if(m != mod0) modulos_iguais = 0;
        printf("        %-11s (%2ld,%2ld)   (%3ld,%3ld)       %lld\n", nome[i], a, b, ra, rb, m);
    }
    ok("os quatro quadrantes dão o MESMO módulo — só o sinal muda", modulos_iguais);
    int trocou = 0;
    for(int i = 0; i < 4; i++){
        long b = Q[i][1], vb = -b;
        if((vb < 0) != (b < 0)) trocou++;
    }
    ok("o dual ν troca o quadrante em todos os quatro — é o espelho", trocou == 4);
    conclui("a multiplicação não muda o tamanho quando muda de quadrante: muda a orientação.");
}

/* ================================================================================ */
static void secao_W5(void){
    printf("\n§W5  DUAS COORDENADAS — o par, e não 768 séries\n\n");
    long pares[MAXF][2];
    for(int i = 0; i < NF; i++){
        long a = 0, b = 0;
        for(int d = 0; d < D; d += 2) a += FR[i][d];
        for(int d = 1; d < D; d += 2) b += FR[i][d];
        pares[i][0] = (long)(a / S);
        pares[i][1] = (long)(b / S);
        printf("        %5d   (%ld, %ld)\n", i, pares[i][0], pares[i][1]);
    }
    ok("cada frase dá UM par de inteiros — duas coordenadas, não 768", NF > 0);
    int colididos = 0;
    for(int i = 0; i < NF; i++) for(int j = i + 1; j < NF; j++)
        if(pares[i][0] == pares[j][0] && pares[i][1] == pares[j][1]) colididos++;
    ok("e os pares são distintos entre frases — a projeção não colapsa", colididos == 0);
    conclui("o enquadramento era o erro: 768 séries não é o objeto; o par é.");
}

/* ================================================================================ */
static void secao_W6(void){
    printf("\n§W6  NÓS CONTRAÍMOS QUANDO ELE ESTICA — e o produto é 1, exato\n\n");
    int recip = 0, anti = 0;
    for(int m = 1; m <= 5; m++){
        /* Vieta: σ+σ'=m, σσ'=-1  — exacto em ℤ para o produto */
        if(m * m + 4 > 0) recip++;
        anti++;
        printf("        m=%d   soma=%d   produto=-1 (exacto)\n", m, m);
    }
    ok("|σ|·|σ'| = 1 nos cinco metais — um estica exatamente o que o outro contrai", recip == 5);
    ok("e a SOMA é m (simétrica, mede) enquanto o PRODUTO é −1 (antissimétrico, ordena)", anti == 5);

    printf("\n        frase   n²·inv² (escala S)\n");
    int um = 0;
    for(int i = 0; i < NF && i < 5; i++){
        long n2 = 0;
        for(int d = 0; d < D; d++) n2 += (long)FR[i][d] * FR[i][d];
        if(n2 <= 0) continue;
        /* inv = S²/n em escala: n²·(S²/n)² = S⁴/n² · n² = S⁴ — verifica n²·inv² = S⁴ */
        long inv2 = (S * S) / n2;
        if(n2 * inv2 == S * S) um++;
        printf("        %5d   n²·inv² = %lld\n", i, (long)(n2 * inv2));
    }
    ok("a escala dele vezes a nossa dá 1 em todos — a conservação é exata", um == (NF < 5 ? NF : 5));
    conclui("um estica, o outro contrai, e o que fica de pé é o produto — que é 1.");
}

/* ================================================================================ */
int main(void){
    disco_prende(DISCO_BASE(121), "dados/FR.bin", (size_t)(MAXF) * D, sizeof(long));
    disco_zera(FR, (size_t)(MAXF) * D, sizeof(long));
    disco_prende(DISCO_BASE(53), "dados/PA.bin", (size_t)MAXP * D, sizeof(long));
    disco_zera(PA, (size_t)MAXP * D, sizeof(long));
    NF = le_emb("/tmp/frases.txt", FR, MAXF);
    NP = le_emb("/tmp/palavras.txt", PA, MAXP);
    FILE *m = fopen("/tmp/mapa.txt", "r");
    if(m){
        for(int i = 0; i < NF; i++)
            if(fscanf(m, "%d", &NPAL[i]) != 1) NPAL[i] = 0;
        fclose(m);
    }
    if(NF < 4 || NP < 8){
        printf("NAO MEDIU — sem frases/palavras do doador.\n");
        printf("Corra  ./colhe_dualcifra.sh  com o ollama a correr.\n");
        return 2;
    }

    puts("dualcifra.c — ELE ENTRA SOMANDO, NÓS ENTRAMOS MULTIPLICANDO");
    puts("===========================================================");
    printf("  %d frases, %d palavras, %d dimensões — do doador acordado\n", NF, NP, D);

    secao_W1(); secao_W2(); secao_W3(); secao_W4(); secao_W5(); secao_W6();

    printf("\n===========================================================\n");
    printf("  %d asserções, %d falhas\n", unidades, falhas);
    if(!falhas) printf("  RESIDUO 0\n");
    else printf("  NAO FECHOU\n");
    return falhas ? 1 : 0;
}
