/*
 * demo_isa_disco.c — tese: protocolo como programa na ISA
 *
 * CAN/J1939 NAO estao na ISA. O DISCO carrega um programa 80-bit que:
 *   1) le raw mapeado em memoria (simula payload CAN)
 *   2) aplica (raw - 1280) / 32   // SPN 110: scale 1/32, offset -40
 *   3) grava grandeza em memoria
 *
 * Comparacao:
 *   C reference (j1939_signal / catalog)  ==  programa ISA  ==  20 C
 *
 * Nao altera .snapshot/arch-closed/
 *
 *   cc -O2 -std=c11 -Wall -DMICRO_AS_LIB -I. \
 *      demo_isa_disco.c micro.c j1939.c j1939_catalog.c j1939_signal.c can.c -lm \
 *      -o demo_isa_disco
 */

#include "micro.h"
#include "j1939.h"
#include "j1939_catalog.h"
#include "j1939_signal.h"

#include <stdio.h>
#include <stdint.h>
#include <string.h>

/* Layout do DISCO de dados (word[]) — memoria mapeada */
enum {
    D_RAW   = 0,   /* entrada: raw SPN 110 = 1920 */
    D_OFF   = 1,   /* constante 1280 = 40*32 */
    D_DEN   = 2,   /* constante 32 */
    D_TMP   = 3,   /* raw - off */
    D_PHYS  = 4,   /* resultado fisico */
    D_SCRATCH = 16
};

/* Programa no "DISCO de instrucoes" (imagem 80-bit) */
static int build_scale_program(uint8_t *prog, int cap)
{
    /* SUB D_RAW, D_OFF -> D_TMP
       DIV D_TMP, D_DEN -> D_PHYS
       HALT */
    Instr80 is[] = {
        { .addr_a = D_RAW, .addr_b = D_OFF, .op = IOP_SUB, .addr_r = D_TMP  },
        { .addr_a = D_TMP, .addr_b = D_DEN, .op = IOP_DIV, .addr_r = D_PHYS },
        { .addr_a = 0,     .addr_b = 0,     .op = IOP_HALT,.addr_r = 0      }
    };
    int n = (int)(sizeof is / sizeof is[0]);
    if (cap < n * 10) return -1;
    for (int i = 0; i < n; i++)
        encode80(&is[i], prog + i * 10);
    return n * 10;
}

static int build_minimal_add(uint8_t *prog, int cap)
{
    /* ADD D_RAW, D_OFF -> D_PHYS ; HALT  (teste do ciclo) */
    Instr80 is[] = {
        { .addr_a = D_RAW, .addr_b = D_OFF, .op = IOP_ADD, .addr_r = D_PHYS },
        { .addr_a = 0, .addr_b = 0, .op = IOP_HALT, .addr_r = 0 }
    };
    if (cap < 20) return -1;
    encode80(&is[0], prog);
    encode80(&is[1], prog + 10);
    return 20;
}

/* Referencia C (auditoria) — NAO usada pelo programa ISA */
static int32_t c_reference_spn110(uint64_t raw_in)
{
    const J1939Signal *sig = j1939_catalog_find(65262, 110);
    if (!sig) return -9999;
    uint8_t pl[8] = {
        (uint8_t)(raw_in & 0xFF),
        (uint8_t)((raw_in >> 8) & 0xFF),
        0,0,0,0,0,0
    };
    J1939Interp it = {
        .sig = sig, .is_signed = 0, .little_endian = 1,
        .na_value = 0xFFFF, .na_bits = 16, .err_bits = 0
    };
    int64_t raw = 0; int st = 0;
    if (j1939_signal_extract(pl, 8, &it, &raw, &st) != 0 || st != J1939_SIG_OK)
        return -9998;
    int32_t phys = 0;
    if (j1939_signal_physical(raw, &it, &phys) != 0)
        return -9997;
    return phys;
}

int main(void)
{
    printf("=== ISA DISCO: protocolo como programa ===\n\n");
    printf("Tese: a ISA nao conhece J1939;\n");
    printf("      o DISCO carrega um programa que realiza a transformacao.\n\n");

    /* ---------- [1] ciclo minimo ADD ---------- */
    {
        printf("[1] CICLO MINIMO (ADD)\n");
        Maquina M;
        maq_clear(&M);
        STORE_VAL(&M, D_RAW, 15);
        STORE_VAL(&M, D_OFF, 27);

        uint8_t prog[32];
        int n = build_minimal_add(prog, 32);
        int steps = 0;
        roda80(&M, prog, n, &steps);

        u64 r = LOAD(&M, D_PHYS);
        printf("    DISCO: word[RAW]=15 word[OFF]=27\n");
        printf("    programa: ADD -> PHYS\n");
        printf("    steps=%d  PHYS=%llu  esperado=42  %s\n\n",
               steps, (unsigned long long)r, r == 42 ? "OK" : "FAIL");
        if (r != 42) return 1;
    }

    /* ---------- [2] programa SPN 110 no DISCO ---------- */
    {
        printf("[2] PROGRAMA ISA = SPN 110 (scale no DISCO)\n");
        printf("    memoria mapeada:\n");
        printf("      D_RAW = 1920   (como se viesse do payload CAN)\n");
        printf("      D_OFF = 1280   (40 * 32)\n");
        printf("      D_DEN = 32\n");
        printf("    programa no DISCO:\n");
        printf("      SUB RAW, OFF -> TMP\n");
        printf("      DIV TMP, DEN -> PHYS\n");
        printf("      HALT\n\n");

        Maquina M;
        maq_clear(&M);
        STORE_VAL(&M, D_RAW, 1920);
        STORE_VAL(&M, D_OFF, 1280);
        STORE_VAL(&M, D_DEN, 32);

        uint8_t prog[64];
        int n = build_scale_program(prog, 64);
        int steps = 0;
        int halted = roda80(&M, prog, n, &steps);

        u64 phys_isa = LOAD(&M, D_PHYS);
        int32_t phys_c = c_reference_spn110(1920);

        printf("    FETCH/DECODE/EXECUTE: steps=%d halted=%d\n", steps, halted);
        printf("    ISA  PHYS = %llu deg C\n", (unsigned long long)phys_isa);
        printf("    C ref     = %d deg C\n", phys_c);
        printf("    esperado  = 20 deg C\n");

        int ok = (phys_isa == 20 && phys_c == 20);
        printf("    C reference == ISA program == 20 C   %s\n\n", ok ? "OK" : "FAIL");
        if (!ok) return 1;
    }

    /* ---------- [3] o que a ISA NAO fez ---------- */
    {
        printf("[3] FRONTEIRA\n");
        printf("    ISA executou: LOAD / SUB / DIV / STORE / HALT\n");
        printf("    ISA NAO executou: opcode CAN, opcode J1939, opcode SPN\n");
        printf("    j1939_*() usado so como referencia C de auditoria\n");
        printf("    transformacao fisica: 100%% programa no DISCO\n\n");
    }

    printf("=== PROVA: PASS ===\n");
    printf("O Microprocessador Fractal nao possui J1939 na ISA;\n");
    printf("ele executa um programa que implementa a transformacao.\n");
    return 0;
}
