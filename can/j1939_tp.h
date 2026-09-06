/*
 * j1939_tp.h — J1939 Transport Protocol (ISO 11783-3 / SAE J1939-21)
 *
 * Camada NOVA. Nao altera:
 *   micro.c, can.c, can_bus.c, j1939.c  (congelados)
 *
 * PGNs de transporte:
 *   0xEC00 (60416) — Connection Management (CM)
 *   0xEB00 (60160) — Data Transfer (DT)
 *
 * Modos:
 *   BAM  — Broadcast Announce Message (um para todos)
 *   RTS/CTS — conexao ponto a ponto
 *
 * Payload util por DT: 7 bytes (1 byte = numero de sequencia).
 * Mensagem completa: ate J1939_TP_MAX_LEN bytes.
 */

#ifndef J1939_TP_H
#define J1939_TP_H

#include "can.h"
#include "j1939.h"
#include <stdint.h>

#define J1939_PGN_TP_CM   0xEC00u   /* 60416 */
#define J1939_PGN_TP_DT   0xEB00u   /* 60160 */

#define J1939_TP_MAX_LEN  1785      /* limite classico J1939 (~255*7) */
#define J1939_TP_MAX_PKTS 255

/* control bytes CM */
enum {
    J1939_TP_RTS  = 16,   /* 0x10 Request to Send */
    J1939_TP_CTS  = 17,   /* 0x11 Clear to Send */
    J1939_TP_EOM  = 19,   /* 0x13 End of Message ACK */
    J1939_TP_BAM  = 32,   /* 0x20 Broadcast Announce */
    J1939_TP_ABORT = 255  /* 0xFF Connection Abort */
};

enum {
    J1939_TP_OK = 0,
    J1939_TP_ERR = -1,
    J1939_TP_BUSY = -2,
    J1939_TP_DONE = 1
};

typedef enum {
    J1939_TP_IDLE = 0,
    J1939_TP_TX_BAM,
    J1939_TP_TX_RTS,
    J1939_TP_TX_WAIT_CTS,
    J1939_TP_TX_DT,
    J1939_TP_RX_BAM,
    J1939_TP_RX_RTS,
    J1939_TP_RX_DT,
    J1939_TP_COMPLETE,
    J1939_TP_ABORTED
} J1939TpState;

typedef struct {
    J1939TpState state;
    uint32_t     pgn;            /* PGN da mensagem transportada */
    uint8_t      sa;             /* source address da sessao */
    uint8_t      da;             /* destination (0xFF = broadcast BAM) */

    uint16_t     total_bytes;
    uint8_t      total_packets;
    uint8_t      next_seq;       /* proximo DT esperado (1-based) ou a enviar */
    uint8_t      packets_sent;

    uint8_t      buf[J1939_TP_MAX_LEN];
    uint16_t     buf_len;        /* bytes validos em buf apos reassembly / a enviar */

    uint8_t      cts_max_packets; /* quantos DT o receptor permite de uma vez */
} J1939TpSession;

void j1939_tp_init(J1939TpSession *s);

/* ---- TX ---- */

/* inicia BAM: prepara CM.BAM + DTs para broadcast */
int j1939_tp_bam_start(J1939TpSession *s, uint8_t sa,
                       uint32_t pgn, const uint8_t *data, uint16_t len);

/* inicia RTS (ponto a ponto) */
int j1939_tp_rts_start(J1939TpSession *s, uint8_t sa, uint8_t da,
                       uint32_t pgn, const uint8_t *data, uint16_t len);

/*
 * Produz o proximo frame CAN da sessao TX (CM ou DT).
 * Retorna J1939_TP_OK se gerou frame, J1939_TP_DONE se terminou, ERR caso contrario.
 */
int j1939_tp_tx_next(J1939TpSession *s, CanFrame *out);

/* ---- RX ---- */

/* alimenta a sessao com um frame CAN recebido (filtra CM/DT pelo PGN do ID) */
int j1939_tp_rx_frame(J1939TpSession *s, const CanFrame *f);

/* mensagem completa disponivel? */
int j1939_tp_complete(const J1939TpSession *s);

/* copia a mensagem reassembled; retorna comprimento ou -1 */
int j1939_tp_message(const J1939TpSession *s, uint8_t *out, uint16_t out_cap);

/* RX RTS: gera CTS / EOMACK (responder_sa = nosso SA) */
int j1939_tp_build_cts(J1939TpSession *s, CanFrame *out, uint8_t responder_sa);
int j1939_tp_build_eom(J1939TpSession *s, CanFrame *out, uint8_t responder_sa);

/* helpers: monta ID 29-bit J1939 (prio=7 default para TP) */
uint32_t j1939_tp_make_id(uint8_t priority, uint32_t pgn, uint8_t sa);

#endif /* J1939_TP_H */
