#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>

typedef uint8_t Word8;
typedef struct { Word8 total, e; } Word;
typedef uint8_t SlotWord;
#define SLOT_WORD_BYTES 1

static long slot_w8_wrap = 0;
static long slot_w8_sat = 0;

static inline uint8_t slot_w8_wrap_u(unsigned long v){
    if(v > 255u){ slot_w8_wrap++; return (uint8_t)(v & 255u); }
    return (uint8_t)v;
}
static inline uint8_t slot_w8_de_long(long v){
    if(v < 0 || (unsigned long)v > 255) slot_w8_wrap++;
    return (uint8_t)v;
}
static inline long slot_long_de_w8(uint8_t b){ return (long)b; }

static inline SlotWord slot_mem_le(int fd, unsigned slot){
    SlotWord w = 0;
    if(fd >= 0) pread(fd, &w, SLOT_WORD_BYTES, (off_t)slot * SLOT_WORD_BYTES);
    return w;
}
static inline void slot_mem_grava(int fd, unsigned slot, SlotWord w){
    if(fd >= 0) pwrite(fd, &w, SLOT_WORD_BYTES, (off_t)slot * SLOT_WORD_BYTES);
}

static Word mem_le(int fd, unsigned slot){
    Word w = {0,0};
    if(fd >= 0){
        w.total = slot_mem_le(fd, slot * 2u);
        w.e     = slot_mem_le(fd, slot * 2u + 1u);
    }
    return w;
}
static void mem_grava(int fd, unsigned slot, Word w){
    if(fd >= 0){
        slot_mem_grava(fd, slot * 2u,     w.total);
        slot_mem_grava(fd, slot * 2u + 1u, w.e);
    }
}

static unsigned char prog_le(int fd, unsigned pc){
    unsigned char b = 0;
    if(pread(fd, &b, 1, (off_t)pc) != 1) return 0;
    return b;
}

typedef struct { Word A, B, R; unsigned pc; unsigned char flags; } Regs;

static Word ula_add(Word a, Word b){
    Word r;
    unsigned s, c = 0;
    /* total */
    s = (a.total ^ b.total) ^ c; c = (a.total & b.total) | (c & (a.total ^ b.total));
    r.total = (Word8)s;
    /* e */
    s = (a.e ^ b.e) ^ c; c = (a.e & b.e) | (c & (a.e ^ b.e));
    r.e = (Word8)s;
    return r;
}

static Word ula_sub(Word a, Word b){
    Word nb; nb.total = (Word8)~b.total; nb.e = (Word8)~b.e;
    return ula_add(a, nb);  /* simplified: a + ~b + 1 */
}

static unsigned MOVE_exec(Regs *r, int fd, unsigned pc, int sentido){
    unsigned slot = (unsigned)prog_le(fd, pc) | ((unsigned)prog_le(fd, pc+1) << 8);
    pc += 2;
    if(sentido > 0){ r->B = r->A; r->A = mem_le(fd, slot); }
    else { mem_grava(fd, slot, r->R); }
    return pc;
}

static int passo(Regs *r, int fprog, int fmem, unsigned prog_len){
    if(r->pc >= prog_len) return 0;
    unsigned pc = r->pc;
    unsigned char op = prog_le(fprog, pc++);
    switch(op){
    case 0: return 0;
    case 1: pc = MOVE_exec(r, fmem, pc, +1); break;
    case 2: pc = MOVE_exec(r, fmem, pc, -1); break;
    case 3: r->R = ula_add(r->A, r->B); break;
    case 4: r->R = ula_sub(r->A, r->B); break;
    default: return 0;
    }
    r->pc = pc;
    return 1;
}

static long rodar(unsigned prog_len, long teto, int fprog, int fmem){
    Regs r; memset(&r, 0, sizeof r);
    long n = 0;
    while(passo(&r, fprog, fmem, prog_len) && n < teto) n++;
    return n;
}

int main(int argc, char **argv){
    if(argc < 3){ fprintf(stderr, "uso: %s <prog.bin> <mem.dat>\n", argv[0]); return 1; }
    int fprog = open(argv[1], O_RDONLY);
    int fmem = open(argv[2], O_RDWR);
    if(fprog < 0 || fmem < 0){ perror("open"); return 1; }

    /* Prepare mem.dat: slot 40 = 0x34, slot 0 = 0x0000, slot 50 = 0x0000 */
    mem_grava(fmem, 40, (Word){0x34, 0x00});
    mem_grava(fmem, 0,  (Word){0x00, 0x00});
    mem_grava(fmem, 50, (Word){0x00, 0x00});

    /* Dump initial state */
    Word w40 = mem_le(fmem, 40);
    Word w0  = mem_le(fmem, 0);
    Word w50 = mem_le(fmem, 50);
    printf("INITIAL:\n");
    printf("  A={total=0x%02x,e=0x%02x} B={total=0x%02x,e=0x%02x} R={total=0x%02x,e=0x%02x}\n",
           0,0,0,0,0,0);
    printf("  fmem[40]={total=0x%02x,e=0x%02x} fmem[0]={total=0x%02x,e=0x%02x} fmem[50]={total=0x%02x,e=0x%02x}\n\n",
           w40.total,w40.e,w0.total,w0.e,w50.total,w50.e);

    /* Load binary */
    off_t bin_size = lseek(fprog, 0, SEEK_END);
    unsigned char *bin = malloc(bin_size);
    pread(fprog, bin, bin_size, 0);

    /* Run */
    long steps = rodar((unsigned)bin_size, 100, fprog, fmem);

    /* Dump final state */
    Word A = mem_le(fmem, 40);  /* wrong - need register state */
    w40 = mem_le(fmem, 40);
    w0  = mem_le(fmem, 0);
    w50 = mem_le(fmem, 50);
    printf("FINAL (from mem.dat):\n");
    printf("  fmem[40]={total=0x%02x,e=0x%02x} fmem[0]={total=0x%02x,e=0x%02x} fmem[50]={total=0x%02x,e=0x%02x}\n",
           w40.total,w40.e,w0.total,w0.e,w50.total,w50.e);
    printf("  steps=%ld\n", steps);

    free(bin);
    close(fprog);
    close(fmem);
    return 0;
}