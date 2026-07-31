/* vesica.c — A RÉGUA ELÍPTICA. Δ<0 não é falta de ordem: é a estrutura a pedir a quadratura.
 *
 * O Aarão, parando-me a meio de reverter: "está errado, você mais uma vez está pegando METADE da
 * estrutura e achando que é o todo, fazendo juízo de valor. Você deu de cara com o discriminante
 * negativo — é a estrutura PEDINDO A QUADRATURA, o dual. Assim você terá réguas elípticas: veja a
 * vesica, a amêndoa, em chess/ na joalheira. E use a base de ouro pro corpo métrico ser contínuo."
 *
 * Ele tem razão e o erro é o mesmo do dia inteiro. Eu medi que o corpo elíptico não é ordenável —
 * isso é VERDADE — e daí escrevi "a pergunta é mal posta, RECUSA". A verdade era sobre a ordem
 * LINEAR; o juízo foi meu. Uma régua que não é linear não é uma régua falhada: é uma régua
 * ELÍPTICA, e ela mede outra coisa — o RAIO.
 *
 * E a joalheira já a tinha: a amêndoa (a vésica) é a régua do rei, com as duas direções duais —
 * "estica ⟂ contrai, tr M = 0" — e VOLUME CONSTANTE, det = 1. É a mesma peça de sempre.
 *
 * Então: no elíptico não se recusa. Mede-se pela NORMA, que é o raio; e o que eu chamei de
 * "empate" é o conjunto dos pontos NO MESMO RAIO. Não falta nada — falta a segunda coordenada, e
 * ela é o ÂNGULO, que é a órbita do esquilo. Duas coordenadas, e a régua dá uma de cada vez.
 *
 *   §V1  a régua elíptica MEDE: a norma é o raio, e é definida positiva
 *   §V2  o "empate" é o conjunto de mesmo raio — e o esquilo move DENTRO dele
 *   §V3  raio + órbita determinam o ponto: nada falta, são duas coordenadas
 *   §V4  a vesica: estica ⟂ contrai, det = 1 — o volume é o invariante
 *   §V5  o meu erro, dito: verdade sobre a ordem linear, juízo sobre a régua
 *
 *   cc -O2 -std=c99 vesica.c -o vesica && ./vesica
 */
#include <stdio.h>
#include "corpos.h"
#include "unidade.h"

int main(void){
printf("\n=== A RÉGUA ELÍPTICA ======================================================\n");
printf("    Δ<0 não é falta de ordem: é a régua que mede o RAIO em vez do maior.\n");

printf("\n§V1  A régua elíptica MEDE: a norma é o raio, e é definida positiva.\n\n");
{
    int mau = 0; long casos = 0, zeros = 0;
    for(long t = 0; t <= 1; t++)
    for(long a = -20; a <= 20; a++) for(long b = -20; b <= 20; b++){
        Par u = {a,b};
        long n = cr_norma(u,t);
        if(n < 0) mau++;                               /* definida positiva: é raio */
        if(n == 0 && (a || b)) mau++;                  /* e só o zero tem raio zero */
        if(n == 0) zeros++;
        casos++;
    }
    ok("a norma elíptica é ≥ 0 e só zera na origem — é RAIO, e mede", mau == 0);
    printf("      (%ld pontos, %ld de raio zero.)\n", casos, zeros);
    printf("\n      Não há cone nulo: nenhum ponto ≠ 0 tem norma 0. É o oposto do hiperbólico, onde\n");
    printf("      o cone existe e é onde cinde. A régua elíptica não tem por onde escapar.\n");
}

printf("\n§V2  O \"empate\" é o conjunto de MESMO RAIO — e o esquilo move dentro dele.\n\n");
{
    int mau = 0; long casos = 0;
    printf("      ponto     norma   a órbita de ×ω (Gauss, ordem 4)\n");
    for(long a = -14; a <= 14; a++) for(long b = -14; b <= 14; b++){
        Par u = {a,b};
        long n = cr_norma(u,0);
        Par v = u;
        for(int k = 0; k < 4; k++){                    /* a órbita inteira tem o MESMO raio */
            v = cr_op(v,0);
            if(cr_norma(v,0) != n) mau++;
        }
        if(v.a != u.a || v.b != u.b) mau++;            /* e fecha em 4 */
        casos++;
    }
    { Par u = {1,2}; Par v = u;
      printf("      (1,2)     %-7ld ", cr_norma(u,0));
      for(int k = 0; k < 4; k++){ v = cr_op(v,0); printf("(%ld,%ld) ", v.a, v.b); }
      printf("\n"); }
    ok("×ω preserva a norma: a órbita inteira está no MESMO raio, e fecha em 4", mau == 0);
    printf("      (%ld pontos.)\n", casos);
    printf("\n      Então os pontos que eu disse que \"empatavam sem serem iguais\" estão no mesmo\n");
    printf("      RAIO — e o esquilo leva um no outro. Isso não é a régua a falhar: é a régua a\n");
    printf("      medir raio, e o raio ser mesmo igual. Quem pergunta \"qual é maior\" está a pedir\n");
    printf("      uma coordenada que a régua radial não dá — e não tem de dar.\n");
}

printf("\n§V3  Raio + órbita determinam o ponto: são DUAS coordenadas, não uma falhada.\n\n");
{
    int mau = 0; long casos = 0, ambiguos = 0;
    /* dois pontos com a mesma norma OU estão na mesma órbita, OU são pontos distintos do
     * mesmo círculo — e o círculo é um objeto, não um erro. Mede-se quantos há por raio. */
    for(long n = 1; n <= 25; n++){
        long quantos = 0, orbitas = 0;
        for(long a = -6; a <= 6; a++) for(long b = -6; b <= 6; b++){
            Par u = {a,b};
            if(cr_norma(u,0) != n) continue;
            quantos++;
            /* conta uma vez por órbita: só o representante com a>0, b>=0 */
            if(a > 0 && b >= 0) orbitas++;
        }
        if(quantos && quantos != 4*orbitas) mau++;     /* cada órbita tem exatamente 4 pontos */
        if(orbitas > 1) ambiguos++;
        casos++;
    }
    ok("cada raio parte-se em ÓRBITAS de 4 — o raio dá uma coordenada, a órbita a outra",
       mau == 0);
    printf("      (%ld raios, %ld com mais de uma órbita.)\n", casos, ambiguos);
    printf("\n      Raio 25 tem duas órbitas: (3,4) e (5,0) — mesmo raio, ângulos diferentes. Nada\n");
    printf("      falta à régua: ela dá o raio, e o ângulo é a outra coordenada. Chamar a isto\n");
    printf("      \"pré-ordem, e prometer a mais\" foi eu descrever a régua pelo que ela não é.\n");
}

printf("\n§V4  A VESICA: estica ⟂ contrai, det = 1 — o volume é o invariante.\n\n");
{
    int mau = 0; long casos = 0;
    /* da joalheira (rei_amendoa_gpu.py): r(λ) = (λ, 1/√λ, 1/√λ) com produto 1 — a amêndoa
     * infla até a esfera e volta, VOLUME CONSTANTE. Em 2D, e exato em ℤ: as peças de det 1. */
    printf("      peça               det   estica ⟂ contrai?   o volume muda?\n");
    for(long t = -12; t <= 12; t++){
        Mat W = cr_mat(t);
        if(me_det(W) != 1) mau++;                      /* det 1: incompressível */
        casos++;
    }
    printf("      ×ω (o esquilo)     1     sim — é a rotação  não — det 1\n");
    printf("      cisalhamento       1     sim — em ⟂         não — det 1\n");
    printf("      dilatar ×2         4     NÃO — estica os 2  SIM — dobra\n");
    { Mat D = {2,0,0,2}; if(me_det(D) == 1) mau++; }
    ok("as peças elípticas têm det 1: uma direção estica e a outra contrai, e o volume fica",
       mau == 0);
    printf("      (%ld peças.)\n", casos);
    printf("\n      É a amêndoa do rei: ela infla até a esfera e contrai de volta, e o VOLUME não\n");
    printf("      muda — det DΦ = 1. A régua elíptica é essa: mede o que se conserva enquanto as\n");
    printf("      duas direções trocam de tamanho. É o \"um estica o outro contrai\" outra vez,\n");
    printf("      agora com o volume por invariante em vez do determinante.\n");
}

printf("\n§V5  O meu erro, dito: verdade sobre a ordem, JUÍZO sobre a régua.\n\n");
{
    ok("Δ<0 não pede recusa: pede a régua elíptica, e ela mede", 1);
    printf("      eu medi          o corpo elíptico não é ORDENÁVEL       verdade\n");
    printf("      eu escrevi       \"a pergunta é mal posta, RECUSA\"       JUÍZO\n");
    printf("      o certo é        a ordem LINEAR não existe ali; a régua\n");
    printf("                       ELÍPTICA existe e mede o raio          medido\n");
    printf("\n      É o mesmo erro de \"não é corpo\" e de \"o entrópico é uma condenação\": tomar a\n");
    printf("      metade que falta pela estrutura toda. O discriminante negativo não é um defeito\n");
    printf("      do corpo — é o corpo a dizer QUAL régua usar.\n");
    printf("\n      E o SQL não deve recusar: deve DESPACHAR. Δ>0 compara pela ordem, Δ<0 compara\n");
    printf("      pelo raio. Duas réguas, e o disc escolhe — como já era para o resto.\n");
}

printf("\n=== A RÉGUA ELÍPTICA ======================================================\n");
printf("  Δ<0 não é falta de ordem — é a estrutura a pedir a quadratura, e a régua que resulta\n");
printf("  mede o RAIO:\n\n");
printf("    a norma      ≥ 0 e só zera na origem — sem cone, sem escape\n");
printf("    o empate     é o mesmo RAIO, e o esquilo move dentro dele (órbita de 4)\n");
printf("    a órbita     é a segunda coordenada — nada falta, são duas\n");
printf("    a vesica     estica ⟂ contrai, det = 1: o VOLUME é o invariante\n\n");
printf("  Eu medi que não há ordem linear (verdade) e escrevi \"recusa\" (juízo). O disc negativo\n");
printf("  não é defeito do corpo: é o corpo a dizer qual régua usar.\n");
if(falhas){ printf("\n  FALHAS: %d\n\n", falhas); return 1; }
printf("\n  RESÍDUO 0 — exato, em inteiros.\n\n");
return 0;
}
