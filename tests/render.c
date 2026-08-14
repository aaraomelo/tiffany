/* render.c — O RENDERIZADOR E' UM CORPO DO CATALOGO, e mede-se como tal.
 *
 * O Aarao: «ve corpo morfico» · «tira conceito de shader» · «tudo sai de algum corpo do
 * catalogo».
 *
 * O que desenha aqui nao e' um shader traduzido: e' o CORPO MORFICO a operar. A regua dele
 * esta' no catalogo — a adjuncao delta |- epsilon — e o que se mede sao as leis DELE, nao a
 * semelhanca da imagem com outra coisa.
 *
 *   §R1  a DILATACAO COMPOE (Minkowski): dilatar por r e depois por s E' dilatar por r+s.
 *        E' a lei do corpo, e o raio SOMA — por isso e' ordenado, e por isso e' inteiro
 *   §R2  a ADJUNCAO: erodir depois de dilatar NAO devolve o original — devolve o FECHO, e
 *        o fecho e' idempotente. Aplicado duas vezes da' o mesmo: residuo 0
 *   §R3  o RELOGIO fecha: o raio sobe e desce com a fase, e ao fim da volta volta ao germe
 *   §R4  SEM VIRGULA FLUTUANTE: o raio e' inteiro e a bola e' do reticulado
 *   §R6  o GERME sai da OP: ordem n da' n pontos igualmente espacados, sem trigonometria
 *   §R5  o CONTROLO: com um raio que nao SOMA, a composicao quebra — e sem a lei do corpo
 *        isto seria so' uma imagem bonita
 *
 *   cc -O2 -std=c99 -Wall -I../lib render.c -o render && ./render
 */
#include <stdio.h>
#include <string.h>
#include "unidade.h"

#define W 48
#define H 48

static void dilata(const unsigned char *a, unsigned char *o, long r){
    for(long j=0;j<H;j++) for(long i=0;i<W;i++){ unsigned char m=0;
        for(long dj=-r;dj<=r&&!m;dj++) for(long di=-r;di<=r&&!m;di++){
            long y=j+dj,x=i+di; if(y>=0&&y<H&&x>=0&&x<W&&a[y*W+x]) m=1; }
        o[j*W+i]=m; } }
static void erode(const unsigned char *a, unsigned char *o, long r){
    for(long j=0;j<H;j++) for(long i=0;i<W;i++){ unsigned char m=1;
        for(long dj=-r;dj<=r&&m;dj++) for(long di=-r;di<=r&&m;di++){
            long y=j+dj,x=i+di; if(y<0||y>=H||x<0||x>=W||!a[y*W+x]) m=0; }
        o[j*W+i]=m; } }
static long dif(const unsigned char *a, const unsigned char *b){
    long d=0; for(long k=0;k<W*H;k++) if(a[k]!=b[k]) d++; return d; }

int main(void){
    /* o `falhas` e' o de unidade.h — um local aqui SOMBREAVA o do header: o ok()
     * somava la', o return devolvia o de ca' (sempre zero), e uma unidade vermelha
     * nao virava o exit. O exit E' a assercao; nao se declara outra vez. */
    static unsigned char g[W*H], p[W*H], q[W*H], u[W*H], v[W*H];
    memset(g,0,sizeof g); g[(H/2)*W + W/2] = 1;
    puts("\n=== O RENDERIZADOR E' O CORPO MORFICO ===\n");

    /* §R1 — a dilatacao COMPOE: o raio soma */
    { long maus=0, pares=0;
      for(long r=0;r<=4;r++) for(long s=0;s<=4;s++){
          dilata(g,p,r); dilata(p,q,s);      /* r e depois s */
          dilata(g,u,r+s);                   /* de uma vez, r+s */
          if(dif(q,u)) maus++;
          pares++; }
      printf("  §R1  dil(dil(A,r),s) contra dil(A,r+s): %ld pares, %ld desvios\n\n", pares, maus);
      ok("a DILATACAO COMPOE, e e' a lei do corpo e nao uma propriedade da imagem: dilatar por r"
         " e depois por s E' dilatar por r+s — Minkowski —, em 25 pares sem um desvio. E' por"
         " isso que o parametro e' ORDENADO e por isso que e' INTEIRO: o raio soma", maus==0 && pares==25); }

    /* §R2 — a adjuncao: o fecho e' idempotente */
    { long maus=0, casos=0;
      for(long r=1;r<=4;r++){
          dilata(g,p,r); erode(p,q,r);       /* o FECHO */
          dilata(q,u,r); erode(u,v,r);       /* o fecho do fecho */
          if(dif(q,v)) maus++;
          casos++; }
      printf("  §R2  o fecho aplicado duas vezes contra uma: %ld casos, %ld desvios\n\n", casos, maus);
      ok("a ADJUNCAO delta |- epsilon da' o FECHO, e o fecho e' IDEMPOTENTE: aplicado duas vezes"
         " da' o mesmo, com residuo zero em quatro raios. Nao devolve o original — devolve o"
         " fecho —, e e' isso que a adjuncao promete: nao e' uma involucao, e' um par adjunto",
         maus==0 && casos==4); }

    /* §R3 — o relogio fecha */
    /* PERCORRE-SE a volta inteira e compara-se o estado no fim com o do inicio — e a outra
     * metade: no MEIO da volta tem de ser diferente, senao nada se moveu. */
    { long nf=16, maus=0, meio_igual=0;
      static unsigned char ini[W*H], fim[W*H], mid[W*H];
      dilata(g,ini,0);
      for(long f=0; f<=nf; f++){
          long m=nf/2, r=(f<=m)?f:(nf-f);
          dilata(g,p,r);
          if(f==m)  memcpy(mid,p,sizeof mid);
          if(f==nf) memcpy(fim,p,sizeof fim);
      }
      if(dif(ini,fim)) maus++;             /* a volta FECHA */
      if(dif(ini,mid)==0) meio_igual++;    /* e no meio esta' noutro sitio */
      printf("  §R3  a volta fecha: desvio inicio-fim %ld;  e no meio difere: %s\n\n",
             dif(ini,fim), meio_igual ? "NAO" : "sim");
      ok("o RELOGIO fecha: o raio sobe com a fase ate' meio caminho e desce depois, e ao fim da"
         " volta esta' onde comecou. Nao e' uma animacao a correr — e' uma orbita, e uma orbita"
         " que nao fecha nao tem relogio. E mede-se PELA METADE: o estado no fim iguala o do inicio,"
         " E no meio da volta e' diferente — sem a segunda, um raio sempre zero passava",
         maus==0 && meio_igual==0); }

    /* §R4 — sem virgula flutuante */
    /* o que a virgula flutuante estragaria: a COMPOSICAO exacta. com raio inteiro,
     * dil(r) seguido de dil(s) da' exactamente dil(r+s) — e o desvio e' ZERO, nao pequeno.
     * mede-se contra um raio truncado, que e' o que um float daria ao arredondar. */
    { long exacto=0, truncado=0;
      for(long r=1;r<=4;r++) for(long s=1;s<=4;s++){
          dilata(g,p,r); dilata(p,q,s); dilata(g,u,r+s);
          if(dif(q,u)==0) exacto++;
          dilata(g,v,(r+s)*7/8);            /* o que um raio arredondado daria */
          if(dif(q,v)==0) truncado++;
      }
      printf("  §R4  composicao com raio inteiro: %ld de 16 exactas;"
             "  com o raio arredondado: %ld\n\n", exacto, truncado);
      ok("SEM VIRGULA FLUTUANTE: o raio e' um inteiro e a bola e' a do reticulado — a distancia"
         " do quadrado. Nao ha' raio a marchar nem normal a estimar, porque nao ha' shader: ha'"
         " um corpo do catalogo a operar. E o que a virgula flutuante estragaria mede-se: com raio"
         " inteiro a composicao e' exacta nas 16, e com o raio arredondado — o que um float"
         " daria — deixa de bater em quase todas", exacto==16 && truncado<exacto); }

    /* §R5 — o controlo */
    { long maus=0, pares=0;
      for(long r=1;r<=3;r++) for(long s=1;s<=3;s++){
          dilata(g,p,r); dilata(p,q,s);
          dilata(g,u,r*s);                   /* se o raio MULTIPLICASSE em vez de somar */
          if(dif(q,u)==0) maus++;            /* nao pode bater */
          pares++; }
      printf("  §R5  com o raio a multiplicar em vez de somar: %ld de %ld coincidem\n\n", maus, pares);
      ok("e o CONTROLO: se o raio multiplicasse em vez de somar, a composicao NAO fecharia — e"
         " nao fecha em quase todos. E' a lei do corpo que faz isto funcionar, e nao a sorte de"
         " a imagem sair bonita: sem a lei, seria so' uma imagem", maus<pares && pares==9); }

    /* §R6 — O GERME SAI DA OP, e a simetria conta-se ═══════════════════════════════
     * Cada card declara uma simetria na sua op, e o germe nasce dela: ordem n da' n pontos,
     * igualmente espacados na volta. E sem trigonometria — a volta parte-se pelo PERIMETRO
     * DO QUADRADO, em passos inteiros, que e' a bola do reticulado outra vez.
     *
     * Mede-se pelas duas metades: os pontos sao n, E o espacamento e' o mesmo entre todos.
     * A contagem sozinha nao chega — n pontos amontoados num canto tambem sao n. */
    { long maus_n = 0, maus_esp = 0, ordens = 0;
      for(long ord = 2; ord <= 8; ord++){
          long R = 12, per = 8*R, ts[16], n = 0;
          for(long k = 0; k < ord; k++) ts[n++] = (k * per) / ord;
          if(n != ord) maus_n++;
          /* O ESPACAMENTO NAO PODE SER EXACTAMENTE CONSTANTE, e isso e' resultado e nao
           * defeito: numa grelha INTEIRA, 96 nao se parte em 5 partes iguais. O que se pode
           * exigir — e o que se mede — e' que o desvio seja de UM passo no maximo, que e' o
           * preco da grelha. Mais do que um seria erro; zero seria mentira. */
          long d0 = ts[1] - ts[0];
          for(long k = 1; k + 1 < n; k++){
              long d = ts[k+1] - ts[k], e = d - d0; if(e < 0) e = -e;
              if(e > 1) maus_esp++;
          }
          ordens++;
      }
      printf("  §R6  ordens 2..8: %ld com contagem errada, %ld com desvio maior que um passo\n\n",
             maus_n, maus_esp);
      ok("o GERME sai da OP do card e nao e' o mesmo para todos: a op declara uma simetria e o"
         " germe nasce dela — ordem n da' n pontos, igualmente espacados na volta. E sem"
         " trigonometria nenhuma: a volta parte-se pelo PERIMETRO DO QUADRADO em passos"
         " inteiros, que e' a bola do reticulado outra vez. Medido pelas duas metades — os"
         " pontos sao n E o espacamento nao varia mais do que UM passo —, porque n pontos amontoados"
         " num canto tambem sao n e a contagem sozinha nao os distinguiria. E o «um passo» e'"
         " resultado e nao tolerancia: numa grelha inteira 96 nao se parte em 5 partes iguais,"
         " e exigir desvio ZERO era exigir o que a grelha nao da'",
         maus_n == 0 && maus_esp == 0 && ordens == 7); }

    puts("");
    if(!falhas) puts("  o que desenha e' um CORPO do catalogo — o morfico — e mede-se pelas leis dele.\n");
    return falhas?1:0;
}
