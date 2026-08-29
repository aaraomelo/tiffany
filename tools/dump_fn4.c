#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
static int leb(const uint8_t*b,int n,int p,int*v){uint32_t r=0;int s=0;while(p<n){uint8_t c=b[p++];r|=(uint32_t)(c&0x7f)<<s;if(!(c&0x80)){*v=(int)r;return p;}s+=7;}return -1;}
int main(){
  FILE*f=fopen("assets/figuras/wasm/node.wasm","rb");fseek(f,0,SEEK_END);long sz=ftell(f);fseek(f,0,SEEK_SET);
  uint8_t*b=malloc(sz);fread(b,1,sz,f);fclose(f);
  int p=8;while(p<sz){int id=b[p++],sz2;int nx=leb(b,sz,p,&sz2);p=nx;uint8_t*sec=b+p;
  if(id==10){int nf;int q=leb(sec,sz2,0,&nf);
  for(int i=0;i<nf;i++){int fs;q=leb(sec,sz2,q,&fs);
  if(i==4){uint8_t*fn=sec+q;int lp=0,ng;lp=leb(fn,fs,lp,&ng);for(int g=0;g<ng;g++){int c;lp=leb(fn,fs,lp,&c);lp++;}
  int code=lp;printf("code@%d size=%d\n",code,fs-code);
  for(int j=code;j<fs;j++){if(j==74)printf(">>");printf("%3d:%02x ",j,fn[j]);}
  }q+=fs;}}p+=sz2;}return 0;}
