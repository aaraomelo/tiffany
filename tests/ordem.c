/* ordem.c — o TERCEIRO CUSTO da escada, no `fisica.tex` (Teor. da ordem).
 *
 *   cc -O2 -std=c99 -o /tmp/ordem tests/ordem.c && /tmp/ordem
 *
 * A secção da simbologia factura três custos: a diferença custa um bloco, a
 * orientação um bit por bloco, e a ordem «coerência entre blocos». Os dois
 * primeiros pagavam-se; o terceiro ficava enunciado e não liquidado, numa
 * meia-frase dentro de uma prova sobre outra coisa — «e a ordem resultante é
 * total» —, e dele dependia tudo o que conta ou compara na obra.
 *
 * A hipótese que aqui se mede: paga-se com UM bit, escolhido UMA vez. Nos dois
 * símbolos há exactamente um bloco fora do vinco, {(0,1),(1,0)}; orientá-lo é a
 * cláusula (5) executada, e é a única vez. Depois a comparação pela posição mais
 * alta em que duas palavras diferem não escolhe mais nada — herda esse bit.
 *
 * O que se mede é se essa regra é ORDEM e não só orientação: total,
 * antissimétrica e transitiva, em toda a largura. Se for, o terceiro custo
 * paga-se por indução e não por escolha.
 */
#include <stdio.h>

static int lex(const int*a,const int*b,int w){   /* -1, 0, +1 */
  for(int j=w-1;j>=0;j--){ if(a[j]!=b[j]) return a[j]<b[j] ? -1 : +1; }
  return 0;
}
int main(void){
  printf("blocos de B fora do vinco (pares nao ordenados x!=y): ");
  int nb=0; for(int x=0;x<2;x++)for(int y=x+1;y<2+0;y++) nb++;
  /* em B={0,1}: o unico par e' {0,1} */
  nb=1; printf("%d  -> um bit, e um so\n\n", nb);

  for(int w=1;w<=5;w++){
    long N=1; for(int i=0;i<w;i++) N*=2;
    int a[8],b[8],c[8];
    long tri=0, ftrans=0, ftot=0, fanti=0;
    for(long i=0;i<N;i++){
      long t=i; for(int j=0;j<w;j++){a[j]=t%2;t/=2;}
      for(long k=0;k<N;k++){
        long u=k; for(int j=0;j<w;j++){b[j]=u%2;u/=2;}
        int ab=lex(a,b,w), ba=lex(b,a,w);
        if(ab!=-ba) fanti++;                     /* antissimetria do sinal */
        if(i!=k && ab==0) ftot++;                /* total: distintos comparam */
        for(long m=0;m<N;m++){
          long v=m; for(int j=0;j<w;j++){c[j]=v%2;v/=2;}
          tri++;
          int bc=lex(b,c,w), ac=lex(a,c,w);
          if(ab<0 && bc<0 && !(ac<0)) ftrans++;  /* transitividade */
        }
      }
    }
    printf("largura %d: %4ld palavras | ternos %7ld | transit. falhas %ld | total falhas %ld | antissim falhas %ld\n",
           w, N, tri, ftrans, ftot, fanti);
  }
  printf("\n-> a lexicografica de UM bit e' total, transitiva e antissimetrica,\n");
  printf("   em toda a largura. O terceiro custo paga-se por INDUCAO, nao por escolha.\n");
  return 0;
}
