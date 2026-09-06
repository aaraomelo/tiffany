/*
 * can.h — modelo digital do controlador CAN (periferico)
 *
 * Camada de enlace/frame. NAO conhece J1939, PGN, SPN nem o micro.
 *
 *   micro.c  <- I/O ->  can.c  <- frame ->  j1939.c
 *
 * Compilar junto com can.c.
 */

#ifndef CAN_H
#define CAN_H

#include <stdint.h>

#define CAN_MAX_DLC 8

typedef struct {
    uint32_t id;              /* 11-bit (standard) ou 29-bit (extended) */
    uint8_t  data[CAN_MAX_DLC];
    uint8_t  dlc;             /* 0..8 */
    uint8_t  extended;        /* 0 = standard 11-bit, 1 = extended 29-bit */
    uint8_t  rtr;             /* remote transmission request */
} CanFrame;

/* status simples do controlador (modelo de referencia) */
enum {
    CAN_ST_OK       = 0,
    CAN_ST_EMPTY    = 1,      /* nada a receber */
    CAN_ST_FULL     = 2,      /* TX ocupado */
    CAN_ST_ERR      = 3,
    CAN_ST_OVERFLOW = 4
};

typedef struct {
    CanFrame rx;
    CanFrame tx;
    uint32_t status;
    uint8_t  rx_pending;      /* 1 se ha frame em rx */
    uint8_t  tx_pending;
} CanController;

void can_init(CanController *c);

/* valida DLC e campos basicos */
int  can_frame_valid(const CanFrame *f);

/* helpers de ID */
int  can_id_is_extended(const CanFrame *f);
uint32_t can_id_mask(const CanFrame *f);   /* 0x7FF ou 0x1FFFFFFF */

/* RX: entrega o frame pendente em *out; retorna CAN_ST_* */
int  can_rx(CanController *c, CanFrame *out);

/* TX: enfileira frame para envio (modelo: fica em c->tx) */
int  can_tx(CanController *c, const CanFrame *in);

/*
 * Injeta um frame no controlador como se tivesse chegado do barramento.
 * (modelo de referencia — nao ha PHY real)
 */
int  can_inject_rx(CanController *c, const CanFrame *f);

/* limpa TX pendente apos "envio" bem-sucedido no modelo */
void can_tx_done(CanController *c);

#endif /* CAN_H */
