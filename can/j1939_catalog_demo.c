/*
 * j1939_catalog_demo.c — catalogo + decoder (sem micro, sem can_bus)
 *
 *   cc -O2 -std=c11 -Wall -I. \
 *      j1939_catalog_demo.c j1939_catalog.c j1939.c can.c -o j1939_catalog_demo
 */

#include "j1939_catalog.h"
#include "j1939.h"
#include "can.h"

#include <stdio.h>
#include <stdint.h>

static int g_fail = 0;
#define CHECK(c, m) do { if (!(c)) { fprintf(stderr, "FAIL: %s\n", m); g_fail++; } } while (0)

static uint32_t make_id(uint8_t pri, uint32_t pgn, uint8_t sa)
{
    return ((uint32_t)(pri & 7) << 26) | ((pgn & 0x3FFFFu) << 8) | sa;
}

int main(void)
{
    printf("=== j1939 catalog ===\n");
    CHECK(j1939_catalog_count() >= 3, "count >= 3");

    const J1939Signal *s110 = j1939_catalog_find(65262, 110);
    const J1939Signal *s190 = j1939_catalog_find(61444, 190);
    const J1939Signal *s84  = j1939_catalog_find(65265, 84);
    CHECK(s110 && s190 && s84, "find three signals");
    CHECK(j1939_catalog_find(1, 1) == NULL, "unknown null");

    /* SPN 110 via catalog + decoder */
    CanFrame f110 = {
        .id = make_id(3, 65262, 0), .dlc = 8, .extended = 1, .rtr = 0,
        .data = { 0x80, 0x07, 0, 0, 0, 0, 0, 0 }
    };
    uint64_t raw = 0;
    int32_t phys = 0;
    CHECK(j1939_read_raw(&f110, s110, &raw) == 0 && raw == 1920, "raw 110");
    CHECK(j1939_scale_i32(raw, s110, &phys) == 0 && phys == 20, "20 C");

    /* SPN 190 */
    CanFrame f190 = {
        .id = make_id(3, 61444, 0), .dlc = 8, .extended = 1, .rtr = 0,
        .data = { 0, 0, 0, 0x40, 0x1F, 0, 0, 0 }
    };
    CHECK(j1939_read_raw(&f190, s190, &raw) == 0 && raw == 8000, "raw 190");
    CHECK(j1939_scale_i32(raw, s190, &phys) == 0 && phys == 1000, "1000 rpm");

    /* SPN 84 */
    CanFrame f84 = {
        .id = make_id(6, 65265, 0), .dlc = 8, .extended = 1, .rtr = 0,
        .data = { 0, 0x00, 0x0A, 0, 0, 0, 0, 0 }
    };
    CHECK(j1939_read_raw(&f84, s84, &raw) == 0 && raw == 2560, "raw 84");
    CHECK(j1939_scale_i32(raw, s84, &phys) == 0 && phys == 10, "10 km/h");

    if (g_fail) {
        printf("[X] %d falha(s)\n", g_fail);
        return 1;
    }
    printf("[OK] catalog + decoder resid 0  (%zu sinais)\n", j1939_catalog_count());
    printf("j1939.c = gramatica; j1939_catalog = significado.\n");
    return 0;
}
