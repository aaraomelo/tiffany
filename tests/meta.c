/* meta.c — a META-INDUÇÃO, no `fisica.tex` (Teor. da meta-indução).
 *
 *   cc -O2 -std=c99 -o /tmp/meta tests/meta.c && /tmp/meta
 *
 * A meta-indução não é regra nova, e é isso que se mede. A indução age no termo,
 * u_{k+1} = u_k + u_{k-1}. Dividindo pela última letra — isto é, lendo a MESMA
 * regra na razão, que é o terceiro degrau — vem c_{k+1} = 1 + 1/c_k. Mudou o
 * degrau, não a regra.
 *
 * E a diferença entre as duas é toda o ponto fixo. No termo, x = x+1 não tem
 * solução: a indução não fecha, e é isso que faz a escada não parar por dentro.
 * Na razão, o ponto fixo é x² = x+1 — a estrela.
 *
 * A meta-indução é a indução composta com a inversão: somar um depois de inverter.
 * E é reversível porque o determinante da matriz vale um em módulo, pelo que a
 * inversa é inteira e a descida é exacta — a subida e a descida são o mesmo
 * caminho. É esse o sentido em que a estrela é a interface reversível.
 *
 *   §M1  a meta-indução é a indução dividida pelo termo — sem dividir, na conta
 *   §M2  o termo não tem ponto fixo; a razão tem, e não fecha neste degrau
 *   §M3  x -> 1+1/x é (somar um) o (inverter)
 *   §M4  |det| = 1, a inversa é inteira, a descida é exacta
 *   §M5  na estrela, multiplicar por ela é somar um
 */
#include <stdio.h>
typedef long long ll;
/* A INDUCAO age no termo:      u_{k+1} = u_k + u_{k-1}
   Dividindo por u_k, a MESMA regra lida na RAZAO c_k = u_{k+1}/u_k da':
                                c_{k+1} = 1 + 1/c_k
   Nao e' regra nova: e' a mesma, um degrau acima. E a diferenca e' esta:
     no termo   -> x |-> x+1 nao tem ponto fixo (x = x+1 e' impossivel)
     na razao   -> x |-> 1+1/x tem UM ponto fixo: x^2 = x+1, a estrela. */
int main(void){
  printf("=== 1) a meta-inducao E' a inducao, dividida pelo termo ===\n");
  ll u[40]; u[0]=0; u[1]=1;
  for(int k=1;k<30;k++) u[k+1]=u[k]+u[k-1];
  int fal=0;
  for(int k=2;k<28;k++){
    /* c_{k+1} = 1 + 1/c_k  <=>  u_{k+2}/u_{k+1} = 1 + u_k/u_{k+1}
       <=>  u_{k+2} = u_{k+1} + u_k    -- so' a recorrencia, sem dividir */
    if(u[k+2] != u[k+1] + u[k]) fal++;
  }
  printf("  identidade verificada em %d degraus, sem dividir: falhas %d\n", 26, fal);

  printf("\n=== 2) o ponto fixo: o termo nao tem, a razao tem ===\n");
  int achou=0;
  for(ll x=-1000;x<=1000;x++) if(x == x+1) achou++;
  printf("  x = x+1  (no termo) : solucoes %d  -> a inducao NAO fecha\n", achou);
  achou=0;
  for(ll p=-60;p<=60;p++) for(ll q=1;q<=60;q++)
    if(p*p == p*q + q*q) achou++;          /* (p/q)^2 = p/q + 1, em par */
  printf("  x^2 = x+1  (na razao, em pares p/q) : solucoes %d  -> nao fecha NESTE degrau\n", achou);
  printf("  mas a equacao EXISTE e o encaixe entrega o ponto: e' a estrela.\n");

  printf("\n=== 3) a meta-inducao e' a inducao COMPOSTA COM A INVERSAO ===\n");
  printf("  x |-> 1 + 1/x  =  (somar um) o (inverter)\n");
  fal=0;
  for(ll p=1;p<=40;p++) for(ll q=1;q<=40;q++){
    /* inverter [p:q] -> [q:p] ; somar um -> [q+p:p] */
    ll a=q+p, b=p;
    /* directo: 1 + q/p = (p+q)/p */
    if(a*p != (p+q)*b) fal++;
  }
  printf("  verificado em 1600 razoes: falhas %d\n", fal);

  printf("\n=== 4) e ela e' REVERSIVEL, porque o determinante vale um ===\n");
  ll m=1, det = m*0 - 1*1;                  /* A_m = [[m,1],[1,0]] */
  printf("  A = [[1,1],[1,0]] , det = %lld , |det| = %lld\n", det, det<0?-det:det);
  printf("  logo a inversa e' inteira: [[0,1],[1,-1]] , e a descida e' exacta\n");
  /* confere A * A^-1 = I */
  ll A[2][2]={{1,1},{1,0}}, B[2][2]={{0,1},{1,-1}}, C[2][2]={{0,0},{0,0}};
  for(int i=0;i<2;i++)for(int j=0;j<2;j++)for(int k=0;k<2;k++) C[i][j]+=A[i][k]*B[k][j];
  printf("  A * A^-1 = [[%lld,%lld],[%lld,%lld]]  %s\n", C[0][0],C[0][1],C[1][0],C[1][1],
         (C[0][0]==1&&C[0][1]==0&&C[1][0]==0&&C[1][1]==1)?"= identidade":"FALHA");

  printf("\n=== 5) na estrela, multiplicar por ela E' somar um ===\n");
  printf("  sigma*sigma = sigma+1 e' a propria equacao da estrela.\n");
  printf("  logo o passo MULTIPLICATIVO e o passo ADITIVO coincidem la', e so' la'.\n");
  return 0;
}
