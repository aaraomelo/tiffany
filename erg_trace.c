#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>

typedef struct {
    uint16_t total;
    uint8_t e;
} Word16;

typedef struct {
    Word16 slots[65536];
    Word16 R;
    Word16 A;
    Word16 B;
    uint16_t pc;
    uint8_t running;
    uint16_t opcode_count;
} ERGState;

static void load_mem(ERGState *s, uint16_t slot, uint16_t val) {
    s->slots[slot].total = val;
    s->slots[slot].e = (val >> 8) & 0xFF;
}

static uint16_t read_mem(ERGState *s, uint16_t slot) {
    return s->slots[slot].total;
}

static void write_mem(ERGState *s, uint16_t slot, uint16_t val) {
    s->slots[slot].total = val;
    s->slots[slot].e = (val >> 8) & 0xFF;
}

static void dump_state(ERGState *s, const char *label) {
    printf("%s:\n", label);
    printf("  A.total=0x%04x A.e=0x%02x\n", s->A.total, s->A.e);
    printf("  B.total=0x%04x B.e=0x%02x\n", s->B.total, s->B.e);
    printf("  R.total=0x%04x R.e=0x%02x\n", s->R.total, s->R.e);
    printf("  PC=0x%04x\n", s->pc);
}

int main(int argc, char **argv) {
    ERGState s;
    memset(&s, 0, sizeof(s));
    s.running = 1;
    s.pc = 0;
    s.opcode_count = 0;

    FILE *f = fopen(argc > 1 ? argv[1] : "mem.dat", "rb");
    if (!f) { perror("mem.dat"); return 1; }
    fread(s.slots, sizeof(Word16), 65536, f);
    fclose(f);

    f = fopen(argc > 2 ? argv[2] : "test.bin", "rb");
    if (!f) { perror("test.bin"); return 1; }
    fseek(f, 0, SEEK_END);
    long bin_size = ftell(f);
    fseek(f, 0, SEEK_SET);
    uint8_t *bin = malloc(bin_size);
    fread(bin, 1, bin_size, f);
    fclose(f);

    while (s.running && s.pc < bin_size) {
        uint8_t op = bin[s.pc];
        uint16_t operand = (bin[s.pc+1] | (bin[s.pc+2] << 8));

        switch (op) {
            case 0x00:
                s.running = 0;
                s.pc += 1;
                break;
            case 0x01:
                s.B = s.A;
                s.A.total = s.slots[operand].total;
                s.A.e = s.slots[operand].e;
                s.pc += 3;
                s.opcode_count++;
                break;
            case 0x02:
                s.R = s.A;
                s.slots[operand].total = s.R.total;
                s.slots[operand].e = s.R.e;
                s.pc += 3;
                s.opcode_count++;
                break;
            case 0x03:
                s.R.total = s.A.total + s.B.total;
                s.R.e = s.A.e + s.B.e;
                s.pc += 1;
                s.opcode_count++;
                break;
            case 0x04:
                s.R.total = s.A.total - s.B.total;
                s.R.e = s.A.e - s.B.e;
                s.pc += 1;
                s.opcode_count++;
                break;
            case 0x05:
                s.pc += 3;
                break;
            case 0x06:
                if (s.A.total == 0) s.pc = operand; else s.pc += 3;
                break;
            case 0x07:
                if (s.A.total != 0) s.pc = operand; else s.pc += 3;
                break;
            case 0x08:
                s.pc = operand;
                break;
            case 0x09:
                { Word16 tmp = s.A; s.A = s.B; s.B = tmp; s.pc += 1; }
                break;
            case 0x0A:
                s.A.total++;
                s.pc += 1;
                break;
            default:
                printf("Unknown opcode 0x%02x at PC=0x%04x\n", op, s.pc);
                s.running = 0;
                break;
        }
    }

    printf("Final state:\n");
    dump_state(&s, "");
    printf("fmem[40]=0x%04x fmem[0]=0x%04x fmem[50]=0x%04x\n",
           read_mem(&s, 40), read_mem(&s, 0), read_mem(&s, 50));
    free(bin);
    return 0;
}