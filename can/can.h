/*
 * can.h — modelo digital do controlador CAN (periferico)
 *
 * Modelado:
 *   frame 11/29-bit, DLC, RTR
 *   mailbox RX/TX
 *   arbitragem (MSB-first, dominante=0 vence)
 *   CRC-15 (polinomio 0x4599)
 *   bit stuffing / destuff
 *   TEC/REC + ERROR_ACTIVE / PASSIVE / BUS_OFF
 *   ACK (via can_bus: precisa de ao menos 1 ouvinte)
 *
 * NAO conhece J1939 nem micro. Multi-no: can_bus.h
 */

#ifndef CAN_H
#define CAN_H

#include <stdint.h>

#define CAN_MAX_DLC 8

typedef struct {
    uint32_t id;
    uint8_t  data[CAN_MAX_DLC];
    uint8_t  dlc;
    uint8_t  extended;
    uint8_t  rtr;
} CanFrame;

enum {
    CAN_ST_OK       = 0,
    CAN_ST_EMPTY    = 1,
    CAN_ST_FULL     = 2,
    CAN_ST_ERR      = 3,
    CAN_ST_OVERFLOW = 4,
    CAN_ST_BUS_OFF  = 5,
    CAN_ST_ACK_ERR  = 6
};

enum {
    CAN_ERR_ACTIVE  = 0,
    CAN_ERR_PASSIVE = 1,
    CAN_ERR_BUS_OFF = 2
};

typedef struct {
    CanFrame rx;
    CanFrame tx;
    uint32_t status;
    uint8_t  rx_pending;
    uint8_t  tx_pending;
    uint16_t tec;
    uint16_t rec;
    uint8_t  err_state;
    uint8_t  lost_arbitration;
    uint8_t  ack_seen;          /* 1 se ultimo TX teve ACK */
} CanController;

void can_init(CanController *c);

int  can_frame_valid(const CanFrame *f);
int  can_id_is_extended(const CanFrame *f);
uint32_t can_id_mask(const CanFrame *f);

int  can_rx(CanController *c, CanFrame *out);
int  can_tx(CanController *c, const CanFrame *in);
int  can_inject_rx(CanController *c, const CanFrame *f);
void can_tx_done(CanController *c);

/* arbitragem */
int can_arb_bit(const CanFrame *f, int k);
int can_arb_len(const CanFrame *f);
int can_arbitrate(const CanFrame *a, const CanFrame *b);

/* CRC-15 */
uint16_t can_crc15(const uint8_t *bits, int nbits);
uint16_t can_frame_crc(const CanFrame *f);

/* bit stuffing */
int can_bit_stuff(const uint8_t *in, int nbits, uint8_t *out, int out_cap);
int can_bit_destuff(const uint8_t *in, int nbits, uint8_t *out, int out_cap);

/* erros e recuperacao */
void can_error_tx(CanController *c);
void can_error_rx(CanController *c);
void can_success_tx(CanController *c);
void can_update_err_state(CanController *c);
void can_recover(CanController *c);   /* modelo: sai de BUS_OFF, zera TEC/REC */

#endif /* CAN_H */
