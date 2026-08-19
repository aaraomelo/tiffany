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

/* f64 como BITS IEEE754, racional exacto num/den — sem o tipo double no compilador */
static unsigned long long le_f64_bits_pq(__int128 num, __int128 den, int neg){
    if(num == 0) return neg ? 0x8000000000000000ULL : 0ULL;
    if(num < 0){ num = -num; neg ^= 1; }
    if(den < 0){ den = -den; neg ^= 1; }
    if(den == 0) return neg ? 0xFFF0000000000000ULL : 0x7FF0000000000000ULL;
    int uexp = 0;
    while(num < den){ num <<= 1; uexp--; if(uexp < -1075) return neg ? 0x8000000000000000ULL : 0ULL; }
    while(num >= den * 2){ den <<= 1; uexp++; if(uexp > 1024) return neg ? 0xFFF0000000000000ULL : 0x7FF0000000000000ULL; }
    __int128 mant = (num << 52) / den;
    __int128 rem  = (num << 52) % den;
    if(rem * 2 > den || (rem * 2 == den && (mant & 1))) mant++;
    if(mant == ((__int128)1 << 53)){ mant >>= 1; uexp++; }
    int bexp = uexp + 1023;
    if(bexp <= 0){
        if(bexp < -52) return neg ? 0x8000000000000000ULL : 0ULL;
        mant = ((num << (52 + bexp)) / den);
        unsigned long long out = (unsigned long long)mant & ((1ULL << 52) - 1);
        return neg ? out | 0x8000000000000000ULL : out;
    }
    if(bexp >= 2047) return neg ? 0xFFF0000000000000ULL : 0x7FF0000000000000ULL;
    unsigned long long out = ((unsigned long long)(bexp & 0x7FF) << 52)
                           | ((unsigned long long)mant & ((1ULL << 52) - 1));
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
    __int128 num = 0; int frac = 0, houve = 0;
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
    __int128 den = 1;
    int e10 = exp - frac;
    if(e10 >= 0){ for(int i = 0; i < e10; i++) num *= 10; }
    else        { for(int i = 0; i < -e10; i++) den *= 10; }
    if(end) *end = s;
    unsigned long long bits = le_f64_bits_pq(num, den, neg);
    long out; memcpy(&out, &bits, 8);
    return out;
}

#endif
