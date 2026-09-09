/* j1939_signal_extract test harness - microtests
 * Tests individual primitives used by the implementation
 */
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>

static uint16_t fmem[65536];
static uint16_t mem[65536];

/* Word16: total (low byte) + e (high byte) */
typedef struct { uint8_t total; uint8_t e; } Word;

static uint16_t make_word(uint8_t total, uint8_t e) {
    return (uint16_t)total | ((uint16_t)e << 8);
}
static uint8_t word_total(uint16_t w) { return (uint8_t)(w & 0xFF); }
static uint8_t word_e(uint16_t w) { return (uint8_t)((w >> 8) & 0xFF); }

/* Canonical TROCA: R = corpo_gira(A, +1) = (A.e, A.total) */
static uint16_t troca_canonical(uint16_t a) {
    uint8_t at = word_total(a);
    uint8_t ae = word_e(a);
    return make_word(ae, at);
}

int main(void) {
    printf("=== Microtest 1: TROCA ===\n");
    uint16_t A1 = make_word(0x12, 0x34);
    uint16_t R1 = troca_canonical(A1);
    printf("A.total=0x%02x A.e=0x%02x\n", word_total(A1), word_e(A1));
    printf("R.total=0x%02x R.e=0x%02x (expected 0x34, 0x12)\n", word_total(R1), word_e(R1));
    printf("TROCA: %s\n", (word_total(R1) == 0x34 && word_e(R1) == 0x12) ? "PASS" : "FAIL");

    printf("\n=== Microtest 2: STORE_IND ===\n");
    memset(mem, 0, sizeof(mem));
    mem[25] = 8;  /* slot_indice = mem[25] = 8 */
    uint16_t R2 = 0x00FF;
    unsigned tgt = mem[25];
    printf("target = %u (expected 8)\n", tgt);
    mem[tgt] = R2;
    printf("mem[8] = 0x%04x (expected 0x00FF)\n", mem[8]);
    printf("STORE_IND: %s\n", (mem[8] == 0x00FF) ? "PASS" : "FAIL");

    printf("\n=== Microtest 3: INC ===\n");
    memset(mem, 0, sizeof(mem));
    mem[10] = 0x0005;
    mem[10]++;
    printf("mem[10] = 0x%04x (expected 0x0006)\n", mem[10]);
    printf("INC: %s\n", (mem[10] == 0x0006) ? "PASS" : "FAIL");

    return 0;
}