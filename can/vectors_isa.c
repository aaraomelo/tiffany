/*
 * vectors_isa.c — vetores determinísticos de equivalência ISA
 *
 * Oráculo: micro.c
 * Futuro:  micro.sv deve reproduzir estado final idêntico.
 *
 *   cc -O2 -std=c11 -Wall -DMICRO_AS_LIB -I. vectors_isa.c micro.c -lm -o vectors_isa
 */

#include "micro.h"
#include <stdio.h>
#include <string.h>
#include <stdint.h>

static int g_fail = 0;

static void enc(uint8_t *p, uint32_t a, uint32_t b, uint8_t op, uint32_t r)
{
    Instr80 I = { .addr_a = a, .addr_b = b, .op = op, .addr_r = r };
    encode80(&I, p);
}

static void put_init(Maquina *M, const uint32_t *addr, const u64 *val, int n)
{
    for (int i = 0; i < n; i++)
        STORE_VAL(M, addr[i], val[i]);
}

static void run(const char *name, const uint8_t *prog, int plen,
                const uint32_t *ia, const u64 *iv, int ni,
                uint32_t raddr, u64 rexp, u64 pc_exp, int max_steps)
{
    printf("  %-6s ", name);
    Maquina M;
    maq_clear(&M);
    put_init(&M, ia, iv, ni);
    int steps = 0;
    int halted = roda80(&M, prog, plen, &steps);
    u64 got = LOAD(&M, raddr);
    int ok = (got == rexp) && halted && (steps <= max_steps) && (M.pc == pc_exp);
    if (!ok) {
        fprintf(stderr, "FAIL %s got=%llu exp=%llu pc=%llu/%llu steps=%d halt=%d\n",
                name, (unsigned long long)got, (unsigned long long)rexp,
                (unsigned long long)M.pc, (unsigned long long)pc_exp, steps, halted);
        g_fail++;
        printf("FAIL\n");
        return;
    }
    printf("OK  result=%llu pc=%llu steps=%d\n",
           (unsigned long long)got, (unsigned long long)M.pc, steps);
}

int main(void)
{
    printf("=== vetores de equivalencia ISA (oraculo micro.c) ===\n\n");
    uint8_t p[80];

    /* V01 ADD */
    enc(p, 0, 1, IOP_ADD, 2); enc(p+10, 0, 0, IOP_HALT, 0);
    { uint32_t a[] = {0,1}; u64 v[] = {15,27};
      run("V01", p, 20, a, v, 2, 2, 42, 10, 2); }

    /* V02 SUB */
    enc(p, 0, 1, IOP_SUB, 2); enc(p+10, 0, 0, IOP_HALT, 0);
    { uint32_t a[] = {0,1}; u64 v[] = {100,42};
      run("V02", p, 20, a, v, 2, 2, 58, 10, 2); }

    /* V03 MUL */
    enc(p, 0, 1, IOP_MUL, 2); enc(p+10, 0, 0, IOP_HALT, 0);
    { uint32_t a[] = {0,1}; u64 v[] = {12,12};
      run("V03", p, 20, a, v, 2, 2, 144, 10, 2); }

    /* V04 DIV */
    enc(p, 0, 1, IOP_DIV, 2); enc(p+10, 0, 0, IOP_HALT, 0);
    { uint32_t a[] = {0,1}; u64 v[] = {100,7};
      run("V04", p, 20, a, v, 2, 2, 14, 10, 2); }

    /* V05 MOV / LOAD-STORE */
    enc(p, 0, 0, IOP_MOV, 3); enc(p+10, 0, 0, IOP_HALT, 0);
    { uint32_t a[] = {0}; u64 v[] = {0xDEADBEEFULL};
      run("V05", p, 20, a, v, 1, 3, 0xDEADBEEFULL, 10, 2); }

    /* V06 JMP */
    enc(p+0, 20, 0, IOP_JMP, 0);
    enc(p+10, 0, 1, IOP_ADD, 5);
    enc(p+20, 2, 0, IOP_MOV, 4);
    enc(p+30, 0, 0, IOP_HALT, 0);
    { uint32_t a[] = {0,1,2}; u64 v[] = {1,1,99};
      run("V06", p, 40, a, v, 3, 4, 99, 30, 3); }

    /* V07 HALT only */
    enc(p, 0, 0, IOP_HALT, 0);
    { uint32_t a[] = {0}; u64 v[] = {0};
      run("V07", p, 10, a, v, 0, 0, 0, 0, 1); }

    /* V08 sequence (10+5)*3=45 */
    enc(p+0, 0, 1, IOP_ADD, 3);
    enc(p+10, 3, 2, IOP_MUL, 4);
    enc(p+20, 0, 0, IOP_HALT, 0);
    { uint32_t a[] = {0,1,2}; u64 v[] = {10,5,3};
      run("V08", p, 30, a, v, 3, 4, 45, 20, 3); }

    /* V09 SPN110 (1920-1280)/32=20 */
    enc(p+0, 0, 1, IOP_SUB, 3);
    enc(p+10, 3, 2, IOP_DIV, 4);
    enc(p+20, 0, 0, IOP_HALT, 0);
    { uint32_t a[] = {0,1,2}; u64 v[] = {1920,1280,32};
      run("V09", p, 30, a, v, 3, 4, 20, 20, 3); }

    /* V10 XOR chain on DISCO data */
    enc(p+0, 0, 1, IOP_XOR, 5);
    enc(p+10, 5, 2, IOP_XOR, 6);
    enc(p+20, 0, 0, IOP_HALT, 0);
    { uint32_t a[] = {0,1,2}; u64 v[] = {0xF0,0x0F,0xFF};
      run("V10", p, 30, a, v, 3, 6, 0, 20, 3); }

    printf("\n");
    if (g_fail) {
        printf("[X] %d falha(s)\n", g_fail);
        return 1;
    }
    printf("[OK] 10 vetores — oraculo micro.c pronto para micro.sv\n");
    printf("micro.c(programa, estado_in) == micro.sv(programa, estado_in)\n");
    return 0;
}
