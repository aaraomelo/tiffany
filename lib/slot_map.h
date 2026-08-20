/* slot_map.h — Regiões fixas de slots no .mem do banco (16 bytes por slot).
 *
 * Partilhado entre banco/sql.c, lib/rt_cf_slot.h e medidores. O mapa completo do SQL
 * continua em sql.c; aqui ficam só as fronteiras que a lib precisa de saber. */
#ifndef SLOT_MAP_H
#define SLOT_MAP_H

#ifndef RT_CF_MAX
#define RT_CF_MAX 48
#endif

/* Fração contínua (acumulador thm:operador): entre S_VIVO (≤1023) e S_LIN (4096).
 * Cada palavra: meta + até RT_CF_MAX termos + flag saturou = S_CF_STRIDE slots. */
#define S_CF         2048u
#define S_CF_STRIDE  (RT_CF_MAX + 2u)
#define S_CF_WORDS   8u
#define S_CF_END     (S_CF + S_CF_WORDS * S_CF_STRIDE)

/* Tamanho mínimo do .mem para a região FC (medidores locais). O sql.c usa mapa maior. */
#define S_CF_MEM_BYTES  ((unsigned long)S_CF_END * 16u)

static inline unsigned cf_slot_base(unsigned word_ix){
    return (unsigned)(S_CF + word_ix * S_CF_STRIDE);
}

#endif
