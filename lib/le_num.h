/* le_num.h — LER NÚMEROS DE TEXTO SEM LIBC: o strtod e o hex do núcleo.
 *
 * O núcleo do tradutor (tests/tex.c) sobe para wasm pelo tools/traduz.c, que não tem a libc:
 * nem sscanf, nem strtod. Estas são as peças que os substituem, inteiras e testadas contra a
 * libc (tests/str2dbl_dual.c compara str2dbl com o strtod, e tem de dar resíduo 0).
 *
 * FONTE ÚNICA: o tex.c e o medidor incluem ESTE ficheiro --- não há cópia a derivar. No wasm,
 * como o traduz salta as linhas `#`, o conteúdo é inlinado no tex_core.c pelo corte (o mesmo
 * que fará ao spline.h/disco.h). A régua não se escreve duas vezes.
 */
#ifndef LE_NUM_H
#define LE_NUM_H

#include <string.h>
#include <stdint.h>

/* um dígito hex -> valor (0..15), ou -1 se não for hex */
static int hex1(int c){
    if(c >= '0' && c <= '9') return c - '0';
    if(c >= 'a' && c <= 'f') return c - 'a' + 10;
    if(c >= 'A' && c <= 'F') return c - 'A' + 10;
    return -1;
}
/* dois dígitos hex -> byte (0..255), ou -1 se algum não for hex --- o `sscanf %2x` do núcleo */
static int hex2(const char *s){
    int a = hex1((unsigned char)s[0]), b = hex1((unsigned char)s[1]);
    return (a < 0 || b < 0) ? -1 : (a << 4) | b;
}

/* divisão exacta sem 128 bits: computa q = floor((num << shift)/den) e r = (num<<shift)%den */
static void divmod_shift_u64(uint64_t num, uint64_t den, unsigned shift, uint64_t *q, uint64_t *r){
    /* long division do inteiro representado por (num<<shift),
       com implementação por bits, sem materializar 128 bits. */
    uint64_t qq = 0, rr = 0;
    /* o maior bit do num fica em 63, então (num<<shift) tem no máx. 64+shift bits */
    for(int i = 63 + (int)shift; i >= 0; i--){
        uint64_t bit = 0;
        if((unsigned)i >= shift) bit = (num >> ((unsigned)i - shift)) & 1ULL;
        rr = (rr << 1) | bit;
        qq <<= 1;
        if(rr >= den){ rr -= den; qq |= 1ULL; }
    }
    *q = qq;
    *r = rr;
}

/* f64 como BITS IEEE754, racional exacto num/den — sem o tipo double no compilador */
static uint64_t le_f64_bits_pq(uint64_t num, uint64_t den, int neg){
    if(num == 0) return neg ? 0x8000000000000000ULL : 0ULL;
    if(den == 0) return neg ? 0xFFF0000000000000ULL : 0x7FF0000000000000ULL;

    int uexp = 0;
    while(num < den){
        num <<= 1;
        uexp--;
        if(uexp < -1075) return neg ? 0x8000000000000000ULL : 0ULL;
    }
    while(den && num >= (den << 1)){
        den <<= 1;
        uexp++;
        if(uexp > 1024) return neg ? 0xFFF0000000000000ULL : 0x7FF0000000000000ULL;
    }

    /* 1 <= num/den < 2 */
    uint64_t mant, rem;
    divmod_shift_u64(num, den, 52, &mant, &rem); /* mant = floor((num<<52)/den) */
    if((rem << 1) > den || ((rem << 1) == den && (mant & 1ULL))) mant++;
    if(mant == (1ULL << 53)){ mant >>= 1; uexp++; }

    int bexp = uexp + 1023;
    if(bexp <= 0){
        if(bexp < -52) return neg ? 0x8000000000000000ULL : 0ULL;
        unsigned shift = (unsigned)(52 + bexp); /* bexp<=0, então shift em [0,52] */
        uint64_t dummy;
        divmod_shift_u64(num, den, shift, &mant, &dummy);
        uint64_t out = mant & ((1ULL << 52) - 1);
        return neg ? out | 0x8000000000000000ULL : out;
    }
    if(bexp >= 2047) return neg ? 0xFFF0000000000000ULL : 0x7FF0000000000000ULL;

    uint64_t out = ((uint64_t)(bexp & 0x7FF) << 52) | (mant & ((1ULL << 52) - 1));
    return neg ? out | 0x8000000000000000ULL : out;
}

/* o `strtod` sem libc: sinal, parte inteira, `.fracção`, expoente `e±dd`. Devolve os 64 bits
 * IEEE754 como `long` e põe *end depois do número (ou =início se não houve número). BATE O
 * strtod byte a byte porque acumula a mantissa como INTEIRO e faz UMA divisão por 10^k ---
 * um só arredondamento, o correcto. O guarda do expoente não consome o `e` de "em"/"ex". */
static long str2dbl(const char *s, const char **end){
    const char *s0 = s;
    while(*s == ' ' || *s == '\t') s++;
    int neg = 0;
    if(*s == '+' || *s == '-'){ neg = (*s == '-'); s++; }
    uint64_t num = 0; int frac = 0, houve = 0;
    while(*s >= '0' && *s <= '9'){ num = num * 10 + (*s - '0'); s++; houve = 1; }
    if(*s == '.'){ s++; while(*s >= '0' && *s <= '9'){ num = num * 10 + (*s - '0'); frac++; s++; houve = 1; } }
    if(!houve){ if(end) *end = s0; return 0; }
    int exp = 0;
    if(*s == 'e' || *s == 'E'){
        const char *t = s + 1; int es = 0;
        if(*t == '+' || *t == '-'){ es = (*t == '-'); t++; }
        if(*t >= '0' && *t <= '9'){
            int e = 0; while(*t >= '0' && *t <= '9'){ e = e * 10 + (*t - '0'); t++; }
            exp = es ? -e : e; s = t;
        }
    }
    uint64_t den = 1;
    int e10 = exp - frac;
    if(e10 >= 0){ for(int i = 0; i < e10; i++) num *= 10; }
    else        { for(int i = 0; i < -e10; i++) den *= 10; }
    if(end) *end = s;
    uint64_t bits = le_f64_bits_pq(num, den, neg);
    long out; memcpy(&out, &bits, 8);
    return out;
}

#endif
