/*
 * j1939.c — decodificacao J1939 (ID -> PGN/SPN -> grandeza)
 */

#include "j1939.h"
#include <limits.h>

uint8_t j1939_priority(uint32_t id) { return (uint8_t)((id >> 26) & 0x7); }
uint8_t j1939_reserved(uint32_t id) { return (uint8_t)((id >> 25) & 0x1); }
uint8_t j1939_dp(uint32_t id)       { return (uint8_t)((id >> 24) & 0x1); }
uint8_t j1939_pf(uint32_t id)       { return (uint8_t)((id >> 16) & 0xff); }
uint8_t j1939_ps(uint32_t id)       { return (uint8_t)((id >> 8) & 0xff); }
uint8_t j1939_sa(uint32_t id)       { return (uint8_t)(id & 0xff); }

int j1939_is_pdu1(uint32_t id)
{
    return j1939_pf(id) < 240;
}

uint8_t j1939_da(uint32_t id)
{
    if (j1939_is_pdu1(id))
        return j1939_ps(id);
    return 0xFF;
}

uint32_t j1939_pgn(uint32_t id)
{
    uint32_t dp = j1939_dp(id);
    uint32_t pf = j1939_pf(id);
    uint32_t ps = j1939_ps(id);
    if (pf < 240)
        return (dp << 16) | (pf << 8);
    return (dp << 16) | (pf << 8) | ps;
}

int j1939_frame_valid(const CanFrame *f)
{
    if (!f) return 0;
    if (!f->extended) return 0;                 /* J1939 usa 29-bit */
    if (f->rtr > 1) return 0;
    if (f->dlc > CAN_MAX_DLC) return 0;
    if (f->id & ~0x1FFFFFFFu) return 0;
    return 1;
}

const J1939Signal J1939_SPN_110 = {
    .pgn = 65262,
    .spn = 110,
    .start_byte = 0,
    .length = 2,
    .offset = -40,
    .scale_num = 1,
    .scale_den = 32,
    .name = "Engine Coolant Temperature",
    .unit = "deg C"
};

int j1939_read_raw(const CanFrame *f, const J1939Signal *sig, uint64_t *raw_out)
{
    if (!f || !sig || !raw_out) return -1;
    if (sig->length < 1 || sig->length > 8) return -1;
    if ((unsigned)sig->start_byte + sig->length > f->dlc) return -1;

    uint64_t raw = 0;
    for (uint8_t i = 0; i < sig->length; i++)
        raw |= (uint64_t)f->data[sig->start_byte + i] << (8 * i);  /* LE */
    *raw_out = raw;
    return 0;
}

/*
 * physical = (raw * num + offset * den) / den
 * Numerador em __int128 para caber raw de ate 8 bytes.
 */
int j1939_scale_rational(uint64_t raw, const J1939Signal *sig,
                         int64_t *num_out, uint32_t *den_out)
{
    if (!sig || !num_out || !den_out) return -1;
    if (sig->scale_den == 0) return -1;

    __int128 num = (__int128)raw * (__int128)sig->scale_num
                 + (__int128)sig->offset * (__int128)sig->scale_den;

    /* se nao couber em int64, recusa (contrato do modelo atual) */
    if (num > (__int128)INT64_MAX || num < (__int128)INT64_MIN)
        return -1;

    *num_out = (int64_t)num;
    *den_out = sig->scale_den;
    return 0;
}

int j1939_scale_i32(uint64_t raw, const J1939Signal *sig, int32_t *physical_out)
{
    int64_t num;
    uint32_t den;
    if (j1939_scale_rational(raw, sig, &num, &den) != 0) return -1;
    int64_t q = num / (int64_t)den;   /* trunc toward zero */
    if (q > INT32_MAX || q < INT32_MIN) return -1;
    *physical_out = (int32_t)q;
    return 0;
}
