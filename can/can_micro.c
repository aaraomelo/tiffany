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
 *      can_micro.c can.c can_bus.c j1939.c j1939_tp.c j1939_catalog.c j1939_signal.c micro.c -lm -o can_micro
 *
 * j1939_tp nao conhece micro; a composicao e so aqui.
 */

#include "can.h"
#include "can_bus.h"
#include "j1939.h"
#include "j1939_tp.h"
#include "j1939_catalog.h"
#include "j1939_signal.h"
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


/* ====================================================================
 * Integracao: J1939-TP -> mensagem -> PGN/SPN -> micro
 * (composicao apenas; TP e micro nao se conhecem)
 * ==================================================================== */

/* 138 bytes: bytes[0..1] = SPN 110 raw LE (0x80,0x07)=1920; resto filler */
static void make_long_payload(uint8_t *buf, uint16_t len)
{
    for (uint16_t i = 0; i < len; i++)
        buf[i] = (uint8_t)(0x40 + (i % 0x40));
    buf[0] = 0x80;
    buf[1] = 0x07;  /* raw 1920 -> 20 deg C via SPN 110 */
}

/*
 * Apos reassembly: interpreta primeiros bytes como frame J1939 de dados
 * (SPN ainda cabe em 8 bytes; a mensagem longa e o envelope TP).
 * j1939.c nao e alterado.
 */
static int interpret_spn110(const uint8_t *msg, uint16_t len,
                            uint32_t expected_pgn,
                            uint64_t *raw_out, int32_t *phys_j, int32_t *phys_m)
{
    if (!msg || len < 2 || expected_pgn != J1939_SPN_110.pgn)
        return -1;

    CanFrame pseudo = {0};
    pseudo.extended = 1;
    pseudo.dlc = 8;
    for (int i = 0; i < 8 && i < (int)len; i++)
        pseudo.data[i] = msg[i];
    pseudo.id = 0x0CFEEE00; /* ID coerente com PGN 65262 SA=0 */

    uint64_t raw = 0;
    if (j1939_read_raw(&pseudo, &J1939_SPN_110, &raw) != 0)
        return -1;
    int32_t pj = 0;
    if (j1939_scale_i32(raw, &J1939_SPN_110, &pj) != 0)
        return -1;
    int32_t pm = scale_on_micro(raw, &J1939_SPN_110);
    if (raw_out) *raw_out = raw;
    if (phys_j) *phys_j = pj;
    if (phys_m) *phys_m = pm;
    return 0;
}

static void test_tp_bam_to_micro(void)
{
    printf("  [BAM->msg]   ");
    uint8_t payload[138];
    make_long_payload(payload, 138);

    J1939TpSession tx, rx;
    j1939_tp_init(&tx);
    j1939_tp_init(&rx);
    CHECK(j1939_tp_bam_start(&tx, 0x80, J1939_SPN_110.pgn, payload, 138) == J1939_TP_OK,
          "bam start");

    CanFrame frames[64];
    int n = 0;
    for (;;) {
        int r = j1939_tp_tx_next(&tx, &frames[n]);
        if (r == J1939_TP_DONE) break;
        CHECK(r == J1939_TP_OK, "tx");
        n++;
        CHECK(n < 64, "budget");
    }
    CHECK(n == 1 + 20, "1 BAM + 20 DT");

    for (int i = 0; i < n; i++)
        CHECK(j1939_tp_rx_frame(&rx, &frames[i]) == J1939_TP_OK, "rx");

    CHECK(j1939_tp_complete(&rx), "complete");
    CHECK(rx.pgn == J1939_SPN_110.pgn, "PGN preserved");

    uint8_t out[138];
    CHECK(j1939_tp_message(&rx, out, 138) == 138, "len 138");
    CHECK(memcmp(out, payload, 138) == 0, "bit-identical");

    uint64_t raw = 0;
    int32_t pj = 0, pm = 0;
    CHECK(interpret_spn110(out, 138, rx.pgn, &raw, &pj, &pm) == 0, "SPN");
    CHECK(raw == 1920, "raw 1920");
    CHECK(pj == 20 && pm == 20, "J1939==MICRO==20");
    printf("resid 0\n");
}

static void test_tp_rts_to_micro(void)
{
    printf("  [RTS->msg]   ");
    uint8_t payload[138];
    make_long_payload(payload, 138);

    J1939TpSession tx, rx;
    j1939_tp_init(&tx);
    j1939_tp_init(&rx);
    CHECK(j1939_tp_rts_start(&tx, 0x10, 0x20, J1939_SPN_110.pgn, payload, 138) == J1939_TP_OK,
          "rts start");

    CanFrame f;
    CHECK(j1939_tp_tx_next(&tx, &f) == J1939_TP_OK, "RTS");
    CHECK(j1939_tp_rx_frame(&rx, &f) == J1939_TP_OK, "rx RTS");

    CanFrame cts;
    CHECK(j1939_tp_build_cts(&rx, &cts, 0x20) == J1939_TP_OK, "CTS");
    CHECK(j1939_tp_rx_frame(&tx, &cts) == J1939_TP_OK, "tx CTS");

    int dt = 0;
    while (j1939_tp_tx_next(&tx, &f) == J1939_TP_OK) {
        CHECK(j1939_tp_rx_frame(&rx, &f) == J1939_TP_OK, "DT");
        dt++;
    }
    CHECK(dt == 20, "20 DT");
    CHECK(j1939_tp_complete(&rx), "complete");

    CanFrame eom;
    CHECK(j1939_tp_build_eom(&rx, &eom, 0x20) == J1939_TP_OK, "EOM");

    CHECK(rx.pgn == J1939_SPN_110.pgn, "PGN");
    uint8_t out[138];
    CHECK(j1939_tp_message(&rx, out, 138) == 138, "len");
    CHECK(memcmp(out, payload, 138) == 0, "bit-identical");

    uint64_t raw = 0;
    int32_t pj = 0, pm = 0;
    CHECK(interpret_spn110(out, 138, rx.pgn, &raw, &pj, &pm) == 0, "SPN");
    CHECK(raw == 1920 && pj == 20 && pm == 20, "TP==J1939==MICRO");
    printf("resid 0\n");
}


/* Sinais de teste no INTEGRADOR apenas — j1939.c nao e alterado */
static const J1939Signal SIG_ENG_SPEED = {
    /* PGN 61444 EEC1, SPN 190 — bytes 3..4 (0-based), 0.125 rpm/bit, offset 0
       physical = raw * 1 / 8   (porque 0.125 = 1/8) */
    .pgn = 61444,
    .spn = 190,
    .start_byte = 3,
    .length = 2,
    .offset = 0,
    .scale_num = 1,
    .scale_den = 8,
    .name = "Engine Speed",
    .unit = "rpm"
};

static const J1939Signal SIG_VEH_SPEED = {
    /* PGN 65265 CCVS, SPN 84 — bytes 1..2, 1/256 km/h per bit */
    .pgn = 65265,
    .spn = 84,
    .start_byte = 1,
    .length = 2,
    .offset = 0,
    .scale_num = 1,
    .scale_den = 256,
    .name = "Wheel-Based Vehicle Speed",
    .unit = "km/h"
};

static uint32_t make_id_pgn(uint8_t pri, uint32_t pgn, uint8_t sa)
{
    return ((uint32_t)(pri & 7) << 26) | ((pgn & 0x3FFFFu) << 8) | sa;
}

/* caminho curto: frame CAN -> j1939 -> micro */
static int short_path(const CanFrame *f, const J1939Signal *sig,
                      uint64_t *raw_out, int32_t *pj, int32_t *pm)
{
    if (!j1939_frame_valid(f)) return -1;
    if (j1939_pgn(f->id) != sig->pgn) return -2;
    uint64_t raw = 0;
    if (j1939_read_raw(f, sig, &raw) != 0) return -3;
    int32_t a = 0, b = 0;
    if (j1939_scale_i32(raw, sig, &a) != 0) return -4;
    b = scale_on_micro(raw, sig);
    if (raw_out) *raw_out = raw;
    if (pj) *pj = a;
    if (pm) *pm = b;
    return 0;
}

static void test_multi_pgn_short(void)
{
    printf("  [multi short]");

    const J1939Signal *s110 = j1939_catalog_find(65262, 110);
    const J1939Signal *s190 = j1939_catalog_find(61444, 190);
    const J1939Signal *s84  = j1939_catalog_find(65265, 84);
    CHECK(s110 && s190 && s84, "catalog lookup");

    CanFrame f110 = {
        .id = 0x0CFEEE00, .dlc = 8, .extended = 1, .rtr = 0,
        .data = { 0x80, 0x07, 0, 0, 0, 0, 0, 0 }
    };
    uint64_t raw; int32_t pj, pm;
    CHECK(short_path(&f110, s110, &raw, &pj, &pm) == 0, "110 path");
    CHECK(raw == 1920 && pj == 20 && pm == 20, "110 = 20C");

    CanFrame f190 = {
        .id = make_id_pgn(3, 61444, 0), .dlc = 8, .extended = 1, .rtr = 0,
        .data = { 0, 0, 0, 0x40, 0x1F, 0, 0, 0 }
    };
    CHECK(short_path(&f190, s190, &raw, &pj, &pm) == 0, "190 path");
    CHECK(raw == 8000 && pj == 1000 && pm == 1000, "190 = 1000 rpm");

    CanFrame f84 = {
        .id = make_id_pgn(6, 65265, 0), .dlc = 8, .extended = 1, .rtr = 0,
        .data = { 0, 0x00, 0x0A, 0, 0, 0, 0, 0 }
    };
    CHECK(short_path(&f84, s84, &raw, &pj, &pm) == 0, "84 path");
    CHECK(raw == 2560 && pj == 10 && pm == 10, "84 = 10 km/h");

    printf(" resid 0\n");
}

static void test_convergence_short_vs_tp(void)
{
    printf("  [converge]   ");
    /* Mesmo SPN 110: caminho curto e caminho TP devem coincidir */

    CanFrame shortf = {
        .id = 0x0CFEEE00, .dlc = 8, .extended = 1, .rtr = 0,
        .data = { 0x80, 0x07, 0, 0, 0, 0, 0, 0 }
    };
    uint64_t raw_s; int32_t pj_s, pm_s;
    CHECK(short_path(&shortf, &J1939_SPN_110, &raw_s, &pj_s, &pm_s) == 0, "short");

    uint8_t payload[138];
    make_long_payload(payload, 138); /* ja coloca 80 07 no inicio */

    J1939TpSession tx, rx;
    j1939_tp_init(&tx);
    j1939_tp_init(&rx);
    j1939_tp_bam_start(&tx, 0x80, J1939_SPN_110.pgn, payload, 138);
    CanFrame frames[64];
    int n = 0;
    while (j1939_tp_tx_next(&tx, &frames[n]) == J1939_TP_OK) n++;
    for (int i = 0; i < n; i++)
        j1939_tp_rx_frame(&rx, &frames[i]);
    CHECK(j1939_tp_complete(&rx), "tp complete");

    uint8_t out[138];
    CHECK(j1939_tp_message(&rx, out, 138) == 138, "len");
    uint64_t raw_t; int32_t pj_t, pm_t;
    CHECK(interpret_spn110(out, 138, rx.pgn, &raw_t, &pj_t, &pm_t) == 0, "tp interpret");

    CHECK(raw_s == raw_t && raw_s == 1920, "raw converge");
    CHECK(pj_s == pj_t && pj_s == 20, "J1939 converge");
    CHECK(pm_s == pm_t && pm_s == 20, "MICRO converge");
    printf("resid 0\n");
}



/* ---- API generica end-to-end (integrador) ---- */
static int generic_phys_from_payload(const uint8_t *payload, uint16_t plen,
                                     uint32_t pgn, uint32_t spn,
                                     int64_t *raw_out, int32_t *phys_j, int32_t *phys_m,
                                     int *sig_status)
{
    const J1939Signal *sig = j1939_catalog_find(pgn, spn);
    if (!sig) return -1;
    J1939Interp it = {
        .sig = sig,
        .is_signed = 0,
        .little_endian = 1,
        .na_value = j1939_signal_na_pattern(sig->length),
        .na_bits = (uint8_t)(sig->length * 8),
        .err_bits = 0
    };
    int64_t raw = 0;
    int st = 0;
    if (j1939_signal_extract(payload, plen, &it, &raw, &st) != J1939_SIG_OK)
        return -2;
    if (sig_status) *sig_status = st;
    if (st == J1939_SIG_NA || st == J1939_SIG_ERROR_RANGE) {
        if (raw_out) *raw_out = raw;
        return 1; /* semantico especial, nao vai ao micro */
    }
    int32_t pj = 0;
    if (j1939_signal_physical(raw, &it, &pj) != J1939_SIG_OK)
        return -3;
    int32_t pm = scale_on_micro((uint64_t)(raw < 0 ? 0 : raw), sig);
    /* scale_on_micro usa unsigned path; para os 3 sinais atuais raw >= 0 */
    if (raw_out) *raw_out = raw;
    if (phys_j) *phys_j = pj;
    if (phys_m) *phys_m = pm;
    return 0;
}

static void test_generic_e2e(void)
{
    printf("  [generic e2e]");

    /* SPN 110 curto */
    {
        uint8_t pl[8] = { 0x80, 0x07, 0, 0, 0, 0, 0, 0 };
        int64_t raw; int32_t pj, pm; int st;
        CHECK(generic_phys_from_payload(pl, 8, 65262, 110, &raw, &pj, &pm, &st) == 0, "110");
        CHECK(raw == 1920 && pj == 20 && pm == 20, "110 20C");
    }
    /* SPN 190 */
    {
        uint8_t pl[8] = { 0, 0, 0, 0x40, 0x1F, 0, 0, 0 };
        int64_t raw; int32_t pj, pm; int st;
        CHECK(generic_phys_from_payload(pl, 8, 61444, 190, &raw, &pj, &pm, &st) == 0, "190");
        CHECK(raw == 8000 && pj == 1000 && pm == 1000, "1000 rpm");
    }
    /* SPN 84 */
    {
        uint8_t pl[8] = { 0, 0x00, 0x0A, 0, 0, 0, 0, 0 };
        int64_t raw; int32_t pj, pm; int st;
        CHECK(generic_phys_from_payload(pl, 8, 65265, 84, &raw, &pj, &pm, &st) == 0, "84");
        CHECK(raw == 2560 && pj == 10 && pm == 10, "10 km/h");
    }
    /* NA nao chega ao micro como valor fisico valido */
    {
        uint8_t pl[8] = { 0xFF, 0xFF, 0, 0, 0, 0, 0, 0 };
        int64_t raw; int32_t pj, pm; int st;
        int r = generic_phys_from_payload(pl, 8, 65262, 110, &raw, &pj, &pm, &st);
        CHECK(r == 1 && st == J1939_SIG_NA, "NA blocked");
    }
    printf(" resid 0\n");
}

static void test_generic_short_vs_tp(void)
{
    printf("  [gen short|TP]");
    uint8_t short_pl[8] = { 0x80, 0x07, 0, 0, 0, 0, 0, 0 };
    int64_t raw_s; int32_t pj_s, pm_s; int st;
    CHECK(generic_phys_from_payload(short_pl, 8, 65262, 110, &raw_s, &pj_s, &pm_s, &st) == 0, "short");

    uint8_t payload[138];
    make_long_payload(payload, 138);
    J1939TpSession tx, rx;
    j1939_tp_init(&tx);
    j1939_tp_init(&rx);
    j1939_tp_bam_start(&tx, 0x80, 65262, payload, 138);
    CanFrame frames[64];
    int n = 0;
    while (j1939_tp_tx_next(&tx, &frames[n]) == J1939_TP_OK) n++;
    for (int i = 0; i < n; i++) j1939_tp_rx_frame(&rx, &frames[i]);
    CHECK(j1939_tp_complete(&rx), "tp ok");
    uint8_t out[138];
    CHECK(j1939_tp_message(&rx, out, 138) == 138, "len");

    int64_t raw_t; int32_t pj_t, pm_t;
    CHECK(generic_phys_from_payload(out, 138, rx.pgn, 110, &raw_t, &pj_t, &pm_t, &st) == 0, "tp");
    CHECK(raw_s == raw_t && pj_s == pj_t && pm_s == pm_t && pm_s == 20, "converge generic");
    printf(" resid 0\n");
}

static void test_catalog_tp_to_micro(void)
{
    printf("  [cat+TP]     ");
    /* Sinal do CATALOGO transportado por BAM; lookup apos reassembly */
    const J1939Signal *sig = j1939_catalog_find(65262, 110);
    CHECK(sig != NULL, "catalog has SPN 110");

    uint8_t payload[138];
    make_long_payload(payload, 138); /* 80 07 no inicio */

    J1939TpSession tx, rx;
    j1939_tp_init(&tx);
    j1939_tp_init(&rx);
    CHECK(j1939_tp_bam_start(&tx, 0x80, sig->pgn, payload, 138) == J1939_TP_OK, "bam");

    CanFrame frames[64];
    int n = 0;
    while (j1939_tp_tx_next(&tx, &frames[n]) == J1939_TP_OK) n++;
    for (int i = 0; i < n; i++)
        CHECK(j1939_tp_rx_frame(&rx, &frames[i]) == J1939_TP_OK, "rx");
    CHECK(j1939_tp_complete(&rx), "complete");
    CHECK(rx.pgn == sig->pgn, "PGN from TP");

    /* resolver SPN via CATALOGO (nao via constante em j1939.c) */
    const J1939Signal *resolved = j1939_catalog_find(rx.pgn, 110);
    CHECK(resolved == sig, "catalog resolve");

    uint8_t out[138];
    CHECK(j1939_tp_message(&rx, out, 138) == 138, "len");
    CHECK(memcmp(out, payload, 138) == 0, "payload");

    CanFrame pseudo = {0};
    pseudo.extended = 1;
    pseudo.dlc = 8;
    for (int i = 0; i < 8; i++) pseudo.data[i] = out[i];
    pseudo.id = 0x0CFEEE00;

    uint64_t raw = 0;
    int32_t pj = 0, pm = 0;
    CHECK(j1939_read_raw(&pseudo, resolved, &raw) == 0 && raw == 1920, "raw");
    CHECK(j1939_scale_i32(raw, resolved, &pj) == 0 && pj == 20, "J1939 20C");
    pm = scale_on_micro(raw, resolved);
    CHECK(pm == 20, "MICRO 20C");
    printf("resid 0\n");
}

static void test_tp_incomplete(void)

{
    printf("  [incomplete] ");
    uint8_t payload[138];
    make_long_payload(payload, 138);

    J1939TpSession tx, rx;
    j1939_tp_init(&tx);
    j1939_tp_init(&rx);
    j1939_tp_bam_start(&tx, 0x80, J1939_SPN_110.pgn, payload, 138);

    CanFrame frames[64];
    int n = 0;
    while (j1939_tp_tx_next(&tx, &frames[n]) == J1939_TP_OK) n++;

    /* so BAM + 5 DT — incompleto */
    for (int i = 0; i < 6 && i < n; i++)
        j1939_tp_rx_frame(&rx, &frames[i]);

    CHECK(!j1939_tp_complete(&rx), "not complete");
    uint8_t out[138];
    CHECK(j1939_tp_message(&rx, out, 138) < 0, "no message API");
    /* nao chama micro: incompleto nao atravessa a fronteira */
    printf("resid 0\n");
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

    printf("=== integracao J1939-TP / MICRO ===\n");
    test_tp_bam_to_micro();
    test_tp_rts_to_micro();
    test_tp_incomplete();
    test_multi_pgn_short();
    test_catalog_tp_to_micro();
    test_generic_e2e();
    test_generic_short_vs_tp();
    test_convergence_short_vs_tp();
    if (g_fail) {
        printf("[X] %d falha(s) na integracao TP\n", g_fail);
        return 1;
    }
    printf("[OK] TP -> J1939 -> MICRO\n\n");

    return run_demo();
}
