/*
 * j1939_tp.c — Transport Protocol J1939 (BAM + RTS/CTS + DT + reassembly)
 */

#include "j1939_tp.h"
#include <string.h>

void j1939_tp_init(J1939TpSession *s)
{
    memset(s, 0, sizeof(*s));
    s->state = J1939_TP_IDLE;
    s->cts_max_packets = 16;
}

uint32_t j1939_tp_make_id(uint8_t priority, uint32_t pgn, uint8_t sa)
{
    /* PDU1 se PF < 240: PGN nos bits 16..8 com PS=DA separado.
       Para TP, PGN 0xEC00 / 0xEB00 tem PF=0xEC / 0xEB >= 240 => PDU2.
       ID = (pri<<26) | (PGN<<8) | SA  com PGN 18 bits (DP:PF:PS). */
    uint32_t pri = (uint32_t)(priority & 7u) << 26;
    uint32_t pgn18 = (pgn & 0x3FFFFu) << 8;
    return pri | pgn18 | (sa & 0xFFu);
}

static uint8_t packets_needed(uint16_t len)
{
    if (len == 0) return 1;
    return (uint8_t)((len + 6) / 7);  /* 7 bytes uteis por DT */
}

static void fill_cm_bam(uint8_t data[8], uint16_t total_bytes,
                        uint8_t total_packets, uint32_t pgn)
{
    data[0] = J1939_TP_BAM;
    data[1] = (uint8_t)(total_bytes & 0xFF);
    data[2] = (uint8_t)((total_bytes >> 8) & 0xFF);
    data[3] = total_packets;
    data[4] = 0xFF; /* reserved */
    data[5] = (uint8_t)(pgn & 0xFF);
    data[6] = (uint8_t)((pgn >> 8) & 0xFF);
    data[7] = (uint8_t)((pgn >> 16) & 0xFF);
}

static void fill_cm_rts(uint8_t data[8], uint16_t total_bytes,
                        uint8_t total_packets, uint32_t pgn)
{
    data[0] = J1939_TP_RTS;
    data[1] = (uint8_t)(total_bytes & 0xFF);
    data[2] = (uint8_t)((total_bytes >> 8) & 0xFF);
    data[3] = total_packets;
    data[4] = 0xFF; /* max packets to send before waiting CTS — FF = all */
    data[5] = (uint8_t)(pgn & 0xFF);
    data[6] = (uint8_t)((pgn >> 8) & 0xFF);
    data[7] = (uint8_t)((pgn >> 16) & 0xFF);
}

static void fill_cm_cts(uint8_t data[8], uint8_t num_packets,
                        uint8_t next_seq, uint32_t pgn)
{
    data[0] = J1939_TP_CTS;
    data[1] = num_packets;
    data[2] = next_seq;
    data[3] = 0xFF;
    data[4] = 0xFF;
    data[5] = (uint8_t)(pgn & 0xFF);
    data[6] = (uint8_t)((pgn >> 8) & 0xFF);
    data[7] = (uint8_t)((pgn >> 16) & 0xFF);
}

static void fill_cm_eom(uint8_t data[8], uint16_t total_bytes,
                        uint8_t total_packets, uint32_t pgn)
{
    data[0] = J1939_TP_EOM;
    data[1] = (uint8_t)(total_bytes & 0xFF);
    data[2] = (uint8_t)((total_bytes >> 8) & 0xFF);
    data[3] = total_packets;
    data[4] = 0xFF;
    data[5] = (uint8_t)(pgn & 0xFF);
    data[6] = (uint8_t)((pgn >> 8) & 0xFF);
    data[7] = (uint8_t)((pgn >> 16) & 0xFF);
}

static void fill_dt(uint8_t data[8], uint8_t seq,
                    const uint8_t *src, uint16_t src_len, uint16_t offset)
{
    data[0] = seq;
    for (int i = 0; i < 7; i++) {
        uint16_t idx = (uint16_t)(offset + (uint16_t)i);
        data[1 + i] = (idx < src_len) ? src[idx] : 0xFF;
    }
}

int j1939_tp_bam_start(J1939TpSession *s, uint8_t sa,
                       uint32_t pgn, const uint8_t *data, uint16_t len)
{
    if (!s || !data || len == 0 || len > J1939_TP_MAX_LEN) return J1939_TP_ERR;
    if (s->state != J1939_TP_IDLE && s->state != J1939_TP_COMPLETE &&
        s->state != J1939_TP_ABORTED)
        return J1939_TP_BUSY;

    memset(s, 0, sizeof(*s));
    s->state = J1939_TP_TX_BAM;
    s->sa = sa;
    s->da = 0xFF;
    s->pgn = pgn & 0x3FFFF;
    s->total_bytes = len;
    s->total_packets = packets_needed(len);
    s->next_seq = 1;
    s->packets_sent = 0;
    s->buf_len = len;
    memcpy(s->buf, data, len);
    s->cts_max_packets = 16;
    return J1939_TP_OK;
}

int j1939_tp_rts_start(J1939TpSession *s, uint8_t sa, uint8_t da,
                       uint32_t pgn, const uint8_t *data, uint16_t len)
{
    if (!s || !data || len == 0 || len > J1939_TP_MAX_LEN) return J1939_TP_ERR;
    if (da == 0xFF) return J1939_TP_ERR; /* use BAM for broadcast */
    if (s->state != J1939_TP_IDLE && s->state != J1939_TP_COMPLETE &&
        s->state != J1939_TP_ABORTED)
        return J1939_TP_BUSY;

    memset(s, 0, sizeof(*s));
    s->state = J1939_TP_TX_RTS;
    s->sa = sa;
    s->da = da;
    s->pgn = pgn & 0x3FFFF;
    s->total_bytes = len;
    s->total_packets = packets_needed(len);
    s->next_seq = 1;
    s->packets_sent = 0;
    s->buf_len = len;
    memcpy(s->buf, data, len);
    s->cts_max_packets = 16;
    return J1939_TP_OK;
}

int j1939_tp_tx_next(J1939TpSession *s, CanFrame *out)
{
    if (!s || !out) return J1939_TP_ERR;
    memset(out, 0, sizeof(*out));
    out->extended = 1;
    out->rtr = 0;
    out->dlc = 8;

    if (s->state == J1939_TP_TX_BAM) {
        /* emite CM.BAM */
        out->id = j1939_tp_make_id(7, J1939_PGN_TP_CM, s->sa);
        /* PDU2: PS is part of PGN; for global address DA is in message, ID SA only */
        fill_cm_bam(out->data, s->total_bytes, s->total_packets, s->pgn);
        s->state = J1939_TP_TX_DT;
        s->next_seq = 1;
        s->cts_max_packets = 0xFF; /* BAM: sem janela CTS */
        return J1939_TP_OK;
    }

    if (s->state == J1939_TP_TX_RTS) {
        out->id = j1939_tp_make_id(7, J1939_PGN_TP_CM, s->sa);
        fill_cm_rts(out->data, s->total_bytes, s->total_packets, s->pgn);
        s->state = J1939_TP_TX_WAIT_CTS;
        return J1939_TP_OK;
    }

    if (s->state == J1939_TP_TX_DT) {
        if (s->packets_sent >= s->total_packets) {
            s->state = J1939_TP_COMPLETE;
            return J1939_TP_DONE;
        }
        /* janela CTS: se esgotou e ainda ha pacotes, espera novo CTS */
        if (s->cts_max_packets == 0) {
            s->state = J1939_TP_TX_WAIT_CTS;
            return J1939_TP_ERR; /* host deve aguardar CTS */
        }
        uint8_t seq = (uint8_t)(s->packets_sent + 1);
        uint16_t off = (uint16_t)(s->packets_sent * 7);
        out->id = j1939_tp_make_id(7, J1939_PGN_TP_DT, s->sa);
        fill_dt(out->data, seq, s->buf, s->buf_len, off);
        s->packets_sent++;
        s->next_seq = (uint8_t)(s->packets_sent + 1);
        if (s->cts_max_packets != 0xFF && s->cts_max_packets > 0)
            s->cts_max_packets--;
        if (s->packets_sent >= s->total_packets)
            s->state = J1939_TP_COMPLETE;
        else if (s->cts_max_packets == 0)
            s->state = J1939_TP_TX_WAIT_CTS;
        return J1939_TP_OK;
    }

    if (s->state == J1939_TP_COMPLETE)
        return J1939_TP_DONE;

    return J1939_TP_ERR;
}

/* processa CTS recebido no lado TX (desbloqueia DT apos RTS) */
static int tp_on_cts(J1939TpSession *s, const uint8_t data[8])
{
    if (s->state != J1939_TP_TX_WAIT_CTS) return J1939_TP_ERR;
    if (data[0] != J1939_TP_CTS) return J1939_TP_ERR;
    s->cts_max_packets = data[1] ? data[1] : 1;
    s->next_seq = data[2] ? data[2] : 1;
    s->state = J1939_TP_TX_DT;
    return J1939_TP_OK;
}

int j1939_tp_rx_frame(J1939TpSession *s, const CanFrame *f)
{
    if (!s || !f || !f->extended || f->dlc < 1) return J1939_TP_ERR;

    uint32_t pgn = j1939_pgn(f->id);
    uint8_t sa = j1939_sa(f->id);

    if (pgn == J1939_PGN_TP_CM) {
        uint8_t ctrl = f->data[0];

        if (ctrl == J1939_TP_BAM) {
            uint16_t total = (uint16_t)(f->data[1] | (f->data[2] << 8));
            uint8_t pkts = f->data[3];
            uint32_t msg_pgn = (uint32_t)f->data[5]
                             | ((uint32_t)f->data[6] << 8)
                             | ((uint32_t)f->data[7] << 16);
            if (total == 0 || total > J1939_TP_MAX_LEN) return J1939_TP_ERR;
            if (pkts == 0 || pkts != packets_needed(total)) return J1939_TP_ERR;

            memset(s, 0, sizeof(*s));
            s->state = J1939_TP_RX_BAM;
            s->sa = sa;
            s->da = 0xFF;
            s->pgn = msg_pgn;
            s->total_bytes = total;
            s->total_packets = pkts;
            s->next_seq = 1;
            s->buf_len = 0;
            return J1939_TP_OK;
        }

        if (ctrl == J1939_TP_RTS) {
            uint16_t total = (uint16_t)(f->data[1] | (f->data[2] << 8));
            uint8_t pkts = f->data[3];
            uint32_t msg_pgn = (uint32_t)f->data[5]
                             | ((uint32_t)f->data[6] << 8)
                             | ((uint32_t)f->data[7] << 16);
            if (total == 0 || total > J1939_TP_MAX_LEN) return J1939_TP_ERR;
            if (pkts == 0 || pkts != packets_needed(total)) return J1939_TP_ERR;

            memset(s, 0, sizeof(*s));
            s->state = J1939_TP_RX_RTS;
            s->sa = sa;
            s->pgn = msg_pgn;
            s->total_bytes = total;
            s->total_packets = pkts;
            s->next_seq = 1;
            s->buf_len = 0;
            s->cts_max_packets = 16;
            /* receptor deve emitir CTS — o host chama j1939_tp_tx_next em sessao
               auxiliar ou usa j1939_tp_build_cts abaixo via estado RX_RTS */
            return J1939_TP_OK;
        }

        if (ctrl == J1939_TP_CTS)
            return tp_on_cts(s, f->data);

        if (ctrl == J1939_TP_EOM) {
            if (s->buf_len == s->total_bytes)
                s->state = J1939_TP_COMPLETE;
            return J1939_TP_OK;
        }

        if (ctrl == J1939_TP_ABORT) {
            s->state = J1939_TP_ABORTED;
            return J1939_TP_OK;
        }

        return J1939_TP_ERR;
    }

    if (pgn == J1939_PGN_TP_DT) {
        /* DT so apos BAM ou apos CTS (estado RX_DT). Nao em RX_RTS. */
        if (s->state != J1939_TP_RX_BAM && s->state != J1939_TP_RX_DT)
            return J1939_TP_ERR;

        uint8_t seq = f->data[0];
        if (seq != s->next_seq) return J1939_TP_ERR; /* out of order */

        uint16_t off = (uint16_t)((seq - 1) * 7);
        for (int i = 0; i < 7; i++) {
            uint16_t idx = (uint16_t)(off + (uint16_t)i);
            if (idx < s->total_bytes) {
                s->buf[idx] = f->data[1 + i];
                if (idx + 1 > s->buf_len)
                    s->buf_len = (uint16_t)(idx + 1);
            }
        }
        s->next_seq++;
        s->state = J1939_TP_RX_DT;

        if (s->buf_len >= s->total_bytes) {
            s->buf_len = s->total_bytes;
            s->state = J1939_TP_COMPLETE;
        }
        return J1939_TP_OK;
    }

    return J1939_TP_ERR;
}

int j1939_tp_complete(const J1939TpSession *s)
{
    return s && s->state == J1939_TP_COMPLETE;
}

int j1939_tp_message(const J1939TpSession *s, uint8_t *out, uint16_t out_cap)
{
    if (!s || !out || s->state != J1939_TP_COMPLETE) return -1;
    if (out_cap < s->buf_len) return -1;
    memcpy(out, s->buf, s->buf_len);
    return (int)s->buf_len;
}

/* gera frame CTS a partir de sessao RX_RTS */
int j1939_tp_build_cts(J1939TpSession *s, CanFrame *out, uint8_t responder_sa)
{
    if (!s || !out || s->state != J1939_TP_RX_RTS) return J1939_TP_ERR;
    memset(out, 0, sizeof(*out));
    out->extended = 1;
    out->dlc = 8;
    out->id = j1939_tp_make_id(7, J1939_PGN_TP_CM, responder_sa);
    fill_cm_cts(out->data, s->total_packets, 1, s->pgn);
    s->state = J1939_TP_RX_DT;
    s->next_seq = 1;
    return J1939_TP_OK;
}

int j1939_tp_build_eom(J1939TpSession *s, CanFrame *out, uint8_t responder_sa)
{
    if (!s || !out || s->state != J1939_TP_COMPLETE) return J1939_TP_ERR;
    memset(out, 0, sizeof(*out));
    out->extended = 1;
    out->dlc = 8;
    out->id = j1939_tp_make_id(7, J1939_PGN_TP_CM, responder_sa);
    fill_cm_eom(out->data, s->total_bytes, s->total_packets, s->pgn);
    return J1939_TP_OK;
}
