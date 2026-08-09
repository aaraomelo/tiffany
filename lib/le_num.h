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

/* o `strtod` sem libc: sinal, parte inteira, `.fracção`, expoente `e±dd`. Devolve o double e põe
 * *end depois do número (ou =início se não houve número). BATE O strtod byte a byte porque acumula
 * a mantissa como INTEIRO e faz UMA divisão/multiplicação por 10^k (potência exacta em double) ---
 * um só arredondamento, o correcto, e não a cadeia de /10 que arredonda a cada passo. O guarda do
 * expoente não consome o `e` de "em"/"ex" (o `e` só é expoente se for seguido de dígito). */
static double str2dbl(const char *s, const char **end){
    const char *s0 = s;
    while(*s == ' ' || *s == '\t') s++;
    int neg = 0;
    if(*s == '+' || *s == '-'){ neg = (*s == '-'); s++; }
    long mant = 0; int frac = 0, houve = 0;
    while(*s >= '0' && *s <= '9'){ mant = mant * 10 + (*s - '0'); s++; houve = 1; }
    if(*s == '.'){ s++; while(*s >= '0' && *s <= '9'){ mant = mant * 10 + (*s - '0'); frac++; s++; houve = 1; } }
    if(!houve){ if(end) *end = s0; return 0.0; }             /* sem número: como o strtod, *end=início */
    int exp = 0;                                             /* expoente, só se `e` for seguido de dígito */
    if(*s == 'e' || *s == 'E'){
        const char *t = s + 1; int es = 0;
        if(*t == '+' || *t == '-'){ es = (*t == '-'); t++; }
        if(*t >= '0' && *t <= '9'){
            int e = 0; while(*t >= '0' && *t <= '9'){ e = e * 10 + (*t - '0'); t++; }
            exp = es ? -e : e; s = t;
        }                                                    /* senão: `e` não é expoente (é a unidade) */
    }
    double val = (double)mant;
    int e10 = exp - frac;
    double p = 1.0;
    if(e10 >= 0){ for(int i = 0; i < e10; i++) p *= 10.0; val *= p; }   /* uma multiplicação */
    else        { for(int i = 0; i < -e10; i++) p *= 10.0; val /= p; }  /* uma divisão */
    if(end) *end = s;
    return neg ? -val : val;
}

#endif
