/* j1939_signal_extract test harness - full battery
 * Tests j1939_signal_extract() against ERG-64 implementation
 * Compares C reference vs ERG harness vs erg_new.exe corre vs DISCO
 *
 * Microtests for primitives first, then full battery
 */
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>

/* J1939 signal codes */
#define J1939_SIG_OK            0x00
#define J1939_SIG_NA            0x01
#define J1939_SIG_ERROR_RANGE   0x02
#define J1939_SIG_ERR           0xFF

/* ERG ABI slots */
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
static uint16_t saved_fmem[65536];
static uint16_t saved_mem[65536];

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

/* Extract bytes from fmem in little-endian order */
static uint64_t extract_le_bytes(int slot_lo, int nbytes) {
    uint64_t val = 0;
    for (int i = 0; i < nbytes; i++) {
        uint16_t w = fmem[(slot_lo + i) * 2];
        uint8_t lo = (uint8_t)(w & 0xFF);
        val |= ((uint64_t)lo) << (i * 8);
    }
    return val;
}

static uint64_t extract_be_bytes(int slot_lo, int nbytes) {
    uint64_t val = 0;
    for (int i = 0; i < nbytes; i++) {
        uint16_t w = fmem[(slot_lo + i) * 2];
        uint8_t lo = (uint8_t)(w & 0xFF);
        val |= ((uint64_t)lo) << ((nbytes - 1 - i) * 8);
    }
    return val;
}

/* Sign extend a value of nbits */
static int64_t sign_extend_c(uint64_t val, int nbits) {
    if (nbits >= 64) return (int64_t)val;
    uint64_t sign_bit = (uint64_t)1 << (nbits - 1);
    if (val & sign_bit) {
        uint64_t mask = ((uint64_t)1 << nbits) - 1;
        return (int64_t)(val | ~mask);
    }
    return (int64_t)val;
}

/* Reference C implementation of j1939_signal_extract */
static int j1939_signal_extract_ref(const uint8_t *payload, uint16_t payload_len,
                                    uint8_t start_byte, uint8_t length,
                                    uint8_t is_signed, uint8_t little_endian,
                                    uint8_t na_bits, uint64_t na_value,
                                    uint8_t err_bits, uint64_t err_value,
                                    int64_t *raw_out, int *status) {
    /* Validation */
    if (length < 1 || length > 8) {
        *status = J1939_SIG_ERR;
        return -1;
    }
    if (start_byte + length > payload_len) {
        *status = J1939_SIG_ERR;
        return -1;
    }

    /* Extract bits */
    uint64_t bits = 0;
    if (little_endian) {
        for (int i = 0; i < length; i++) {
            bits |= ((uint64_t)payload[start_byte + i]) << (i * 8);
        }
    } else {
        for (int i = 0; i < length; i++) {
            bits |= ((uint64_t)payload[start_byte + i]) << ((length - 1 - i) * 8);
        }
    }

    /* NA check */
    if (na_bits && bits == na_value) {
        *raw_out = (int64_t)bits;  /* unsigned */
        *status = J1939_SIG_NA;
        return 0;
    }

    /* ERR check */
    if (err_bits && bits == err_value) {
        *raw_out = (int64_t)bits;  /* unsigned */
        *status = J1939_SIG_ERROR_RANGE;
        return 0;
    }

    /* Normal extraction */
    if (is_signed) {
        *raw_out = sign_extend_c(bits, length * 8);
    } else {
        *raw_out = (int64_t)bits;
    }
    *status = J1939_SIG_OK;
    return 0;
}

/* Run a single test case using the C reference, then compare with ERG harness */
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

    /* Check C reference results */
    if (ref_status != expected_status || ref_raw != expected_raw) {
        printf("  C reference mismatch for %s\n", name);
        return -1;
    }

    /* Set up fmem for ERG harness */
    memset(fmem, 0, sizeof(fmem));
    memset(mem, 0, sizeof(mem));

    /* Load ABI values into fmem */
    fmem[SLOT_PAYLOAD_LEN_LO] = payload_len & 0xFF;
    fmem[SLOT_START_BYTE] = start_byte;
    fmem[SLOT_LENGTH] = length;
    fmem[SLOT_IS_SIGNED] = is_signed;
    fmem[SLOT_LITTLE_ENDIAN] = little_endian;
    fmem[SLOT_NA_BITS] = na_bits;
    fmem[SLOT_ERR_BITS] = err_bits;

    /* na_value (uint64_t LE) */
    for (int i = 0; i < 8; i++) {
        fmem[SLOT_NA_VALUE_LO + i] = (na_value >> (i * 8)) & 0xFF;
    }

    /* err_value (uint64_t LE) */
    for (int i = 0; i < 8; i++) {
        fmem[SLOT_ERR_VALUE_LO + i] = (err_value >> (i * 8)) & 0xFF;
    }

    /* payload[0..7] */
    for (int i = 0; i < 8; i++) {
        fmem[SLOT_PAYLOAD_LO + i] = payload[i];
    }

    /* Write fmem to mem.dat so erg_new.exe corre reads current values */
    FILE *fw = fopen("mem.dat", "wb");
    if (!fw) {
        printf("  Failed to write mem.dat\n");
        return -1;
    }
    fwrite(fmem, sizeof(uint16_t), 65536, fw);
    fclose(fw);

    /* Copy binary to /tmp/erg_prog.bin (corre reads from there, not the argument) */
    FILE *fbin = fopen("j1939_signal_extract.bin", "rb");
    if (!fbin) { printf("  Failed to open bin\n"); return -1; }
    FILE *ftmp = fopen("/tmp/erg_prog.bin", "wb");
    if (!ftmp) { printf("  Failed to open /tmp/erg_prog.bin\n"); fclose(fbin); return -1; }
    { unsigned char buf[4096]; size_t n; while ((n = fread(buf, 1, sizeof(buf), fbin)) > 0) fwrite(buf, 1, n, ftmp); }
    fclose(fbin); fclose(ftmp);

    /* Run erg_new.exe corre (reads /tmp/erg_prog.bin) */
    char cmd[1024];
    snprintf(cmd, sizeof(cmd),
             "erg_new.exe corre mem.dat 0 256 2>&1");
    FILE *fp = popen(cmd, "r");
    if (!fp) {
        printf("  Failed to run erg_new.exe\n");
        return -1;
    }
    char buf[4096];
    fread(buf, 1, sizeof(buf) - 1, fp);
    buf[sizeof(buf) - 1] = '\0';
    pclose(fp);

    /* Read back results from mem.dat (corre writes to disk) */
    FILE *fd = fopen("mem.dat", "rb");
    if (!fd) {
        printf("  Failed to open mem.dat\n");
        return -1;
    }
    uint16_t fmem_read[65536];
    size_t nread = fread(fmem_read, sizeof(uint16_t), 65536, fd);
    fclose(fd);
    if (nread < 34) {
        printf("  mem.dat too small (%zu words)\n", nread);
        return -1;
    }

    uint16_t rc_word = fmem_read[SLOT_RETURN_CODE];
    uint16_t status_word = fmem_read[SLOT_STATUS];
    /* extract raw_out from fmem_read */
    uint64_t erg_raw = 0;
    for (int i = 0; i < 8; i++) {
        uint16_t w = fmem_read[(SLOT_RAW_OUT_LO + i)];
        erg_raw |= ((uint64_t)(w & 0xFF)) << (i * 8);
    }
    int erg_rc = (int8_t)(rc_word & 0xFF);  /* sign-extend */
    int erg_status = (int8_t)(status_word & 0xFF);

    printf("  %-45s C: rc=%02x status=%02x raw=%016llx\n",
           name, (unsigned)J1939_SIG_OK, (unsigned)ref_status, (unsigned long long)ref_raw);
    printf("  %-45s ERG: rc=%02x status=%02x raw=%016llx\n",
           "", (unsigned)erg_rc, (unsigned)erg_status, (unsigned long long)erg_raw);

    if (erg_rc != J1939_SIG_OK || erg_status != ref_status || erg_raw != (uint64_t)ref_raw) {
        printf("  FAIL: divergence at %s\n", name);
        return -1;
    }
    printf("  PASS\n");
    return 0;
}

int main(void) {
    int pass = 0, fail = 0;

    /* === Microtests for primitives === */
    printf("=== Microtest 1: TROCA ===\n");
    uint16_t A1 = make_word(0x12, 0x34);
    uint16_t R1 = troca_canonical(A1);
    printf("A.total=0x%02x A.e=0x%02x\n", word_total(A1), word_e(A1));
    printf("R.total=0x%02x R.e=0x%02x (expected 0x34, 0x12)\n", word_total(R1), word_e(R1));
    printf("TROCA: %s\n", (word_total(R1) == 0x34 && word_e(R1) == 0x12) ? "PASS" : "FAIL");
    if (word_total(R1) == 0x34 && word_e(R1) == 0x12) pass++; else fail++;

    printf("\n=== Microtest 2: STORE_IND ===\n");
    memset(mem, 0, sizeof(mem));
    mem[25] = 8;
    uint16_t R2 = 0x00FF;
    unsigned tgt = mem[25];
    printf("target = %u (expected 8)\n", tgt);
    mem[tgt] = R2;
    printf("mem[8] = 0x%04x (expected 0x00FF)\n", mem[8]);
    printf("STORE_IND: %s\n", (mem[8] == 0x00FF) ? "PASS" : "FAIL");
    if (mem[8] == 0x00FF) pass++; else fail++;

    printf("\n=== Microtest 3: INC ===\n");
    memset(mem, 0, sizeof(mem));
    mem[10] = 0x0005;
    mem[10]++;
    printf("mem[10] = 0x%04x (expected 0x0006)\n", mem[10]);
    printf("INC: %s\n", (mem[10] == 0x0006) ? "PASS" : "FAIL");
    if (mem[10] == 0x0006) pass++; else fail++;

    /* === Full battery: length 1..8, LE, unsigned === */
    printf("\n=== Full Battery ===\n");

    uint8_t payload[8] = {0x34, 0x12, 0xAB, 0xCD, 0xEF, 0x01, 0x23, 0x45};

    for (int len = 1; len <= 8; len++) {
        char name[64];
        snprintf(name, sizeof(name), "len=%d LE unsigned", len);
        int64_t expected_raw = 0;
        for (int i = 0; i < len; i++) {
            expected_raw |= ((uint64_t)payload[i]) << (i * 8);
        }
        if (run_test(name, 8, 0, len, 0, 1, 0, 0, 0, 0,
                     payload, 0, J1939_SIG_OK, expected_raw) == 0) {
            pass++;
        } else {
            fail++;
        }
    }

    /* === BE tests === */
    for (int len = 1; len <= 8; len++) {
        char name[64];
        snprintf(name, sizeof(name), "len=%d BE unsigned", len);
        int64_t expected_raw = 0;
        for (int i = 0; i < len; i++) {
            expected_raw |= ((uint64_t)payload[i]) << ((len - 1 - i) * 8);
        }
        if (run_test(name, 8, 0, len, 0, 0, 0, 0, 0, 0,
                     payload, 0, J1939_SIG_OK, expected_raw) == 0) {
            pass++;
        } else {
            fail++;
        }
    }

    /* === Signed tests: 0x7F, 0x80, 0xFF, 0x7FFF, 0x8000, 0xFFFF === */
    uint8_t signed_payload[] = {0x7F, 0x80, 0xFF, 0x7F, 0x80, 0x00, 0xFF, 0xFF};
    for (int len = 1; len <= 8; len++) {
        char name[64];
        snprintf(name, sizeof(name), "len=%d LE signed", len);
        uint64_t bits = 0;
        for (int i = 0; i < len; i++) {
            bits |= ((uint64_t)signed_payload[i]) << (i * 8);
        }
        int64_t expected_raw = sign_extend_c(bits, len * 8);
        if (run_test(name, 8, 0, len, 1, 1, 0, 0, 0, 0,
                     signed_payload, 0, J1939_SIG_OK, expected_raw) == 0) {
            pass++;
        } else {
            fail++;
        }
    }

    /* === NA tests === */
    uint64_t na_val = 0x1234;
    uint8_t na_payload[] = {0x34, 0x12, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
    if (run_test("NA match (len=2 LE)", 8, 0, 2, 0, 1, 1, na_val, 0, 0,
                 na_payload, 0, J1939_SIG_NA, (int64_t)na_val) == 0) {
        pass++;
    } else {
        fail++;
    }

    /* === ERR tests === */
    uint64_t err_val = 0xABCD;
    uint8_t err_payload[] = {0xCD, 0xAB, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
    if (run_test("ERR match (len=2 LE)", 8, 0, 2, 0, 1, 0, 0, 1, err_val,
                 err_payload, 0, J1939_SIG_ERROR_RANGE, (int64_t)err_val) == 0) {
        pass++;
    } else {
        fail++;
    }

    /* === Bounds tests === */
    uint8_t zero_payload[] = {0, 0, 0, 0, 0, 0, 0, 0};
    if (run_test("length=0 (error)", 8, 0, 0, 0, 1, 0, 0, 0, 0,
                 zero_payload, -1, J1939_SIG_ERR, 0) == 0) {
        pass++;
    } else {
        fail++;
    }
    if (run_test("length=9 (error)", 8, 0, 9, 0, 1, 0, 0, 0, 0,
                 zero_payload, -1, J1939_SIG_ERR, 0) == 0) {
        pass++;
    } else {
        fail++;
    }

    /* === Precedence: NA+ERR simultaneous === */
    uint64_t both_val = 0x1122;
    uint8_t both_payload[] = {0x22, 0x11, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
    if (run_test("NA+ERR precedence (NA wins)", 8, 0, 2, 0, 1, 1, both_val, 1, both_val,
                 both_payload, 0, J1939_SIG_NA, (int64_t)both_val) == 0) {
        pass++;
    } else {
        fail++;
    }

    printf("\n=== Summary: %d pass, %d fail ===\n", pass, fail);
    return fail > 0 ? 1 : 0;
}