/* Bridge test: check ERG signal_extract NA check path */
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>

static uint16_t fmem[65536];
static uint16_t mem[65536];
static uint8_t bin[65536];
static int blen, halted;
static uint16_t A,B,R,PC,flags;

static void reset_fmem(void){memset(fmem,0,sizeof(fmem));}
static void init(const uint16_t*m){for(int i=0;i<65536;i++)mem[i]=m[i*2]|(m[i*2+1]<<8);A=B=R=PC=flags=0;halted=0;}
static unsigned read_arg(const uint8_t*c,int pos){return(unsigned)c[pos]|((unsigned)c[pos+1]<<8);}
static unsigned slot_indice(unsigned ptr){return(unsigned)mem[ptr]|((unsigned)mem[ptr+1]<<8);}
static int step(int trace){
  if(halted||PC>=(uint16_t)blen)return 1;
  uint8_t op=bin[PC];
  switch(op){
  case 0x00:halted=1;PC++;break;
  case 0x01:{unsigned slot=read_arg(bin,PC+1);PC+=3;B=A;A=mem[slot];break;}
  case 0x02:{unsigned slot=read_arg(bin,PC+1);PC+=3;mem[slot]=R;break;}
  case 0x03:R=A+B;flags=0;if(R==0)flags|=1;PC++;break;
  case 0x04:R=A-B;flags=0;if(R==0)flags|=1;PC++;break;
  case 0x09:R=A-B;flags=0;if(R==0)flags|=1;PC++;break;
  case 0x0A:{int16_t rel=(int8_t)bin[PC+1]|(bin[PC+2]<<8);PC+=3;PC=(uint16_t)((int16_t)PC+rel);break;}
  case 0x0B:{int16_t rel=(int8_t)bin[PC+1]|(bin[PC+2]<<8);PC+=3;if(flags&1)PC=(uint16_t)((int16_t)PC+rel);break;}
  case 0x0C:{int16_t rel=(int8_t)bin[PC+1]|(bin[PC+2]<<8);PC+=3;if(!(flags&1))PC=(uint16_t)((int16_t)PC+rel);break;}
  case 0x0E:{unsigned ptr=read_arg(bin,PC+1);PC+=3;B=A;unsigned idx=slot_indice(ptr);A=mem[idx];break;}
  case 0x11:{uint16_t t=A;A=R;R=t;PC++;break;}
  case 0x18:{unsigned ptr=read_arg(bin,PC+1);PC+=3;unsigned tgt=slot_indice(ptr);mem[tgt]=R;break;}
  case 0x1A:{unsigned slot=read_arg(bin,PC+1);PC+=3;mem[slot]++;break;}
  case 0x28:R=~A;PC++;break;
  default:printf("  UNKNOWN opcode 0x%02x at PC=%d\n",op,PC);return -1;
  }
  return 0;
}
static void run(int max){for(int i=0;i<max&&!halted;i++)step(0);}
static uint8_t*readbin(const char*p,int*l){FILE*f=fopen(p,"rb");if(!f)return NULL;fseek(f,0,SEEK_END);long sz=ftell(f);fseek(f,0,SEEK_SET);uint8_t*b=malloc(sz);fread(b,1,sz,f);fclose(f);*l=(int)sz;return b;}

int main(void){
  /* First test: LE_len2 path - payload 0x34 0x12 */
  blen=0;uint8_t*b=readbin("j1939_signal_extract.bin",&blen);
  if(!b){fprintf(stderr,"ERRO abrir bin\n");return 1;}
  memcpy(bin,b,blen);free(b);

  printf("=== Test: NA match case ===\n");
  reset_fmem();
  fmem[0*2]=2;fmem[2*2]=0;fmem[3*2]=2;fmem[4*2]=0;fmem[5*2]=1;fmem[6*2]=1;
  fmem[8*2]=0x34;fmem[9*2]=0x12; /* na_value=0x1234 LE */
  /* payload = 0x34 0x12 = na_value */
  uint8_t payload[8] = {0x34, 0x12, 0,0,0,0,0,0};
  for(int i=0;i<8;i++) fmem[(40+i)*2] = payload[i]; /* SLOT_PAYLOAD_LO+i = slots 20-27? */
  /* SLOT_PAYLOAD_LO=40 per test.c? Let's check what test.c uses */
  init(fmem);
  run(400);

  uint64_t raw = 0;
  for(int i=0;i<8;i++) raw |= ((uint64_t)fmem[(8+i)*2])<<(8*i);
  int status = fmem[33*2];
  int rc = fmem[32*2];
  printf("ERG: rc=0x%02x status=0x%02x raw=0x%016llx\n", rc, status, (unsigned long long)raw);
  printf("Expected: rc=0x00 status=0x01 (NA) raw=0x1234\n");
  return 0;
}
