#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>

#define SLOT_PAYLOAD_LEN_LO     0
#define SLOT_PAYLOAD_LEN_HI     1
#define SLOT_START_BYTE         2
#define SLOT_LENGTH             3
#define SLOT_IS_SIGNED          4
#define SLOT_LITTLE_ENDIAN      5
#define SLOT_NA_BITS            6
#define SLOT_ERR_BITS           7
#define SLOT_NA_VALUE_LO        8
#define SLOT_ERR_VALUE_LO       16
#define SLOT_RAW_OUT_LO         24
#define SLOT_RETURN_CODE        32
#define SLOT_STATUS             33
#define SLOT_PAYLOAD_LO         40

static uint16_t fmem[65536];
static uint16_t mem[65536];

typedef struct { uint8_t total; uint8_t e; } Word;
static uint16_t make_word(uint8_t total, uint8_t e) { return (uint16_t)total | ((uint16_t)e << 8); }
static uint8_t word_total(uint16_t w) { return (uint8_t)(w & 0xFF); }
static uint8_t word_e(uint16_t w) { return (uint8_t)((w >> 8) & 0xFF); }

int main(void) {
    memset(fmem, 0, sizeof(fmem));
    memset(mem, 0, sizeof(mem));
    fmem[SLOT_PAYLOAD_LEN_LO] = 8;
    fmem[SLOT_START_BYTE] = 0;
    fmem[SLOT_LENGTH] = 1;
    fmem[SLOT_IS_SIGNED] = 0;
    fmem[SLOT_LITTLE_ENDIAN] = 1;
    uint8_t payload[8] = {0x34, 0x12, 0xAB, 0xCD, 0xEF, 0x01, 0x23, 0x45};
    for (int i = 0; i < 8; i++) fmem[SLOT_PAYLOAD_LO + i] = payload[i];
    FILE *fw = fopen("mem.dat", "wb");
    fwrite(fmem, sizeof(uint16_t), 65536, fw);
    fclose(fw);
    printf("slot 40 = 0x%04x\n", fmem[40] | (fmem[41] << 8));
    printf("slot 0  = 0x%04x\n", fmem[0] | (fmem[1] << 8));
    return 0;
}