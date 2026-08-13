/* fala.h — Protocolo próprio da assistente (local).
 *
 *   banda = sha256(Assinatura(corpo_cliente))   — cada cliente na sua banda
 *   bump  = XOR keystream(banda)                — J, ida = volta
 *   a fala é a interface — não há API de grafo
 *
 * Frame TCP (127.0.0.1):
 *   magic "TFAL" | ver=1 | op | seq u32BE | len u32BE | payload
 *   HELLO: Assinatura em claro
 *   resto: bump(UTF-8)
 *
 * Ops: HELLO=1 FALA=2 RESPOSTA=3 APRENDE=4 NAO_SEI=5 ERR=255
 */
#ifndef FALA_H
#define FALA_H
#include "banda.h"

#define FALA_VER 1
#define FALA_HELLO    1
#define FALA_FALA     2
#define FALA_RESPOSTA 3
#define FALA_APRENDE  4
#define FALA_NAO_SEI  5
#define FALA_ERR      255

#define FALA_HDR 14
#define FALA_MAX 8192
#define FALA_PORT 47314

static int fala_magic_ok(const unsigned char *in){
    return in[0]=='T' && in[1]=='F' && in[2]=='A' && in[3]=='L';
}
static void fala_be32(unsigned char *p, unsigned v){
    p[0]=(unsigned char)(v>>24); p[1]=(unsigned char)(v>>16);
    p[2]=(unsigned char)(v>>8);  p[3]=(unsigned char)v;
}
static unsigned fala_rb32(const unsigned char *p){
    return ((unsigned)p[0]<<24)|((unsigned)p[1]<<16)|((unsigned)p[2]<<8)|(unsigned)p[3];
}

/* O hash É a assinatura: banda = sha256(Assinatura). */
static void fala_banda_de_assinatura(const char *assinatura, unsigned char *banda){
    banda_de(assinatura, banda);
}

static void fala_hex16(const unsigned char *banda, char *out /*33*/){
    static const char *H = "0123456789abcdef";
    for(int i = 0; i < 16; i++){
        out[2*i]   = H[banda[i] >> 4];
        out[2*i+1] = H[banda[i] & 15];
    }
    out[32] = 0;
}

/* Empacota. banda=NULL ou op=HELLO → payload em claro. */
static int fala_empacota(unsigned char *out, size_t cap,
                         unsigned char op, unsigned seq,
                         const unsigned char *banda,
                         const char *texto, size_t ntxt){
    if(ntxt > FALA_MAX) return -1;
    if(cap < FALA_HDR + ntxt) return -1;
    out[0]='T'; out[1]='F'; out[2]='A'; out[3]='L';
    out[4]=FALA_VER; out[5]=op;
    fala_be32(out+6, seq);
    fala_be32(out+10, (unsigned)ntxt);
    if(ntxt){
        if(banda && op != FALA_HELLO){
            unsigned char ks[FALA_MAX];
            keystream(banda, ks, ntxt);
            bump((const unsigned char*)texto, ks, out + FALA_HDR, ntxt);
        } else {
            memcpy(out + FALA_HDR, texto, ntxt);
        }
    }
    return (int)(FALA_HDR + ntxt);
}

static int fala_desempacota(const unsigned char *in, size_t n,
                            unsigned char *op, unsigned *seq,
                            const unsigned char *banda,
                            char *buf, size_t buflen, size_t *nout){
    if(n < FALA_HDR) return -1;
    if(!fala_magic_ok(in)) return -2;
    if(in[4] != FALA_VER) return -3;
    *op = in[5];
    *seq = fala_rb32(in + 6);
    unsigned len = fala_rb32(in + 10);
    if(FALA_HDR + (size_t)len > n) return -4;
    if((size_t)len + 1 > buflen) return -5;
    if(len == 0){ buf[0]=0; *nout=0; return 0; }
    if(banda && *op != FALA_HELLO){
        unsigned char ks[FALA_MAX];
        if(len > FALA_MAX) return -5;
        keystream(banda, ks, len);
        bump(in + FALA_HDR, ks, (unsigned char*)buf, len);
    } else {
        memcpy(buf, in + FALA_HDR, len);
    }
    buf[len] = 0;
    *nout = len;
    return 0;
}

#endif
