// debug_harness.c — simulate harness mem.dat preparation and verify erg_new.exe reads it
#include <stdio.h>
#include <stdint.h>
#include <string.h>

static uint16_t fmem[65536];

int main(void) {
    uint8_t payload[8] = {0x34, 0x12, 0xAB, 0xCD, 0xEF, 0x01, 0x23, 0x45};
    int SLOT_PAYLOAD_LO = 40;

    memset(fmem, 0, sizeof(fmem));
    for (int i = 0; i < 8; i++) {
        fmem[SLOT_PAYLOAD_LO + i] = payload[i];
    }

    FILE *fw = fopen("mem.dat", "wb");
    fwrite(fmem, sizeof(uint16_t), 65536, fw);
    fclose(fw);

    // Verify what's in mem.dat at slot 40
    uint8_t raw[2];
    fseek(fw = fopen("mem.dat", "rb"), SLOT_PAYLOAD_LO * 2, SEEK_SET);
    fread(raw, 1, 2, fw);
    fclose(fw);
    printf("slot %d: total=0x%02x e=0x%02x -> 0x%04x (expected 0x%02x)\n",
           SLOT_PAYLOAD_LO, raw[0], raw[1], raw[0] | (raw[1]<<8), payload[0]);
    return 0;
}