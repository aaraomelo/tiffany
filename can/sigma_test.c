/* sigma_test.c — Teste exaustivo Sigma(x) = popcount(x) na ISA ERG-64
 *
 * Implementa a máquina ERG-64 inline e executa sigma.bin diretamente,
 * evitando o overhead de spawn de processo para cada valor.
 *
 * Correcoes em relacao a versao anterior:
 *  1. rodar() NAO apaga mais a memoria nem os registos —
 *     quem chama prepara o estado e depois corre.
 *  2. ula_add() corrigido: o componente e usa b.e (nao b.total).
 *  3. opcodes usam #include "../lib/isa.h" — a fonte canonica,
 *     nao constantes magicas.
 *  4. reset_cpu() extraido do rodar() para reutilizacao no main().
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#include "../lib/isa.h"

typedef struct { uint8_t total, e; } Word;

static Word ula_add(Word a, Word b){
    return (Word){(uint8_t)((int)a.total + (int)b.total),
                  (uint8_t)((int)a.e   + (int)b.e)};
}
static Word ula_and(Word a, Word b){
    Word n = {(uint8_t)~(a.total & b.total), (uint8_t)~(a.e & b.e)};
    return (Word){(uint8_t)~(n.total & n.total), (uint8_t)~(n.e & n.e)};
}
static int zero_w(Word w){ return w.total == 0 && w.e == 0; }

static unsigned char prog[4096];
static int prog_len;
static Word mem[20][2];
static Word A, B, R;
static unsigned pc;
static unsigned char flags;

static void reset_cpu(void){
    A.total = A.e = B.total = B.e = R.total = R.e = 0;
    pc = 0;
    flags = 0;
}

static void mem_grava(unsigned slot, Word w){
    mem[slot][0] = w;
}
static Word mem_le(unsigned slot){
    return mem[slot][0];
}

static int passo(void){
    if(pc >= (unsigned)prog_len) return 0;
    unsigned char op = prog[pc++];
    switch(op){
    case OP_HALT:  return 0;
    case OP_LOAD: { unsigned slot = prog[pc] | (prog[pc+1]<<8); pc+=2; B = A; A = mem_le(slot); break; }
    case OP_STORE:{ unsigned slot = prog[pc] | (prog[pc+1]<<8); pc+=2; mem_grava(slot, R); break; }
    case OP_ADD:   R = ula_add(A, B); break;
    case OP_AND:   R = ula_and(A, B); break;
    case OP_CMP: { unsigned char f = 0; if(zero_w(A) && zero_w(B)) f |= FL_ZERO; flags = f; break; }
    case OP_JZ: { int rel = (int)(int16_t)(prog[pc] | (prog[pc+1]<<8)); pc+=2; if(flags & FL_ZERO) pc = (unsigned)((int)pc + rel); break; }
    case OP_INC: { unsigned slot = prog[pc] | (prog[pc+1]<<8); pc+=2; Word w = mem_le(slot); w = ula_add(w, (Word){1,0}); mem_grava(slot, w); R = w; break; }
    default: return 0;
    }
    return 1;
}

static long rodar(long teto){
    long n = 0;
    while(passo() && ++n < teto);
    return n;
}

static int v_peso(uint8_t x){
    int n = 0;
    for(int i = 0; i < 8; i++) n += (x >> i) & 1;
    return n;
}

int main(void){
    FILE *f = fopen("sigma.bin", "rb");
    if(!f){ printf("ERRO: nao abriu sigma.bin\n"); return 2; }
    prog_len = (int)fread(prog, 1, sizeof prog, f);
    fclose(f);

    int pass = 0, fail = 0;
    for(int x = 0; x < 256; x++){
        memset(mem, 0, sizeof mem);

        mem[0][0]  = (Word){0, 0};
        mem[1][0]  = (Word){(uint8_t)x, 0};
        mem[2][0]  = (Word){0, 0};

        int masks[] = {1,2,4,8,16,32,64,128};
        for(int k = 0; k < 8; k++)
            mem[10+k][0] = (Word){(uint8_t)masks[k], 0};

        reset_cpu();

        rodar(1000);

        int result = (int)mem[2][0].total;
        int expected = v_peso((uint8_t)x);
        if(result == expected) pass++; else fail++;
    }

    printf("=== SIGMA ISA TEST ===\n");
    printf("casos: 256  pass: %d  fail: %d\n", pass, fail);
    if(fail == 0){
        printf("sigma.erg: CORRECTA\n");
        printf("sigma_test.exe: 256/256 PASS\n");
    } else {
        printf("sigma_test.exe: FALHA\n");
    }
    return fail == 0 ? 0 : 1;
}