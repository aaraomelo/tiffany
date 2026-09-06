/*
 * j1939_catalog.c — tabela de sinais J1939
 *
 * Apenas dados + lookup. Escala usa j1939_scale_* do decoder congelado.
 */

#include "j1939_catalog.h"

/*
 * Layouts tipicos SAE J1939 (bytes 0-based no payload):
 *   SPN 110  PGN 65262  bytes 0-1   1/32 C/bit  offset -40
 *   SPN 190  PGN 61444  bytes 3-4   0.125 rpm/bit
 *   SPN 84   PGN 65265  bytes 1-2   1/256 km/h/bit
 */

static const J1939Signal CATALOG[] = {
    {
        .pgn = 65262,
        .spn = 110,
        .start_byte = 0,
        .length = 2,
        .offset = -40,
        .scale_num = 1,
        .scale_den = 32,
        .name = "Engine Coolant Temperature",
        .unit = "deg C"
    },
    {
        .pgn = 61444,
        .spn = 190,
        .start_byte = 3,
        .length = 2,
        .offset = 0,
        .scale_num = 1,
        .scale_den = 8,   /* 0.125 = 1/8 */
        .name = "Engine Speed",
        .unit = "rpm"
    },
    {
        .pgn = 65265,
        .spn = 84,
        .start_byte = 1,
        .length = 2,
        .offset = 0,
        .scale_num = 1,
        .scale_den = 256,
        .name = "Wheel-Based Vehicle Speed",
        .unit = "km/h"
    },
};

static const size_t CATALOG_N = sizeof(CATALOG) / sizeof(CATALOG[0]);

const J1939Signal *j1939_catalog_find(uint32_t pgn, uint32_t spn)
{
    for (size_t i = 0; i < CATALOG_N; i++)
        if (CATALOG[i].pgn == pgn && CATALOG[i].spn == spn)
            return &CATALOG[i];
    return NULL;
}

const J1939Signal *j1939_catalog_find_spn(uint32_t spn)
{
    for (size_t i = 0; i < CATALOG_N; i++)
        if (CATALOG[i].spn == spn)
            return &CATALOG[i];
    return NULL;
}

size_t j1939_catalog_count(void)
{
    return CATALOG_N;
}

const J1939Signal *j1939_catalog_at(size_t index)
{
    if (index >= CATALOG_N) return NULL;
    return &CATALOG[index];
}
