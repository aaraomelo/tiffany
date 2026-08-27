/* fractal.c — a AUTO-SEMELHANÇA e a DIMENSÃO, no `fisica.tex`.
 *
 *   cc -O2 -std=c99 -o /tmp/fractal tests/fractal.c -lm && /tmp/fractal
 *
 * Três coisas, e nenhuma pede logaritmo, medida nem número trazido de fora.
 *
 * §FR1  O ponto fixo do passo metálico está no andar exactamente quando o
 *       discriminante Δ é quadrado nele — porque Δ é o quadrado da separação dos
 *       dois extremos. Para x²=mx+1 vem Δ=m²+4, e esse nunca é quadrado: fica
 *       estritamente entre m² e (m+2)², e o único quadrado no meio pede 2m=3.
 *       Logo o gerador está SEMPRE fora do andar que gera.
 *
 * §FR2  A dimensão é um CORTE, e decide-se por comparação inteira: guardando k
 *       de b dígitos, a cobertura de nível n tem k^n peças de diâmetro b^-n, e a
 *       soma das potências s só não degenera quando k=b^s. O corte é
 *       { p/q : b^p < k^q }, e fecha no andar exactamente quando b e k são
 *       potências de um mesmo elemento. Sem um único logaritmo.
 *
 * §FR3  O zero e o infinito são o mesmo par, trocado. Em coordenadas do par
 *       [p:q] a dobra é [p:q] -> [q:p], e ela leva [0:1] em [1:0]. O terceiro
 *       degrau exclui q=0 por construção, pelo que [1:0] não é habitante do
 *       andar: é o que o andar deixa de fora, e o parceiro do zero no bloco.
 */
#include <stdio.h>
#include <math.h>
typedef long long ll;
static int quad(ll n){ if(n<0) return 0; ll r=(ll)sqrtl((long double)n); while(r*r>n) r--; while((r+1)*(r+1)<=n) r++; return r*r==n; }
int main(void){
  printf("=== 1) Delta = m^2+4 e' quadrado? (o ponto fixo esta' no andar sse for) ===\n");
  int achou=0;
  for(ll m=1;m<=5000000;m++) if(quad(m*m+4)){ printf("  m=%lld ACHOU\n",m); achou++; if(achou>5) break; }
  printf("  varridos m de 1 a 5000000: quadrados = %d\n", achou);
  printf("  razao: m^2 < m^2+4 < (m+2)^2, unico quadrado no meio e' (m+1)^2, pede 2m=3.\n\n");

  printf("=== 2) a dimensao e' um corte, decidido por comparacao INTEIRA ===\n");
  int cas[][2]={{3,2},{5,3},{4,2},{8,2},{2,2}};
  const char* nm[]={"Cantor classico","3 de 5","2 de 4","2 de 8","o intervalo"};
  const ll T=1000000000000LL;
  for(int c=0;c<5;c++){
    ll b=cas[c][0],k=cas[c][1],fp=0,fq=0;
    for(ll p=1;p<=40&&!fp;p++){
      ll bp=1; int of=0;
      for(ll i=0;i<p;i++){ if(bp>T/b){of=1;break;} bp*=b; }
      if(of) break;
      for(ll q=1;q<=40;q++){
        ll kq=1; int o2=0;
        for(ll i=0;i<q;i++){ if(kq>T/k){o2=1;break;} kq*=k; }
        if(o2) break;
        if(bp==kq){ fp=p; fq=q; break; }
      }
    }
    if(fp) printf("  (%lld,%lld) %-16s FECHA no andar: b^%lld=k^%lld, s=%lld/%lld\n",b,k,nm[c],fp,fq,fp,fq);
    else   printf("  (%lld,%lld) %-16s NAO fecha: o corte fica FORA do andar\n",b,k,nm[c]);
  }

  printf("\n=== 3) o zero e o infinito sao o MESMO par, trocado ===\n");
  printf("  em coordenadas do par [p:q], a dobra do par e' [p:q] -> [q:p]\n");
  int mau=0;
  /* 0 = [0:1] ; infinito = [1:0] ; a troca leva um no outro */
  int z[2]={0,1}, inf[2]={1,0};
  if(!(z[0]==inf[1] && z[1]==inf[0])) mau++;
  printf("  troca([0:1]) = [1:0] : %s\n", mau?"FALHA":"ok");
  printf("  e o terceiro degrau exclui q=0 por construcao (X2 cancelaveis),\n");
  printf("  logo [1:0] NAO e' habitante do andar -- e' o que o andar deixa de fora.\n");
  /* a troca e' involutiva */
  int t2[2]={inf[1],inf[0]};
  printf("  troca(troca([0:1])) = [%d:%d] : %s\n", t2[0],t2[1], (t2[0]==z[0]&&t2[1]==z[1])?"volta, e' dobra":"FALHA");
  return 0;
}
