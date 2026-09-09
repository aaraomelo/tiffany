#ifdef _WIN32
#include <windows.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <io.h>
#define pread(fd,buf,len,off) read(fd,buf,len)
#define pwrite(fd,buf,len,off) write(fd,buf,len)
#define O_BINARY _O_BINARY
#define O_RDONLY _O_RDONLY
#define O_RDWR _O_RDWR
#define O_CREAT _O_CREAT
#define O_TRUNC _O_TRUNC
#else
#define _GNU_SOURCE
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#endif

static int fprog = -1;
static int fmem = -1;

static unsigned char prog_le(unsigned pc){
    unsigned char b = 0x00;
    if(pread(fprog, &b, 1, (long)pc) != 1) return 0x00;
    return b;
}

typedef struct { uint8_t total; uint8_t e; } Word;
typedef struct { Word A, B, R; unsigned pc; unsigned char flags; } Regs;

static Word mem_le(unsigned slot){
    Word w = {0,0};
    if(fmem >= 0){
        uint8_t a=0,b=0;
        pread(fmem, &a, 1, (long)(slot*2));
        pread(fmem, &b, 1, (long)(slot*2+1));
        w.total = a; w.e = b;
    }
    return w;
}

static void mem_grava(unsigned slot, Word w){
    if(fmem >= 0){
        uint8_t a=w.total, b=w.e;
        pwrite(fmem, &a, 1, (long)(slot*2));
        pwrite(fmem, &b, 1, (long)(slot*2+1));
    }
}

typedef struct { uint8_t baixo, alto; } W16;
static uint8_t ula_soma_c(uint8_t a, uint8_t b, uint8_t cin, uint8_t *cout){
    uint8_t s=0, c=cin;
    for(int i=0;i<8;i++){
        uint8_t ai=(a>>i)&1, bi=(b>>i)&1;
        uint8_t x=ai^bi, si=x^c, c1=ai&bi, c2=c&x;
        c=(c1|c2); s|=(si<<i);
    }
    *cout=c; return s;
}
static W16 ula_add16(W16 a, W16 b, uint8_t *t){
    W16 r; uint8_t c1=0,c2=0;
    r.baixo=ula_soma_c(a.baixo,b.baixo,0,&c1);
    r.alto=ula_soma_c(a.alto,b.alto,c1,&c2);
    if(t)*t=c2; return r;
}
static Word ula_add(Word a, Word b){
    W16 ra={a.total,a.e}, rb={b.total,b.e}, rx;
    uint8_t dummy;
    rx=ula_add16(ra,rb,&dummy);
    Word r={rx.baixo,rx.alto}; return r;
}

static unsigned MOVE_exec(Regs *r, unsigned pc, int sentido){
    unsigned slot = (unsigned)prog_le(pc) | ((unsigned)prog_le(pc+1)<<8);
    pc += 2;
    printf("  MOVE_exec(sentido=%d, slot=%u): ", sentido, slot);
    if(sentido > 0){
        printf("B=A=(%02x,%02x); ", r->A.total, r->A.e);
        r->B = r->A;
        Word w = mem_le(slot);
        printf("A=mem[%u]=(%02x,%02x)\n", slot, w.total, w.e);
        r->A = w;
    } else {
        printf("mem[%u]=R=(%02x,%02x)\n", slot, r->R.total, r->R.e);
        mem_grava(slot, r->R);
    }
    return pc;
}

static int passo(Regs *r, unsigned prog_len){
    if(r->pc >= prog_len){ printf("  passo: pc(%u) >= prog_len(%u) -> return 0\n", r->pc, prog_len); return 0; }
    unsigned pc = r->pc;
    unsigned char op = prog_le(pc++);
    printf("  passo: pc=%u op=0x%02x\n", pc-1, op);
    switch(op){
    case 0x00: return 0;
    case 0x01: pc = MOVE_exec(r, pc, +1); break;
    case 0x02: pc = MOVE_exec(r, pc, -1); break;
    case 0x03: r->R = ula_add(r->A, r->B); printf("    ADD: R=(%02x,%02x)\n", r->R.total, r->R.e); break;
    case 0x04: printf("    SUB (not implemented in debug)\n"); break;
    case 0x05: printf("    TROCA\n"); break;
    case 0x1a: printf("    JZ\n"); break;
    case 0x1b: printf("    JMP\n"); break;
    default: printf("    unknown op\n"); break;
    }
    r->pc = pc;
    return 1;
}

static long rodar(unsigned prog_len, long teto){
    Regs r; memset(&r, 0, sizeof r);
    long n = 0;
    printf("rodar: prog_len=%u, teto=%ld, initial pc=%u\n", prog_len, teto, r.pc);
    while(passo(&r, prog_len)){
        if(++n >= teto){ printf("rodar: hit teto %ld\n", teto); break; }
        if(n > 20){ printf("rodar: stopping at 20 steps for debug\n"); break; }
    }
    printf("rodar: final n=%ld, final pc=%u\n", n, r.pc);
    return n;
}

int main(int argc, char **argv){
    if(argc < 4){ fprintf(stderr, "uso: debug_corre <prog.bin> <mem.dat> [teto]\n"); return 2; }
    long teto = (argc >= 5) ? strtol(argv[4], NULL, 0) : 1000000;
    printf("argv[1]=%s argv[2]=%s argv[3]=%s teto=%ld\n", argv[1], argv[2], argv[3], teto);
    fprog = open(argv[1], O_RDONLY | O_BINARY);
    printf("fprog open: %d\n", fprog);
    long len = lseek(fprog, 0, SEEK_END);
    printf("prog_len=%ld\n", len);
    unsigned char *b = malloc(len);
    pread(fprog, b, len, 0);
    printf("first %ld bytes: ", len);
    for(long i=0; i<len && i<20; i++) printf("%02x ", b[i]);
    printf("\n");
    lseek(fprog, 0, SEEK_SET);
    fmem = open(argv[2], O_RDWR | O_CREAT | O_BINARY, 0644);
    printf("fmem open: %d\n", fmem);
    long passos = rodar((unsigned)len, teto);
    printf("passos=%ld\n", passos);
    return 0;
}