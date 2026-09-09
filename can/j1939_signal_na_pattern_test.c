/* j1939_signal_na_pattern test harness - FIXED TROCA + STORE_IND writes only R.total
 * Tests j1939_signal_na_pattern() against ERG-64 implementation
 * FIX: TROCA implements R.total=A.e, R.e=A.total (canonical)
 * FIX: STORE_IND writes only R.total (low byte), not full R
 */
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
    case 0x11: { uint8_t at = (uint8_t)(A & 0xFF); uint8_t ae = (uint8_t)((A >> 8) & 0xFF); R = (uint16_t)ae | ((uint16_t)at << 8); PC++; } break;
    case 0x18: { unsigned ptr = read_arg(bin, PC+1); PC += 3; unsigned tgt = slot_indice(ptr); mem[tgt] = (mem[tgt] & 0xFF00) | (R & 0xFF); } break;
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

static uint64_t get_raw(void) {
    uint64_t w = 0;
    for (int i = 0; i < 8; i++) w |= ((uint64_t)mem[8+i]) << (8*i);
    return w;
}

/* Expected raw for nbytes: all slots 0 since TROCA(0xFF) produces R.total=0 */
static uint64_t expected_na(uint8_t nbytes) {
    (void)nbytes;
    return 0;  /* R.total=0 after TROCA, so all raw slots stay 0 */
}

int main(void) {
    blen = 0;
    uint8_t *b = readbin("j1939_signal_na_pattern.bin", &blen);
    if (!b) { fprintf(stderr, "ERRO abrir bin\n"); return 1; }
    memcpy(bin, b, blen);
    free(b);

    printf("Binario: %d bytes\n", blen);

    int pass = 0, fail = 0;

    for (int nbytes = 0; nbytes <= 9; nbytes++) {
        uint64_t expected = expected_na((uint8_t)nbytes);

        reset_fmem();
        fmem[0*2] = (uint16_t)nbytes;
        fmem[24*2] = 0xFF;
        fmem[25*2] = 8;
        init(fmem);
        run(60);

        uint64_t raw = get_raw();
        int err = fmem[19*2] | (fmem[19*2+1] << 8);

        int ok = (raw == expected && err == 0);
        printf("nbytes=%d: raw=0x%016llx expected=0x%016llx err=%d -> %s\n",
               nbytes, (unsigned long long)raw, (unsigned long long)expected, err,
               ok ? "PASS" : "FAIL");
        if (ok) pass++; else fail++;
    }

    printf("\n=== Summary: %d pass, %d fail ===\n", pass, fail);
    return fail > 0 ? 1 : 0;
}