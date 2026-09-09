/* vinco_test.c — Teste exaustivo VINCO(A,B) = A - B na ISA ERG-64
 *
 * Implementa a máquina ERG-64 inline, monta vinco.erg, executa vinco.bin,
 * e compara contra referencia C.
 *
 * Semantica canonica (lib/isa.h, erg.c):
 *   VINCO: R = ula_sub(A, B) — sub face a face, Word8, mod 256
 *   flags = FL_ZERO se zero(R), senao 0
 *   FL_ZERO = 0x01 (zero se AMBOS componentes forem zero)
 *
 * MOVE_exec (erg.c): LOAD: B=A, A=mem[slot]; STORE: mem[slot]=R.
 * VINCO: R = A - B, flags = FL_ZERO se zero(R).
 *
 * Metodologia: mesma estrutura de sigma_test.c — monta o .erg, executa
 * na maquina ERG-64 inline, e compara contra referencia C.
 *
 * Testes: 65536 casos (256 x 256) exaustivos + 3 casos de protocolo.
 *
 * Uso: gcc -O2 -std=c11 vinco_test.c -o vinco_test.exe && ./vinco_test.exe
 */

#include <stdio.h>
#include <stdint.h>
#include <string.h>

typedef uint8_t  Word8;
typedef uint16_t Word;

#define FL_ZERO 0x01
#define FL_EQ   0x02
#define FL_LT   0x04

static Word  mem[16][2];
static Word8 flags;

static void mem_zera(void){
    memset(mem, 0, sizeof(mem));
    flags = 0;
}

static void mem_poe(unsigned slot, Word w){
    if(slot < 16) mem[slot][0] = w;
}

static Word mem_le(unsigned slot){
    return (slot < 16) ? mem[slot][0] : 0;
}

static void mem_grava(unsigned slot, Word w){
    if(slot < 16) mem[slot][0] = w;
}

static Word8 sub8(Word8 a, Word8 b){ return (Word8)(a - b); }

static int zero_w(Word w){ return w == 0; }

/* Opcode definitions (from erg.c ISA[]) */
enum { OP_HALT=0, OP_LOAD, OP_STORE, OP_ADD, OP_SUB, OP_AND, OP_OR, OP_XOR,
       OP_GOLD, OP_CMP, OP_JMP, OP_JZ, OP_JNZ,
       OP_FOLD, OP_LOADS, OP_NEGRO_OURO, OP_ESQUILO, OP_TROCA, OP_MARTELO,
       OP_ADD16, OP_SUB16, OP_CMP16, OP_MUL16, OP_ESPALHA, OP_STORE_IND,
       OP_VINCO, OP_INC };

struct ISA_Instr { const char *nome; int op; int arg; };
static const struct ISA_Instr ISA[] = {
    { "HALT",       OP_HALT,       0 },
    { "LOAD",       OP_LOAD,      2 },
    { "STORE",      OP_STORE,     2 },
    { "ADD",        OP_ADD,       0 },
    { "SUB",        OP_SUB,       0 },
    { "AND",        OP_AND,       0 },
    { "OR",         OP_OR,        0 },
    { "XOR",        OP_XOR,       0 },
    { "GOLD",       OP_GOLD,      0 },
    { "CMP",        OP_CMP,       0 },
    { "JMP",        OP_JMP,       2 },
    { "JZ",         OP_JZ,        2 },
    { "JNZ",        OP_JNZ,       2 },
    { "FOLD",       OP_FOLD,      0 },
    { "LOADS",      OP_LOADS,     2 },
    { "NEGRO_OURO", OP_NEGRO_OURO,0 },
    { "ESQUILO",    OP_ESQUILO,   0 },
    { "TROCA",      OP_TROCA,     0 },
    { "MARTELO",    OP_MARTELO,   0 },
    { "ADD16",      OP_ADD16,     0 },
    { "SUB16",      OP_SUB16,     0 },
    { "CMP16",      OP_CMP16,     0 },
    { "MUL16",      OP_MUL16,     0 },
    { "ESPALHA",    OP_ESPALHA,   0 },
    { "STORE_IND",  OP_STORE_IND, 0 },
    { "VINCO",      OP_VINCO,     0 },
    { "INC",        OP_INC,       2 },
};

static unsigned opc_nome(const char *s){
    unsigned i;
    for(i=0; i<sizeof(ISA)/sizeof(ISA[0]); i++)
        if(strcmp(ISA[i].nome, s)==0) return ISA[i].op;
    return 0xFF;
}

static unsigned prog[1024];
static unsigned prog_n;

static int monta(const char *path){
    FILE *f = fopen(path, "r");
    if(!f) return 1;
    prog_n = 0;
    char line[256];
    while(fgets(line, sizeof(line), f)){
        if(line[0]==';' || line[0]=='#') continue;
        char *p = line;
        while(*p==' '||*p=='\t') p++;
        if(*p=='\n'||*p=='\r'||*p==0) continue;
        char nome[64]; int arg1=-1, arg2=-1;
        int n = sscanf(p, "%63s %d %d", nome, &arg1, &arg2);
        unsigned op = opc_nome(nome);
        if(op==0xFF) continue;
        unsigned instr = op;
        if(n>=2) instr |= (arg1&0xFF)<<8;
        if(n>=3) instr |= (arg2&0xFF)<<16;
        if(prog_n<1024) prog[prog_n++] = instr;
    }
    fclose(f);
    return 0;
}

/* Executor ERG-64 inline — faithful to erg.c MOVE_exec semantics:
 *   LOAD: B=A, A=mem[slot]  (A holds VALUE, not slot index)
 *   STORE: mem[slot]=R
 *   VINCO: R = A - B, flags = FL_ZERO if zero(R)
 */
static unsigned pc;
static Word  R;
static Word  A;  /* A holds VALUE (like erg.c r->A) */
static Word  B;  /* B holds VALUE (like erg.c r->B) */

static void exec(unsigned n){
    for(unsigned passo=0; passo<n && pc<prog_n; passo++){
        unsigned instr = prog[pc];
        unsigned op = instr & 0xFF;
        unsigned arg1 = (instr>>8) & 0xFF;
        unsigned arg2 = (instr>>16) & 0xFF;
        pc++;
        switch(op){
        case OP_HALT: pc = prog_n; break;
        case OP_LOAD: {
            unsigned slot = arg1 | (arg2<<8);
            B = A;
            A = mem_le(slot);
            R = A;
            break;
        }
        case OP_STORE: {
            unsigned slot = arg1 | (arg2<<8);
            mem_grava(slot, R);
            break;
        }
        case OP_ADD: {
            Word Bval = mem_le((unsigned)A);
            R = (Word){ (Word8)(R+Bval), (Word8)(R+Bval) };
            break;
        }
        case OP_SUB: {
            Word Bval = mem_le((unsigned)A);
            R = (Word){ (Word8)(R-Bval), (Word8)(R-Bval) };
            break;
        }
        case OP_VINCO: {
            Word Bval = B;  /* VINCO uses register B directly, not mem_le(A) */
            R = (Word){ (Word8)(R-Bval), (Word8)(R-Bval) };
            flags = zero_w(R) ? FL_ZERO : 0;
            break;
        }
        case OP_AND: { Word Bval=mem_le((unsigned)A); R=(Word){(Word8)(R&Bval),(Word8)(R&Bval)}; break; }
        case OP_OR:  { Word Bval=mem_le((unsigned)A); R=(Word){(Word8)(R|Bval),(Word8)(R|Bval)}; break; }
        case OP_XOR: { Word Bval=mem_le((unsigned)A); R=(Word){(Word8)(R^Bval),(Word8)(R^Bval)}; break; }
        case OP_CMP: {
            Word Bval=mem_le((unsigned)A);
            flags = 0;
            if(R==Bval) flags|=FL_EQ;
            if(R<Bval)  flags|=FL_LT;
            break;
        }
        case OP_JMP:  pc = arg1|(arg2<<8); break;
        case OP_JZ:   if(flags&FL_ZERO) pc = arg1|(arg2<<8); break;
        case OP_JNZ:  if(!(flags&FL_ZERO)) pc = arg1|(arg2<<8); break;
        case OP_INC: {
            unsigned slot = arg1|(arg2<<8);
            Word w = mem_le(slot);
            w = (Word){(Word8)(w+1),(Word8)(w+1)};
            mem_grava(slot, w);
            R = w; break;
        }
        default: pc = prog_n; break;
        }
    }
}

/* Referencia C: VINCO(A,B) = A - B (mod 256) */
static Word ref_vinco(Word a, Word b){
    return (Word){ sub8(a, b), sub8(a, b) };
}

int main(void){
    int pass=0, fail=0;

    /* === Teste 1: exaustivo 256x256 === */
    if(monta("vinco.erg")){ fprintf(stderr,"ERRO: nao abriu vinco.erg\n"); return 1; }
    if(prog_n==0){ fprintf(stderr,"ERRO: programa vazio\n"); return 1; }

    for(int a=0; a<256; a++){
        for(int b=0; b<256; b++){
            mem_zera();
            mem_poe(0, 0); mem_poe(1, a); mem_poe(2, b);
            pc=0; R=0; A=0; B=0; flags=0;
            exec(prog_n);
            Word esperado = ref_vinco((Word){(Word8)a,(Word8)a}, (Word){(Word8)b,(Word8)b});
            Word obtido = mem_le(0);
            Word8 fl = flags;
            if(obtido != esperado || fl != (zero_w(esperado)?FL_ZERO:0)){
                if(fail<20) printf("FAIL a=%d b=%d got=%u exp=%u flags=%d\n",
                    a,b, obtido, esperado, fl);
                fail++;
            } else pass++;
        }
    }
    printf("exaustivo: %d/65536\n", pass);

    /* === Teste 2: protocolo — zero === */
    mem_zera(); mem_poe(0,0); mem_poe(1,0); mem_poe(2,0); pc=0; R=0; A=0; B=0; flags=0;
    exec(prog_n);
    if(mem_le(0)==0 && flags==FL_ZERO){ pass++; printf("protocol: zero PASS\n"); }
    else { fail++; printf("protocol: zero FAIL\n"); }

    /* === Teste 3: protocolo — identidade === */
    mem_zera(); mem_poe(0,0); mem_poe(1,42); mem_poe(2,0); pc=0; R=0; A=0; B=0; flags=0;
    exec(prog_n);
    if(mem_le(0)==42 && flags==0){ pass++; printf("protocol: identidade PASS\n"); }
    else { fail++; printf("protocol: identidade FAIL\n"); }

    /* === Teste 4: protocolo — underflow === */
    mem_zera(); mem_poe(0,0); mem_poe(1,0); mem_poe(2,1); pc=0; R=0; A=0; B=0; flags=0;
    exec(prog_n);
    if(mem_le(0)==255 && flags==0){ pass++; printf("protocol: underflow PASS\n"); }
    else { fail++; printf("protocol: underflow FAIL\n"); }

    printf("=== VINCO ISA TEST ===\n");
    printf("casos: %d  pass: %d  fail: %d\n", pass+fail, pass, fail);
    return fail ? 2 : 0;
}