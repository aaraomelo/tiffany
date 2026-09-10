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

static uint16_t A, B, R, PC;
static uint8_t halted, Z;
static uint8_t mem[65536];

static const char *opname(uint8_t op) {
    switch (op) {
        case 0x00: return "HALT";
        case 0x01: return "LOAD";
        case 0x02: return "STORE";
        case 0x03: return "ADD";
        case 0x04: return "SUB";
        case 0x05: return "CMP";
        case 0x06: return "JZ";
        case 0x07: return "JNZ";
        case 0x08: return "JMP";
        case 0x09: return "TROCA";
        case 0x0A: return "INC";
        case 0x0B: return "STORE_IND";
        default: return "???";
    }
}

static void step(int max_pc, int step_num, int fd) {
    if (halted || PC >= (uint16_t)max_pc) return;
    uint8_t op = mem[PC];
    printf("Step %d: PC=%u opcode=0x%02x (%s)", step_num, PC, op, opname(op));
    PC++;
    switch (op) {
        case 0x00: halted = 1; printf(" [HALT]"); break;
        case 0x01: A = mem[PC]; printf(" A=mem[%u]=0x%04x", PC, A); PC++; break;
        case 0x02: printf(" mem[%u]=A=0x%04x", B, A); mem[B] = A; break;
        case 0x03: A = A + B; printf(" A=0x%04x", A); break;
        case 0x04: A = A - B; printf(" A=0x%04x", A); break;
        case 0x05: Z = (A == B); printf(" Z=%d", Z); break;
        case 0x06: { int16_t rel = (int16_t)(mem[PC] | (mem[PC+1] << 8)); PC += 2; printf(" JZ rel=%d Z=%d", rel, Z); if (Z) PC += rel; } break;
        case 0x07: { int16_t rel = (int16_t)(mem[PC] | (mem[PC+1] << 8)); PC += 2; printf(" JNZ rel=%d Z=%d", rel, Z); if (!Z) PC += rel; } break;
        case 0x08: { int16_t rel = (int16_t)(mem[PC] | (mem[PC+1] << 8)); PC += 2; printf(" JMP rel=%d", rel); PC += rel; } break;
        case 0x09: { uint16_t t = R; R = A; A = t; printf(" TROCA"); } break;
        case 0x0A: mem[B]++; printf(" INC mem[%u]=0x%04x", B, mem[B]); break;
        case 0x0B: printf(" mem[%u]=R=0x%04x", B, A); mem[B] = A; break;
        default: printf(" UNKNOWN"); halted = 1; break;
    }
    printf("  [A=0x%04x B=0x%04x R=0x%04x PC=%u]\n", A, B, R, PC);
}

static void run(int max_steps, int fd) {
    for (int i = 0; i < max_steps && !halted; i++) step(10000, i, fd);
}

int main(void) {
    /* Create mem.dat */
    int fd = open("can/erg_le.dat", O_RDWR | O_CREAT | O_TRUNC, 0644);
    if (fd < 0) { printf("Cannot open can/erg_le.dat\n"); return 1; }

    uint8_t zero = 0;
    for (int i = 0; i < 256; i++) {
        lseek(fd, i, SEEK_SET);
        write(fd, &zero, 1);
    }

    /* Set payload: slot 40 = 0x0018 (0x34 in total) */
    Word w = { 0x18, 0 };
    mem_grava(fd, 40, w);
    /* Set zero: slot 0 = 0x0000 */
    w.total = 0; w.e = 0;
    mem_grava(fd, 0, w);

    /* Verify */
    printf("=== BEFORE EXECUTION ===\n");
    Word v40 = mem_le(fd, 40);
    Word v0 = mem_le(fd, 0);
    printf("  mem[40] = {0x%02x, 0x%02x}\n", v40.total, v40.e);
    printf("  mem[0]  = {0x%02x, 0x%02x}\n", v0.total, v0.e);
    printf("\n");

    /* Load binary */
    FILE *f = fopen("can/microtest_storeind.bin", "rb");
    if (!f) { printf("Cannot open can/microtest_storeind.bin\n"); return 1; }
    fseek(f, 0, SEEK_END);
    int binlen = ftell(f);
    fseek(f, 0, SEEK_SET);
    uint8_t *bin = malloc(binlen);
    fread(bin, 1, binlen, f);
    fclose(f);

    printf("=== DISASSEMBLY ===\n");
    int pc = 0;
    while (pc < binlen) {
        uint8_t op = bin[pc];
        printf("  Offset %d: 0x%02x (%s)\n", pc, op, opname(op));
        switch (op) {
            case 0x00: pc++; break;
            case 0x01: printf("    operand: slot %u\n", bin[pc+1]); pc += 2; break;
            case 0x02: printf("    operand: slot %u\n", bin[pc+1]); pc += 2; break;
            case 0x03: pc++; break;
            case 0x04: pc++; break;
            case 0x05: pc++; break;
            case 0x06: printf("    rel: %d\n", (int8_t)bin[pc+1]); pc += 2; break;
            case 0x07: printf("    rel: %d\n", (int8_t)bin[pc+1]); pc += 2; break;
            case 0x08: printf("    rel: %d\n", (int16_t)(bin[pc+1] | (bin[pc+2] << 8))); pc += 3; break;
            case 0x09: pc++; break;
            case 0x0A: pc++; break;
            case 0x0B: printf("    operand: slot %u\n", bin[pc+1]); pc += 2; break;
            default: pc++; break;
        }
    }
    printf("\n");

    /* Execute with erg_new.exe */
    printf("=== EXECUTING WITH erg_new.exe ===\n");
    int ret = system("cd /c/Users/jssim/aarao/tiffany/can && ../banco/erg_new.exe corre can/microtest_storeind.bin can/erg_le.dat");
    printf("ret=%d\n\n", ret);

    /* Read results */
    printf("=== AFTER EXECUTION ===\n");
    printf("  mem[40] = {0x%02x, 0x%02x}\n", mem_le(fd, 40).total, mem_le(fd, 40).e);
    printf("  mem[0]  = {0x%02x, 0x%02x}\n", mem_le(fd, 0).total, mem_le(fd, 0).e);
    printf("  mem[24] = {0x%02x, 0x%02x}  <-- STORE_IND target\n", mem_le(fd, 24).total, mem_le(fd, 24).e);
    printf("  mem[50] = {0x%02x, 0x%02x}  <-- STORE target (not used here)\n", mem_le(fd, 50).total, mem_le(fd, 50).e);

    /* Also verify with erg_new.exe ve */
    printf("\n=== VERIFY WITH erg_new.exe ve ===\n");
    system("cd /c/Users/jssim/aarao/tiffany/can && ../banco/erg_new.exe ve can/erg_le.dat 24 2>&1");
    system("cd /c/Users/jssim/aarao/tiffany/can && ../banco/erg_new.exe ve can/erg_le.dat 40 2>&1");
    system("cd /c/Users/jssim/aarao/tiffany/can && ../banco/erg_new.exe ve can/erg_le.dat 50 2>&1");

    free(bin);
    close(fd);
    return 0;
}