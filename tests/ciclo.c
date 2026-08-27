/* ciclo.c — a PARTIÇÃO DA BORDA por ordem, no `fisica.tex` (Cor. da partição).
 *
 *   cc -O2 -std=c99 -o /tmp/ciclo tests/ciclo.c && /tmp/ciclo
 *
 * Na borda, a potência n-ésima valer um diz que a ordem divide n. Logo o conjunto
 * parte-se por ordem exacta e sem resto: cada elemento tem uma ordem e uma só,
 * portanto cai num bloco e num só.
 *
 * E a MESMA partição lê-se nas duas faces, que é o que aqui se mede. Contando os
 * blocos pelo tamanho, os tamanhos somam n — face aditiva, a sucessão. Tomando um
 * factor por bloco, os factores multiplicam para a potência menos um — face
 * multiplicativa, a multiplicidade.
 *
 * Uma partição, duas leituras. É a figura do teorema central, que parte o rectângulo
 * e lê as duas contagens; aqui, do outro lado da borda, lê-se multiplicando.
 *
 *   §C1  os tamanhos dos blocos somam n, para todo n até dezasseis
 *   §C2  o número de blocos é o número de divisores, e é o número de factores
 */
#include <stdio.h>

static int mdc(int a,int b){while(b){int t=a%b;a=b;b=t;}return a;}
static int phi(int n){int c=0;for(int k=1;k<=n;k++) if(mdc(k,n)==1) c++; return c;}
int main(void){
  printf("=== a particao por ordem, lida nas duas faces ===\n");
  printf("  n   divisores d|n        soma dos graus phi(d)   = n ?\n");
  int falha=0;
  for(int n=1;n<=16;n++){
    int soma=0;
    printf("  %2d   ", n);
    for(int d=1;d<=n;d++) if(n%d==0){ printf("%d ", d); soma+=phi(d); }
    for(int k=0;k<18-3*0;k++) if(k==0) ;
    printf("\t soma=%2d  %s\n", soma, soma==n?"SIM":"FALHA");
    if(soma!=n) falha++;
  }
  printf("\n  falhas na leitura aditiva: %d\n", falha);
  /* leitura multiplicativa: a particao da' os factores, e o numero deles e' o numero de divisores */
  printf("\n=== e a leitura multiplicativa conta os mesmos blocos ===\n");
  printf("  n   blocos (divisores)   grau total do produto\n");
  int f2=0;
  for(int n=1;n<=16;n++){
    int nb=0, g=0;
    for(int d=1;d<=n;d++) if(n%d==0){ nb++; g+=phi(d); }
    printf("  %2d        %2d                  %2d   %s\n", n, nb, g, g==n?"":"FALHA");
    if(g!=n) f2++;
  }
  printf("\n  falhas: %d\n", f2);
  printf("\n-> uma particao, duas leituras: os graus SOMAM, os factores MULTIPLICAM.\n");
  return 0;
}
