/* slot_mem.h — Word de 16 bytes e leitura/escrita por slot num fd (o .mem do banco).
 *
 * Mesma peça que banco/sql.c (Word), banco/conversa.c (Slot) e isa_disk.h — {total, e}.
 * O banco vê sempre um slot a ser lido e um slot a ser escrito; isto é a API mínima
 * para C operar directamente no disco sem carregar a palavra inteira.
 *
 * RETRAÇÃO (torre_alg.h): Σ∘Π = Id — palavra FC ↔ coeficientes nos slots.
 * Soma = soma de coeficientes + carry; produto = convolução + carry. */
#ifndef SLOT_MEM_H
#define SLOT_MEM_H

#ifndef _POSIX_C_SOURCE
#define _POSIX_C_SOURCE 200809L
#endif

#include <unistd.h>
#include <sys/types.h>

#if !defined(__USE_XOPEN2K) && !defined(_GNU_SOURCE)
extern ssize_t pread(int, void *, size_t, off_t);
extern ssize_t pwrite(int, const void *, size_t, off_t);
#endif

typedef struct { long total, e; } SlotWord;

#define SLOT_WORD_BYTES 16

static inline SlotWord slot_mem_le(int fd, unsigned slot){
    SlotWord w = {0, 0};
    if(fd >= 0)
        pread(fd, &w, SLOT_WORD_BYTES, (off_t)slot * SLOT_WORD_BYTES);
    return w;
}

static inline void slot_mem_grava(int fd, unsigned slot, SlotWord w){
    if(fd >= 0)
        pwrite(fd, &w, SLOT_WORD_BYTES, (off_t)slot * SLOT_WORD_BYTES);
}

#endif
