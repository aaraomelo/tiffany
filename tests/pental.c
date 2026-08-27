#include <stdio.h>
static int quad(long long n){long long r=0;while(r*r<n)r++;return r*r==n;}
static int pot2(long long n){return n>0 && (n&(n-1))==0;}
int main(void){
  printf("=== o que a duplicidade conta ===\n");
  printf("  as LEIS: elementos em {2,3} x operacoes em {1,2}  ->  2^2 = %d\n", 2*2);
  printf("  os SIMBOLOS: tres blocos, uma escolha por bloco   ->  2^3 = %d\n", 2*2*2);
  printf("  as ESPECIES da dobra: ordem um ou ordem dois      ->  2^1 = %d\n\n", 2);
  printf("=== e o que a estrela faz a essa conta ===\n");
  printf("   m   Delta=m^2+4   quadrado?   potencia de dois?\n");
  for(long m=0;m<=8;m++){
    long D=m*m+4;
    printf("  %2ld      %3ld         %-9s   %s%s\n", m, D,
      quad(D)?"SIM":"nao", pot2(D)?"SIM":"nao",
      (m==1)?"     <-- a ESTRELA":"");
  }
  printf("\n  varridos m de 1 a 100000:\n");
  int nq=0, np=0;
  for(long long m=1;m<=100000;m++){ long long D=m*m+4; if(quad(D))nq++; if(pot2(D))np++; }
  printf("    Delta quadrado: %d   |   Delta potencia de dois: %d\n", nq, np);
  printf("\n-> a estrela nao e' quadrado (logo o ponto fixo sai do andar)\n");
  printf("   nem potencia de dois (logo a duplicidade nao a conta).\n");
  return 0;
}
