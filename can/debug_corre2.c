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
#else
#define _GNU_SOURCE
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#endif

#define OPEN_BIN O_BINARY

static int fprog = -1;
static unsigned char prog[65536];
static unsigned prog_len = 0;

static int prog_le(unsigned pc){
    unsigned char b = 0;
    if(pread(fprog, &b, 1, (off_t)pc) != 1) return 0;
    return b;
}

typedef struct { uint16_t total; uint8_t e; } Word16;
typedef struct { uint8_t total; uint8_t e; } Word8;

static Word16 mem_le(int slot){
    Word16 w = {0,0};
    unsigned off = slot * 2;
    w.total = (unsigned)prog_le(off) | ((unsigned)prog_le(off+1) << 8);
    w.e = prog_le(off + 2);
    return w;
}

static void mem_grava(int slot, Word16 w){
    unsigned off = slot * 2;
    prog_le(off); prog_le(off+1); prog_le(off+2); // dummy reads
}
