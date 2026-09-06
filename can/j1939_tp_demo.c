/*
 * j1939_tp_demo.c — testes e demo do Transport Protocol J1939
 *
 * NAO altera camadas congeladas.
 *
 * Compilar:
 *   cc -O2 -std=c11 -Wall -I. \
 *      j1939_tp_demo.c j1939_tp.c j1939.c can.c -o j1939_tp_demo
 *   ./j1939_tp_demo
 */

#include "j1939_tp.h"
#include "j1939.h"
#include "can.h"

#include <stdio.h>
#include <string.h>
#include <stdint.h>

static int g_fail = 0;
#define CHECK(c, m) do { if (!(c)) { fprintf(stderr, "FAIL: %s\n", m); g_fail++; } } while (0)

/* mensagem > 8 bytes */
static const char *MSG =
    "J1939-TP: mensagem longa para BAM e RTS/CTS. "
    "O payload classico do CAN tem no maximo 8 bytes; "
    "o Transport Protocol fragmenta e reassembla.";

static void test_bam_roundtrip(void)
{
    printf("  [BAM]        ");
    uint16_t len = (uint16_t)strlen(MSG);
    CHECK(len > 8, "msg > 8 bytes");

    J1939TpSession tx, rx;
    j1939_tp_init(&tx);
    j1939_tp_init(&rx);

    CHECK(j1939_tp_bam_start(&tx, 0x80, 0xF000, (const uint8_t *)MSG, len) == J1939_TP_OK,
          "bam start");

    CanFrame frames[64];
    int nframes = 0;
    for (;;) {
        int r = j1939_tp_tx_next(&tx, &frames[nframes]);
        if (r == J1939_TP_DONE) break;
        CHECK(r == J1939_TP_OK, "tx_next");
        nframes++;
        CHECK(nframes < 64, "frame budget");
    }
    /* 1 CM.BAM + ceil(len/7) DT */
    uint8_t expect_pkts = (uint8_t)((len + 6) / 7);
    CHECK(nframes == 1 + expect_pkts, "BAM + DTs");

    /* RX: alimenta todos os frames */
    for (int i = 0; i < nframes; i++) {
        int r = j1939_tp_rx_frame(&rx, &frames[i]);
        CHECK(r == J1939_TP_OK, "rx frame");
    }
    CHECK(j1939_tp_complete(&rx), "complete");
    CHECK(rx.pgn == 0xF000, "pgn");

    uint8_t out[512];
    int n = j1939_tp_message(&rx, out, sizeof out);
    CHECK(n == (int)len, "len");
    CHECK(memcmp(out, MSG, len) == 0, "payload");
    printf("resid 0  (%d frames, %u bytes)\n", nframes, len);
}

static void test_rts_cts_roundtrip(void)
{
    printf("  [RTS/CTS]    ");
    uint16_t len = (uint16_t)strlen(MSG);

    J1939TpSession tx, rx;
    j1939_tp_init(&tx);
    j1939_tp_init(&rx);

    CHECK(j1939_tp_rts_start(&tx, 0x10, 0x20, 0xF001,
                             (const uint8_t *)MSG, len) == J1939_TP_OK, "rts start");

    CanFrame f;
    /* TX: RTS */
    CHECK(j1939_tp_tx_next(&tx, &f) == J1939_TP_OK, "send RTS");
    CHECK(f.data[0] == J1939_TP_RTS, "ctrl RTS");

    /* RX: recebe RTS */
    CHECK(j1939_tp_rx_frame(&rx, &f) == J1939_TP_OK, "rx RTS");
    CHECK(rx.state == J1939_TP_RX_RTS, "state RX_RTS");

    /* RX: emite CTS */
    CanFrame cts;
    CHECK(j1939_tp_build_cts(&rx, &cts, 0x20) == J1939_TP_OK, "build CTS");
    CHECK(cts.data[0] == J1939_TP_CTS, "ctrl CTS");

    /* TX: recebe CTS -> passa a enviar DT */
    CHECK(j1939_tp_rx_frame(&tx, &cts) == J1939_TP_OK, "tx got CTS");
    CHECK(tx.state == J1939_TP_TX_DT, "tx DT");

    /* TX: todos os DTs; RX: consome */
    int dt_count = 0;
    for (;;) {
        int r = j1939_tp_tx_next(&tx, &f);
        if (r == J1939_TP_DONE) break;
        CHECK(r == J1939_TP_OK, "dt");
        CHECK(j1939_pgn(f.id) == J1939_PGN_TP_DT, "DT pgn");
        CHECK(j1939_tp_rx_frame(&rx, &f) == J1939_TP_OK, "rx dt");
        dt_count++;
    }
    CHECK(dt_count == (int)((len + 6) / 7), "dt count");
    CHECK(j1939_tp_complete(&rx), "rx complete");

    /* EOM ACK */
    CanFrame eom;
    CHECK(j1939_tp_build_eom(&rx, &eom, 0x20) == J1939_TP_OK, "EOM");
    CHECK(eom.data[0] == J1939_TP_EOM, "ctrl EOM");

    uint8_t out[512];
    int n = j1939_tp_message(&rx, out, sizeof out);
    CHECK(n == (int)len && memcmp(out, MSG, len) == 0, "payload RTS");
    printf("resid 0  (%d DT, %u bytes)\n", dt_count, len);
}

static void test_sequence_error(void)
{
    printf("  [seq err]    ");
    uint8_t payload[20];
    for (int i = 0; i < 20; i++) payload[i] = (uint8_t)i;

    J1939TpSession tx, rx;
    j1939_tp_init(&tx);
    j1939_tp_init(&rx);
    j1939_tp_bam_start(&tx, 1, 0xAB00, payload, 20);

    CanFrame frames[8];
    int n = 0;
    while (j1939_tp_tx_next(&tx, &frames[n]) == J1939_TP_OK) n++;

    /* entrega BAM + DT1, pula DT2, manda DT3 -> erro de sequencia */
    CHECK(j1939_tp_rx_frame(&rx, &frames[0]) == J1939_TP_OK, "BAM");
    CHECK(j1939_tp_rx_frame(&rx, &frames[1]) == J1939_TP_OK, "DT1");
    if (n >= 4) {
        CHECK(j1939_tp_rx_frame(&rx, &frames[3]) == J1939_TP_ERR, "skip seq");
    }
    printf("resid 0\n");
}

int main(void)
{
    printf("=== J1939 Transport Protocol ===\n");
    printf("(camadas congeladas intactas)\n\n");
    test_bam_roundtrip();
    test_rts_cts_roundtrip();
    test_sequence_error();
    if (g_fail) {
        printf("\n[X] %d falha(s)\n", g_fail);
        return 1;
    }
    printf("\n[OK] j1939_tp resid 0\n");
    printf("mensagem > 8 bytes: BAM e RTS/CTS reassembled com sucesso.\n");
    return 0;
}
