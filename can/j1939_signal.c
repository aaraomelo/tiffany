/*
 * j1939_signal.c — extracao / signedness / scale / NA
 */

#include "j1939_signal.h"
#include <limits.h>
#include <string.h>

uint64_t j1939_signal_na_pattern(uint8_t nbytes)
{
    if (nbytes == 0 || nbytes > 8) return 0;
    if (nbytes == 8) return ~0ULL;
    return (1ULL << (nbytes * 8)) - 1ULL;
}

static uint64_t extract_le(const uint8_t *p, uint8_t nbytes)
{
    uint64_t v = 0;
    for (uint8_t i = 0; i < nbytes; i++)
        v |= (uint64_t)p[i] << (8 * i);
    return v;
}

static uint64_t extract_be(const uint8_t *p, uint8_t nbytes)
{
    uint64_t v = 0;
    for (uint8_t i = 0; i < nbytes; i++)
        v = (v << 8) | p[i];
    return v;
}

static int64_t sign_extend(uint64_t v, uint8_t nbits)
{
    if (nbits == 0 || nbits >= 64) return (int64_t)v;
    uint64_t sign = 1ULL << (nbits - 1);
    if (v & sign) {
        uint64_t mask = ~((1ULL << nbits) - 1ULL);
        return (int64_t)(v | mask);
    }
    return (int64_t)v;
}

int j1939_signal_extract(const uint8_t *payload, uint16_t payload_len,
                         const J1939Interp *it,
                         int64_t *raw_out, int *status)
{
    if (!payload || !it || !it->sig || !raw_out || !status)
        return J1939_SIG_ERR;
    *status = J1939_SIG_ERR;

    const J1939Signal *s = it->sig;
    if (s->length < 1 || s->length > 8) return J1939_SIG_ERR;
    if ((uint16_t)s->start_byte + s->length > payload_len) return J1939_SIG_ERR;

    const uint8_t *p = payload + s->start_byte;
    uint64_t bits = it->little_endian ? extract_le(p, s->length)
                                      : extract_be(p, s->length);
    uint8_t nbits = (uint8_t)(s->length * 8);

    /* NA / error patterns (comparados ao padrao de bits do campo) */
    if (it->na_bits && bits == it->na_value) {
        *raw_out = (int64_t)bits;
        *status = J1939_SIG_NA;
        return J1939_SIG_OK;
    }
    if (it->err_bits && bits == it->err_value) {
        *raw_out = (int64_t)bits;
        *status = J1939_SIG_ERROR_RANGE;
        return J1939_SIG_OK;
    }

    if (it->is_signed)
        *raw_out = sign_extend(bits, nbits);
    else
        *raw_out = (int64_t)bits;

    *status = J1939_SIG_OK;
    return J1939_SIG_OK;
}

int j1939_signal_physical(int64_t raw, const J1939Interp *it, int32_t *phys_out)
{
    if (!it || !it->sig || !phys_out) return J1939_SIG_ERR;
    if (it->sig->scale_den == 0) return J1939_SIG_ERR;

    /* physical = raw * num / den + offset */
    int64_t num = raw * (int64_t)it->sig->scale_num
                + (int64_t)it->sig->offset * (int64_t)it->sig->scale_den;
    int64_t q = num / (int64_t)it->sig->scale_den;
    if (q > INT32_MAX || q < INT32_MIN) return J1939_SIG_ERR;
    *phys_out = (int32_t)q;
    return J1939_SIG_OK;
}
