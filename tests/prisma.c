/* prisma.c — O CORPO PRISMÁTICO: o triângulo deformado até encher, e o círculo do rei.
 *
 * O Aarão: "a cifra real que eu proponho é o triângulo; precisamos deformá-lo de modo a preencher
 * a área inteira, é equivalente a transformá-lo num círculo, e círculo é ouro do rei. É semelhante
 * ao hipercorpo com a curva de Hilbert, só que no caso triangular Hilbert é quadrado. Começa da
 * reta e aplica o conjunto de Cantor — dá no mesmo, triângulo vira círculo."
 *
 * O hipercorpo tinha 16 vértices (o tesseracto) e o gerador era o caminho por arestas. Aqui são
 * TRÊS, e o caminho por arestas de um triângulo é a rotação de 120°. Então a pergunta não é de
 * opinião: QUE CORPO É A ROTAÇÃO DE ORDEM 3? Tem traço e determinante, e o catálogo responde.
 *
 * E a segunda pergunta é a do Cantor: a reta subdividida em três, e o que isso dá.
 *
 *   §S1  o gerador do triângulo: três vértices, e cada passo é uma ARESTA
 *   §S2  a rotação de ordem 3 tem régua — e ela é ELÍPTICA, que é o redondo
 *   §S3  o Cantor: a reta em três, e a dimensão que sai disso
 *   §S4  onde o prismático cai no catálogo — e se abre lugar novo ou não
 *
 *   cc -O2 -std=c99 prisma.c -o prisma && ./prisma
 */
#include <stdio.h>
#include <string.h>
#include "corpos.h"
#include "contrato.h"
#include "cifra.h"
#include "unidade.h"

int main(void){
printf("\n=== O CORPO PRISMÁTICO — o triângulo, e o círculo do rei ==================\n");

printf("\n§S1  O gerador do triângulo: três vértices, e cada passo é uma ARESTA.\n\n");
{
    /* no tesseracto o gerador era g(i) = i^(i>>1), 16 vertices, um bit por passo. Num triangulo
     * sao TRES vertices e o caminho por arestas e o ciclo 0->1->2->0: a rotacao. */
    int v[3] = { 0, 1, 2 }, mau = 0, arestas = 0;
    printf("      i    vértice   passo\n");
    for(int i = 0; i < 3; i++){
        printf("      %-4d %-9d %s\n", i, v[i], i ? "aresta" : "início");
        if(i){ if(v[i] != (v[i-1] + 1) % 3) mau++; else arestas++; }
    }
    printf("      (e do 2 volta ao 0: fecha o ciclo)\n\n");
    ok("o gerador percorre os três vértices, cada passo por uma aresta", mau == 0 && arestas == 2);
    printf("      É o mesmo desenho do hipercorpo, com três em vez de dezasseis: o poliedro\n");
    printf("      percorre-se a si próprio, e o gerador é esse caminho.\n");
}

printf("\n§S2  A rotação de ordem 3 tem régua — e ela é ELÍPTICA.\n\n");
{
    /* a rotacao de 120 graus: traco = 2cos(120) = -1, determinante = 1 */
    Regua R = { -1, 1 };
    long D = ct_assinatura(R);
    printf("      rotação de 120°   traço = %ld   det = %ld   Δ = %ld\n\n", R.B, R.C, D);
    ok("Δ < 0: a rotação de ordem 3 é ELÍPTICA — e elíptico é o REDONDO", D < 0);
    /* e ela tem mesmo ordem 3: tres voltas e volta ao ponto */
    long mau = 0;
    for(long a = -8; a <= 8; a++) for(long b = -8; b <= 8; b++){
        Par x = { a, b }, y = x;
        for(int k = 0; k < 3; k++){ Par n = { -y.b, y.a + R.B*y.b }; y = n; }
        if(y.a != x.a || y.b != x.b) mau++;
    }
    printf("      289 pontos rodados TRÊS vezes: %ld que não voltam\n", mau);
    ok("a rotação tem mesmo ordem 3 — três voltas e resíduo 0", mau == 0);
    printf("\n      E aqui está o que o Aarão disse, e não fui eu que o pus: TRIÂNGULO VIRA\n");
    printf("      CÍRCULO. A régua do triângulo é a elíptica, e a elíptica é a redonda — a\n");
    printf("      norma definida positiva, sem cone nulo. Não houve deformação a fazer: a\n");
    printf("      rotação que percorre o triângulo JÁ É a que fecha o círculo.\n");
}

printf("\n§S3  O Cantor: a reta em três, e o que sai disso.\n\n");
{
    /* o conjunto de Cantor: a reta dividida em 3, fica-se com 2. A cada nivel, 2^k pedacos de
     * comprimento 3^-k. O que se mede: a medida vai a ZERO e a contagem vai a INFINITO. */
    /* A MEDIDA É UM RACIONAL EXACTO, 2^k/3^k, e é aqui que ela estava a mentir. Havia
     *
     *     long med = 1.0;   …   med = med * 2.0 / 3.0;   …   ok(…, med < 0.1 …)
     *
     * e a Fase A, ao trocar o tipo, matou a construção inteira: 1*2.0/3.0 trunca para
     * ZERO no primeiro nível, a coluna «medida total» imprimia 1,0,0,0,0,0,0 e a
     * asserção «a medida vai a zero» passou a ser «0 < 0.1» — verdadeira antes de o
     * Cantor começar. A coluna do comprimento era pior: `1.0/med*pedacos*med/pedacos*0
     * + (long)(k?1:1)`, que é uma divisão por zero anulada por um `*0` e um 1 fixo.
     *
     * Em ℚ não há nada a truncar: 2^k e 3^k são inteiros e a razão é a classe. */
    printf("      nível   pedaços   comprimento de cada   medida total\n");
    Par med = ra_classe((Par){1,1});
    long pedacos = 1, tres_k = 1, mau = 0, decresce = 0;
    for(int k = 0; k <= 6; k++){
        printf("      %-7d %-9ld 1/%-19ld %ld/%ld\n", k, pedacos, tres_k, med.a, med.b);
        if(med.a * tres_k != pedacos * med.b) mau++;      /* medida = pedaços/3^k */
        if(k < 6){
            Par ant = med;
            pedacos *= 2; tres_k *= 3;
            med = ra_prod(med, ra_classe((Par){2,3}));
            if(ra_cmp(med, ant) < 0) decresce++;          /* encolhe SEMPRE, e conta-se */
        }
    }
    /* «vai a zero» com o ε EXIBIDO e a testemunha devolvida: dado ε = 1/N, existe k com
     * 2^k/3^k < ε. Sem o k a frase é uma esperança; com ele é uma medida. */
    long k_test = -1;
    { Par m = ra_classe((Par){1,1}), eps = ra_classe((Par){1,100});
      for(long k = 0; k <= 40 && k_test < 0; k++){
          if(ra_cmp(m, eps) < 0){ k_test = k; break; }
          m = ra_prod(m, ra_classe((Par){2,3}));
      } }
    printf("\n      a medida encolhe por 2/3 a cada nível: %ld de 6 passos ESTRITAMENTE\n",
           decresce);
    printf("      e dado ε = 1/100, a testemunha é k = %ld — 2^k/3^k < ε a partir dali\n",
           k_test);
    ok("A MEDIDA VAI A ZERO E A CONTAGEM VAI AO INFINITO, e as duas em ℚ EXACTO: a medida"
       " do nível k é 2^k/3^k, confere-se contra pedaços/3^k em todos os sete níveis, e"
       " encolhe ESTRITAMENTE nos seis passos. E «vai a zero» diz-se como se deve — com o"
       " ε racional EXIBIDO (1/100) e o k que o cumpre devolvido, não com um limiar meu"
       " contra um valor truncado",
       mau == 0 && pedacos == 64 && decresce == 6 && k_test > 0 && k_test <= 40);
    printf("\n      E É POR ISSO QUE O CANTOR SOZINHO NÃO ENCHE. Ele tira o meio e o que fica\n");
    printf("      tem medida nula — é pó, não é área. Para ENCHER o triângulo a subdivisão tem\n");
    printf("      de GUARDAR os três, e não dois: é a diferença entre o Cantor e o Sierpinski\n");
    printf("      cheio. Fica dito, porque eu quase escrevi que dava no mesmo.\n");
}

printf("\n§S4  Onde o prismático cai no catálogo.\n\n");
{
    long a[64], b[64];
    size_t na = cifra_geral(0, 0, -1, 1, -1, a, 64);      /* o prismático: razão −1, sinal +1 */
    size_t nb = cifra_geral(0, 0,  1, 1,  1, b, 64);      /* o fractal/Eisenstein: (1,1) */
    printf("      prismático (-1,1)   [");
    for(size_t k = 0; k < na; k++) printf("%s%ld", k?";":"", a[k]);
    printf("]\n      fractal    ( 1,1)   [");
    for(size_t k = 0; k < nb; k++) printf("%s%ld", k?";":"", b[k]);
    printf("]\n\n");
    printf("      Δ dos dois: %ld e %ld\n", ct_assinatura((Regua){-1,1}), ct_assinatura((Regua){1,1}));
    ok("o prismático tem o MESMO Δ do fractal — os dois são Eisenstein, Δ = −3",
       ct_assinatura((Regua){-1,1}) == ct_assinatura((Regua){1,1}));
    int igual = (na == nb) && !memcmp(a, b, na*sizeof(long));
    printf("      e a cifra é a mesma? %s\n", igual ? "sim" : "não");
    ok("mas a CIFRA distingue-os — mesmo Δ não é mesmo lugar", !igual);
    printf("\n      Então o prismático NÃO abre lugar novo por Δ (é Eisenstein, como o fractal e\n");
    printf("      o relógio) mas TEM cifra própria, porque a régua é outra: o traço é −1 e não 1.\n");
    printf("      É a mesma família redonda, com a rotação para o outro lado.\n");
}

printf("\n");
return falhas ? 1 : 0;
}
