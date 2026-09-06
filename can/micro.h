/*
 * micro.h — interface publica do Microprocessador Fractal (modelo de referencia)
 *
 * Dependencia: api.h (ISA canonica)
 *
 * Uso:
 *   #include "micro.h"
 *   cc -DMICRO_AS_LIB -c micro.c
 *   cc can_micro.c micro.o -lm -o can_micro
 *
 *   ou:
 *   cc -DMICRO_AS_LIB -I. can_micro.c micro.c -lm -o can_micro
 */

#ifndef MICRO_H
#define MICRO_H

#include "api.h"
#include <stdint.h>

/* ---- memoria / maquina ---- */
#define MICRO_MEM_WORDS  4096u
#define MICRO_ADDR_MASK  0xFFFFFFu

typedef struct {
    u64  word[MICRO_MEM_WORDS];
    u64  canal;
    u64  pc;
} Maquina;

void maq_clear(Maquina *M);
u64  addr_fisico(u64 addr24);

/* MOVE: primitiva de transferencia de estado */
u64  MOVE(Maquina *M, u64 destino, int sentido);  /* +1 LOAD, -1 STORE, 0 JMP */
u64  LOAD(Maquina *M, u64 addr);
void STORE(Maquina *M, u64 addr);
void JMP(Maquina *M, u64 dest_pc);
void STORE_VAL(Maquina *M, u64 addr, u64 val);

/* DIV (nao esta em api.h) */
u64  DIV(u64 num, u64 den, u64 *rem);
u64  DIVQ(u64 a, u64 b);

/* instrucao 80-bit */
enum {
    IOP_HALT = 0,
    IOP_ADD  = 1,
    IOP_SUB  = 2,
    IOP_MUL  = 3,
    IOP_DIV  = 4,
    IOP_AND  = 5,
    IOP_OR   = 6,
    IOP_XOR  = 7,
    IOP_NOT  = 8,
    IOP_MOV  = 9,
    IOP_JMP  = 10
};

typedef struct {
    uint32_t addr_a;
    uint32_t addr_b;
    uint8_t  op;
    uint32_t addr_r;
} Instr80;

void encode80(const Instr80 *I, uint8_t out[10]);
void decode80(const uint8_t in[10], Instr80 *I);
int  exec80(Maquina *M, const Instr80 *I, int *jumped);
int  roda80(Maquina *M, const uint8_t *prog, int nbytes, int *passos);

#endif /* MICRO_H */
