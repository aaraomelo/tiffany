/* j1939_read_raw test - fixed simulator v21 - branch trace + all cases */
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
static uint16_t fmem[65536];
static uint16_t mem[65536];
static uint8_t bin[65536];
static int blen;
static uint16_t A,B,R,PC,flags;
static int halted;

static void reset_fmem(void){memset(fmem,0,sizeof(fmem));}
static void set_payload(const uint8_t p[8]){for(int i=0;i<8;i++)fmem[i*2]=p[i];}
static void set_input(uint8_t s,uint8_t l){fmem[16*2]=s;fmem[17*2]=l;}
static void set_constants(void){fmem[20*2]=0;fmem[21*2]=1;fmem[22*2]=8;fmem[23*2]=7;}
static uint64_t get_raw(void){uint64_t w=0;for(int i=0;i<8;i++)w|=((uint64_t)fmem[(8+i)*2])<<(8*i);return w;}
static void init(const uint16_t*m){for(int i=0;i<65536;i++)mem[i]=m[i*2]|(m[i*2+1]<<8);A=B=R=PC=flags=0;halted=0;}

static unsigned read_arg(const uint8_t*c,int pos){return(unsigned)c[pos]|((unsigned)c[pos+1]<<8);}
static unsigned slot_indice(unsigned ptr){return(unsigned)mem[ptr]|((unsigned)mem[ptr+1]<<8);}

static int step(int trace){
  if(halted||PC>=(uint16_t)blen){if(trace)printf("  HALTED at PC=%d\n",PC);return 1;}
  uint8_t op=bin[PC];
  if(trace)printf("  PC=%d op=0x%02x A=0x%04x B=0x%04x R=0x%04x flags=%d mem[19]=0x%04x\n",PC,op,A,B,R,flags,mem[19]);
  switch(op){
  case 0x00:halted=1;if(trace)printf("    HALT\n");PC++;break;
  case 0x01:{unsigned slot=read_arg(bin,PC+1);PC+=3;B=A;A=mem[slot];if(trace)printf("    LOAD %u: A=0x%04x\n",slot,A);}break;
  case 0x02:{unsigned slot=read_arg(bin,PC+1);PC+=3;mem[slot]=R;if(trace)printf("    STORE %d: mem=0x%04x\n",slot,R);}break;
  case 0x03:R=A+B;flags=0;if(R==0)flags|=1;PC++;if(trace)printf("    ADD: R=0x%04x flags=%d\n",R,flags);break;
  case 0x04:R=A-B;flags=0;if(R==0)flags|=1;PC++;if(trace)printf("    SUB: R=0x%04x flags=%d\n",R,flags);break;
  case 0x09:R=A-B;flags=0;if(R==0)flags|=1;PC++;if(trace)printf("    CMP: R=0x%04x flags=%d\n",R,flags);break;
  case 0x0A:{int16_t rel=(int8_t)bin[PC+1]|(bin[PC+2]<<8);PC+=3;PC=(uint16_t)((int16_t)PC+rel);if(trace)printf("    JMP rel=%d -> PC=%d\n",rel,PC);}break;
  case 0x0B:{int16_t rel=(int8_t)bin[PC+1]|(bin[PC+2]<<8);PC+=3;if(flags&1)PC=(uint16_t)((int16_t)PC+rel);if(trace)printf("    JZ taken=%d -> PC=%d\n",flags&1,PC);}break;
  case 0x0C:{int16_t rel=(int8_t)bin[PC+1]|(bin[PC+2]<<8);PC+=3;if(!(flags&1))PC=(uint16_t)((int16_t)PC+rel);if(trace)printf("    JNZ taken=%d -> PC=%d\n",!(flags&1),PC);}break;
  case 0x0E:{unsigned ptr=read_arg(bin,PC+1);PC+=3;B=A;unsigned idx=slot_indice(ptr);A=mem[idx];if(trace)printf("    LOADS %u -> idx=%d -> A=0x%04x\n",ptr,idx,A);}break;
  case 0x11:{uint16_t t=A;A=R;R=t;PC++;if(trace)printf("    TROCA A<->R\n");}break;
  case 0x18:{unsigned ptr=read_arg(bin,PC+1);PC+=3;unsigned tgt=slot_indice(ptr);mem[tgt]=R;if(trace)printf("    STORE_IND ptr=%u -> tgt=%d <- R=0x%04x\n",ptr,tgt,R);}break;
  case 0x1A:{unsigned slot=read_arg(bin,PC+1);PC+=3;mem[slot]++;if(trace)printf("    INC %u: mem=0x%04x\n",slot,mem[slot]);}break;
  default:printf("    UNKNOWN opcode 0x%02x at PC=%d\n",op,PC);return -1;
  }
  return 0;
}

static void run(int max){for(int i=0;i<max&&!halted;i++)step(1);}
static uint8_t*readbin(const char*p,int*l){FILE*f=fopen(p,"rb");if(!f)return NULL;fseek(f,0,SEEK_END);long sz=ftell(f);fseek(f,0,SEEK_SET);uint8_t*b=malloc(sz);fread(b,1,sz,f);fclose(f);*l=(int)sz;return b;}

int main(void){
  blen=0;uint8_t*b=readbin("j1939_read_raw.bin",&blen);
  if(!b){fprintf(stderr,"ERRO abrir bin\n");return 1;}
  memcpy(bin,b,blen);free(b);
  printf("Binario: %d bytes\n",blen);

  // Single trace: s=0, l=1, payload=all-FF
  printf("\n=== Single trace: s=0 l=1 ===\n");
  uint8_t payload[8] = {0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF};
  reset_fmem();set_payload(payload);set_input(0,1);set_constants();
  init(fmem);
  run(60);
  uint64_t raw = get_raw();
  int err = fmem[19*2];
  printf("raw=0x%016llx err=%d\n", (unsigned long long)raw, err);
  printf("fmem[0..7] (payload): ");
  for(int i=0;i<8;i++) printf("0x%04x ", fmem[i*2]);
  printf("\nfmem[8..15] (raw):    ");
  for(int i=0;i<8;i++) printf("0x%04x ", fmem[(8+i)*2]);
  printf("\nfmem[16]=0x%04x fmem[17]=0x%04x fmem[18]=0x%04x fmem[19]=0x%04x\n",
         fmem[16*2], fmem[17*2], fmem[18*2], fmem[19*2]);

  // Check mem state
  printf("\nmem[16..19]: 0x%04x 0x%04x 0x%04x 0x%04x\n", mem[16], mem[17], mem[18], mem[19]);
  printf("mem[20..23]: 0x%04x 0x%04x 0x%04x 0x%04x\n", mem[20], mem[21], mem[22], mem[23]);
  printf("mem[24..25]: 0x%04x 0x%04x\n", mem[24], mem[25]);

  // Check payload integrity
  printf("\nPayload integrity check:\n");
  int payload_ok = 1;
  for(int i=0;i<8;i++) {
    int ok = (fmem[i*2] == 0xFF);
    printf("  fmem[%d]=0x%04x (expected 0xFF) -> %s\n", i, fmem[i*2], ok?"OK":"CORRUPTED");
    if (!ok) payload_ok = 0;
  }

  // Detailed results
  printf("\nDetailed results:\n");
  printf("  raw=0x%016llx\n", (unsigned long long)raw);
  printf("  err=%d (expected 0)\n", err);
  printf("  payload integrity: %s\n", payload_ok?"OK":"CORRUPTED");

  return 0;
}