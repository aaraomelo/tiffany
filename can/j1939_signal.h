/*
 * j1939_signal.h — interpretacao numerica de sinais J1939
 *
 * Contrato:
 *   j1939.c           = gramatica (frame -> campos)
 *   j1939_tp.c        = transporte
 *   j1939_catalog.c   = metadados (somente dados)
 *   j1939_signal.c    = extracao + signedness + scale + NA  (esta camada)
 *   can_micro.c       = composicao
 *   micro.c           = execucao
 *
 * NAO altera camadas congeladas.
 * O catalogo continua sem logica de execucao.
 */

#ifndef J1939_SIGNAL_H
#define J1939_SIGNAL_H

#include "j1939.h"
#include <stdint.h>

enum {
    J1939_SIG_OK = 0,
    J1939_SIG_ERR = -1,
    J1939_SIG_NA = 1,       /* not available */
    J1939_SIG_ERROR_RANGE = 2  /* error indicator */
};

/* metadados de interpretacao (fora do catalogo de protocolo) */
typedef struct {
    const J1939Signal *sig;   /* metadados do catalogo (ou estaticos) */
    uint8_t  is_signed;       /* 0 = unsigned, 1 = two's complement no campo */
    uint8_t  little_endian;   /* 1 = LE (padrao J1939) */
    uint64_t na_value;        /* padrao "not available"; 0 = desabilitado se na_bits==0 */
    uint8_t  na_bits;         /* largura do padrao NA em bits (0 = sem NA) */
    uint64_t err_value;       /* padrao error; 0 + err_bits==0 = desabilitado */
    uint8_t  err_bits;
} J1939Interp;

/*
 * Extrai valor bruto do payload (byte-aligned via sig->start_byte/length).
 * Aplica endianess e signedness ao campo de (length*8) bits.
 * *status = J1939_SIG_OK | NA | ERROR_RANGE | ERR
 * *raw_out = valor numerico (unsigned bit pattern ou sinal estendido em int64 via cast)
 */
int j1939_signal_extract(const uint8_t *payload, uint16_t payload_len,
                         const J1939Interp *it,
                         int64_t *raw_out, int *status);

/*
 * physical = raw * num/den + offset   (aritmetica assinada int64)
 * Usa scale do J1939Signal apontado por it->sig.
 */
int j1939_signal_physical(int64_t raw, const J1939Interp *it, int32_t *phys_out);

/* helpers de padrao J1939: NA = todos os bits 1 no campo de n bytes */
uint64_t j1939_signal_na_pattern(uint8_t nbytes);

#endif /* J1939_SIGNAL_H */
