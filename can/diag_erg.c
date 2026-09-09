/* diag_erg.c — trace each step with actual binary content */
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>

static uint16_t fmem[65536];
static uint16_t mem[65536];

static uint16_t A, B, R;
static uint16_t PC;
static uint8_t halted;
static uint8_t Z;

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

static void step(int max_pc, int step_num) {
    if (halted || PC >= (uint16_t)max_pc) return;
    uint8_t op = mem[PC];
    printf("Step %d: PC=%u opcode=0x%02x (%s)\n", step_num, PC, op, opname(op));
    PC++;
    switch (op) {
        case 0x00: halted = 1; break;
        case 0x01: A = mem[PC]; printf("  A = mem[%u] = 0x%04x\n", PC, A); PC++; break;
        case 0x02: printf("  mem[%u] = A = 0x%04x\n", B, A); mem[B] = A; break;
        case 0x03: A = A + B; printf("  A = 0x%04x\n", A); break;
        case 0x04: A = A - B; printf("  A = 0x%04x\n", A); break;
        case 0x05: Z = (A == B); printf("  Z = %d (A=0x%04x B=0x%04x)\n", Z, A, B); break;
        case 0x06: { int16_t rel = (int16_t)(mem[PC] | (mem[PC+1] << 8)); PC += 2; printf("  JZ rel=%d Z=%d newPC=%u\n", rel, Z, PC); if (Z) PC += rel; } break;
        case 0x07: { int16_t rel = (int16_t)(mem[PC] | (mem[PC+1] << 8)); PC += 2; printf("  JNZ rel=%d Z=%d newPC=%u\n", rel, Z, PC); if (!Z) PC += rel; } break;
        case 0x08: { int16_t rel = (int16_t)(mem[PC] | (mem[PC+1] << 8)); PC += 2; printf("  JMP rel=%d newPC=%u\n", rel, PC+rel); PC += rel; } break;
        case 0x09: { uint16_t t = R; R = A; A = t; printf("  TROCA A=0x%04x R=0x%04x\n", A, R); } break;
        case 0x0A: mem[B]++; printf("  INC mem[%u] = 0x%04x\n", B, mem[B]); break;
        case 0x0B: mem[B] = A; printf("  STORE_IND mem[%u] = A = 0x%04x\n", B, A); break;
        default: printf("  Unknown opcode 0x%02x\n", op); halted = 1; break;
    }
}

static void run(int max_pc, int max_steps) {
    for (int i = 0; i < max_steps && !halted; i++) step(max_pc, i);
}

int main(void) {
    FILE *f = fopen("j1939_signal_extract.bin", "rb");
    if (!f) { printf("Cannot open j1939_signal_extract.bin\n"); return 1; }
    fseek(f, 0, SEEK_END);
    int binlen = ftell(f);
    fseek(f, 0, SEEK_SET);
    uint8_t *bin = malloc(binlen);
    fread(bin, 1, binlen, f);
    fclose(f);
    printf("Binary: %d bytes\n", binlen);

    /* Print binary bytes */
    printf("Bytes: ");
    for (int i = 0; i < binlen; i++) {
        printf("%02x ", bin[i]);
    }
    printf("\n");

    /* Load binary into mem */
    memcpy(mem, bin, binlen);

    /* Set up fmem with test data */
    memset(fmem, 0, sizeof(fmem));
    memset(mem, 0, sizeof(mem));

    fmem[40*2] = 0x0034;  /* payload[0] = 0x34 */
    fmem[41*2] = 0x0012;  /* payload[1] = 0x12 */

    /* Run */
    run(binlen, 100);

    printf("\nFinal state: A=0x%04x B=0x%04x R=0x%04x PC=%u halted=%d\n", A, B, R, PC, halted);
    printf("fmem[40] = 0x%04x (expected 0x0034)\n", fmem[40*2]);
    printf("fmem[41] = 0x%04x (expected 0x0012)\n", fmem[41*2]);
    printf("fmem[24] = 0x%04x (expected 0x0034)\n", fmem[24*2]);

    free(bin);
    return 0;
}