/* j1939_signal_extract expanded validation harness
 * Cobertura sistematica do contrato inteiro:
 *   Dimensao 1 - Endianness: len 1..8 LE + BE
 *   Dimensao 2 - Signedness: unsigned, signed pos, signed sinal, len=8
 *   Dimensao 3 - NA: disabled, match, no-match, NA+ERR precedence
 *   Dimensao 4 - ERROR_RANGE: disabled, match, no-match, NA+ERR
 *   Dimensao 5 - Bounds: start=0, start=7 len=1, start+len=8, start+len>8, len=0, len=9
 *   Dimensao 6 - Edge values: 0x00, 0xFF, 0x12 34, 0xAA 55
 *
 * Compara C reference vs ERG REAL (erg_new.exe corre)
 * raw_out, return_code, status
 *
 * Compilar: gcc -I../lib -I. -o expanded_test expanded.c ../lib/pread_posix.c ../tools/libc.c -lm
 * Executar: ./expanded_test  (da pasta can/)
 */
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>

#define J1939_SIG_OK            0x00
#define J1939_SIG_NA            0x01
#define J1939_SIG_ERROR_RANGE   0x02
#define J1939_SIG_ERR           0xFF

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

static int64_t sign_extend_c(uint64_t val, int nbits) {
    if (nbits >= 64) return (int64_t)val;
    uint64_t sign_bit = (uint64_t)1 << (nbits - 1);
    if (val & sign_bit) {
        uint64_t mask = ((uint64_t)1 << nbits) - 1;
        return (int64_t)(val | ~mask);
    }
    return (int64_t)val;
}

static int j1939_signal_extract_ref(const uint8_t *payload, uint16_t payload_len,
                                    uint8_t start_byte, uint8_t length,
                                    uint8_t is_signed, uint8_t little_endian,
                                    uint8_t na_bits, uint64_t na_value,
                                    uint8_t err_bits, uint64_t err_value,
                                    int64_t *raw_out, int *status) {
    if (length < 1 || length > 8) { *status = J1939_SIG_ERR; return -1; }
    if (start_byte + length > payload_len) { *status = J1939_SIG_ERR; return -1; }
    uint64_t bits = 0;
    if (little_endian) {
        for (int i = 0; i < length; i++)
            bits |= ((uint64_t)payload[start_byte + i]) << (i * 8);
    } else {
        for (int i = 0; i < length; i++)
            bits |= ((uint64_t)payload[start_byte + i]) << ((length - 1 - i) * 8);
    }
    if (na_bits && bits == na_value) { *raw_out = (int64_t)bits; *status = J1939_SIG_NA; return 0; }
    if (err_bits && bits == err_value) { *raw_out = (int64_t)bits; *status = J1939_SIG_ERROR_RANGE; return 0; }
    if (is_signed) *raw_out = sign_extend_c(bits, length * 8);
    else *raw_out = (int64_t)bits;
    *status = J1939_SIG_OK;
    return 0;
}

static int write_mem_dat(void) {
    FILE *fw = fopen("mem.dat", "wb");
    if (!fw) return -1;
    fwrite(fmem, sizeof(uint16_t), 65536, fw);
    fclose(fw);
    return 0;
}

static int run_test(const char *name,
                    uint8_t payload_len, uint8_t start_byte, uint8_t length,
                    uint8_t is_signed, uint8_t little_endian,
                    uint8_t na_bits, uint64_t na_value,
                    uint8_t err_bits, uint64_t err_value,
                    const uint8_t payload[8],
                    int expected_rc, int expected_status, int64_t expected_raw) {
    /* C reference */
    int64_t ref_raw;
    int ref_status;
    j1939_signal_extract_ref(payload, payload_len, start_byte, length,
                              is_signed, little_endian, na_bits, na_value,
                              err_bits, err_value, &ref_raw, &ref_status);

    /* Set up fmem */
    memset(fmem, 0, sizeof(fmem));
    memset(mem, 0, sizeof(mem));
    fmem[SLOT_PAYLOAD_LEN_LO] = payload_len & 0xFF;
    fmem[SLOT_START_BYTE] = start_byte;
    fmem[SLOT_LENGTH] = length;
    fmem[SLOT_IS_SIGNED] = is_signed;
    fmem[SLOT_LITTLE_ENDIAN] = little_endian;
    fmem[SLOT_NA_BITS] = na_bits;
    fmem[SLOT_ERR_BITS] = err_bits;
    for (int i = 0; i < 8; i++)
        fmem[SLOT_NA_VALUE_LO + i] = (na_value >> (i * 8)) & 0xFF;
    for (int i = 0; i < 8; i++)
        fmem[SLOT_ERR_VALUE_LO + i] = (err_value >> (i * 8)) & 0xFF;
    for (int i = 0; i < 8; i++)
        fmem[SLOT_PAYLOAD_LO + i] = payload[i];

    if (write_mem_dat() < 0) { printf("  FAIL %s: cannot write mem.dat\n", name); return -1; }

    /* Copy bin to /tmp/erg_prog.bin - use absolute path */
    FILE *fbin = fopen("j1939_signal_extract.bin", "rb");
    if (!fbin) { printf("  FAIL %s: cannot open bin\n", name); return -1; }
    FILE *ftmp = fopen("/tmp/erg_prog.bin", "wb");
    if (!ftmp) { fclose(fbin); printf("  FAIL %s: cannot open /tmp/erg_prog.bin\n", name); return -1; }
    { unsigned char buf[4096]; size_t n; while ((n = fread(buf, 1, sizeof(buf), fbin)) > 0) fwrite(buf, 1, n, ftmp); }
    fclose(fbin); fclose(ftmp);

    /* Run erg_new.exe corre - use absolute path */
    char cmd[1024];
    snprintf(cmd, sizeof(cmd), "/c/Users/jssim/aarao/tiffany/banco/erg_new.exe corre mem.dat 0 256 2>&1");
    FILE *fp = popen(cmd, "r");
    if (!fp) { printf("  FAIL %s: cannot run erg_new.exe\n", name); return -1; }
    char buf[4096];
    fread(buf, 1, sizeof(buf) - 1, fp);
    buf[sizeof(buf) - 1] = '\0';
    pclose(fp);

    /* Read back results */
    FILE *fd = fopen("mem.dat", "rb");
    if (!fd) { printf("  FAIL %s: cannot open mem.dat\n", name); return -1; }
    uint16_t fmem_read[65536];
    size_t nread = fread(fmem_read, sizeof(uint16_t), 65536, fd);
    fclose(fd);
    if (nread < 34) { printf("  FAIL %s: mem.dat too small (%zu)\n", name, nread); return -1; }

    uint64_t erg_raw = 0;
    for (int i = 0; i < 8; i++) {
        uint16_t w = fmem_read[(SLOT_RAW_OUT_LO + i)];
        erg_raw |= ((uint64_t)(w & 0xFF)) << (i * 8);
    }
    int erg_rc = (int8_t)(fmem_read[SLOT_RETURN_CODE] & 0xFF);
    int erg_status = (int8_t)(fmem_read[SLOT_STATUS] & 0xFF);

    printf("  %-45s C: rc=%02x status=%02x raw=%016llx\n", name, (unsigned)J1939_SIG_OK, (unsigned)ref_status, (unsigned long long)ref_raw);
    printf("  %-45s ERG: rc=%02x status=%02x raw=%016llx\n", "", (unsigned)erg_rc, (unsigned)erg_status, (unsigned long long)erg_raw);

    if (erg_rc != J1939_SIG_OK || erg_status != ref_status || erg_raw != (uint64_t)ref_raw) {
        printf("  FAIL: divergence at %s\n", name);
        return -1;
    }
    printf("  PASS\n");
    return 0;
}

int main(void) {
    int pass = 0, fail = 0;
    int dim1 = 0, dim2 = 0, dim3 = 0, dim4 = 0, dim5 = 0, dim6 = 0;
    int first_fail = -1;
    uint8_t payload[8] = {0x34, 0x12, 0xAB, 0xCD, 0xEF, 0x01, 0x23, 0x45};
    uint8_t zero[8] = {0};
    uint8_t ff[8] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
    uint8_t pattern1[8] = {0x12, 0x34, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
    uint8_t pattern2[8] = {0xAA, 0x55, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

    printf("=== D1 Endianness: len 1..8 LE + BE unsigned ===\n");
    for (int len = 1; len <= 8; len++) {
        int64_t exp = 0;
        for (int i = 0; i < len; i++) exp |= ((uint64_t)payload[i]) << (i * 8);
        dim1++;
        if (run_test("LE unsigned", 8, 0, len, 0, 1, 0, 0, 0, 0, payload, 0, J1939_SIG_OK, exp) == 0) pass++; else { fail++; if (first_fail < 0) first_fail = pass+fail; }
        dim1++;
        if (run_test("BE unsigned", 8, 0, len, 0, 0, 0, 0, 0, 0, payload, 0, J1939_SIG_OK, exp) == 0) pass++; else { fail++; if (first_fail < 0) first_fail = pass+fail; }
    }

    printf("=== D2 Signedness ===\n");
    { uint8_t sp[]={0x7F,0x00,0x00,0x00,0x00,0x00,0x00,0x00}; int len=1; int64_t exp=0x7F; dim2++; if (run_test("LE signed pos len=1",8,0,len,1,1,0,0,0,0,sp,0,J1939_SIG_OK,exp)==0) pass++; else { fail++; if(first_fail<0)first_fail=pass+fail; } }
    { uint8_t sn[]={0x80,0x00,0x00,0x00,0x00,0x00,0x00,0x00}; int len=1; int64_t exp=(int64_t)(int8_t)0x80; dim2++; if (run_test("LE signed neg len=1",8,0,len,1,1,0,0,0,0,sn,0,J1939_SIG_OK,exp)==0) pass++; else { fail++; if(first_fail<0)first_fail=pass+fail; } }
    { uint8_t sf[]={0xFF,0x00,0x00,0x00,0x00,0x00,0x00,0x00}; int len=1; int64_t exp=(int64_t)(int8_t)0xFF; dim2++; if (run_test("LE signed 0xFF len=1",8,0,len,1,1,0,0,0,0,sf,0,J1939_SIG_OK,exp)==0) pass++; else { fail++; if(first_fail<0)first_fail=pass+fail; } }
    { uint8_t sf2[]={0xFF,0xFF,0x00,0x00,0x00,0x00,0x00,0x00}; int len=2; int64_t exp=(int64_t)(int16_t)0xFFFF; dim2++; if (run_test("LE signed 0xFFFF len=2",8,0,len,1,1,0,0,0,0,sf2,0,J1939_SIG_OK,exp)==0) pass++; else { fail++; if(first_fail<0)first_fail=pass+fail; } }
    { uint8_t s8[]={0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF}; int len=8; int64_t exp=(int64_t)0xFFFFFFFFFFFFFFFFULL; dim2++; if (run_test("LE signed 0xFF len=8",8,0,len,1,1,0,0,0,0,s8,0,J1939_SIG_OK,exp)==0) pass++; else { fail++; if(first_fail<0)first_fail=pass+fail; } }

    printf("=== D3 NA ===\n");
    { dim3++; uint8_t p[]={0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00}; int64_t exp=0; if (run_test("NA disabled",8,0,1,0,1,0,0,0,0,p,0,J1939_SIG_OK,exp)==0) pass++; else { fail++; if(first_fail<0)first_fail=pass+fail; } }
    { dim3++; uint8_t p[]={0x34,0x12,0x00,0x00,0x00,0x00,0x00,0x00}; int64_t exp=(int64_t)0x1234; if (run_test("NA match",8,0,2,0,1,1,0x1234,0,0,p,0,J1939_SIG_NA,exp)==0) pass++; else { fail++; if(first_fail<0)first_fail=pass+fail; } }
    { dim3++; uint8_t p[]={0x34,0x12,0x00,0x00,0x00,0x00,0x00,0x00}; int64_t exp=(int64_t)0x1234; if (run_test("NA no-match",8,0,2,0,1,1,0x0000,0,0,p,0,J1939_SIG_OK,exp)==0) pass++; else { fail++; if(first_fail<0)first_fail=pass+fail; } }
    { dim3++; uint8_t p[]={0x22,0x11,0x00,0x00,0x00,0x00,0x00,0x00}; int64_t exp=(int64_t)0x1122; if (run_test("NA+ERR NA wins",8,0,2,0,1,1,0x1122,1,0x1122,p,0,J1939_SIG_NA,exp)==0) pass++; else { fail++; if(first_fail<0)first_fail=pass+fail; } }

    printf("=== D4 ERROR_RANGE ===\n");
    { dim4++; uint8_t p[]={0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00}; int64_t exp=0; if (run_test("ERR disabled",8,0,1,0,1,0,0,0,0,p,0,J1939_SIG_OK,exp)==0) pass++; else { fail++; if(first_fail<0)first_fail=pass+fail; } }
    { dim4++; uint8_t p[]={0xCD,0xAB,0x00,0x00,0x00,0x00,0x00,0x00}; int64_t exp=(int64_t)0xABCD; if (run_test("ERR match",8,0,2,0,1,0,0,1,0xABCD,p,0,J1939_SIG_ERROR_RANGE,exp)==0) pass++; else { fail++; if(first_fail<0)first_fail=pass+fail; } }
    { dim4++; uint8_t p[]={0xCD,0xAB,0x00,0x00,0x00,0x00,0x00,0x00}; int64_t exp=(int64_t)0xABCD; if (run_test("ERR no-match",8,0,2,0,1,0,0,1,0x0000,p,0,J1939_SIG_OK,exp)==0) pass++; else { fail++; if(first_fail<0)first_fail=pass+fail; } }
    { dim4++; uint8_t p[]={0x22,0x11,0x00,0x00,0x00,0x00,0x00,0x00}; int64_t exp=(int64_t)0x1122; if (run_test("NA+ERR both match",8,0,2,0,1,1,0x1122,1,0x1122,p,0,J1939_SIG_NA,exp)==0) pass++; else { fail++; if(first_fail<0)first_fail=pass+fail; } }

    printf("=== D5 Bounds ===\n");
    { dim5++; uint8_t p[]={0x34,0x12,0x00,0x00,0x00,0x00,0x00,0x00}; int64_t exp=(int64_t)0x1234; if (run_test("start=0",8,0,2,0,1,0,0,0,0,p,0,J1939_SIG_OK,exp)==0) pass++; else { fail++; if(first_fail<0)first_fail=pass+fail; } }
    { dim5++; uint8_t p[]={0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x45}; int64_t exp=(int64_t)0x45; if (run_test("start=7 len=1",8,7,1,0,1,0,0,0,0,p,0,J1939_SIG_OK,exp)==0) pass++; else { fail++; if(first_fail<0)first_fail=pass+fail; } }
    { dim5++; uint8_t p[]={0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x45}; int64_t exp=(int64_t)0x45; if (run_test("start+len=8",8,7,1,0,1,0,0,0,0,p,0,J1939_SIG_OK,exp)==0) pass++; else { fail++; if(first_fail<0)first_fail=pass+fail; } }
    { dim5++; uint8_t p[]={0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00}; int64_t exp=0; if (run_test("start+len>8",8,7,2,0,1,0,0,0,0,p,-1,J1939_SIG_ERR,exp)==0) pass++; else { fail++; if(first_fail<0)first_fail=pass+fail; } }
    { dim5++; uint8_t p[]={0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00}; int64_t exp=0; if (run_test("length=0",8,0,0,0,1,0,0,0,0,p,-1,J1939_SIG_ERR,exp)==0) pass++; else { fail++; if(first_fail<0)first_fail=pass+fail; } }
    { dim5++; uint8_t p[]={0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00}; int64_t exp=0; if (run_test("length=9",8,0,9,0,1,0,0,0,0,p,-1,J1939_SIG_ERR,exp)==0) pass++; else { fail++; if(first_fail<0)first_fail=pass+fail; } }

    printf("=== D6 Edge values ===\n");
    { dim6++; int64_t exp=0; if (run_test("all zero",8,0,1,0,1,0,0,0,0,zero,0,J1939_SIG_OK,exp)==0) pass++; else { fail++; if(first_fail<0)first_fail=pass+fail; } }
    { dim6++; int64_t exp=(int64_t)0xFF; if (run_test("all 0xFF len=1",8,0,1,0,1,0,0,0,0,ff,0,J1939_SIG_OK,(int64_t)0xFF)==0) pass++; else { fail++; if(first_fail<0)first_fail=pass+fail; } }
    { dim6++; int64_t exp=(int64_t)0x34; if (run_test("first byte diff",8,0,1,0,1,0,0,0,0,payload,0,J1939_SIG_OK,(int64_t)0x34)==0) pass++; else { fail++; if(first_fail<0)first_fail=pass+fail; } }
    { dim6++; int64_t exp=(int64_t)0x45; if (run_test("last byte diff",8,7,1,0,1,0,0,0,0,payload,0,J1939_SIG_OK,(int64_t)0x45)==0) pass++; else { fail++; if(first_fail<0)first_fail=pass+fail; } }
    { dim6++; int64_t exp=(int64_t)0x3412; if (run_test("pattern 0x12 0x34 LE",8,0,2,0,1,0,0,0,0,pattern1,0,J1939_SIG_OK,(int64_t)0x3412)==0) pass++; else { fail++; if(first_fail<0)first_fail=pass+fail; } }
    { dim6++; int64_t exp=(int64_t)0x55AA; if (run_test("pattern 0xAA 0x55 LE",8,0,2,0,1,0,0,0,0,pattern2,0,J1939_SIG_OK,(int64_t)0x55AA)==0) pass++; else { fail++; if(first_fail<0)first_fail=pass+fail; } }

    int total = dim1 + dim2 + dim3 + dim4 + dim5 + dim6;
    printf("\n=== Resultados ===\n");
    printf("casos testados: %d\n", total);
    printf("PASS: %d\n", pass);
    printf("FAIL: %d\n", fail);
    printf("primeiro divergente: %s\n", first_fail < 0 ? "nenhum" : "sim");
    printf("cobertura por dimensao:\n");
    printf("  D1 Endianness: %d\n", dim1);
    printf("  D2 Signedness: %d\n", dim2);
    printf("  D3 NA: %d\n", dim3);
    printf("  D4 ERROR_RANGE: %d\n", dim4);
    printf("  D5 Bounds: %d\n", dim5);
    printf("  D6 Edge values: %d\n", dim6);

    return fail > 0 ? 1 : 0;
}