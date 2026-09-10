#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>

typedef uint8_t Word8;
typedef struct { Word8 total, e; } Word;

#define SLOT_WORD_BYTES 1

static inline Word8 slot_w8_wrap_u(unsigned long v){
    return (uint8_t)(v & 0xFF);
}
static inline Word8 slot_w8_de_long(long v){ return (uint8_t)v; }
static inline long slot_long_de_w8(Word8 b){ return (long)b; }

static inline Word8 slot_mem_le(int fd, unsigned slot){
    Word8 w = 0;
    if(fd >= 0) {
        lseek(fd, (off_t)slot * SLOT_WORD_BYTES, SEEK_SET);
        read(fd, &w, SLOT_WORD_BYTES);
    }
    return w;
}
static inline void slot_mem_grava(int fd, unsigned slot, Word8 w){
    if(fd >= 0) {
        lseek(fd, (off_t)slot * SLOT_WORD_BYTES, SEEK_SET);
        write(fd, &w, SLOT_WORD_BYTES);
    }
}

static inline Word mem_le(int fd, unsigned slot){
    Word w = {0,0};
    w.total = slot_mem_le(fd, slot * 2u);
    w.e     = slot_mem_le(fd, slot * 2u + 1u);
    return w;
}
static inline void mem_grava(int fd, unsigned slot, Word w){
    slot_mem_grava(fd, slot * 2u,     w.total);
    slot_mem_grava(fd, slot * 2u + 1u, w.e);
}

int main(void) {
    /* Create mem.dat: slot 40 = payload[0] = 0x12, etc. */
    int fd = open("can/erg_le.dat", O_RDWR | O_CREAT | O_TRUNC, 0644);
    if (fd < 0) { printf("Cannot open can/erg_le.dat\n"); return 1; }

    /* Initialize: all zeros */
    uint8_t zero = 0;
    for (int i = 0; i < 256; i++) {
        lseek(fd, i, SEEK_SET);
        write(fd, &zero, 1);
    }

    /* Payload: 12 34 56 78 9A BC DE F0 */
    /* Each slot = 2 bytes (total, e). Slot 40..47 = payload bytes */
    Word8 payload[8] = {0x12, 0x34, 0x56, 0x78, 0x9A, 0xBC, 0xDE, 0xF0};
    for (int i = 0; i < 8; i++) {
        Word w = { payload[i], 0 };  /* total = payload byte, e = 0 */
        mem_grava(fd, 40 + i, w);
    }

    /* Verify payload */
    printf("=== PAYLOAD VERIFICATION ===\n");
    for (int i = 0; i < 8; i++) {
        Word w = mem_le(fd, 40 + i);
        printf("  slot %d = {total=0x%02x, e=0x%02x} (%s)\n", 40+i, w.total, w.e, w.total == payload[i] && w.e == 0 ? "OK" : "FAIL");
    }
    printf("\n");
    close(fd);

    /* Now run erg_new.exe */
    printf("=== EXECUTING ===\n");
    int ret = system("cd /c/Users/jssim/aarao/tiffany/can && ../banco/erg_new.exe corre can/j1939_le_v2.bin can/erg_le.dat");
    printf("ret=%d\n\n", ret);

    /* Read results */
    fd = open("can/erg_le.dat", O_RDONLY);
    if (fd < 0) { printf("Cannot open result file\n"); return 1; }

    printf("=== OUTPUT ===\n");
    printf("raw_out slots 24..31:\n");
    for (int i = 0; i < 8; i++) {
        Word w = mem_le(fd, 24 + i);
        printf("  slot %d = {total=0x%02x, e=0x%02x}\n", 24+i, w.total, w.e);
    }
    Word ret_word = mem_le(fd, 32);
    Word status_word = mem_le(fd, 33);
    printf("return_code (slot 32) = {total=0x%02x, e=0x%02x}\n", ret_word.total, ret_word.e);
    printf("status (slot 33) = {total=0x%02x, e=0x%02x}\n", status_word.total, status_word.e);
    close(fd);

    /* Verify LE extraction: raw_out bytes 24..31 should be 12 34 56 78 9A BC DE F0 */
    printf("\n=== VERIFICATION ===\n");
    Word8 expected[8] = {0x12, 0x34, 0x56, 0x78, 0x9A, 0xBC, 0xDE, 0xF0};
    int pass = 1;
    fd = open("can/erg_le.dat", O_RDONLY);
    for (int i = 0; i < 8; i++) {
        Word w = mem_le(fd, 24 + i);
        if (w.total != expected[i] || w.e != 0) {
            printf("FAIL slot 24+%d: expected {0x%02x, 0x00}, got {0x%02x, 0x%02x}\n", i, expected[i], w.total, w.e);
            pass = 0;
        }
    }
    if (pass) printf("ALL 8 bytes PASS\n");
    close(fd);

    return pass ? 0 : 1;
}