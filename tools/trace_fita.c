/* trace_fita.c — corre fita e imprime passos em STORE_IND / LOADS */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include "../lib/isa.h"
#include "../lib/word_isa.h"
#include "../lib/slot_mem.h"

#define NULO 8
#define MEM_SLOTS 65500
#define SLOT 1

static int fmem = -1, fprog = -1;

static Word mem_le(unsigned slot) {
    Word w = {0, 0};
    if (fmem >= 0) {
        SlotWord a = 0, b = 0;
        pread(fmem, &a, SLOT, (off_t)(slot * 2u));
        pread(fmem, &b, SLOT, (off_t)(slot * 2u + 1u));
        w.total = a;
        w.e = b;
    }
    return w;
}

static void mem_grava(unsigned slot, Word w) {
    if (fmem >= 0) {
        SlotWord a = (SlotWord)(int8_t)w.total, b = (SlotWord)(int8_t)w.e;
        pwrite(fmem, &a, SLOT, (off_t)(slot * 2u));
        pwrite(fmem, &b, SLOT, (off_t)(slot * 2u + 1u));
    }
}

static unsigned char prog_le(unsigned pc) {
    unsigned char b = 0;
    if (pread(fprog, &b, 1, (off_t)pc) != 1) return 0;
    return b;
}

typedef struct { Word A, B, R; unsigned pc; unsigned char flags; } Regs;

static unsigned slot_indice(unsigned ptr) {
    Word idx = mem_le(ptr);
    return (unsigned)idx.total | ((unsigned)idx.e << 8);
}

static int passo(Regs *r, unsigned prog_len) {
    if (r->pc >= prog_len) return 0;
    unsigned pc = r->pc;
    unsigned char op = prog_le(pc++);
    int trace = (op == OP_LOADS || op == OP_STORE_IND);
    switch (op) {
    case OP_HALT: return 0;
    case OP_LOAD: {
        unsigned slot = (unsigned)prog_le(pc) | ((unsigned)prog_le(pc + 1) << 8);
        pc += 2;
        r->B = r->A;
        r->A = mem_le(slot);
        break;
    }
    case OP_LOADS: {
        unsigned ptr = (unsigned)prog_le(pc) | ((unsigned)prog_le(pc + 1) << 8);
        pc += 2;
        r->B = r->A;
        r->A = mem_le(slot_indice(ptr));
        if (trace) fprintf(stderr, "pc=%u LOADS ptr=%u idx=%u A={%d,%d}\n",
                          pc - 3, ptr, slot_indice(ptr), r->A.total, r->A.e);
        break;
    }
    case OP_STORE: {
        unsigned slot = (unsigned)prog_le(pc) | ((unsigned)prog_le(pc + 1) << 8);
        pc += 2;
        mem_grava(slot, r->R);
        break;
    }
    case OP_STORE_IND: {
        unsigned ptr = (unsigned)prog_le(pc) | ((unsigned)prog_le(pc + 1) << 8);
        pc += 2;
        unsigned tgt = slot_indice(ptr);
        if (trace) fprintf(stderr, "pc=%u STORE_IND ptr=%u idx=%u R={%d,%d}\n",
                          pc - 3, ptr, tgt, r->R.total, r->R.e);
        mem_grava(tgt, r->R);
        break;
    }
    case OP_ADD: {
        r->R.total = (Word8)(r->A.total + r->B.total);
        r->R.e = (Word8)(r->A.e + r->B.e);
        break;
    }
    case OP_SUB: {
        r->R.total = (Word8)(r->A.total - r->B.total);
        r->R.e = (Word8)(r->A.e - r->B.e);
        break;
    }
    case OP_CMP: {
        unsigned char f = 0;
        if (r->A.total == 0 && r->A.e == 0 && r->B.total == 0 && r->B.e == 0) f |= 1;
        if (r->A.total == r->B.total && r->A.e == r->B.e) f |= 2;
        else if (r->A.total < r->B.total) f |= 4;
        r->flags = f;
        break;
    }
    case OP_JMP: {
        int rel = (int)(int16_t)((unsigned)prog_le(pc) | ((unsigned)prog_le(pc + 1) << 8));
        r->pc = (unsigned)((int)pc + rel);
        return 1;
    }
    case OP_JZ: {
        int rel = (int)(int16_t)((unsigned)prog_le(pc) | ((unsigned)prog_le(pc + 1) << 8));
        r->pc = (r->flags & 1) ? (unsigned)((int)pc + rel) : pc;
        return 1;
    }
    case OP_JNZ: {
        int rel = (int)(int16_t)((unsigned)prog_le(pc) | ((unsigned)prog_le(pc + 1) << 8));
        r->pc = !(r->flags & 1) ? (unsigned)((int)pc + rel) : pc;
        return 1;
    }
    case OP_ADD16: {
        unsigned lo = (unsigned)(r->A.total + r->B.total);
        unsigned hi = (unsigned)(r->A.e + r->B.e + (lo >> 8));
        r->R.total = (Word8)(lo & 0xff);
        r->R.e = (Word8)(hi & 0xff);
        break;
    }
    case OP_SUB16: {
        int lo = (int)r->A.total - (int)r->B.total;
        int hi = (int)r->A.e - (int)r->B.e;
        if (lo < 0) { lo += 256; hi--; }
        r->R.total = (Word8)(lo & 0xff);
        r->R.e = (Word8)(hi & 0xff);
        break;
    }
    case OP_MUL16: {
        unsigned a = (unsigned)r->A.total | ((unsigned)r->A.e << 8);
        unsigned b = (unsigned)r->B.total | ((unsigned)r->B.e << 8);
        unsigned p = a * b;
        r->R.total = (Word8)(p & 0xff);
        r->R.e = (Word8)((p >> 8) & 0xff);
        break;
    }
    case OP_TROCA: {
        Word8 t = r->A.total;
        r->A.total = r->A.e;
        r->A.e = t;
        r->R = r->A;
        break;
    }
    default:
        fprintf(stderr, "op desconhecido %d pc=%u\n", op, pc - 1);
        return 0;
    }
    r->pc = pc;
    return 1;
}

static void seed_mem(const char *script) {
    unsigned char arena[65536];
    memset(arena, 0, sizeof arena);
    int n = (int)strlen(script);
    memcpy(arena + 256, script, (size_t)n);
    arena[24576] = (unsigned char)(n & 255);
    arena[24578 + 1] = 0;
    for (int s = 0; s < 65536; s++) {
        SlotWord z = (s < 65536) ? arena[s] : 0;
        pwrite(fmem, &z, SLOT, (off_t)((NULO + s) * 2u));
    }
    int consts[][2] = {{1200,8},{1201,24576},{1202,1},{1203,256},{1204,0},{1205,8192},
        {1206,5},{1207,127},{1208,101},{1209,99},{1210,2},{1211,104},{1212,3},{1213,111},
        {1214,4},{1215,32},{1216,10},{1217,13},{1218,16384},{1219,24578},{1220,24580}};
    for (size_t i = 0; i < sizeof consts / sizeof consts[0]; i++) {
        int slot = consts[i][0], v = consts[i][1];
        SlotWord a = (SlotWord)(v & 255), b = (SlotWord)((v >> 8) & 255);
        pwrite(fmem, &a, SLOT, (off_t)(slot * 2u));
        pwrite(fmem, &b, SLOT, (off_t)(slot * 2u + 1u));
    }
}

int main(int argc, char **argv) {
    if (argc < 3) {
        fprintf(stderr, "uso: trace_fita <fita.bin> <mem.dat>\n");
        return 1;
    }
    fprog = open(argv[1], O_RDONLY);
    fmem = open(argv[2], O_RDWR);
    if (fprog < 0 || fmem < 0) return 1;
    off_t len = lseek(fprog, 0, SEEK_END);
    seed_mem("echo bench\n");
    Regs r = {0};
    long steps = 0;
    while (passo(&r, (unsigned)len) && steps < 500000) steps++;
    fprintf(stderr, "passos=%ld pc=%u\n", steps, r.pc);
    Word w603 = mem_le(603), w602 = mem_le(602), w601 = mem_le(601);
    Word out = mem_le(16392), nout = mem_le(24586);
    printf("601=%d 602=%d 603={%d,%d} out16392=%d nout24586=%d\n",
           w601.total | (w601.e << 8), w602.total | (w602.e << 8),
           w603.total, w603.e, out.total, nout.total | (nout.e << 8));
    return 0;
}
