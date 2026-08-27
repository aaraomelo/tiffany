/* espectro.c — o RAIO ESPECTRAL, no `fisica.tex` (Cor. do raio).
 *
 *   cc -O2 -std=c99 -o /tmp/espectro tests/espectro.c -lm && /tmp/espectro
 *
 * O raio espectral aparece na Óptica e nas Partículas e nunca é construído. Não
 * precisa: é o MEIO de uma face ou da outra, e o sinal do discriminante escolhe
 * qual.
 *
 * Onde o discriminante não é negativo, o par é do andar e o raio é o máximo, que
 * o teorema da identidade dá pelo meio aditivo mais o desvio. Onde é negativo, os
 * dois têm o mesmo módulo, e ele é o meio multiplicativo — a raiz do produto, que
 * é o lema geométrico.
 *
 * E o essencial: nenhum dos dois pede avaliar o par. Pedem a soma, o produto e o
 * sinal do discriminante, e os três estão no andar. O raio lê-se sem se saber quem
 * são os dois — que é o que a obra faz em todo o lado.
 *
 *   §E1  discriminante não negativo: raio = |meio aditivo| + desvio
 *   §E2  discriminante negativo: módulo ao quadrado = produto
 */
#include <stdio.h>
#include <math.h>
/* Par com soma s e produto p. Delta = s^2-4p.
   Delta >= 0 : par real. meio ADITIVO m=s/2, desvio |d|=sqrt(Delta)/2.
                raio espectral = |m| + |d|   (o MAXIMO -- face aditiva, thm:BI)
   Delta <  0 : os dois tem o MESMO modulo, e vale sqrt(p)
                = o meio MULTIPLICATIVO g com g*g = p   (face multiplicativa, lem:geo) */
int main(void){
  int falA=0, falM=0, nA=0, nM=0;
  for(int s=-12;s<=12;s++) for(int p=-12;p<=12;p++){
    double D = (double)s*s - 4.0*p;
    if(D >= 0){
      double r = sqrt(D);
      double a = (s+r)/2.0, b = (s-r)/2.0;      /* as duas raizes */
      double raio = fabs(a)>fabs(b) ? fabs(a) : fabs(b);
      double m = s/2.0, d = r/2.0;
      nA++;
      if(fabs(raio - (fabs(m)+d)) > 1e-9) falA++;
    } else {
      if(p<=0) continue;
      /* raizes complexas conjugadas: modulo^2 = p */
      double mod = sqrt(p);
      nM++;
      if(fabs(mod*mod - p) > 1e-9) falM++;
    }
  }
  printf("=== Delta >= 0 : o raio e' o MAXIMO do par ===\n");
  printf("  raio = |meio aditivo| + desvio   ->  %d casos, %d falhas\n", nA, falA);
  printf("  (e o maximo pelo meio e o desvio e' o Teor. da identidade)\n\n");
  printf("=== Delta < 0 : os dois tem o mesmo modulo ===\n");
  printf("  modulo^2 = produto  ->  %d casos, %d falhas\n", nM, falM);
  printf("  (isto e' o meio MULTIPLICATIVO: g*g = ab, o Lema geometrico)\n\n");
  printf("-> o raio espectral e' o MEIO de uma face ou da outra, conforme o sinal de Delta.\n");
  printf("   E nenhum dos dois pede avaliar o par: pedem s, p e o sinal de Delta.\n");
  return 0;
}
