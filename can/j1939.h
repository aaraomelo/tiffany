/*
 * j1939.h — semântica J1939 sobre frames CAN
 *
 * Conhece ID 29-bit, PGN, SA/DA, SPN, escala.
 * NAO conhece o micro nem o controlador CAN fisico.
 *
 * Contrato de sinal:
 *   length ∈ [1, 8]  (payload CAN completo)
 *   raw    = uint64_t little-endian
 *
 * Escala (adaptador assinado, fora da ISA u64):
 *   physical = (raw * scale_num + offset * scale_den) / scale_den
 */

#ifndef J1939_H
#define J1939_H

#include "can.h"
#include <stdint.h>

/* campos do ID estendido J1939 (29 bits) */
uint8_t  j1939_priority(uint32_t id);
uint8_t  j1939_reserved(uint32_t id);
uint8_t  j1939_dp(uint32_t id);
uint8_t  j1939_pf(uint32_t id);
uint8_t  j1939_ps(uint32_t id);
uint8_t  j1939_sa(uint32_t id);
uint32_t j1939_pgn(uint32_t id);

int      j1939_is_pdu1(uint32_t id);
uint8_t  j1939_da(uint32_t id);   /* DA em PDU1; 0xFF se PDU2 */

/*
 * Frame J1939 valido (estrutural):
 *   f != NULL, extended == 1, rtr ∈ {0,1}, dlc ≤ 8, id ≤ 0x1FFFFFFF
 */
int j1939_frame_valid(const CanFrame *f);

typedef struct {
    uint32_t    pgn;
    uint32_t    spn;
    uint8_t     start_byte;   /* 0-based no payload */
    uint8_t     length;       /* bytes, 1..8 */
    int32_t     offset;
    uint32_t    scale_num;
    uint32_t    scale_den;
    const char *name;
    const char *unit;
} J1939Signal;

/* raw unsigned LE, ate 8 bytes */
int j1939_read_raw(const CanFrame *f, const J1939Signal *sig, uint64_t *raw_out);

int j1939_scale_i32(uint64_t raw, const J1939Signal *sig, int32_t *physical_out);

int j1939_scale_rational(uint64_t raw, const J1939Signal *sig,
                         int64_t *num_out, uint32_t *den_out);

extern const J1939Signal J1939_SPN_110;

#endif /* J1939_H */
