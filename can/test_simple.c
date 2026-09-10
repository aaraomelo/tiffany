
#include <stdio.h>
#include <stdint.h>
#include <string.h>

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

int main(void) {
    memset(fmem, 0, sizeof(fmem));
    
    /* Set up a simple test: payload[0]=0x34, len=1, LE, unsigned, no NA, no ERR */
    fmem[SLOT_PAYLOAD_LEN_LO] = 8;
    fmem[SLOT_START_BYTE] = 0;
    fmem[SLOT_LENGTH] = 1;
    fmem[SLOT_IS_SIGNED] = 0;
    fmem[SLOT_LITTLE_ENDIAN] = 1;
    fmem[SLOT_NA_BITS] = 0;
    fmem[SLOT_ERR_BITS] = 0;
    fmem[SLOT_PAYLOAD_LO] = 0x34;  /* payload[0] = 0x34 */
    
    FILE *fw = fopen("mem.dat", "wb");
    fwrite(fmem, sizeof(uint16_t), 65536, fw);
    fclose(fw);
    
    /* Copy bin to /tmp/erg_prog.bin */
    FILE *fbin = fopen("j1939_signal_extract.bin", "rb");
    FILE *ftmp = fopen("/tmp/erg_prog.bin", "wb");
    { unsigned char buf[4096]; size_t n; while ((n = fread(buf, 1, sizeof(buf), fbin)) > 0) fwrite(buf, 1, n, ftmp); }
    fclose(fbin); fclose(ftmp);
    
    /* Run erg_new.exe corre */
    system("/c/Users/jssim/aarao/tiffany/banco/erg_new.exe corre mem.dat 0 256");
    
    /* Read back results */
    FILE *fd = fopen("mem.dat", "rb");
    uint16_t fmem_read[65536];
    fread(fmem_read, sizeof(uint16_t), 65536, fd);
    fclose(fd);
    
    uint64_t erg_raw = 0;
    for (int i = 0; i < 8; i++) {
        uint16_t w = fmem_read[(SLOT_RAW_OUT_LO + i)];
        erg_raw |= ((uint64_t)(w & 0xFF)) << (i * 8);
    }
    int erg_rc = (int8_t)(fmem_read[SLOT_RETURN_CODE] & 0xFF);
    int erg_status = (int8_t)(fmem_read[SLOT_STATUS] & 0xFF);
    
    printf("ERG: rc=%02x status=%02x raw=%016llx\n", (unsigned)erg_rc, (unsigned)erg_status, (unsigned long long)erg_raw);
    printf("Expected: rc=00 status=00 raw=0000000000000034\n");
    
    return 0;
}
