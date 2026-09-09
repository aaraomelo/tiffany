/* microtest harness: TROCA + STORE_IND + INC */
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>

static uint16_t fmem[65536];
static uint16_t mem[65536];
static uint8_t bin[65536];
static int blen;
static uint16_t A, B, R, PC, flags;
static int halted;

static void reset_fmem(void) { memset(fmem, 0, sizeof(fmem)); }
static void init(const uint16_t *m) {
    for (int i = 0; i < 65536; i++) mem[i] = m[i*2] | (m[i*2+1] << 8);
    A = B = R = PC = flags = 0; halted = 0;
}

static unsigned read_arg(const uint8_t *c, int pos) {
    return (unsigned)c[pos] | ((unsigned)c[pos+1] << 8);
}

static unsigned slot_indice(unsigned ptr) {
    return (unsigned)mem[ptr] | ((unsigned)mem[ptr+1] << 8);
}

static int step(int trace) {
    if (halted || PC >= (uint16_t)blen) return 1;
    uint8_t op = bin[PC];
    switch (op) {
    case 0x00: halted = 1; PC++; break;
    case 0x01: { unsigned slot = read_arg(bin, PC+1); PC += 3; B = A; A = mem[slot]; } break;
    case 0x02: { unsigned slot = read_arg(bin, PC+1); PC += 3; mem[slot] = R; } break;
    case 0x03: R = A + B; flags = 0; if (R == 0) flags |= 1; PC++; break;
    case 0x04: R = A - B; flags = 0; if (R == 0) flags |= 1; PC++; break;
    case 0x09: R = A - B; flags = 0; if (R == 0) flags |= 1; PC++; break;
    case 0x0A: { int16_t rel = (int16_t)((int8_t)bin[PC+1] | (bin[PC+2] << 8)); PC += 3; PC = (uint16_t)((int16_t)PC + rel); } break;
    case 0x0B: { int16_t rel = (int16_t)((int8_t)bin[PC+1] | (bin[PC+2] << 8)); PC += 3; if (flags & 1) PC = (uint16_t)((int16_t)PC + rel); } break;
    case 0x0C: { int16_t rel = (int16_t)((int8_t)bin[PC+1] | (bin[PC+2] << 8)); PC += 3; if (!(flags & 1)) PC = (uint16_t)((int16_t)PC + rel); } break;
    case 0x0E: { unsigned ptr = read_arg(bin, PC+1); PC += 3; B = A; unsigned idx = slot_indice(ptr); A = mem[idx]; } break;
    case 0x11: { uint16_t t = A; A = R; R = t; PC++; } break;
    case 0x18: { unsigned ptr = read_arg(bin, PC+1); PC += 3; unsigned tgt = slot_indice(ptr); mem[tgt] = R; } break;
    case 0x1A: { unsigned slot = read_arg(bin, PC+1); PC += 3; mem[slot]++; } break;
    default: printf("    UNKNOWN opcode 0x%02x at PC=%d\n", op, PC); return -1;
    }
    return 0;
}

static void run(int max) { for (int i = 0; i < max && !halted; i++) step(0); }
static uint8_t *readbin(const char *p, int *l) {
    FILE *f = fopen(p, "rb"); if (!f) return NULL;
    fseek(f, 0, SEEK_END); long sz = ftell(f); fseek(f, 0, SEEK_SET);
    uint8_t *b = malloc(sz); fread(b, 1, sz, f); fclose(f); *l = (int)sz; return b;
}

/* Word type matching erg.c: total (low byte) + e (high byte) */
typedef struct { uint8_t total; uint8_t e; } Word;

static uint16_t make_word(uint8_t total, uint8_t e) {
    return (uint16_t)total | ((uint16_t)e << 8);
}

static uint8_t word_total(uint16_t w) { return (uint8_t)(w & 0xFF); }
static uint8_t word_e(uint16_t w) { return (uint8_t)((w >> 8) & 0xFF); }

/* Canonical TROCA: R = corpo_gira(A, +1) = (A.e, A.total) */
static uint16_t troca_canonical(uint16_t a) {
    uint8_t t = word_total(a);
    uint8_t ee = word_e(a);
    return make_word(ee, t);  /* R.total = A.e, R.e = A.total */
}

/* Current harness TROCA: swap(A, R) */
static uint16_t troca_harness(uint16_t a, uint16_t r) {
    (void)r;
    return a;  /* swap: R gets A, A gets old R */
}

int main(void) {
    printf("=== Test 1: TROCA isolated ===\n");
    uint16_t A1 = make_word(0x12, 0x34);
    uint16_t R1 = make_word(0x00, 0x00);
    uint16_t R1_harness = troca_harness(A1, R1);
    uint16_t R1_canonical = troca_canonical(A1);
    printf("A.total=0x%02x A.e=0x%02x\n", word_total(A1), word_e(A1));
    printf("harness TROCA: R.total=0x%02x R.e=0x%02x (expected 0x34, 0x12)\n",
           word_total(R1_harness), word_e(R1_harness));
    printf("canonical TROCA: R.total=0x%02x R.e=0x%02x\n",
           word_total(R1_canonical), word_e(R1_canonical));
    printf("TROCA test: %s\n", (word_total(R1_canonical) == 0x34 && word_e(R1_canonical) == 0x12) ? "PASS" : "FAIL");

    printf("\n=== Test 2: STORE_IND + TROCA ===\n");
    reset_fmem();
    fmem[25*2] = 0x0008;  /* ptr_raw = 8 */
    fmem[24*2] = 0x00FF;  /* slot 24 = 0xFF */
    init(fmem);
    /* A = 0xFF (total), 0x00 (e). R = 0xFF (total), 0x00 (e) before TROCA */
    uint16_t A2 = make_word(0xFF, 0x00);
    uint16_t R2 = make_word(0xFF, 0x00);
    /* TROCA: R.total = A.e = 0, R.e = A.total = 0xFF */
    uint16_t R2_new = troca_canonical(A2);
    printf("Before TROCA: A.total=0x%02x A.e=0x%02x\n", word_total(A2), word_e(A2));
    printf("After TROCA: R.total=0x%02x R.e=0x%02x\n", word_total(R2_new), word_e(R2_new));
    /* STORE_IND 25: target = slot_indice(25) = 8 */
    unsigned tgt = slot_indice(25);
    printf("target = %u\n", tgt);
    mem[tgt] = R2_new;
    printf("mem[8] = 0x%04x (expected 0x0000)\n", mem[8]);

    printf("\n=== Test 3: STORE_IND + INC ===\n");
    reset_fmem();
    fmem[25*2] = 0x0008;  /* ptr_raw = 8 */
    fmem[24*2] = 0x00FF;  /* slot 24 = 0xFF */
    init(fmem);
    uint16_t A3 = make_word(0xFF, 0x00);
    uint16_t R3 = make_word(0xFF, 0x00);
    /* TROCA */
    R3 = troca_canonical(A3);
    /* STORE_IND 25 */
    tgt = slot_indice(25);
    mem[tgt] = R3;
    /* INC 25 */
    mem[25]++;
    /* TROCA */
    R3 = troca_canonical(A3);
    /* STORE_IND 25 */
    tgt = slot_indice(25);
    mem[tgt] = R3;
    /* INC 25 */
    mem[25]++;
    /* TROCA */
    R3 = troca_canonical(A3);
    /* STORE_IND 25 */
    tgt = slot_indice(25);
    mem[tgt] = R3;

    printf("mem[8] = 0x%04x (expected 0x0000)\n", mem[8]);
    printf("mem[9] = 0x%04x (expected 0x0000)\n", mem[9]);
    printf("mem[10] = 0x%04x (expected 0x0000)\n", mem[10]);
    printf("mem[25] = 0x%04x (expected 11)\n", mem[25]);

    return 0;
}