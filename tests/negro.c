/* negro.c — o FRACTAL NEGRO e a conservação do par, no `fisica.tex`.
 *
 *   cc -O2 -std=c99 -o /tmp/negro tests/negro.c -lm && /tmp/negro
 *
 * O fractal claro repete a forma; o dual dele conserva a diferença. «Negro» aqui
 * é definido pela ESTRUTURA e não pela aparência: é o lado do bloco cuja medida
 * cresce, isto é, o de expoente positivo.
 *
 * A dobra é a multiplicativa, r -> 1/r, e sob ela a dimensão é antissimétrica:
 * s vai em -s. Daí saem DUAS conservações, e não são a mesma — o produto das
 * medidas vale um, e a soma das dimensões vale zero. São as duas réguas da
 * inversão, uma por face.
 *
 * E o que se mede é o essencial: cada lado move-se em TODOS os passos, e o par
 * não se move em NENHUM. Logo «a medida cresce» é verdade de metade do bloco, e
 * a seta não é propriedade do passo — é uma polaridade lida sem a outra ponta.
 *
 *   §N1  produto das medidas = 1 e soma das dimensões = 0, em várias dimensões
 *   §N2  o ponto fixo é r=1, onde os dois lados coincidem — a estrela
 *   §N3  cada lado move-se em todos os passos; o par, em nenhum
 */
#include <stdio.h>
#include <math.h>
/* O fractal tem DUAL, e a conservacao e' o produto.
   Medida do lado que cresce:  S+ = r^(s)        com s = d-1 > 0
   Medida do lado dual:        S- = r^(-s)       (raio dual: r -> 1/r)
   Duas conservacoes, e nao sao a mesma:
     produto das medidas   = 1   (multiplicativa)
     soma dos expoentes    = 0   (aditiva)
   Ponto fixo: r = 1, onde os dois lados coincidem -- a estrela. */
int main(void){
  printf("=== a dobra multiplicativa r -> 1/r, e as duas reguas ===\n");
  int falProd=0, falSoma=0, fix=0, n=0;
  for(int d=2; d<=6; d++){
    double s = d-1;
    for(int i=1;i<=40;i++){
      double r = 0.25*i;              /* raios, sem virgula na leitura */
      double Sn = pow(r, s), Sb = pow(1.0/r, s);
      double prod = Sn*Sb;
      double soma = s + (-s);
      n++;
      if(fabs(prod-1.0) > 1e-9) falProd++;
      if(fabs(soma)     > 1e-12) falSoma++;
      if(fabs(r-1.0) < 1e-12 && fabs(Sn-Sb) < 1e-12) fix++;
    }
  }
  printf("  varridos %d casos (dimensao 2..6, 40 raios)\n", n);
  printf("  produto das medidas = 1 : falhas %d\n", falProd);
  printf("  soma dos expoentes  = 0 : falhas %d\n", falSoma);
  printf("  pontos onde os dois lados coincidem: %d (um por dimensao, e e' r=1)\n\n", fix);

  printf("=== o par nao se move, e cada lado move-se sempre ===\n");
  double s=2.0, r=1.0; int sobe=0, desce=0, parado=0, passos=19;
  double Sn=pow(r,s), Sb=pow(1.0/r,s);
  for(int k=0;k<passos;k++){
    r *= 1.3;
    double Sn2=pow(r,s), Sb2=pow(1.0/r,s);
    if(Sn2>Sn) sobe++;
    if(Sb2<Sb) desce++;
    if(fabs(Sn2*Sb2 - Sn*Sb) < 1e-9) parado++;
    Sn=Sn2; Sb=Sb2;
  }
  printf("  em %d passos: o que cresce subiu em %d, o dual desceu em %d,\n", passos, sobe, desce);
  printf("  e o PAR nao se moveu em %d -- isto e', em todos.\n", parado);
  printf("\n-> a seta nao e' do tempo: e' uma polaridade lida sem a outra.\n");
  return 0;
}
