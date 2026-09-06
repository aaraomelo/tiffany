/*
 * can_micro.c — integracao + auditoria CAN
 *
 * Camadas (contratos congelados onde indicado):
 *   micro.c   — FECHADO
 *   j1939.c   — FECHADO
 *   can.c     — frame / arb / CRC / stuff / TEC / ACK
 *   can_bus.c — multi-no + tick
 *
 * Compilar:
 *   cc -O2 -std=c11 -Wall -DMICRO_AS_LIB -I. \
 *      can_micro.c can.c can_bus.c j1939.c micro.c -lm -o can_micro
 */

#include "can.h"
#include "can_bus.h"
#include "j1939.h"
#include "micro.h"

#include <stdio.h>
#include <stdint.h>
#include <string.h>

enum {
    M_RAW = 0, M_OFF = 1, M_DEN = 2, M_NUM = 3, M_QUOT = 4, M_REM = 5
};

static int g_fail = 0;
#define CHECK(c, m) do { if (!(c)) { fprintf(stderr, "FAIL: %s\n", m); g_fail++; } } while (0)

/* -------------------- testes can -------------------- */

static void test_can_layer(void)
{
    printf("  [can]        ");
    CanFrame ok = { .id = 0x0CFEEE00, .dlc = 8, .extended = 1, .rtr = 0 };
    CHECK(can_frame_valid(&ok), "29-bit ok");

    CanFrame bad_rtr = ok; bad_rtr.rtr = 2;
    CHECK(!can_frame_valid(&bad_rtr), "rtr>1");

    CanFrame bad_id = ok; bad_id.id = 0x20000000;
    CHECK(!can_frame_valid(&bad_id), "id 30-bit");

    CanFrame std = { .id = 0x100, .dlc = 2, .extended = 0, .rtr = 0 };
    CHECK(can_frame_valid(&std), "11-bit ok");

    CanController c;
    can_init(&c);
    CHECK(can_inject_rx(&c, &ok) == CAN_ST_OK, "inject");
    CHECK(can_inject_rx(&c, &ok) == CAN_ST_OVERFLOW, "overflow");
    CanFrame out;
    CHECK(can_rx(&c, &out) == CAN_ST_OK, "rx");
    CHECK(out.id == ok.id, "rx id");
    CHECK(can_rx(&c, &out) == CAN_ST_EMPTY, "empty");
    CHECK(can_tx(&c, &ok) == CAN_ST_OK, "tx");
    CHECK(can_tx(&c, &ok) == CAN_ST_FULL, "tx full");
    can_tx_done(&c);
    CHECK(can_tx(&c, &ok) == CAN_ST_OK, "tx after done");
    printf("resid 0\n");
}

static void test_j1939_layer(void)
{
    printf("  [j1939]      ");
    CanFrame f = {
        .id = 0x0CFEEE00,
        .data = { 0x80, 0x07, 0, 0, 0, 0, 0, 0 },
        .dlc = 8, .extended = 1, .rtr = 0
    };
    CHECK(j1939_frame_valid(&f), "j1939 valid");
    CanFrame not_ext = f; not_ext.extended = 0;
    CHECK(!j1939_frame_valid(&not_ext), "std not j1939");

    CHECK(j1939_pgn(f.id) == 65262, "PGN");
    CHECK(j1939_priority(f.id) == 3, "prio");
    CHECK(j1939_reserved(f.id) == 0, "R");
    CHECK(j1939_dp(f.id) == 0, "DP");
    CHECK(j1939_pf(f.id) == 0xFE, "PF");
    CHECK(j1939_ps(f.id) == 0xEE, "PS");
    CHECK(j1939_sa(f.id) == 0, "SA");
    CHECK(!j1939_is_pdu1(f.id), "PDU2");

    uint64_t raw = 0;
    CHECK(j1939_read_raw(&f, &J1939_SPN_110, &raw) == 0 && raw == 1920, "raw");
    J1939Signal wide = J1939_SPN_110;
    wide.length = 8;
    CHECK(j1939_read_raw(&f, &wide, &raw) == 0 && raw == 1920, "raw 8b");

    int32_t phys = 0;
    CHECK(j1939_scale_i32(1920, &J1939_SPN_110, &phys) == 0 && phys == 20, "20C");
    CHECK(j1939_scale_i32(0, &J1939_SPN_110, &phys) == 0 && phys == -40, "-40C");
    printf("resid 0\n");
}

static void test_can_protocol_audit(void)
{
    printf("  [protocol]   ");

    /* [1] arbitragem 0x100 < 0x200 < 0x300 */
    CanFrame fa = { .id = 0x300, .dlc = 1, .data = {0xAA}, .extended = 0, .rtr = 0 };
    CanFrame fb = { .id = 0x100, .dlc = 1, .data = {0xBB}, .extended = 0, .rtr = 0 };
    CanFrame fc = { .id = 0x200, .dlc = 1, .data = {0xCC}, .extended = 0, .rtr = 0 };
    CHECK(can_arbitrate(&fb, &fa) < 0, "100<300");
    CHECK(can_arbitrate(&fb, &fc) < 0, "100<200");
    CHECK(can_arbitrate(&fc, &fa) < 0, "200<300");

    /* [2] empate mesmo ID */
    CanFrame s1 = { .id = 0x42, .extended = 0, .dlc = 0, .rtr = 0 };
    CanFrame s2 = { .id = 0x42, .extended = 0, .dlc = 0, .rtr = 0 };
    CHECK(can_arbitrate(&s1, &s2) == 0, "empate ID");

    /* [3][4] standard / extended */
    CanFrame st = { .id = 0x100, .extended = 0, .dlc = 0, .rtr = 0 };
    CanFrame ex = { .id = 0x0CFEEE00, .extended = 1, .dlc = 0, .rtr = 0 };
    CHECK(can_arb_len(&st) == 11, "len 11");
    CHECK(can_arb_len(&ex) == 29, "len 29");
    CanFrame e2 = { .id = 0x0CFEEE01, .extended = 1, .dlc = 0, .rtr = 0 };
    CHECK(can_arbitrate(&ex, &e2) < 0, "ext menor");

    /* [5] DLC 0..8 */
    for (uint8_t d = 0; d <= 8; d++) {
        CanFrame fd = { .id = 1, .dlc = d, .extended = 0, .rtr = 0 };
        CHECK(can_frame_valid(&fd), "dlc");
    }
    CanFrame bad = { .id = 1, .dlc = 9, .extended = 0, .rtr = 0 };
    CHECK(!can_frame_valid(&bad), "dlc9");

    /* [6][7] stuffing / destuff */
    uint8_t ones[5] = {1,1,1,1,1};
    uint8_t zeros[5] = {0,0,0,0,0};
    uint8_t buf[64], out[64];
    int ns = can_bit_stuff(ones, 5, buf, 64);
    CHECK(ns == 6 && buf[5] == 0, "stuff 11111->0");
    int nd = can_bit_destuff(buf, ns, out, 64);
    CHECK(nd == 5, "destuff 11111");

    ns = can_bit_stuff(zeros, 5, buf, 64);
    CHECK(ns == 6 && buf[5] == 1, "stuff 00000->1");
    nd = can_bit_destuff(buf, ns, out, 64);
    CHECK(nd == 5, "destuff 00000");

    uint8_t long1[40];
    for (int i = 0; i < 40; i++) long1[i] = 1;
    ns = can_bit_stuff(long1, 40, buf, 64);
    CHECK(ns > 40, "40 uns stuffed");
    nd = can_bit_destuff(buf, ns, out, 64);
    CHECK(nd == 40, "40 destuff");
    int same = 1;
    for (int i = 0; i < 40; i++) if (out[i] != 1) same = 0;
    CHECK(same, "40 ones ok");

    uint8_t six[] = {1,1,1,1,1,1};
    CHECK(can_bit_destuff(six, 6, out, 64) < 0, "stuff error");

    /* [8] CRC */
    CanFrame fr = { .id = 0x123, .dlc = 2, .data = {0x11, 0x22}, .extended = 0, .rtr = 0 };
    CanFrame fr2 = fr; fr2.data[0] = 0x12;
    uint16_t c1 = can_frame_crc(&fr);
    CHECK(c1 == can_frame_crc(&fr) && c1 != 0, "CRC stable");
    CHECK(c1 != can_frame_crc(&fr2), "CRC data change");

    /* [9][1] bus + ACK com 3 ECUs */
    CanController ecu1, ecu2, ecu3;
    can_init(&ecu1); can_init(&ecu2); can_init(&ecu3);
    CanBus bus;
    can_bus_init(&bus);
    can_bus_attach(&bus, &ecu1);
    can_bus_attach(&bus, &ecu2);
    can_bus_attach(&bus, &ecu3);
    can_tx(&ecu1, &fa);
    can_tx(&ecu2, &fb);
    can_tx(&ecu3, &fc);

    int w = can_bus_tick(&bus);
    CHECK(w == 1 && bus.last_won.id == 0x100, "tick1 0x100");
    CHECK(ecu2.ack_seen && !ecu2.tx_pending, "ACK + TX done");
    CanFrame rx;
    CHECK(can_rx(&ecu1, &rx) == CAN_ST_OK && rx.id == 0x100, "rx1");
    CHECK(can_rx(&ecu3, &rx) == CAN_ST_OK && rx.id == 0x100, "rx3");

    w = can_bus_tick(&bus);
    CHECK(w == 2 && bus.last_won.id == 0x200, "tick2 0x200");
    w = can_bus_tick(&bus);
    CHECK(w == 0 && bus.last_won.id == 0x300, "tick3 0x300");
    CHECK(can_bus_tx_pending_count(&bus) == 0, "empty");

    /* [9] ACK sem receptor */
    CanController solo;
    can_init(&solo);
    CanBus bus1;
    can_bus_init(&bus1);
    can_bus_attach(&bus1, &solo);
    can_tx(&solo, &fb);
    w = can_bus_tick(&bus1);
    CHECK(solo.status == CAN_ST_ACK_ERR, "ACK err");
    CHECK(solo.tx_pending && solo.tec == 8, "TX hold TEC+8");

    /* [11] TEC ACTIVE -> PASSIVE -> BUS_OFF */
    CanController e;
    can_init(&e);
    for (int i = 0; i < 15; i++) can_error_tx(&e);
    CHECK(e.tec == 120 && e.err_state == CAN_ERR_ACTIVE, "ACTIVE 120");
    can_error_tx(&e);
    CHECK(e.tec == 128 && e.err_state == CAN_ERR_PASSIVE, "PASSIVE 128");
    while (e.tec < 256) can_error_tx(&e);
    CHECK(e.err_state == CAN_ERR_BUS_OFF, "BUS_OFF");
    CHECK(can_tx(&e, &fb) == CAN_ST_BUS_OFF, "TX blocked");

    /* [12] recover */
    can_recover(&e);
    CHECK(e.err_state == CAN_ERR_ACTIVE && e.tec == 0, "recover");
    CHECK(can_tx(&e, &fb) == CAN_ST_OK, "TX after recover");

    printf("resid 0\n");
}

/* -------------------- integracao J1939 + micro -------------------- */

static int32_t scale_on_micro(uint64_t raw, const J1939Signal *sig)
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

static int run_demo(void)
{
    CanFrame frame = {
        .id = 0x0CFEEE00,
        .data = { 0x80, 0x07, 0, 0, 0, 0, 0, 0 },
        .dlc = 8, .extended = 1, .rtr = 0
    };

    printf("=== INTEGRACAO CAN -> J1939 -> MICRO ===\n\n");

    CanController ctrl;
    can_init(&ctrl);
    if (!can_frame_valid(&frame) || can_inject_rx(&ctrl, &frame) != CAN_ST_OK)
        return 1;
    CanFrame rx;
    if (can_rx(&ctrl, &rx) != CAN_ST_OK) return 1;

    printf("CAN  ID=0x%08X DLC=%u\n", rx.id, rx.dlc);
    printf("J1939 PGN=%u pri=%u SA=0x%02X\n",
           j1939_pgn(rx.id), j1939_priority(rx.id), j1939_sa(rx.id));

    if (j1939_pgn(rx.id) != J1939_SPN_110.pgn) return 1;

    uint64_t raw = 0;
    if (j1939_read_raw(&rx, &J1939_SPN_110, &raw) != 0) return 1;

    int32_t phys_j = 0, phys_m = 0;
    j1939_scale_i32(raw, &J1939_SPN_110, &phys_j);
    phys_m = scale_on_micro(raw, &J1939_SPN_110);

    printf("SPN 110 raw=%llu  J1939=%d C  MICRO=%d C\n",
           (unsigned long long)raw, phys_j, phys_m);

    if (phys_j != phys_m || phys_j != 20) return 1;
    printf("RESULTADO: Engine Coolant Temperature = 20 deg C\n");
    return 0;
}

int main(void)
{
    printf("=== auditoria CAN / J1939 ===\n");
    test_can_layer();
    test_j1939_layer();
    test_can_protocol_audit();
    if (g_fail) {
        printf("[X] %d falha(s)\n", g_fail);
        return 1;
    }
    printf("[OK] can + j1939 + protocol resid 0\n\n");
    return run_demo();
}
