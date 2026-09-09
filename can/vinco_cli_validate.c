
#include <stdio.h>
#include <stdint.h>
#include <string.h>

typedef uint8_t  Word8;
typedef uint16_t Word;

static Word mem[16][2];
static Word8 flags;

static void mem_zera(void){ memset(mem, 0, sizeof(mem)); flags = 0; }
static void mem_poe(unsigned slot, Word w){ if(slot<16) mem[slot][0]=w; }
static Word mem_le(unsigned slot){ return (slot<16)?mem[slot][0]:0; }
static void mem_grava(unsigned slot, Word w){ if(slot<16) mem[slot][0]=w; }
static Word8 sub8(Word8 a, Word8 b){ return (Word8)(a-b); }
static int zero_w(Word w){ return w==0; }

enum { OP_HALT=0, OP_LOAD, OP_STORE, OP_ADD, OP_SUB, OP_AND, OP_OR, OP_XOR,
       OP_GOLD, OP_CMP, OP_JMP, OP_JZ, OP_JNZ,
       OP_FOLD, OP_LOADS, OP_NEGRO_OURO, OP_ESQUILO, OP_TROCA, OP_MARTELO,
       OP_ADD16, OP_SUB16, OP_CMP16, OP_MUL16, OP_ESPALHA, OP_STORE_IND,
       OP_VINCO, OP_INC };

unsigned prog[1024], prog_n;
unsigned pc; Word R, A, B;

int monta(const char *path){
    FILE *f=fopen(path,"r"); if(!f) return 1;
    prog_n=0; char line[256];
    while(fgets(line,sizeof(line),f)){
        if(line[0]==';'||line[0]=='#') continue;
        char *p=line; while(*p==' '||*p=='\t') p++;
        if(*p=='\n'||*p=='\r'||*p==0) continue;
        char nome[64]; int arg1=-1,arg2=-1;
        int n=sscanf(p,"%63s %d %d",nome,&arg1,&arg2);
        unsigned op=0xFF;
        if(strcmp(nome,"HALT")==0) op=OP_HALT;
        else if(strcmp(nome,"LOAD")==0) op=OP_LOAD;
        else if(strcmp(nome,"VINCO")==0) op=OP_VINCO;
        else if(strcmp(nome,"STORE")==0) op=OP_STORE;
        else if(strcmp(nome,"ADD")==0) op=OP_ADD;
        else if(strcmp(nome,"SUB")==0) op=OP_SUB;
        if(op==0xFF) continue;
        unsigned instr=op;
        if(n>=2) instr|=(arg1&0xFF)<<8;
        if(n>=3) instr|=(arg2&0xFF)<<16;
        if(prog_n<1024) prog[prog_n++]=instr;
    }
    fclose(f); return 0;
}

void exec(unsigned n){
    for(unsigned passo=0;passo<n&&pc<prog_n;passo++){
        unsigned instr=prog[pc]; unsigned op=instr&0xFF;
        unsigned arg1=(instr>>8)&0xFF; unsigned arg2=(instr>>16)&0xFF;
        pc++;
        switch(op){
        case OP_HALT: pc=prog_n; break;
        case OP_LOAD: { unsigned slot=arg1|(arg2<<8); B=A; A=mem_le(slot); R=A; break; }
        case OP_STORE: { unsigned slot=arg1|(arg2<<8); mem_grava(slot,R); break; }
        case OP_VINCO: { Word Bval=B; R=(Word){(Word8)(R-Bval),(Word8)(R-Bval)}; flags=zero_w(R)?1:0; break; }
        default: pc=prog_n; break;
        }
    }
}

int main(void){
    int pass=0, fail=0;
    monta("/tmp/vinco_bin.erg");
    for(int a=0;a<256;a++) for(int b=0;b<256;b++){
        mem_zera(); mem_poe(1,a); mem_poe(2,b); pc=0; R=0; A=0; B=0; flags=0;
        exec(prog_n);
        Word esperado=(Word){(Word8)(a-b),(Word8)(a-b)};
        Word obtido=mem_le(0);
        Word8 fl=flags;
        if(obtido!=esperado || fl!=(zero_w(esperado)?1:0)){
            if(fail<5) printf("FAIL a=%d b=%d got=%u exp=%u flags=%d\n",a,b,obtido,esperado,fl);
            fail++;
        } else pass++;
    }
    printf("VINCO CLI validation: %d/65536 pass, %d fail\n", pass, fail);
    return fail?2:0;
}
