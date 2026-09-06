/*
 * j1939_signal_demo.c — interpretacao numerica isolada
 *
 *   cc -O2 -std=c11 -Wall -I. \
 *      j1939_signal_demo.c j1939_signal.c j1939_catalog.c j1939.c can.c -o j1939_signal_demo
 *
 * Sem micro, sem TP, sem can_bus.
 */

#include "j1939_signal.h"
#include "j1939_catalog.h"
#include "j1939.h"

#include <stdio.h>
#include <stdint.h>
#include <string.h>

static int g_fail = 0;
#define CHECK(c, m) do { if (!(c)) { fprintf(stderr, "FAIL: %s\n", m); g_fail++; } } while (0)

int main(void)
{
    printf("=== j1939 signal interpretation ===\n");

    const J1939Signal *s110 = j1939_catalog_find(65262, 110);
    const J1939Signal *s190 = j1939_catalog_find(61444, 190);
    const J1939Signal *s84  = j1939_catalog_find(65265, 84);
    CHECK(s110 && s190 && s84, "catalog");

    /* --- SPN 110: unsigned 16, scale 1/32, offset -40 --- */
    {
        uint8_t payload[8] = { 0x80, 0x07, 0, 0, 0, 0, 0, 0 };
        J1939Interp it = {
            .sig = s110, .is_signed = 0, .little_endian = 1,
            .na_value = 0xFFFF, .na_bits = 16, .err_value = 0, .err_bits = 0
        };
        int64_t raw = 0; int st = 0;
        CHECK(j1939_signal_extract(payload, 8, &it, &raw, &st) == 0, "110 extract");
        CHECK(st == J1939_SIG_OK && raw == 1920, "110 raw");
        int32_t phys = 0;
        CHECK(j1939_signal_physical(raw, &it, &phys) == 0 && phys == 20, "110 20C");
    }

    /* --- offset negativo ja coberto; NA = 0xFFFF --- */
    {
        uint8_t payload[8] = { 0xFF, 0xFF, 0, 0, 0, 0, 0, 0 };
        J1939Interp it = {
            .sig = s110, .is_signed = 0, .little_endian = 1,
            .na_value = 0xFFFF, .na_bits = 16, .err_bits = 0
        };
        int64_t raw = 0; int st = 0;
        CHECK(j1939_signal_extract(payload, 8, &it, &raw, &st) == 0, "110 NA extract");
        CHECK(st == J1939_SIG_NA, "110 NA status");
    }

    /* --- SPN 190: unsigned, 0.125 rpm --- */
    {
        uint8_t payload[8] = { 0, 0, 0, 0x40, 0x1F, 0, 0, 0 }; /* 8000 */
        J1939Interp it = {
            .sig = s190, .is_signed = 0, .little_endian = 1, .na_bits = 0
        };
        int64_t raw = 0; int st = 0;
        CHECK(j1939_signal_extract(payload, 8, &it, &raw, &st) == 0, "190 extract");
        CHECK(raw == 8000, "190 raw");
        int32_t phys = 0;
        CHECK(j1939_signal_physical(raw, &it, &phys) == 0 && phys == 1000, "1000 rpm");
    }

    /* --- SPN 84 --- */
    {
        uint8_t payload[8] = { 0, 0x00, 0x0A, 0, 0, 0, 0, 0 }; /* 2560 */
        J1939Interp it = {
            .sig = s84, .is_signed = 0, .little_endian = 1, .na_bits = 0
        };
        int64_t raw = 0; int st = 0;
        CHECK(j1939_signal_extract(payload, 8, &it, &raw, &st) == 0, "84 extract");
        CHECK(raw == 2560, "84 raw");
        int32_t phys = 0;
        CHECK(j1939_signal_physical(raw, &it, &phys) == 0 && phys == 10, "10 km/h");
    }

    /* --- signed 16-bit: valor -100, scale 1, offset 0 --- */
    {
        static const J1939Signal S_SIGNED = {
            .pgn = 0, .spn = 9999, .start_byte = 0, .length = 2,
            .offset = 0, .scale_num = 1, .scale_den = 1,
            .name = "TEST_SIGNED", .unit = "x"
        };
        /* -100 as int16 LE = 0xFF9C */
        uint8_t payload[2] = { 0x9C, 0xFF };
        J1939Interp it = {
            .sig = &S_SIGNED, .is_signed = 1, .little_endian = 1, .na_bits = 0
        };
        int64_t raw = 0; int st = 0;
        CHECK(j1939_signal_extract(payload, 2, &it, &raw, &st) == 0, "signed extract");
        CHECK(raw == -100, "signed -100");
        int32_t phys = 0;
        CHECK(j1939_signal_physical(raw, &it, &phys) == 0 && phys == -100, "phys -100");
    }

    /* --- endianess: BE vs LE --- */
    {
        static const J1939Signal S_BE = {
            .pgn = 0, .spn = 0, .start_byte = 0, .length = 2,
            .offset = 0, .scale_num = 1, .scale_den = 1,
            .name = "BE", .unit = ""
        };
        uint8_t payload[2] = { 0x12, 0x34 }; /* BE = 0x1234, LE = 0x3412 */
        J1939Interp be = { .sig = &S_BE, .is_signed = 0, .little_endian = 0, .na_bits = 0 };
        J1939Interp le = { .sig = &S_BE, .is_signed = 0, .little_endian = 1, .na_bits = 0 };
        int64_t rb = 0, rl = 0; int st = 0;
        CHECK(j1939_signal_extract(payload, 2, &be, &rb, &st) == 0 && rb == 0x1234, "BE");
        CHECK(j1939_signal_extract(payload, 2, &le, &rl, &st) == 0 && rl == 0x3412, "LE");
    }

    /* --- campo fora do payload --- */
    {
        uint8_t payload[1] = { 0 };
        J1939Interp it = { .sig = s110, .little_endian = 1, .na_bits = 0 };
        int64_t raw = 0; int st = 0;
        CHECK(j1939_signal_extract(payload, 1, &it, &raw, &st) == J1939_SIG_ERR, "oob");
    }

    if (g_fail) {
        printf("[X] %d falha(s)\n", g_fail);
        return 1;
    }
    printf("[OK] signal interpretation resid 0\n");
    printf("catalog = metadados; signal = extracao/conversao.\n");
    return 0;
}
