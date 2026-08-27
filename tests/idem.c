/* idem.c — CARDINALIDADE e ORDEM, no `fisica.tex` (Teor. das duas conservações).
 *
 *   cc -O2 -std=c99 -o /tmp/idem tests/idem.c && /tmp/idem
 *
 * Duas maneiras de conservar: conservar QUANTOS há, ou conservar QUAL é qual.
 *
 * O lado cardinal lê-se por IDEMPOTÊNCIA — repetir não cria indivíduo novo, e por
 * isso ele carrega clones: a individualidade não é o que a operação conserva. O
 * lado ordinal lê-se por POTÊNCIA — a paridade guarda a direcção, e é a memória
 * da repetição.
 *
 * E há um preço exacto, que é o achado. Um idempotente não trivial é um divisor
 * de zero, porque x*x=x é x(x-1)=0. A implicação vale NUM SENTIDO só: haver
 * divisores de zero não obriga a haver idempotente não trivial (o suporte de
 * quatro tem os primeiros e não os segundos).
 *
 * Donde partir em duas metades TEM custo: os projectores (1±d)/2 são idempotentes
 * não triviais e multiplicam a zero. Fabricar a cisão é fabricar divisores de
 * zero — e é por isso que a ordem não vive nesse lado: a razão só é transitiva
 * onde eles não existem.
 *
 *   §I1  idempotente não trivial => divisores de zero (um sentido só)
 *   §I2  os projectores das duas metades: idempotentes, e o produto é zero
 *   §I3  os dois valores próprios: +1 é idempotente, -1 tem ordem dois
 */
#include <stdio.h>
/* A afirmacao: cardinalidade = idempotencia ; ordem = potencia.
   E: idempotente nao trivial  <=>  ha divisores de zero.
   Porque x*x = x  e'  x(x-1) = 0. */
int main(void){
  printf("=== 1) idempotente nao trivial OBRIGA divisores de zero ===\n");
  printf("  x*x=x  <=>  x(x-1)=0.  Se nao ha divisores de zero, x=0 ou x=1.\n\n");
  printf("  suporte  idempotentes   quais            ha divisor de zero?\n");
  for(int N=2;N<=12;N++){
    int cnt=0, lista[16], nl=0, dz=0;
    for(int x=0;x<N;x++) if((x*x)%N==x){ cnt++; if(nl<16) lista[nl++]=x; }
    for(int a=1;a<N && !dz;a++) for(int b=1;b<N;b++) if((a*b)%N==0){ dz=1; break; }
    printf("   %2d       %2d          ", N, cnt);
    for(int i=0;i<nl;i++) printf("%d ", lista[i]);
    for(int i=nl;i<4;i++) printf("  ");
    printf("        %s%s\n", dz?"SIM":"nao", (cnt>2)==(dz!=0)?"":"  <-- DISCORDA");
  }
  printf("\n  regra: mais de dois idempotentes  =>  ha divisores de zero.\n");
  int falha=0;
  for(int N=2;N<=200;N++){
    int cnt=0, dz=0;
    for(int x=0;x<N;x++) if((x*x)%N==x) cnt++;
    for(int a=1;a<N && !dz;a++) for(int b=1;b<N;b++) if((a*b)%N==0){ dz=1; break; }
    if(cnt>2 && !dz) falha++;
  }
  printf("  varridos suportes de 2 a 200: falhas = %d\n\n", falha);

  printf("=== 2) as duas metades sao projectores, e eles multiplicam a ZERO ===\n");
  printf("  P+ = (1+d)/2  e  P- = (1-d)/2, com d a dobra (d^2 = 1).\n");
  printf("  P+ P+ = P+ ; P- P- = P- ; P+ P- = 0 ; P+ + P- = 1\n");
  /* verificacao sobre pares, onde a dobra e' a troca */
  int f1=0,f2=0,f3=0,f4=0,n=0;
  for(int x=-6;x<=6;x++)for(int y=-6;y<=6;y++){
    int Pp[2]={x+y,x+y}, Pm[2]={x-y,-(x-y)};   /* vezes dois, p/ ficar inteiro */
    /* P+ aplicado outra vez: (a+b, a+b) -> soma = 2(x+y), metade = x+y : fixo */
    int Pp2[2]={(Pp[0]+Pp[1])/2,(Pp[0]+Pp[1])/2};
    int Pm2[2]={(Pm[0]-Pm[1])/2,-((Pm[0]-Pm[1])/2)};
    n++;
    if(Pp2[0]!=Pp[0]) f1++;                    /* P+ idempotente */
    if(Pm2[0]!=Pm[0]) f2++;                    /* P- idempotente */
    if((Pp[0]-Pp[1])!=0) f3++;                 /* P- aplicado a P+ da zero */
    if((Pp[0]+Pm[0])/1 != 2*x) f4++;           /* P+ + P- = identidade */
  }
  printf("  varridos %d pares: P+ idem %d falhas | P- idem %d | P- o P+ = 0 : %d | soma=id : %d\n",
         n,f1,f2,f3,f4);

  printf("\n=== 3) os dois valores proprios: um e' idempotente, o outro tem ordem dois ===\n");
  printf("  (+1)^2 = %d  -> +1 e' idempotente: aplicar outra vez nao muda nada\n", 1*1);
  printf("  (-1)^2 = %d  mas -1 != 1 -> tem ORDEM DOIS: precisa de duas para voltar\n", (-1)*(-1));
  printf("\n-> a metade simetrica CLONA (idempotente, nao individua);\n");
  printf("   a metade cruzada CONTA A PARIDADE (potencia, guarda a direccao).\n");
  return 0;
}
