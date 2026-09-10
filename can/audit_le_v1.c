#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>

static uint16_t mem[65536];

int main(void) {
    FILE *f = fopen("can/j1939_le_v1.bin", "rb");
    if (!f) { printf("Cannot open can/j1939_le_v1.bin\n"); return 1; }
    fseek(f, 0, SEEK_END);
    int binlen = ftell(f);
    fseek(f, 0, SEEK_SET);
    uint8_t *bin = malloc(binlen);
    fread(bin, 1, binlen, f);
    fclose(f);

    printf("=== BINARY DUMP ===\n");
    printf("Size: %d bytes\n", binlen);
    printf("Bytes: ");
    for (int i = 0; i < binlen; i++) printf("%02x ", bin[i]);
    printf("\n\n");

    printf("=== DISASSEMBLY ===\n");
    int pc = 0;
    while (pc < binlen) {
        uint8_t op = bin[pc];
        printf("Offset %d (0x%02x): 0x%02x", pc, pc, op);
        switch (op) {
            case 0x00: printf(" (HALT)"); pc++; break;
            case 0x01: printf(" (LOAD) operand=0x%02x (%d)", bin[pc+1], bin[pc+1]); pc += 2; break;
            case 0x02: printf(" (STORE) operand=0x%02x (%d)", bin[pc+1], bin[pc+1]); pc += 2; break;
            case 0x03: printf(" (ADD)"); pc++; break;
            case 0x04: printf(" (SUB)"); pc++; break;
            case 0x05: printf(" (CMP)"); pc++; break;
            case 0x06: printf(" (JZ) rel=0x%02x%02x (%d)", bin[pc+1], bin[pc+2], (int16_t)(bin[pc+1] | (bin[pc+2] << 8))); pc += 3; break;
            case 0x07: printf(" (JNZ) rel=0x%02x%02x (%d)", bin[pc+1], bin[pc+2], (int16_t)(bin[pc+1] | (bin[pc+2] << 8))); pc += 3; break;
            case 0x08: printf(" (JMP) rel=0x%02x%02x (%d)", bin[pc+1], bin[pc+2], (int16_t)(bin[pc+1] | (bin[pc+2] << 8))); pc += 3; break;
            case 0x09: printf(" (TROCA)"); pc++; break;
            case 0x0A: printf(" (INC)"); pc++; break;
            case 0x0B: printf(" (STORE_IND) operand=0x%02x (%d)", bin[pc+1], bin[pc+1]); pc += 2; break;
            default: pc++; break;
        }
        printf("\n");
    }
    printf("\n");

    /* Initialize memory */
    memset(mem, 0, sizeof(mem));

    /* Payload: 12 34 56 78 9A BC DE F0 */
    mem[40] = 0x0012;  /* payload[0] = 0x12 */
    mem[41] = 0x0034;  /* payload[1] = 0x34 */
    mem[42] = 0x0056;  /* payload[2] = 0x56 */
    mem[43] = 0x0078;  /* payload[3] = 0x78 */
    mem[44] = 0x009A;  /* payload[4] = 0x9A */
    mem[45] = 0x00BC;  /* payload[5] = 0xBC */
    mem[46] = 0x00DE;  /* payload[6] = 0xDE */
    mem[47] = 0x00F0;  /* payload[7] = 0xF0 */

    printf("=== INPUT ===\n");
    printf("Payload (slots 40..47):\n");
    for (int i = 0; i < 8; i++) printf("  mem[%d] = 0x%04x\n", 40+i, mem[40+i]);
    printf("\n");

    /* Execute with erg_new.exe */
    int ret = system("cd /c/Users/jssim/aarao/tiffany/can && ../banco/erg_new.exe corre can/j1939_le_v1.bin /c/tmp/erg_le.dat");
    printf("Execute ret=%d\n\n", ret);

    printf("=== OUTPUT ===\n");
    printf("raw_out slots 24..31:\n");
    for (int i = 0; i < 8; i++) {
        printf("  mem[%d] = 0x%04x\n", 24+i, mem[24+i]);
    }
    printf("return_code = mem[32] = 0x%04x\n", mem[32]);
    printf("status = mem[33] = 0x%04x\n", mem[33]);

    /* Verify LE: raw_out bytes 24..31 should be 12 34 56 78 9A BC DE F0 */
    printf("\n=== VERIFICATION ===\n");
    uint8_t expected[8] = {0x12, 0x34, 0x56, 0x78, 0x9A, 0xBC, 0xDE, 0xF0};
    int pass = 1;
    for (int i = 0; i < 8; i++) {
        uint8_t got = (uint8_t)(mem[24+i] & 0xFF);
        if (got != expected[i]) {
            printf("FAIL slot %d: expected 0x%02x, got 0x%02x\n", 24+i, expected[i], got);
            pass = 0;
        }
    }
    if (pass) printf("ALL 8 bytes PASS\n");

    free(bin);
    return pass ? 0 : 1;
}