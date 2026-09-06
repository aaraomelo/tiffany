/*
 * can_micro.c — integracao: CAN -> J1939 -> Micro -> grandeza fisica
 *
 * NAO e controlador CAN. NAO e o nucleo do micro.
 * E o teste/integrador das tres camadas:
 *
 *   can.c     — frame / controller (modelo)
 *   j1939.c   — PGN / SPN / escala assinada
 *   micro.c   — MOVE / ALU / DIV (realizacao digital da conta)
 *
 * Fluxo:
 *   inject RX (can)
 *     -> can_rx
 *     -> j1939_pgn / j1939_read_raw
 *     -> MOVE: carrega raw na Maquina
 *     -> ALU/DIV do micro (opcional, paralelo a escala j1939)
 *     -> grandeza
 *
 * Compilacao:
 *   cc -O2 -std=c11 -Wall -DMICRO_AS_LIB -I. \
 *      can_micro.c can.c j1939.c micro.c -lm -o can_micro
 *   ./can_micro
 */

#include "can.h"
#include "j1939.h"
#include "micro.h"

#include <stdio.h>
#include <stdint.h>

/* slots na Maquina para a integracao */
enum {
    M_RAW = 0,
    M_OFF = 1,    /* |offset| * den  (positivo, para SUB na ISA) */
    M_DEN = 2,
    M_NUM = 3,
    M_QUOT = 4,
    M_REM = 5
};

static void print_j1939_id(uint32_t id)
{
    printf("J1939 ID\n");
    printf("  priority = %u\n", j1939_priority(id));
    printf("  reserved = %u\n", j1939_reserved(id));
    printf("  DP       = %u\n", j1939_dp(id));
    printf("  PF       = 0x%02X\n", j1939_pf(id));
    printf("  PS       = 0x%02X\n", j1939_ps(id));
    printf("  SA       = 0x%02X\n", j1939_sa(id));
    printf("  PDU      = %s\n", j1939_is_pdu1(id) ? "PDU1" : "PDU2");
    if (j1939_is_pdu1(id))
        printf("  DA       = 0x%02X\n", j1939_da(id));
    printf("  PGN      = %u (0x%05X)\n", j1939_pgn(id), j1939_pgn(id));
}

/*
 * Realiza a escala do SPN 110 no micro:
 *   physical = (raw - 40*32) / 32    quando raw >= 1280
 * via MOVE + SUB + DIV (ISA u64).
 *
 * Para casos raw < offset_raw a interpretacao assinada fica em j1939.c;
 * o micro demonstra a rota positiva (contrato do exemplo).
 */
static int32_t scale_on_micro(uint64_t raw, const J1939Signal *sig)
{
    Maquina M;
    maq_clear(&M);

    u64 off = (u64)(-(sig->offset)) * (u64)sig->scale_den;  /* 1280 */
    STORE_VAL(&M, M_RAW, raw);
    STORE_VAL(&M, M_OFF, off);
    STORE_VAL(&M, M_DEN, sig->scale_den);

    /* programa 80-bit: SUB(raw,off)->num; DIV(num,den)->quot */
    Instr80 is[] = {
        { M_RAW, M_OFF, IOP_SUB, M_NUM },
        { M_NUM, M_DEN, IOP_DIV, M_QUOT },
        { 0, 0, IOP_HALT, 0 }
    };
    uint8_t prog[30];
    for (int i = 0; i < 3; i++)
        encode80(&is[i], prog + 10 * i);

    int steps = 0;
    roda80(&M, prog, 30, &steps);
    (void)steps;
    return (int32_t)LOAD(&M, M_QUOT);
}


/* ====================================================================
 * Testes unitarios can + j1939 (sem micro)
 * ==================================================================== */

static int g_fail = 0;
#define CHECK(c, m) do { if (!(c)) { fprintf(stderr, "FAIL: %s\n", m); g_fail++; } } while (0)

static void test_can_layer(void)
{
    printf("  [can]        ");
    CanFrame ok = { .id = 0x0CFEEE00, .dlc = 8, .extended = 1, .rtr = 0 };
    CHECK(can_frame_valid(&ok), "frame 29-bit ok");

    CanFrame bad_rtr = ok; bad_rtr.rtr = 2;
    CHECK(!can_frame_valid(&bad_rtr), "rtr>1 invalido");

    CanFrame bad_id = ok; bad_id.id = 0x20000000;
    CHECK(!can_frame_valid(&bad_id), "id 30-bit invalido");

    CanFrame std = { .id = 0x100, .dlc = 2, .extended = 0, .rtr = 0 };
    CHECK(can_frame_valid(&std), "frame 11-bit ok");

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
    CHECK(j1939_frame_valid(&f), "j1939 frame valid");

    CanFrame not_ext = f; not_ext.extended = 0;
    CHECK(!j1939_frame_valid(&not_ext), "standard nao e j1939");

    CHECK(j1939_pgn(f.id) == 65262, "PGN 65262");
    CHECK(j1939_priority(f.id) == 3, "prio 3");
    CHECK(j1939_reserved(f.id) == 0, "reserved 0");
    CHECK(j1939_dp(f.id) == 0, "dp 0");
    CHECK(j1939_pf(f.id) == 0xFE, "pf FE");
    CHECK(j1939_ps(f.id) == 0xEE, "ps EE");
    CHECK(j1939_sa(f.id) == 0, "sa 0");
    CHECK(!j1939_is_pdu1(f.id), "PDU2");

    uint64_t raw = 0;
    CHECK(j1939_read_raw(&f, &J1939_SPN_110, &raw) == 0, "read_raw");
    CHECK(raw == 1920, "raw 1920");

    /* length 8 bytes */
    J1939Signal wide = J1939_SPN_110;
    wide.length = 8;
    wide.start_byte = 0;
    CHECK(j1939_read_raw(&f, &wide, &raw) == 0, "raw 8 bytes");
    CHECK(raw == 1920, "raw 8b still 1920 (zeros)");

    int32_t phys = 0;
    CHECK(j1939_scale_i32(1920, &J1939_SPN_110, &phys) == 0, "scale");
    CHECK(phys == 20, "20 deg C");

    /* valor abaixo do offset: escala assinada negativa */
    CHECK(j1939_scale_i32(0, &J1939_SPN_110, &phys) == 0, "scale 0");
    CHECK(phys == -40, "0 raw -> -40 deg C");

    printf("resid 0\n");
}

static int run_demo(void)

{
    /*
     * Frame J1939:
     *   priority=3, PGN=65262=0xFEEE, SA=0
     *   ID extended = 0x0CFEEE00
     *   data: 80 07 -> raw=1920 -> 20 deg C
     */
    CanFrame frame = {
        .id = 0x0CFEEE00,
        .data = { 0x80, 0x07, 0, 0, 0, 0, 0, 0 },
        .dlc = 8,
        .extended = 1,
        .rtr = 0
    };

    printf("=== INTEGRACAO CAN -> J1939 -> MICRO ===\n\n");

    /* --- camada CAN --- */
    CanController ctrl;
    can_init(&ctrl);

    if (!can_frame_valid(&frame)) {
        printf("ERRO: frame invalido\n");
        return 1;
    }
    if (can_inject_rx(&ctrl, &frame) != CAN_ST_OK) {
        printf("ERRO: inject RX\n");
        return 1;
    }

    CanFrame rx;
    if (can_rx(&ctrl, &rx) != CAN_ST_OK) {
        printf("ERRO: can_rx vazio\n");
        return 1;
    }

    printf("CAN (can.c)\n");
    printf("  extended = %u\n", rx.extended);
    printf("  ID       = 0x%08X\n", rx.id);
    printf("  DLC      = %u\n", rx.dlc);
    printf("  DATA     =");
    for (unsigned i = 0; i < rx.dlc; i++)
        printf(" %02X", rx.data[i]);
    printf("\n\n");

    /* --- camada J1939 --- */
    print_j1939_id(rx.id);
    printf("\n");

    uint32_t pgn = j1939_pgn(rx.id);
    if (pgn != J1939_SPN_110.pgn) {
        printf("PGN %u != %u (SPN 110); demo encerrada.\n",
               pgn, J1939_SPN_110.pgn);
        return 1;
    }

    uint64_t raw = 0;
    if (j1939_read_raw(&rx, &J1939_SPN_110, &raw) != 0) {
        printf("ERRO: j1939_read_raw\n");
        return 1;
    }

    int64_t num = 0;
    uint32_t den = 0;
    j1939_scale_rational(raw, &J1939_SPN_110, &num, &den);

    int32_t phys_j = 0;
    j1939_scale_i32(raw, &J1939_SPN_110, &phys_j);

    printf("J1939 (j1939.c)\n");
    printf("  SPN %u: %s\n", J1939_SPN_110.spn, J1939_SPN_110.name);
    printf("  raw      = %llu (0x%04llX)\n", (unsigned long long)raw, (unsigned long long)raw);
    printf("  racional = %lld / %u\n", (long long)num, den);
    printf("  fisico   = %d %s  (escala assinada no adaptador)\n",
           phys_j, J1939_SPN_110.unit);

    /* --- camada MICRO (realizacao digital da conta positiva) --- */
    int32_t phys_m = scale_on_micro(raw, &J1939_SPN_110);
    printf("\nMICRO (micro.c via MOVE + instr 80-bit SUB/DIV)\n");
    printf("  fisico   = %d %s\n", phys_m, J1939_SPN_110.unit);

    if (phys_j != phys_m) {
        printf("\nERRO: j1939 escala (%d) != micro (%d)\n", phys_j, phys_m);
        return 1;
    }

    printf("\nRESULTADO\n");
    printf("  %s = %d %s\n", J1939_SPN_110.name, phys_m, J1939_SPN_110.unit);

    printf("\nCAMADAS\n");
    printf("  can.c    : frame RX (inject -> can_rx)\n");
    printf("  j1939.c  : ID -> PGN/SPN -> raw -> escala assinada\n");
    printf("  micro.c  : MOVE + ALU/DIV (realizacao digital)\n");
    printf("  can_micro: integracao apenas\n");

    return (phys_m == 20) ? 0 : 1;
}

int main(void)
{
    printf("=== testes can + j1939 ===\n");
    test_can_layer();
    test_j1939_layer();
    if (g_fail) {
        printf("[X] %d falha(s) nas camadas\n", g_fail);
        return 1;
    }
    printf("[OK] can + j1939 resid 0\n\n");
    return run_demo();
}
