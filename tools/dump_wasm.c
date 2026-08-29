#include <stdio.h>
#include <stdlib.h>
static int leb(const unsigned char*b,int n,int p,int*v){int s=0,r=0;while(p<n){int c=b[p++];r|=(c&0x7f)<<s;if(!(c&0x80)){*v=r;return p;}s+=7;}return -1;}
int main(){
  FILE*f=fopen("assets/figuras/wasm/node.wasm","rb");fseek(f,0,SEEK_END);long sz=ftell(f);fseek(f,0,SEEK_SET);unsigned char*b=malloc(sz);fread(b,1,sz,f);fclose(f);
  int p=8;while(p<sz){int id=b[p++],sz2;int nx=leb(b,sz,p,&sz2);p=nx;const unsigned char*sec=b+p;
  if(id==10){int nf,q=leb(sec,sz2,0,&nf);q=leb(sec,sz2,q,&nf);for(int i=0;i<nf;i++){int fs;q=leb(sec,sz2,q,&fs);if(i==4){const unsigned char*fn=sec+q;int lp=0,ng;lp=leb(fn,fs,lp,&ng);for(int g=0;g<ng;g++){int c,t;lp=leb(fn,fs,lp,&c);lp++;}printf("func4 code start %d len %d\n",lp,fs-lp);for(int j=60;j<90&&lp+j<fs;j++)printf("%3d 0x%02x\n",lp+j,fn[lp+j]);}q+=fs;}}
  p+=sz2;}return 0;}
