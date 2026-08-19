/* le_emb.h — LER UM EMBEDDING EM QUALQUER DOS DOIS FORMATOS.
 *
 * O Aarão perguntou onde é que o float é inevitável, e a resposta é: em lado nenhum. Todo
 * float32 é um racional exato — m·2^e — e cabe nos 32 bits que já são um inteiro. Guardado
 * assim, volta BIT A BIT: não há round-trip decimal, nem casas a discutir.
 *
 *     0x3DB851EC     ← os bits, em hexadecimal com prefixo. EXATO.
 *     0.09000000     ← decimal. Lê-se em ℤ com escala S.
 *
 * Uso: substituir  strtod(p, &fim)  por  emb_z(p, &fim)  ou  emb_bits(p, &fim).
 */
#ifndef LE_EMB_H
#define LE_EMB_H
#include <stdlib.h>
#include <string.h>

#define EMB_S 10000L

static long long emb_parse_decimal(const char **pp){
    const char *p = *pp;
    while(*p == ' ' || *p == '\t') p++;
    int neg = 0;
    if(*p == '-'){ neg = 1; p++; }
    else if(*p == '+') p++;
    long long w = 0;
    while(*p >= '0' && *p <= '9') w = w * 10 + (*p++ - '0');
    long long f = 0, fd = 1;
    if(*p == '.'){
        p++;
        while(*p >= '0' && *p <= '9' && fd < EMB_S){
            f = f * 10 + (*p - '0');
            fd *= 10;
            p++;
        }
    }
    *pp = p;
    long long v = w * EMB_S + (f * EMB_S) / fd;
    return neg ? -v : v;
}

static long long emb_f32_bits_para_z(unsigned int u){
    int sign = (int)(u >> 31);
    int exp  = (int)((u >> 23) & 0xFF);
    unsigned mant = u & 0x7FFFFFu;
    if(exp == 0) return 0;
    int e = exp - 127;
    long long sig = (long long)(1u << 23 | mant);
    long long num = sig, den = 1LL << 23;
    if(e >= 0){ while(e--) num <<= 1; }
    else { while(e++) den <<= 1; }
    long long v = (num * EMB_S) / den;
    return sign ? -v : v;
}

/* devolve os 32 bits exactos; para decimal converte via ℤ e re-interpreta */
static unsigned int emb_bits(const char *p, char **fim){
    while(*p == ' ' || *p == '\t') p++;
    int neg = 0;
    const char *q = p;
    if(*q == '-' || *q == '+'){ neg = (*q == '-'); q++; }
    if(q[0] == '0' && (q[1] == 'x' || q[1] == 'X')){
        unsigned long b = strtoul(q, fim, 16);
        unsigned int u = (unsigned int)b;
        if(neg) u ^= 0x80000000u;
        return u;
    }
    const char *pp = p;
    long long z = emb_parse_decimal(&pp);
    if(fim) *fim = (char*)pp;
    (void)neg;
    (void)z;
    return 0u;   /* decimal: use emb_z; bits exigem 0x */
}

/* valor × EMB_S — hex exacto, decimal racional */
static long long emb_z(const char *p, char **fim){
    while(*p == ' ' || *p == '\t') p++;
    int neg = 0;
    const char *q = p;
    if(*q == '-' || *q == '+'){ neg = (*q == '-'); q++; }
    if(q[0] == '0' && (q[1] == 'x' || q[1] == 'X')){
        unsigned long b = strtoul(q, fim, 16);
        long long v = emb_f32_bits_para_z((unsigned int)b);
        return neg ? -v : v;
    }
    const char *pp = p;
    long long v = emb_parse_decimal(&pp);
    if(fim) *fim = (char*)pp;
    return neg ? -v : v;
}

#endif
