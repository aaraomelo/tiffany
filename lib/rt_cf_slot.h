/* rt_cf_slot.h — A PALAVRA (FC) NO DISCO: um termo por slot, lado a lado.
 *
 * O acumulador RtCf em RAM (reta.h) é vista de compatibilidade. A representação
 * canónica é SLOT: cada quociente a_k ocupa um par {total, e} consecutivo — a mesma
 * peça que sql.c (Word), conversa.c (Slot) e isa_disk.h.
 *
 *   slot base+0            {sinal, n}
 *   slot base+1 .. base+n  {a_k, 0}
 *   slot base+RT_CF_MAX+1  {saturou, 0}
 *
 * Backend:
 *   w->fd < 0  → isa_disk.h (projectado, medidores sem ficheiro)
 *   w->fd ≥ 0  → slot_mem.h pread/pwrite no .mem do banco
 *
 * Decisão Trio 13: FC não é long vs int32 — é SLOT no disco.
 *
 * Pipeline RETRAÇÃO (torre_alg.h): cone desce (Π escreve quocientes nos slots),
 * espiral sobe (Σ recompõe). Ver corpo_algebrico.tex def:cone thm:cone. */
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

static inline long rt_cf_slot_banco(unsigned word_ix){
    return (long)cf_slot_base(word_ix);
}

#define RT_CF_FD_ISA    (-1)

typedef struct { long base; int fd; } RtCfSlot;

static inline RtCfSlot rt_cf_slot_word(unsigned word_ix, int fd){
    return (RtCfSlot){ rt_cf_slot_banco(word_ix), fd };
}

static inline long rt_cf_slot_meta(long base){ return base; }
static inline long rt_cf_slot_term(long base, int k){ return base + 1 + k; }
static inline long rt_cf_slot_flag(long base){ return base + RT_CF_MAX + 1; }

static inline void rt_cf_slot_put(RtCfSlot *w, long slot, long t, long e){
    if(w->fd < 0) isa_word(slot, t, e);
    else          slot_mem_grava(w->fd, (unsigned)slot, (SlotWord){t, e});
}

static inline void rt_cf_slot_get(RtCfSlot *w, long slot, long *t, long *e){
    if(w->fd < 0) isa_read(slot, t, e);
    else { SlotWord sw = slot_mem_le(w->fd, (unsigned)slot); *t = sw.total; *e = sw.e; }
}

static inline void rt_cf_slot_init(RtCfSlot *w, int sinal, long base){
    w->base = base;
    if(!w->fd) w->fd = RT_CF_FD_ISA;
    rt_cf_slot_put(w, rt_cf_slot_meta(base), sinal, 0);
    rt_cf_slot_put(w, rt_cf_slot_flag(base), 0, 0);
}

static inline int rt_cf_slot_sinal(const RtCfSlot *w){
    long t, e; rt_cf_slot_get((RtCfSlot*)w, rt_cf_slot_meta(w->base), &t, &e);
    return (int)t;
}
static inline int rt_cf_slot_n(const RtCfSlot *w){
    long t, e; rt_cf_slot_get((RtCfSlot*)w, rt_cf_slot_meta(w->base), &t, &e);
    return (int)e;
}
static inline int rt_cf_slot_saturou(const RtCfSlot *w){
    long t, e; rt_cf_slot_get((RtCfSlot*)w, rt_cf_slot_flag(w->base), &t, &e);
    return (int)t;
}

static inline int rt_cf_slot_escreve(RtCfSlot *w, long a){
    long base = w->base, sg, n;
    rt_cf_slot_get(w, rt_cf_slot_meta(base), &sg, &n);
    if(n >= RT_CF_MAX){
        rt_cf_slot_put(w, rt_cf_slot_flag(base), 1, 0);
        return 0;
    }
    rt_cf_slot_put(w, rt_cf_slot_term(base, (int)n), a, 0);
    rt_cf_slot_put(w, rt_cf_slot_meta(base), sg, n + 1);
    return 1;
}

static inline long rt_cf_slot_termo(const RtCfSlot *w, int k){
    long t, e;
    rt_cf_slot_get((RtCfSlot*)w, rt_cf_slot_term(w->base, k), &t, &e);
    return t;
}

static inline int rt_cf_slot_le(const RtCfSlot *w, int k, long *p, long *q){
    long sg, nn;
    rt_cf_slot_get((RtCfSlot*)w, rt_cf_slot_meta(w->base), &sg, &nn);
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
    long sg, n;
    rt_cf_slot_get((RtCfSlot*)w, rt_cf_slot_meta(w->base), &sg, &n);
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

/* abre .mem para S_CF palavras — medidores do pipe (entrega, fator) */
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
