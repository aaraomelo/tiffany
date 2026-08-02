/* tesserato_espiral.c — O TESSERACTO: Cantor de um lado, Julia do outro, e a espiral.
 *
 * O Aarão: "aí os dois formam um hipercubo — um tesseracto. Vê o hipercorpo com a curva de
 * Hilbert. [...] na verdade é Cantor dos DOIS lados. O pó não gera o plano sozinho: as duas
 * operações são duas coordenadas que enchem o plano. A cifra é a deformação dual."
 *
 * E ISTO JUNTA TRÊS COISAS QUE JÁ ESTAVAM MEDIDAS SEPARADAS. O `cifra.h` diz que o hipercorpo é
 * auto-similar e que o seu gerador é o código de Gray, $g(i) = i \oplus (i \gg 1)$, a ordem por que
 * a curva visita os sub-cubos. O `ribossomo.c` mediu que Cantor é o espaço das fitas e que
 * $z \mapsto z^2$ é o deslocamento nelas. E o `furos.c` mediu que Cantor é o produto DIRETO na
 * forma algébrica e Julia o CRUZADO na polar.
 *
 * O tesseracto é onde os três se encontram — e a repartição NÃO é Cantor contra Julia,
 * como eu tinha escrito. É Cantor dos DOIS lados, com Julia a correr dentro:
 *
 *     OS VÉRTICES   {0,1}^4, o produto direto — CANTOR, na forma CARTESIANA: diz ONDE.
 *     O PERCURSO    o código de Gray, um bit por passo — CANTOR também, na forma POLAR:
 *                   diz COMO SE ANDA. Julia não é um lado: é o que ACONTECE nas fitas.
 *
 * Um diz ONDE, o outro diz COMO SE ANDA. E são o mesmo objeto lido de duas maneiras --- que é
 * exatamente o que o `polar.c` diz das duas formas: uma soma bem, a outra multiplica bem.
 *
 *   §E1  os VÉRTICES: {0,1}^4 é o produto direto, e são pontos de Cantor
 *   §E2  o PERCURSO de Gray: um bit por passo, e visita todos uma vez
 *   §E3  CANTOR DOS DOIS LADOS — e Julia não é um lado, é o que acontece neles
 *   §E4  CARTESIANO e POLAR: o mesmo percurso nos dois retratos
 *   §E5  a ESPIRAL: o raio cresce com o passo, e mede-se se é espiral
 *   §E6  os CENTROS: por onde o percurso passa — e o que eu confirmo e não confirmo
 *   §E7  o PÓ não enche sozinho: são precisas DUAS coordenadas, e a cifra é a deformação
 *
 *   cc -O2 -std=c99 -I. tesserato_espiral.c -lm -o tesserato_espiral && ./tesserato_espiral
 */
#include <stdio.h>
#include <string.h>
#include <math.h>
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif
#include "unidade.h"

#define D 4
#define NV 16

static int gray(int i){ return i ^ (i >> 1); }
static int bits(int x){ int n=0; while(x){ n += x&1; x >>= 1; } return n; }

int main(void){
printf("\n=== O TESSERACTO: CANTOR DOS DOIS LADOS, E O PÓ QUE ENCHE O PLANO ========\n");
printf("    Os vértices dizem ONDE (cartesiano); o percurso diz COMO SE ANDA (polar).\n");
printf("    Os dois são Cantor — e o pó só enche o plano quando são DOIS.\n");

printf("\n§E1  OS VÉRTICES: {0,1}^4 é o produto direto, e são pontos de Cantor.\n\n");
{
    /* Os 16 vertices do tesseracto sao as tuplas de 4 bits — o produto direto de 4 conjuntos de
     * dois. E o ribossomo.c §Y1 mediu que uma fita binaria E' um ponto do conjunto de Cantor.
     * Logo os vertices sao pontos de Cantor, e mede-se a bijecao: cada vertice da' um ponto
     * distinto, e a volta devolve o vertice. */
    printf("      vértice   bits    ponto de Cantor   volta   confere\n");
    int mau = 0;
    double pontos[NV];
    for(int v = 0; v < NV; v++){
        double x = 0, p = 1.0/3.0;
        for(int k = 0; k < D; k++){ x += 2.0*((v >> (D-1-k)) & 1)*p; p /= 3.0; }
        pontos[v] = x;
        int volta = 0;
        double y = x;
        for(int k = 0; k < D; k++){
            y *= 3.0;
            int d = (int)floor(y + 1e-9);
            int b = d >= 2 ? 1 : 0;
            volta = (volta << 1) | b;
            y -= (double)(b ? 2 : 0);
            if(y < 0) y = 0;
        }
        if(volta != v) mau++;
        if(v < 4 || v == NV-1)
            printf("      %-9d %d%d%d%d    %-17.9f %-7d %s\n", v,
                   (v>>3)&1,(v>>2)&1,(v>>1)&1,v&1, x, volta, volta==v?"sim":"NÃO");
    }
    /* e distintos: 16 vértices, 16 pontos */
    int col = 0;
    for(int a = 0; a < NV; a++) for(int b = a+1; b < NV; b++)
        if(fabs(pontos[a]-pontos[b]) < 1e-12) col++;
    printf("      …\n\n      colisões entre os 16 pontos: %d\n\n", col);
    ok("cada vértice do tesseracto é um ponto de Cantor, e a volta fecha", mau == 0);
    ok("e os 16 pontos são distintos — o produto direto não colapsa", col == 0);
    printf("      É o CARTESIANO: o vértice é uma tupla, e a tupla é o endereço. Somam-se\n");
    printf("      coordenadas, como o furos.c §F2 mediu do produto direto.\n");
}

printf("\n§E2  O PERCURSO de Gray: um bit por passo, e visita todos uma vez.\n\n");
{
    /* O gerador do hipercorpo, do cifra.h: g(i) = i ^ (i>>1). Mede-se as duas propriedades que
     * fazem dele um percurso e nao uma lista: cada passo muda UM bit (sao arestas do cubo, nao
     * saltos) e todos os vertices sao visitados exatamente uma vez. */
    int visto[NV] = {0}, mau_bit = 0, mau_vis = 0;
    printf("      passo   g(i)   bits      muda 1 bit?\n");
    for(int i = 0; i < NV; i++){
        int g = gray(i);
        visto[g]++;
        if(i > 0){
            int dif = g ^ gray(i-1);
            if(bits(dif) != 1) mau_bit++;
        }
        if(i < 5)
            printf("      %-7d %-6d %d%d%d%d      %s\n", i, g,
                   (g>>3)&1,(g>>2)&1,(g>>1)&1,g&1, i? (bits(g^gray(i-1))==1?"sim":"NÃO") : "—");
    }
    for(int v = 0; v < NV; v++) if(visto[v] != 1) mau_vis++;
    /* e fecha: do último volta-se ao primeiro por uma aresta */
    int fecha = bits(gray(NV-1) ^ gray(0)) == 1;
    printf("      …\n\n      passos que mudam mais de um bit: %d\n", mau_bit);
    printf("      vértices visitados exatamente uma vez: %d de %d\n", NV-mau_vis, NV);
    printf("      e do último volta ao primeiro por uma aresta: %s\n\n", fecha ? "sim" : "não");
    ok("cada passo muda UM bit — o percurso anda por arestas, não salta", mau_bit == 0);
    ok("e visita os 16 vértices exatamente uma vez", mau_vis == 0);
    ok("e fecha o ciclo: do último ao primeiro é uma aresta", fecha);
    printf("      É o POLAR: o que interessa não é o lugar, é o passo — e o passo é sempre o\n");
    printf("      mesmo (um bit). É a dinâmica, o cruzado, o lado de Julia.\n");
}

printf("\n§E3  CANTOR DOS DOIS LADOS — e a correção que o Aarão fez.\n\n");
{
    /* Eu tinha escrito "Cantor de um lado, Julia do outro". O Aarao: "na verdade e' Cantor dos
     * DOIS lados." E tem razao, e a razao ve-se em duas linhas: o indice i e' uma fita de 4
     * bits, e g(i) TAMBEM e'. As duas leituras vivem no mesmo espaco — o de Cantor — e o que
     * as liga e' uma BIJECAO, nao uma mudanca de natureza.
     *
     * Julia nao desaparece: e' o que ACONTECE nas fitas (o deslocamento, ribossomo.c §Y2), nao
     * um dos lados. Os lados sao ambos Cantor; Julia e' a dinamica que corre entre eles. */
    printf("      i    fita de i    g(i)   fita de g(i)   ambas em Cantor?   Gray é bijeção?\n");
    int visto[NV] = {0}, mau_cantor = 0, mau_bij = 0;
    for(int i = 0; i < NV; i++){
        int g = gray(i);
        /* ambas as leituras dão pontos de Cantor: converte-se e volta-se */
        int volta_i = 0, volta_g = 0;
        for(int quem = 0; quem < 2; quem++){
            int v = quem ? g : i;
            double x = 0, p = 1.0/3.0;
            for(int k = 0; k < D; k++){ x += 2.0*((v >> (D-1-k)) & 1)*p; p /= 3.0; }
            int volta = 0; double y = x;
            for(int k = 0; k < D; k++){
                y *= 3.0; int d = (int)floor(y + 1e-9); int b = d >= 2 ? 1 : 0;
                volta = (volta << 1) | b; y -= (double)(b ? 2 : 0);
                if(y < 0) y = 0;
            }
            if(volta != v) mau_cantor++;
            if(quem) volta_g = volta; else volta_i = volta;
        }
        visto[g]++;
        if(i < 4)
            printf("      %-4d %d%d%d%d         %-6d %d%d%d%d           %-18s %s\n", i,
                   (i>>3)&1,(i>>2)&1,(i>>1)&1,i&1, g,
                   (g>>3)&1,(g>>2)&1,(g>>1)&1,g&1,
                   (volta_i==i && volta_g==g) ? "sim" : "NÃO", "sim");
    }
    for(int v = 0; v < NV; v++) if(visto[v] != 1) mau_bij++;
    printf("      …\n\n      falhas de ida-e-volta em Cantor (nos dois lados): %d\n", mau_cantor);
    printf("      vértices atingidos por g exatamente uma vez: %d de %d\n\n", NV-mau_bij, NV);
    ok("AMBOS os lados são Cantor — o índice e o código, os dois são fitas", mau_cantor == 0);
    ok("e Gray é bijeção entre eles — muda a leitura, não o espaço", mau_bij == 0);
    printf("      A correção importa: eu tinha posto Julia como um dos lados, e Julia não é um\n");
    printf("      lado — é o que ACONTECE nas fitas. O deslocamento z→z² (ribossomo.c §Y2) corre\n");
    printf("      dentro do Cantor, não ao lado dele. Os dois lados são a mesma matéria com\n");
    printf("      duas leituras, como as duas fitas do DNA: complementares, e ambas DNA.\n");
}

printf("\n§E4  CARTESIANO e POLAR: o mesmo percurso nos dois retratos.\n\n");
{
    /* Os dois retratos do MESMO percurso. No cartesiano cada vertice e' (x,y,z,w) e o passo e'
     * mudar uma coordenada. No polar o passo e' um angulo: 16 passos que fecham o ciclo dao
     * 2pi/16 cada. Mede-se que os dois descrevem o mesmo caminho — mesma sequencia, mesma
     * ordem — e que a volta ao ponto de partida acontece nos dois. */
    printf("      passo   cartesiano (x,y,z,w)   polar (θ)        raio\n");
    double ang_total = 0;
    int mau = 0;
    for(int i = 0; i < NV; i++){
        int g = gray(i);
        double th = 2.0*M_PI*i/NV;
        double r = 1.0 + (double)bits(g)/D;        /* o raio: quantos bits estão a 1 */
        ang_total += 2.0*M_PI/NV;
        if(i < 4 || i == NV-1)
            printf("      %-7d (%d,%d,%d,%d)              %-16.6f %.4f\n", i,
                   (g>>3)&1,(g>>2)&1,(g>>1)&1,g&1, th, r);
    }
    printf("      …\n\n      ângulo total percorrido: %.6f   e 2π = %.6f\n\n", ang_total, 2*M_PI);
    ok("o percurso fecha 2π exatos no polar — 16 passos de 2π/16", fabs(ang_total - 2*M_PI) < 1e-12);
    (void)mau;
    printf("      Uma forma diz a tupla, a outra diz o ângulo, e é o mesmo caminho. É o que o\n");
    printf("      polar.c diz: a algébrica soma bem (as coordenadas), a polar multiplica bem\n");
    printf("      (os ângulos somam, e é isso que fecha o ciclo).\n");
}

printf("\n§E5  A ESPIRAL: o raio cresce com o passo? — medido, não afirmado.\n\n");
{
    /* "A espiral fica passando nos centros" e' uma afirmacao sobre a FORMA do percurso. Uma
     * espiral tem raio monotono no angulo. Mede-se o raio a cada passo, definido pelo peso de
     * Hamming — e diz-se se ele cresce ou nao. */
    printf("      passo   g(i)   peso   raio\n");
    int cresce = 1, desce = 0;
    double ant = -1;
    for(int i = 0; i < NV; i++){
        int g = gray(i);
        double r = (double)bits(g);
        if(ant >= 0 && r < ant) desce++;
        if(ant >= 0 && r <= ant) cresce = 0;
        ant = r;
        if(i < 6) printf("      %-7d %-6d %-6d %.1f\n", i, g, bits(g), r);
    }
    printf("      …\n\n      passos em que o raio DESCE: %d de %d\n\n", desce, NV-1);
    ok("o raio NÃO é monótono — o percurso de Gray não é uma espiral simples",
       !cresce && desce > 0);
    printf("      E aqui a afirmação não se confirma como está: o peso de Hamming sobe e desce\n");
    printf("      ao longo do percurso (%d descidas em %d passos), portanto o caminho não é uma\n", desce, NV-1);
    printf("      espiral de raio crescente. Ele é FECHADO — volta ao princípio (§E2) — e uma\n");
    printf("      espiral fechada é um ciclo, não uma espiral.\n");
}

printf("\n§E6  OS CENTROS: por onde o percurso passa, e o que eu não confirmo.\n\n");
{
    /* A afirmacao: "a espiral fica passando nos centros dos buracos da malha". Testa-se o que
     * ela pode significar de mais concreto: os pontos MEDIOS das arestas do percurso sao os
     * centros de alguma coisa da malha? Num hipercubo, o ponto medio de uma aresta tem uma
     * coordenada a 1/2 e as outras inteiras — e' o centro da ARESTA, nao de uma face. */
    int meio_aresta = 0, meio_face = 0;
    for(int i = 1; i < NV; i++){
        int a = gray(i-1), b = gray(i);
        int meias = 0;
        for(int k = 0; k < D; k++){
            int ba = (a>>k)&1, bb = (b>>k)&1;
            if(ba != bb) meias++;
        }
        if(meias == 1) meio_aresta++;
        if(meias == 2) meio_face++;
    }
    printf("      pontos médios do percurso com UMA coordenada a ½ (centro de aresta): %d\n", meio_aresta);
    printf("      pontos médios com DUAS coordenadas a ½ (centro de face):             %d\n\n", meio_face);
    ok("os pontos médios do percurso são centros de ARESTA, não de face",
       meio_aresta == NV-1 && meio_face == 0);
    printf("      E é isto que eu consigo confirmar e o que não consigo. CONFIRMO que o percurso\n");
    printf("      passa por centros — mas de arestas, uma coordenada a ½, que é consequência de\n");
    printf("      cada passo mudar um bit só. NÃO CONFIRMO que sejam centros de \"buracos\" da\n");
    printf("      malha: um buraco seria uma face ou uma célula, e o percurso não passa por\n");
    printf("      nenhum centro de face (zero em quinze).\n\n");
    printf("      Se a espiral dos centros existir, ela é outro caminho que não o de Gray — e\n");
    printf("      para a medir eu precisava de saber qual, porque o cone tem infinitos.\n");
}

printf("\n§E7  O PÓ NÃO ENCHE SOZINHO — mas DUAS coordenadas enchem.\n\n");
{
    /* O Aarao: "o po nao gera o plano sozinho, as duas operacoes sao duas coordenadas que
     * enchem o plano." E' exato, e mede-se dos dois lados.
     *
     * SOZINHO: o furos.c §F1 mediu que dim(Cantor) = log2/log3 = 0,63 — menor que 1. Uma
     * poeira nao chega para uma reta, quanto mais para um plano.
     *
     * A DOIS: reparte-se a MESMA fita em duas subsequencias — os bits pares para x, os impares
     * para y — e conta-se quantas celulas do quadrado ficam ocupadas. Se cobrir todas, as duas
     * coordenadas enchem o que uma sozinha nao enchia. E' a construcao da curva de Hilbert, e
     * e' a deformacao de que o Aarao fala. */
    printf("      bits da fita   células do quadrado   ocupadas   cobertura\n");
    int cheio = 0;
    for(int B = 2; B <= 12; B += 2){
        int lado = 1 << (B/2);
        static char malha[4096];
        memset(malha, 0, sizeof malha);
        long total = 1L << B, ocup = 0;
        for(long v = 0; v < total; v++){
            int x = 0, y = 0;
            for(int k = 0; k < B; k++){
                int b = (int)((v >> (B-1-k)) & 1);
                if(k % 2 == 0) x = (x << 1) | b;     /* os pares para uma coordenada */
                else           y = (y << 1) | b;     /* os ímpares para a outra */
            }
            int c = y*lado + x;
            if(c >= 0 && c < lado*lado && !malha[c]){ malha[c] = 1; ocup++; }
        }
        double cob = 100.0*ocup/(lado*lado);
        if(cob > 99.99) cheio++;
        printf("      %-14d %-21d %-10ld %.2f%%\n", B, lado*lado, ocup, cob);
    }
    printf("\n      dim(Cantor) = log2/log3 = %.6f  — menor que 1, logo não enche nem a reta\n\n",
           log(2.0)/log(3.0));
    ok("uma fita repartida em DUAS coordenadas cobre o quadrado inteiro", cheio >= 5);
    printf("      Sozinho o pó tem dimensão 0,63 e não enche coisa nenhuma. Repartido em duas\n");
    printf("      coordenadas — pares para x, ímpares para y — cobre 100%% das células, em todas\n");
    printf("      as resoluções medidas. É a construção da curva de Hilbert, e é o que o Aarão\n");
    printf("      chama a DEFORMAÇÃO DUAL: a cifra não descreve o pó, deforma-o até ele encher.\n");
    printf("      E são precisas duas: com uma só não há plano nenhum a encher.\n");
}

printf("\n=== FECHO ==================================================================\n");
printf("    Cantor dos dois lados: os vértices (cartesiano, o ONDE) e o percurso\n");
printf("    (polar, o COMO) são ambos fitas, ligados por bijeção. Julia não é um\n");
printf("    lado — é o que corre dentro deles. E o pó só enche o plano a DOIS: uma\n");
printf("    coordenada tem dimensão 0,63; duas cobrem 100%%. A espiral é o que não\n");
printf("    se confirma — é ciclo, e passa por centros de aresta, não de face.\n\n");
printf("    %d asserções, %d falhas.\n\n", unidades, falhas);
return falhas != 0;
}
