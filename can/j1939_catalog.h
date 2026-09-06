/*
 * j1939_catalog.h — catálogo de sinais J1939 (PGN/SPN)
 *
 * Camada NOVA. Nao altera:
 *   micro.c, can.c, can_bus.c, j1939.c, j1939_tp.c  (congelados)
 *
 *   j1939.c        = gramatica do protocolo (ID, PGN, raw, escala generica)
 *   j1939_catalog  = significado dos sinais (tabela PGN/SPN)
 *
 * O catálogo nao conhece o micro nem o CAN bus.
 */

#ifndef J1939_CATALOG_H
#define J1939_CATALOG_H

#include "j1939.h"
#include <stddef.h>

/* procura sinal por (pgn, spn); retorna ponteiro estatico ou NULL */
const J1939Signal *j1939_catalog_find(uint32_t pgn, uint32_t spn);

/* procura por SPN (primeiro match); NULL se nao houver */
const J1939Signal *j1939_catalog_find_spn(uint32_t spn);

/* numero de entradas no catalogo */
size_t j1939_catalog_count(void);

/* entrada por indice [0, count) */
const J1939Signal *j1939_catalog_at(size_t index);

#endif /* J1939_CATALOG_H */
