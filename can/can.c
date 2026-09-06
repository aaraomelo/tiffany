/*
 * can.c — controlador CAN (modelo de referencia, auditado)
 */

#include "can.h"
#include <string.h>

void can_init(CanController *c)
{
    memset(c, 0, sizeof(*c));
    c->status = CAN_ST_EMPTY;
    c->err_state = CAN_ERR_ACTIVE;
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
    if (c->err_state == CAN_ERR_BUS_OFF) return CAN_ST_BUS_OFF;
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
    if (c->err_state == CAN_ERR_BUS_OFF) return CAN_ST_BUS_OFF;
    if (!can_frame_valid(in)) return CAN_ST_ERR;
    if (c->tx_pending) {
        c->status = CAN_ST_FULL;
        return CAN_ST_FULL;
    }
    c->tx = *in;
    c->tx_pending = 1;
    c->lost_arbitration = 0;
    c->ack_seen = 0;
    c->status = CAN_ST_OK;
    return CAN_ST_OK;
}

int can_inject_rx(CanController *c, const CanFrame *f)
{
    if (!c || !f) return CAN_ST_ERR;
    if (c->err_state == CAN_ERR_BUS_OFF) return CAN_ST_BUS_OFF;
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
    c->lost_arbitration = 0;
}

int can_arb_len(const CanFrame *f)
{
    if (!f) return 0;
    return f->extended ? 29 : 11;
}

int can_arb_bit(const CanFrame *f, int k)
{
    int n = can_arb_len(f);
    if (!f || k < 0 || k >= n) return -1;
    return (int)((f->id >> (n - 1 - k)) & 1u);
}

int can_arbitrate(const CanFrame *a, const CanFrame *b)
{
    if (!a || !b) return 0;
    int na = can_arb_len(a);
    int nb = can_arb_len(b);
    int n = na < nb ? na : nb;
    for (int k = 0; k < n; k++) {
        int ba = can_arb_bit(a, k);
        int bb = can_arb_bit(b, k);
        if (ba != bb) {
            if (ba == 0 && bb == 1) return -1;
            if (ba == 1 && bb == 0) return  1;
        }
    }
    if (na == nb) {
        if (a->id < b->id) return -1;
        if (a->id > b->id) return  1;
        return 0; /* mesmo ID: empate */
    }
    /* misturado 11/29: apos empate nos 11 bits, extended continua — simplificado:
       quem tem menos bits ja terminou; modelo trata como empate na regiao comum */
    return 0;
}

uint16_t can_crc15(const uint8_t *bits, int nbits)
{
    uint16_t crc = 0;
    for (int i = 0; i < nbits; i++) {
        uint16_t bit = bits[i] & 1u;
        uint16_t msb = (crc >> 14) & 1u;
        crc <<= 1;
        if (msb ^ bit)
            crc ^= 0x4599u;
        crc &= 0x7FFFu;
    }
    return crc;
}

uint16_t can_frame_crc(const CanFrame *f)
{
    if (!f) return 0;
    uint8_t bits[29 + 1 + 4 + 64];
    int n = 0;
    int alen = can_arb_len(f);
    for (int k = 0; k < alen; k++)
        bits[n++] = (uint8_t)can_arb_bit(f, k);
    bits[n++] = f->rtr & 1u;
    bits[n++] = (f->dlc >> 3) & 1u;
    bits[n++] = (f->dlc >> 2) & 1u;
    bits[n++] = (f->dlc >> 1) & 1u;
    bits[n++] = (f->dlc >> 0) & 1u;
    for (int i = 0; i < f->dlc && i < CAN_MAX_DLC; i++)
        for (int b = 7; b >= 0; b--)
            bits[n++] = (uint8_t)((f->data[i] >> b) & 1u);
    return can_crc15(bits, n);
}

/*
 * Bit stuffing: apos 5 bits iguais, insere o complementar.
 * O stuff bit inicia uma nova corrida (run=1, last=stuff).
 */
int can_bit_stuff(const uint8_t *in, int nbits, uint8_t *out, int out_cap)
{
    if (!in || !out || nbits < 0) return -1;
    int n = 0, run = 0;
    uint8_t last = 2; /* sentinela != 0 e != 1 */
    for (int i = 0; i < nbits; i++) {
        uint8_t b = in[i] & 1u;
        if (n >= out_cap) return -1;
        out[n++] = b;
        if (b == last)
            run++;
        else {
            run = 1;
            last = b;
        }
        if (run == 5) {
            if (n >= out_cap) return -1;
            uint8_t s = (uint8_t)(1u - b);
            out[n++] = s;
            last = s;
            run = 1;
        }
    }
    return n;
}

int can_bit_destuff(const uint8_t *in, int nbits, uint8_t *out, int out_cap)
{
    if (!in || !out || nbits < 0) return -1;
    int n = 0, run = 0;
    uint8_t last = 2;
    for (int i = 0; i < nbits; i++) {
        uint8_t b = in[i] & 1u;
        if (n >= out_cap) return -1;
        out[n++] = b;
        if (b == last)
            run++;
        else {
            run = 1;
            last = b;
        }
        if (run == 5) {
            /* proximo deve ser stuff bit (complementar) e e descartado */
            i++;
            if (i >= nbits) return -1;
            uint8_t s = in[i] & 1u;
            if (s == b) return -1; /* stuff error */
            last = s;
            run = 1;
        }
        if (run > 5) return -1;
    }
    return n;
}

void can_update_err_state(CanController *c)
{
    if (!c) return;
    if (c->tec >= 256) {
        c->err_state = CAN_ERR_BUS_OFF;
        c->status = CAN_ST_BUS_OFF;
    } else if (c->tec >= 128 || c->rec >= 128) {
        c->err_state = CAN_ERR_PASSIVE;
    } else {
        c->err_state = CAN_ERR_ACTIVE;
    }
}

void can_error_tx(CanController *c)
{
    if (!c) return;
    /* ISO: +8 por erro de TX; permite chegar a 256 */
    uint32_t t = (uint32_t)c->tec + 8u;
    c->tec = (t > 255u) ? 256u : (uint16_t)t;
    can_update_err_state(c);
}

void can_error_rx(CanController *c)
{
    if (!c) return;
    if (c->rec < 255) c->rec = (uint16_t)(c->rec + 1);
    can_update_err_state(c);
}

void can_success_tx(CanController *c)
{
    if (!c) return;
    if (c->tec > 0) c->tec--;
    can_update_err_state(c);
}

void can_recover(CanController *c)
{
    if (!c) return;
    c->tec = 0;
    c->rec = 0;
    c->err_state = CAN_ERR_ACTIVE;
    c->status = CAN_ST_EMPTY;
    c->tx_pending = 0;
    c->rx_pending = 0;
    c->lost_arbitration = 0;
    c->ack_seen = 0;
}
