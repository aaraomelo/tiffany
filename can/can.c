/*
 * can.c — modelo digital do controlador CAN
 *
 * Periferico: frame 11/29-bit, DLC, DATA. Sem J1939, sem micro.
 *
 * Contrato can_frame_valid:
 *   extended ∈ {0,1}
 *   rtr      ∈ {0,1}
 *   dlc      ∈ [0,8]
 *   id       ∈ [0, 0x7FF]     se standard
 *            ∈ [0, 0x1FFFFFFF] se extended
 */

#include "can.h"
#include <string.h>

void can_init(CanController *c)
{
    memset(c, 0, sizeof(*c));
    c->status = CAN_ST_EMPTY;
}

int can_frame_valid(const CanFrame *f)
{
    if (!f) return 0;
    if (f->extended > 1) return 0;
    if (f->rtr > 1) return 0;
    if (f->dlc > CAN_MAX_DLC) return 0;
    if (f->extended) {
        if (f->id & ~0x1FFFFFFFu) return 0;
    } else {
        if (f->id & ~0x7FFu) return 0;
    }
    return 1;
}

int can_id_is_extended(const CanFrame *f)
{
    return f && f->extended;
}

uint32_t can_id_mask(const CanFrame *f)
{
    return (f && f->extended) ? 0x1FFFFFFFu : 0x7FFu;
}

int can_rx(CanController *c, CanFrame *out)
{
    if (!c || !out) return CAN_ST_ERR;
    if (!c->rx_pending) {
        c->status = CAN_ST_EMPTY;
        return CAN_ST_EMPTY;
    }
    *out = c->rx;
    c->rx_pending = 0;
    c->status = CAN_ST_OK;
    return CAN_ST_OK;
}

int can_tx(CanController *c, const CanFrame *in)
{
    if (!c || !in) return CAN_ST_ERR;
    if (!can_frame_valid(in)) return CAN_ST_ERR;
    if (c->tx_pending) {
        c->status = CAN_ST_FULL;
        return CAN_ST_FULL;
    }
    c->tx = *in;
    c->tx_pending = 1;
    c->status = CAN_ST_OK;
    return CAN_ST_OK;
}

int can_inject_rx(CanController *c, const CanFrame *f)
{
    if (!c || !f) return CAN_ST_ERR;
    if (!can_frame_valid(f)) return CAN_ST_ERR;
    if (c->rx_pending) {
        c->status = CAN_ST_OVERFLOW;
        return CAN_ST_OVERFLOW;
    }
    c->rx = *f;
    c->rx_pending = 1;
    c->status = CAN_ST_OK;
    return CAN_ST_OK;
}

void can_tx_done(CanController *c)
{
    if (!c) return;
    c->tx_pending = 0;
}
