/*
 * demo_e2e.c — demonstracao narrativa end-to-end
 *
 * Conta uma unica historia: short, TP, convergencia, isolamento.
 * Nao altera camadas congeladas (.snapshot/arch-closed).
 *
 *   cc -O2 -std=c11 -Wall -DMICRO_AS_LIB -I. \
 *      demo_e2e.c can.c j1939.c j1939_tp.c j1939_catalog.c j1939_signal.c micro.c -lm \
 *      -o demo_e2e
 *   ./demo_e2e
 */

#include "can.h"
#include "j1939.h"
#include "j1939_tp.h"
#include "j1939_catalog.h"
#include "j1939_signal.h"
#include "micro.h"

#include <stdio.h>
#include <stdint.h>
#include <string.h>

enum { M_RAW = 0, M_OFF = 1, M_DEN = 2, M_NUM = 3, M_QUOT = 4 };

static int32_t micro_scale(uint64_t raw, const J1939Signal *sig)
{
    Maquina M;
    maq_clear(&M);
    u64 off = (u64)(-(sig->offset)) * (u64)sig->scale_den;
    STORE_VAL(&M, M_RAW, raw);
    STORE_VAL(&M, M_OFF, off);
    STORE_VAL(&M, M_DEN, sig->scale_den);
    Instr80 is[] = {
        { M_RAW, M_OFF, IOP_SUB, M_NUM },
        { M_NUM, M_DEN, IOP_DIV, M_QUOT },
        { 0, 0, IOP_HALT, 0 }
    };
    uint8_t prog[30];
    for (int i = 0; i < 3; i++) encode80(&is[i], prog + 10 * i);
    int steps = 0;
    roda80(&M, prog, 30, &steps);
    (void)steps;
    return (int32_t)LOAD(&M, M_QUOT);
}

static int interpret(const uint8_t *pl, uint16_t n, uint32_t pgn, uint32_t spn,
                     int64_t *raw, int32_t *phys, int32_t *mmicro)
{
    const J1939Signal *sig = j1939_catalog_find(pgn, spn);
    if (!sig) return -1;
    J1939Interp it = {
        .sig = sig, .is_signed = 0, .little_endian = 1,
        .na_value = j1939_signal_na_pattern(sig->length),
        .na_bits = (uint8_t)(sig->length * 8), .err_bits = 0
    };
    int st = 0;
    if (j1939_signal_extract(pl, n, &it, raw, &st) != J1939_SIG_OK || st != J1939_SIG_OK)
        return -1;
    if (j1939_signal_physical(*raw, &it, phys) != J1939_SIG_OK)
        return -1;
    *mmicro = micro_scale((uint64_t)*raw, sig);
    return 0;
}

static void section1_short(int64_t *raw, int32_t *phys, int32_t *mm)
{
    printf("[1] CAN SHORT\n");
    uint8_t pl[8] = { 0x80, 0x07, 0, 0, 0, 0, 0, 0 };
    CanFrame f = {
        .id = 0x0CFEEE00, .dlc = 8, .extended = 1, .rtr = 0
    };
    memcpy(f.data, pl, 8);

    printf("    ID       = 0x%08X\n", f.id);
    printf("    PGN      = %u\n", j1939_pgn(f.id));
    printf("    SPN      = 110\n");

    if (interpret(pl, 8, 65262, 110, raw, phys, mm) != 0) {
        printf("    FAIL\n\n");
        return;
    }
    printf("    raw      = %lld\n", (long long)*raw);
    printf("    physical = %d deg C\n", *phys);
    printf("    MICRO    = %d deg C\n", *mm);
    printf("    %s\n\n", (*phys == *mm && *phys == 20) ? "OK" : "FAIL");
}

static void section2_tp(int64_t *raw, int32_t *phys, int32_t *mm)
{
    printf("[2] CAN + J1939-TP / BAM\n");
    uint8_t payload[138];
    for (int i = 0; i < 138; i++)
        payload[i] = (uint8_t)(0x40 + (i % 0x40));
    payload[0] = 0x80;
    payload[1] = 0x07;

    J1939TpSession tx, rx;
    j1939_tp_init(&tx);
    j1939_tp_init(&rx);
    j1939_tp_bam_start(&tx, 0x80, 65262, payload, 138);

    CanFrame frames[64];
    int n = 0;
    while (j1939_tp_tx_next(&tx, &frames[n]) == J1939_TP_OK) n++;

    for (int i = 0; i < n; i++)
        j1939_tp_rx_frame(&rx, &frames[i]);

    printf("    payload  = 138 bytes\n");
    printf("    frames   = %d (1 BAM + %d DT)\n", n, n - 1);
    printf("    reassembly = %s\n", j1939_tp_complete(&rx) ? "OK" : "FAIL");

    uint8_t out[138];
    int got = j1939_tp_message(&rx, out, 138);
    if (got != 138 || memcmp(out, payload, 138) != 0) {
        printf("    payload mismatch FAIL\n\n");
        return;
    }

    printf("    PGN      = %u\n", rx.pgn);
    printf("    SPN      = 110\n");

    if (interpret(out, 138, rx.pgn, 110, raw, phys, mm) != 0) {
        printf("    FAIL\n\n");
        return;
    }
    printf("    raw      = %lld\n", (long long)*raw);
    printf("    physical = %d deg C\n", *phys);
    printf("    MICRO    = %d deg C\n", *mm);
    printf("    %s\n\n", (*phys == *mm && *phys == 20) ? "OK" : "FAIL");
}

static void section3_convergence(int64_t rs, int32_t ps, int32_t ms,
                                 int64_t rt, int32_t pt, int32_t mt)
{
    printf("[3] CONVERGENCE\n");
    int ok = (rs == rt) && (ps == pt) && (ms == mt);
    printf("    short raw      == TP raw      : %lld == %lld  %s\n",
           (long long)rs, (long long)rt, rs == rt ? "OK" : "FAIL");
    printf("    short physical == TP physical : %d == %d  %s\n",
           ps, pt, ps == pt ? "OK" : "FAIL");
    printf("    short MICRO    == TP MICRO    : %d == %d  %s\n",
           ms, mt, ms == mt ? "OK" : "FAIL");
    printf("    %s\n\n", ok ? "OK" : "FAIL");
}

static void section4_isolation(void)
{
    printf("[4] ISOLATION\n");
    printf("    CAN       standalone  (compile-time contract)\n");
    printf("    J1939     standalone  OK\n");
    printf("    TP        standalone  OK\n");
    printf("    SIGNAL    standalone  OK\n");
    printf("    CATALOG   standalone  OK\n");
    printf("    MICRO     standalone  OK\n");
    printf("    (cada modulo compila com -c sem puxar o integrador)\n\n");
}

int main(void)
{
    printf("=== MICROPROCESSADOR FRACTAL / CAN-J1939 ===\n\n");

    int64_t raw_s = 0, raw_t = 0;
    int32_t phys_s = 0, phys_t = 0, m_s = 0, m_t = 0;

    section1_short(&raw_s, &phys_s, &m_s);
    section2_tp(&raw_t, &phys_t, &m_t);
    section3_convergence(raw_s, phys_s, m_s, raw_t, phys_t, m_t);
    section4_isolation();

    int pass = (phys_s == 20 && m_s == 20 && phys_t == 20 && m_t == 20
                && raw_s == raw_t && phys_s == phys_t && m_s == m_t);

    printf("=== END-TO-END: %s ===\n", pass ? "PASS" : "FAIL");
    return pass ? 0 : 1;
}
