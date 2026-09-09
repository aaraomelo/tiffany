// trace_load.c — standalone trace interpreter for debugging LOAD semantics
// Reads erg binary, simulates LOAD + HALT, prints register state
#include <stdio.h>
#include <stdint.h>
#include <string.h>

typedef struct { uint16_t total; uint8_t e; } Word16;

static Word16 mem[256]; // mem.dat slots as Word16
static Word16 A, B, R;

int main(int argc, char **argv) {
    if (argc < 3) { fprintf(stderr, "usage: trace_load <bin> <mem.dat>\n"); return 1; }

    // Load mem.dat (each slot = 2 bytes: total, e)
    FILE *fm = fopen(argv[2], "rb");
    if (!fm) { perror("mem.dat"); return 1; }
    fread(mem, sizeof(Word16), 256, fm);
    fclose(fm);

    // Load erg binary
    FILE *fb = fopen(argv[1], "rb");
    if (!fb) { perror("bin"); return 1; }
    uint8_t code[1024];
    int n = fread(code, 1, 1024, fb);
    fclose(fb);

    // Initialize registers
    A.total = 0; A.e = 0;
    B.total = 0; B.e = 0;
    R.total = 0; R.e = 0;

    // Simple disasm + execute
    int pc = 0;
    while (pc < n) {
        uint8_t op = code[pc++];
        uint8_t arg = code[pc++];
        printf("PC=%d op=0x%02x arg=0x%02x", pc-2, op, arg);

        switch (op) {
            case 0x01: // LOAD slot
                B.total = A.total; B.e = A.e;
                A.total = mem[arg].total; A.e = mem[arg].e;
                printf("  LOAD %d: B=A, A=mem[%d]", arg, arg);
                break;
            case 0x02: // STORE slot
                mem[arg].total = R.total; mem[arg].e = R.e;
                printf("  STORE %d: mem[%d]=R", arg, arg);
                break;
            case 0x03: // ADD
                R.total = A.total + B.total;
                printf("  ADD: R=A+B");
                break;
            case 0x04: // SUB
                R.total = A.total - B.total;
                printf("  SUB: R=A-B");
                break;
            case 0x05: // TROCA
                { uint16_t t=R.total; R.total=R.e; R.e=t; }
                printf("  TROCA");
                break;
            case 0x1a: // JZ
                printf("  JZ"); pc = (arg < 256) ? arg : pc; break;
            case 0x1b: // JMP
                printf("  JMP"); pc = arg; break;
            case 0x00: // HALT
                printf("  HALT"); pc = n; break;
            default:
                printf("  ???"); pc = n; break;
        }
        printf("  A=%04x(e=%02x) B=%04x(e=%02x) R=%04x(e=%02x)\n",
               A.total, A.e, B.total, B.e, R.total, R.e);
        if (op == 0x00) break;
    }

    // Print final mem.dat slots
    printf("\nmem.dat after:\n");
    for (int i = 0; i < 256; i++) {
        if (mem[i].total != 0 || mem[i].e != 0)
            printf("  slot %d: total=0x%02x e=0x%02x -> 0x%04x\n", i, mem[i].total, mem[i].e, mem[i].total | (mem[i].e<<8));
    }
    return 0;
}