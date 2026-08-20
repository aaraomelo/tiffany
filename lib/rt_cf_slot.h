/* rt_cf_slot.h — FC no disco: 1 byte = 1 slot (Word_8).
 *
 *   base+0                     sinal — um BIT (0 = +, 1 = −), não um byte assinado
 *   base+1                     n
 *   base+2+2k, base+3+2k       a_k = Word_8² (baixo, alto) — o par da Lei 7
 *   base+2+2·RT_CF_MAX         saturou
 *
 * O QUOCIENTE NÃO CABE NUM ÁTOMO. O envelope de um a_k é Word_8² (0..65535), que é a
 * mesma Word da ISA (sql.c, erg.c): o termo é um PAR de átomos, não um átomo. Um a_k
 * acima disso não se enrola calado — acende `saturou` e a escrita RECUSA-SE.
 *
 * E o sinal é um BIT. Guardá-lo como byte assinado num envelope sem sinal devolvia
 * −1 como 255, e o −1/3 voltava 255/3 sem que asserção nenhuma o visse. */
#ifndef RT_CF_SLOT_H
#define RT_CF_SLOT_H

#include "isa_disk.h"
#include "slot_mem.h"
#include "slot_map.h"

#ifndef _POSIX_C_SOURCE
#define _POSIX_C_SOURCE 200809L
#endif
#include <fcntl.h>
#include <unistd.h>

#ifndef RT_CF_MAX
#define RT_CF_MAX 48
#endif

#define RT_CF_SLOT_BASE S_CF
#define RT_CF_FD_ISA    (-1)

typedef struct { long base; int fd; } RtCfSlot;

static inline long rt_cf_slot_banco(unsigned word_ix){
    return (long)cf_slot_base(word_ix);
}

static inline RtCfSlot rt_cf_slot_word(unsigned word_ix, int fd){
    return (RtCfSlot){ rt_cf_slot_banco(word_ix), fd };
}

/* o maior a_k que o par de átomos segura — acima disto RECUSA-SE (não enrola) */
#define RT_CF_TERMO_MAX 65535L

static inline long rt_cf_slot_sinal_sl(long base){ return base; }
static inline long rt_cf_slot_n_sl(long base){ return base + 1; }
static inline long rt_cf_slot_term(long base, int k){ return base + 2 + 2*k; }
static inline long rt_cf_slot_flag(long base){ return base + 2 + 2*RT_CF_MAX; }

/* UM ÁTOMO — sem sinal dos dois lados do backend. O que aqui entra é 0..255. */
static inline void rt_cf_slot_put1(RtCfSlot *w, long slot, long v){
    if(w->fd < 0) isa_put((unsigned)slot, (uint8_t)(v & 255L));
    else          slot_mem_grava(w->fd, (unsigned)slot, (SlotWord)(v & 255L));
}

static inline long rt_cf_slot_get1(const RtCfSlot *w, long slot){
    if(w->fd < 0) return (long)isa_get((unsigned)slot);
    return slot_long_de_w8(slot_mem_le(w->fd, (unsigned)slot));
}

/* O PAR: a_k = baixo + 256·alto, dois átomos consecutivos (Word_8²). */
static inline void rt_cf_slot_put2(RtCfSlot *w, long slot, long v){
    rt_cf_slot_put1(w, slot,     v & 255L);
    rt_cf_slot_put1(w, slot + 1, (v >> 8) & 255L);
}

static inline long rt_cf_slot_get2(const RtCfSlot *w, long slot){
    return rt_cf_slot_get1(w, slot) | (rt_cf_slot_get1(w, slot + 1) << 8);
}

/* Compat: par em dois slots consecutivos (classe ℤ = dois átomos). */
static inline void rt_cf_slot_put(RtCfSlot *w, long slot, long t, long e){
    rt_cf_slot_put1(w, slot, t);
    rt_cf_slot_put1(w, slot + 1, e);
}

static inline void rt_cf_slot_get(RtCfSlot *w, long slot, long *t, long *e){
    *t = rt_cf_slot_get1(w, slot);
    *e = rt_cf_slot_get1(w, slot + 1);
}

static inline void rt_cf_slot_init(RtCfSlot *w, int sinal, long base){
    w->base = base;
    if(!w->fd) w->fd = RT_CF_FD_ISA;
    rt_cf_slot_put1(w, rt_cf_slot_sinal_sl(base), sinal < 0 ? 1 : 0);
    rt_cf_slot_put1(w, rt_cf_slot_n_sl(base), 0);
    rt_cf_slot_put1(w, rt_cf_slot_flag(base), 0);
}

/* o bit volta a ser sinal: 0 → +1, 1 → −1 */
static inline int rt_cf_slot_sinal(const RtCfSlot *w){
    return rt_cf_slot_get1(w, rt_cf_slot_sinal_sl(w->base)) ? -1 : 1;
}
static inline int rt_cf_slot_n(const RtCfSlot *w){
    return (int)rt_cf_slot_get1(w, rt_cf_slot_n_sl(w->base));
}
static inline int rt_cf_slot_saturou(const RtCfSlot *w){
    return (int)rt_cf_slot_get1(w, rt_cf_slot_flag(w->base));
}

static inline int rt_cf_slot_escreve(RtCfSlot *w, long a){
    long base = w->base;
    long n = rt_cf_slot_get1(w, rt_cf_slot_n_sl(base));
    if(n >= RT_CF_MAX || a < 0 || a > RT_CF_TERMO_MAX){
        rt_cf_slot_put1(w, rt_cf_slot_flag(base), 1);
        return 0;
    }
    rt_cf_slot_put2(w, rt_cf_slot_term(base, (int)n), a);
    rt_cf_slot_put1(w, rt_cf_slot_n_sl(base), n + 1);
    return 1;
}

static inline long rt_cf_slot_termo(const RtCfSlot *w, int k){
    return rt_cf_slot_get2(w, rt_cf_slot_term(w->base, k));
}

static inline int rt_cf_slot_le(const RtCfSlot *w, int k, long *p, long *q){
    long nn = rt_cf_slot_get1(w, rt_cf_slot_n_sl(w->base));
    if(k < 0 || k >= (int)nn) return 0;
    long p0 = 1, q0 = 0, p1 = rt_cf_slot_termo(w, 0), q1 = 1;
    for(int i = 1; i <= k; i++){
        long ak = rt_cf_slot_termo(w, i);
        long np = ak*p1 + p0, nq = ak*q1 + q0;
        p0 = p1; q0 = q1; p1 = np; q1 = nq;
    }
    if(p) *p = p1;
    if(q) *q = q1;
    return 1;
}

static inline void rt_cf_slot_de(int sinal, long p, long q, RtCfSlot *w){
    long base = w->base ? w->base : RT_CF_SLOT_BASE;
    if(!w->fd) w->fd = RT_CF_FD_ISA;
    rt_cf_slot_init(w, (p == 0) ? 1 : sinal, base);
    long P = p < 0 ? -p : p, Q = q < 0 ? -q : q;
    while(Q != 0){
        if(!rt_cf_slot_escreve(w, P / Q)) return;
        long r = P % Q;
        P = Q; Q = r;
    }
}

static inline int rt_cf_slot_para(const RtCfSlot *w, long *p, long *q){
    long sg = rt_cf_slot_sinal(w);
    long n  = rt_cf_slot_get1(w, rt_cf_slot_n_sl(w->base));
    if(n == 0){ if(p) *p = 0; if(q) *q = 1; return 1; }
    long P = 1, Q = 0;
    for(int k = (int)n - 1; k >= 0; k--){
        long ak = rt_cf_slot_termo(w, k);
        if(ak != 0 && P > 4611686018427387903L / (ak > 1 ? ak : 1)) return 0;
        long np = ak*P + Q;
        Q = P; P = np;
    }
    if(p) *p = sg * P;
    if(q) *q = Q;
    return 1;
}

static inline int rt_cf_slot_mem_abre(const char *path){
    int fd = open(path, O_RDWR | O_CREAT | O_TRUNC, 0644);
    if(fd >= 0) ftruncate(fd, (off_t)S_CF_MEM_BYTES);
    return fd;
}

static inline int rt_cf_slot_copia_termos(const RtCfSlot *w, long *a, int max){
    int n = rt_cf_slot_n(w);
    if(n > max) n = max;
    for(int i = 0; i < n; i++) a[i] = rt_cf_slot_termo(w, i);
    return n;
}

#endif
