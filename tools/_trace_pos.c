#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>

typedef struct { const char *nome; int op; int operando; } Instr;
#define OP_JZ 12
#define OP_JMP 13
static Instr isa[] = {
    {"LOAD", 1, 2}, {"STORE", 2, 2}, {"CMP", 3, 0}, {"JZ", 12, 1},
    {"JMP", 13, 1}, {"ADD", 4, 0}, {"SUB16", 5, 0}, {"ADD16", 6, 0},
    {"TROCA", 7, 0}, {"HALT", 0, 0}, {NULL,0,0}
};
static Instr* acha(const char*m){ for(int i=0;isa[i].nome;i++) if(!strcmp(isa[i].nome,m)) return &isa[i]; return NULL; }

int main(){
  FILE*f=fopen("conecthus/backends/bash/bash_corre.erg","rb");
  char line[256]; int pos=0, linha=0;
  while(fgets(line,sizeof line,f)){
    linha++;
    char*s=line; while(*s&&isspace((unsigned char)*s))s++;
    if(*s==';'||*s=='\n'||!*s) continue;
    if(*s==':'){
      char nm[64]; sscanf(s+1,"%63s",nm);
      if(!strcmp(nm,"EQ0")) printf("EQ0 at pos %d line %d\n", pos, linha);
      continue;
    }
    char mnem[64]={0}, arg[64]={0};
    sscanf(s,"%63s %63s",mnem,arg);
    for(char*q=mnem;*q;q++) *q=toupper((unsigned char)*q);
    Instr*in=acha(mnem); if(!in){ printf("unknown %s line %d\n",mnem,linha); continue; }
    if(!strcmp(mnem,"JZ") && !strcmp(arg,"EQ0")) printf("JZ EQ0 at pos %d (jump target will be EQ0.end) line %d\n", pos, linha);
    pos += 1 + (in->operando==2?2:in->operando==1?2:0);
  }
  printf("final pos %d\n", pos);
  fclose(f);
  return 0;
}
