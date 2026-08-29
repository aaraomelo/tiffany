/* arena.h — layout DISCO partilhado (interpretar.c, erg metal, canal).
 * fis:def:coord2 — horizontal OFF_* (célula), vertical k (visita no laço).
 * fis:thm:zeroinf — NULO_DISCO=8: andar 0–∞; rodata ≥65536 → RODATA_RELOC+. */
#ifndef SHELL_ARENA_H
#define SHELL_ARENA_H

#define ARENA_SIZE 65536
#define OFF_NIN  24576
#define OFF_NOUT 24578
#define OFF_SEQ  24580
#define OFF_GSUM 24582   /* 2 bytes: total de incrementos G (host) */
#define OFF_IN   256
#define OFF_OUT  16384
#define CAP      8192
#define OFF_G    24608   /* G_visit(x): visitas host — NÃO é |π^{-1}(x)| sem hipótese */
#define G_CELLS  256

/* rodata wasm ≥65536 remapeia para 65400+ (wasm_erg.c); tag semeada pelo host. */
#define RODATA_RELOC_BASE 65400
#define RODATA_TAG (RODATA_RELOC_BASE + 8)  /* wasm 65544 → arena 65408 */

#endif
