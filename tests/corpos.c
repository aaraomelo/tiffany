/* corpos.c — o ponto fixo NÃO está fora de todos os corpos, no `fisica.tex`.
 *
 *   cc -O2 -std=c99 -o /tmp/corpos tests/corpos.c && /tmp/corpos
 *
 * A afirmação tentadora é «o gerador está fora de todos os corpos». É falsa, e o
 * medidor diz onde. O ponto fixo de x²=mx+1 existe no corpo exactamente quando
 * Δ=m²+4 é quadrado nele — e em face finita isso acontece para cerca de metade
 * dos metais, pelo critério de Euler.
 *
 * O que é verdade, e é mais forte por ser exacto: ele está fora dos DEGRAUS da
 * escada, onde Δ nunca é quadrado — e aí a razão é a das duas desigualdades,
 * m² < m²+4 < (m+2)², e não uma varredura.
 *
 * Donde o corte não é defeito do andar: é o que acontece quando a equação do
 * ponto fixo não fecha no corpo onde a órbita corre. Noutro corpo a mesma órbita
 * cai no ponto fixo e não há corte a fazer.
 *
 *   §C1  quantos metais têm ponto fixo, por corpo — pelo critério de Euler
 */
#include <stdio.h>

static int pot(long b,long e,long p){ long r=1; b%=p; while(e){ if(e&1) r=r*b%p; b=b*b%p; e>>=1;} return (int)r; }
int main(void){
  printf("=== Delta = m^2+4 e' quadrado, por corpo ===\n");
  long P[] = {5,7,11,13,127,257};
  for(int i=0;i<6;i++){
    long p=P[i]; int sim=0, tot=0;
    for(long m=1;m<p;m++){
      long D=(m*m+4)%p; tot++;
      if(D==0 || pot(D,(p-1)/2,p)==1) sim++;      /* criterio de Euler */
    }
    printf("  corpo de %3ld elementos: o ponto fixo EXISTE em %3d de %3d metais (%d%%)\n",
           p, sim, tot, (int)(100.0*sim/tot));
  }
  printf("\n-> NAO esta' fora de todos os corpos: em face finita existe para cerca de metade.\n");
  printf("   O que e' verdade: esta' fora dos DEGRAUS da escada, onde Delta nunca e' quadrado.\n");
  printf("   Logo o corte nao e' defeito do andar -- e' o que acontece quando a equacao\n");
  printf("   do ponto fixo nao fecha no corpo onde a orbita corre.\n");
  return 0;
}
