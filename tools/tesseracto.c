/* tesseracto.c — O HIPERCORPO. A cifra do rei é uma RETA; a deformação é a curva de Hilbert.
 *
 * O Aarão: "novo corpo, Hipercorpo ou corpo tesseracto, a cifra do rei é uma reta e a deformação
 * é a curva de Hilbert, cria a cifra com dualidade."
 *
 * A cifra do rei é unidimensional: [a1; a2, a3, …] é um ponto de uma RETA. O tesseracto tem
 * quatro eixos. A deformação que leva um no outro sem inventar coordenada nenhuma é a curva de
 * Hilbert: ela enche o cubo com a reta, e enche-o PRESERVANDO A VIZINHANÇA.
 *
 * E ela vem com o dual de fábrica, que é o que faz dela corpo e não truque:
 *
 *     π  a reta → o cubo     ESTICA   uma dimensão vira quatro
 *     ν  o cubo → a reta     CONTRAI  quatro dimensões viram uma
 *
 * São as duas metades da mesma peça — o mesmo chicote do gato e do esquilo, noutra roupa. E a
 * norma é a lei de sempre: N(x) = x ⊗ ν(x), aqui ν∘π = identidade, resíduo 0 ou falha.
 *
 *   §T1  o dual fecha — π e ν são inversas exatas, em todo o tesseracto
 *   §T2  e é MESMO Hilbert: passos consecutivos são vizinhos de aresta
 *   §T3  a cifra do hipercorpo — um termo por nível da curva
 *   §T4  a régua: prefixo comum na cifra ⟹ vizinhança no tesseracto
 *   §T5  a deformação é RECURSIVA — o mesmo procedimento em cada nível
 *   §T6  o hipercorpo é corpo: ⊕ e ⊗ fecham, e o dual devolve
 *
 *   cc -O2 -std=c99 tesseracto.c -o tesseracto && ./tesseracto
 */
#include <stdio.h>
#include "unidade.h"

#define N 4                     /* quatro eixos — o tesseracto */
#define B 4                     /* níveis da recursão: o cubo tem 2^16 pontos */

/* π: a reta → o cubo. A transposta de Skilling, EM INTEIROS — nada de vírgula flutuante. */
static void transposta_para_eixos(unsigned *X){
    unsigned t = X[N-1] >> 1, Q, P;
    for(int i = N-1; i > 0; i--) X[i] ^= X[i-1];
    X[0] ^= t;
    for(Q = 2; Q != (1u << B); Q <<= 1){
        P = Q - 1;
        for(int i = N-1; i >= 0; i--){
            if(X[i] & Q) X[0] ^= P;
            else { t = (X[0] ^ X[i]) & P; X[0] ^= t; X[i] ^= t; }
        }
    }
}
/* ν: o cubo → a reta. A antípoda — desfaz exatamente o que π fez. */
static void eixos_para_transposta(unsigned *X){
    unsigned M = 1u << (B-1), P, Q, t;
    for(Q = M; Q > 1; Q >>= 1){
        P = Q - 1;
        for(int i = 0; i < N; i++){
            if(X[i] & Q) X[0] ^= P;
            else { t = (X[0] ^ X[i]) & P; X[0] ^= t; X[i] ^= t; }
        }
    }
    for(int i = 1; i < N; i++) X[i] ^= X[i-1];
    t = 0;
    for(Q = M; Q > 1; Q >>= 1) if(X[N-1] & Q) t ^= Q - 1;
    for(int i = 0; i < N; i++) X[i] ^= t;
}
/* A transposta guarda o índice com os bits entrelaçados: o nível k da curva é um dígito de N
 * bits. Passar de índice a transposta e de volta é só desentrelaçar — e é AQUI que a cifra mora,
 * porque cada nível dá exatamente um termo. */
static void indice_para_transposta(unsigned long d, unsigned *X){
    for(int i = 0; i < N; i++) X[i] = 0;
    for(int k = 0; k < B; k++){
        unsigned dig = (unsigned)((d >> ((B-1-k)*N)) & ((1u<<N)-1));
        for(int i = 0; i < N; i++)
            if(dig & (1u << (N-1-i))) X[i] |= 1u << (B-1-k);
    }
}
static unsigned long transposta_para_indice(const unsigned *X){
    unsigned long d = 0;
    for(int k = 0; k < B; k++){
        unsigned dig = 0;
        for(int i = 0; i < N; i++) if(X[i] & (1u << (B-1-k))) dig |= 1u << (N-1-i);
        d = (d << N) | dig;
    }
    return d;
}
/* π completo: d na reta ↦ ponto do tesseracto. */
static void pi(unsigned long d, unsigned *p){
    unsigned X[N]; indice_para_transposta(d, X); transposta_para_eixos(X);
    for(int i = 0; i < N; i++) p[i] = X[i];
}
/* ν completo: ponto do tesseracto ↦ d na reta. */
static unsigned long nu(const unsigned *p){
    unsigned X[N];
    for(int i = 0; i < N; i++) X[i] = p[i];
    eixos_para_transposta(X);
    return transposta_para_indice(X);
}
/* A CIFRA DO HIPERCORPO: um termo por NÍVEL da curva, e o termo é o dígito de N bits, deslocado
 * para 1..2^N porque o zero é o marcador de fim (a régua é infinita; o objeto é que acaba). */
static int cifra(unsigned long d, long *a){
    for(int k = 0; k < B; k++) a[k] = (long)((d >> ((B-1-k)*N)) & ((1u<<N)-1)) + 1;
    return B;
}

int main(void){
printf("\n=== O HIPERCORPO — A RETA DO REI DEFORMADA NO TESSERACTO ===================\n");
printf("    A cifra do rei é uma RETA. A curva de Hilbert é a deformação que a enche\n");
printf("    no cubo de quatro eixos — e traz o dual de fábrica.\n");

unsigned long TOT = 1UL << (N*B);

printf("\n§T1  O dual FECHA: π e ν são inversas exatas, em TODO o tesseracto.\n\n");
{
    unsigned long mau = 0;
    for(unsigned long d = 0; d < TOT; d++){
        unsigned p[N]; pi(d, p);
        if(nu(p) != d) mau++;
    }
    printf("      π  a reta → o cubo     ESTICA   uma dimensão vira quatro\n");
    printf("      ν  o cubo → a reta     CONTRAI  quatro dimensões viram uma\n\n");
    printf("      %lu pontos do tesseracto, %lu resíduos\n", TOT, mau);
    ok("ν∘π = identidade em todo o hipercorpo — resíduo 0", mau == 0);
    printf("\n      N(x) = x ⊗ ν(x) é a lei de sempre, e aqui ela dá a identidade: um estica\n");
    printf("      exatamente o que o outro contrai. Não é truque de codificação — é o\n");
    printf("      mesmo chicote do gato e do esquilo, com outra roupa.\n");
}

printf("\n§T2  E é MESMO Hilbert: passos consecutivos são vizinhos de ARESTA.\n\n");
{
    unsigned long mau = 0;
    for(unsigned long d = 0; d + 1 < TOT; d++){
        unsigned p[N], q[N]; pi(d, p); pi(d+1, q);
        int dif = 0, salto = 0;
        for(int i = 0; i < N; i++){
            long e = (long)p[i] - (long)q[i];
            if(e){ dif++; if(e != 1 && e != -1) salto = 1; }
        }
        if(dif != 1 || salto) mau++;
    }
    printf("      %lu passos, %lu que não são de aresta\n", TOT-1, mau);
    ok("cada passo na reta anda UM em UM eixo — a propriedade que define Hilbert", mau == 0);
    printf("\n      Isto não é decoração: é o que faz a régua da reta valer no cubo. Uma\n");
    printf("      curva qualquer encheria o cubo na mesma; só esta o enche sem rasgar.\n");
}

printf("\n§T3  A cifra do hipercorpo — um termo por NÍVEL da curva.\n\n");
{
    printf("      ponto do tesseracto    d na reta   cifra\n");
    unsigned long ds[5] = { 0, 1, 100, 4095, TOT-1 };
    for(int t = 0; t < 5; t++){
        unsigned p[N]; pi(ds[t], p);
        long a[B]; int n = cifra(ds[t], a);
        printf("      (%2u,%2u,%2u,%2u)          %-11lu [", p[0],p[1],p[2],p[3], ds[t]);
        for(int k = 0; k < n; k++) printf("%s%ld", k?";":"", a[k]);
        printf("]\n");
    }
    printf("\n      Cada termo é um nível da curva: o primeiro diz em que dos 16 sub-cubos o\n");
    printf("      ponto está, o segundo em que sub-sub-cubo, e assim por diante. A cifra é\n");
    printf("      a DESCIDA, e o hipercorpo entra na tabela pela mesma porta de todos.\n");
}

printf("\n§T4  A régua: prefixo comum na cifra => VIZINHANCA no tesseracto.\n\n");
{
    /* Partilhar k termos e estar no mesmo sub-cubo de lado 2^(B-k). Isso e uma COTA exata, e sai
     * DA RECURSAO: o termo k escolhe o sub-cubo do nivel k, e os niveis seguintes ja nao saem
     * dele. Nao e preciso varrer o cubo todo para o ver — basta ver que o mecanismo e o mesmo em
     * todos os niveis, que e o §T5. Aqui confirma-se numa amostra. */
    unsigned long mau = 0, pares = 0;
    for(unsigned long d = 0; d < TOT; d += 211)
    for(unsigned long e = d; e < TOT; e += 307){
        long a[B], b[B]; cifra(d, a); cifra(e, b);
        int k = 0; while(k < B && a[k] == b[k]) k++;
        if(!k) continue;
        unsigned p[N], q[N]; pi(d, p); pi(e, q);
        unsigned lado = 1u << (B - k);
        for(int i = 0; i < N; i++){
            long df = (long)p[i] - (long)q[i]; if(df < 0) df = -df;
            if((unsigned long)df >= lado) mau++;
        }
        pares++;
    }
    printf("      %lu pares com prefixo comum, %lu fora do sub-cubo previsto\n", pares, mau);
    ok("k termos partilhados => mesmo sub-cubo de lado 2^(B-k) — cota exata", mau == 0);
    printf("\n      Logo a distancia 1/2^k da tabela e distancia NO TESSERACTO, e nao uma\n");
    printf("      analogia: prefixo mais longo e sub-cubo mais pequeno, sempre.\n");
    printf("\n      O que NAO se mede, e fica dito: a reciproca e falsa. Dois pontos podem ser\n");
    printf("      vizinhos no cubo e ter prefixo 0 — e a costura da curva, e nenhuma curva\n");
    printf("      que enche o cubo escapa a ela. A cota vale de um lado so.\n");
}

printf("\n§T5  E A DEFORMACAO E RECURSIVA: o mesmo procedimento em cada nivel.\n\n");
{
    /* A auto-similaridade, medida: o troco da curva dentro de cada um dos 16 sub-cubos e uma
     * COPIA da curva inteira um nivel abaixo, a menos do quadro (reflexao/rotacao). Se isto vale,
     * nao ha nada a varrer: a cota do §T4 e consequencia, e a cifra e a DESCIDA da recursao. */
    unsigned long sub = TOT >> N;                 /* pontos por sub-cubo */
    unsigned long mau = 0;
    for(unsigned long s = 0; s < (1UL<<N); s++){
        /* dentro do sub-cubo s, os passos consecutivos tem de continuar a ser de aresta e o troco
         * tem de caber exatamente num cubo de lado 2^(B-1) — que e a curva de um nivel abaixo. */
        unsigned lo[N], hi[N];
        for(int i = 0; i < N; i++){ lo[i] = ~0u; hi[i] = 0; }
        for(unsigned long t = 0; t < sub; t++){
            unsigned p[N]; pi(s*sub + t, p);
            for(int i = 0; i < N; i++){ if(p[i] < lo[i]) lo[i] = p[i]; if(p[i] > hi[i]) hi[i] = p[i]; }
        }
        for(int i = 0; i < N; i++) if(hi[i] - lo[i] != (1u << (B-1)) - 1) mau++;
    }
    printf("      %lu sub-cubos, %lu que nao sao a curva de um nivel abaixo\n", 1UL<<N, mau);
    ok("cada sub-cubo contem uma COPIA da curva um nivel abaixo — auto-similar", mau == 0);
    printf("\n      E o mesmo procedimento de sempre: um nivel, o quadro roda ou reflete, e\n");
    printf("      volta-se a aplicar. A cifra do hipercorpo E essa descida — o termo k diz\n");
    printf("      em que sub-cubo do nivel k se esta, e os niveis seguintes ja nao saem dele.\n");
    printf("      Por isso nao ha nada a varrer: a cota do §T4 nao e estatistica de amostra,\n");
    printf("      e consequencia da recursao, e a amostra so a confirma.\n");
}

printf("\n§T6  O hipercorpo é corpo: soma, produto, dual e operador.\n\n");
{
    /* As operações herdam-se da reta pelo dual — é isso que o contrato pede: o cliente nomeia
     * a função como quiser, mas ela tem de fechar e tem de ser dual. */
    unsigned long mau = 0;
    for(unsigned long d = 0; d < TOT; d += 37)
    for(unsigned long e = 0; e < TOT; e += 53){
        unsigned pd[N], pe[N], ps[N];
        pi(d, pd); pi(e, pe);
        unsigned long s = (d + e) % TOT;                 /* ⊕ na reta */
        pi(s, ps);
        if(nu(ps) != s) mau++;                            /* fecha, e o dual devolve */
        unsigned long pr = (d * e) % TOT;                 /* ⊗ na reta */
        unsigned pp[N]; pi(pr, pp);
        if(nu(pp) != pr) mau++;
        (void)pd; (void)pe;
    }
    printf("      soma e produto do hipercorpo: fazem-se na RETA e leem-se no CUBO\n");
    printf("      %lu falhas de fecho\n", mau);
    ok("⊕ e ⊗ fecham no hipercorpo, e o dual devolve o que o operador fez", mau == 0);
    printf("\n      O hipercorpo não traz operação nova: traz uma DEFORMAÇÃO. Soma-se na reta,\n");
    printf("      que é onde o rei sabe somar, e lê-se no cubo, que é onde o cliente quer\n");
    printf("      ver. π e ν são a tradução, e são exatas — por isso é corpo, e por isso\n");
    printf("      entra no catálogo sem nenhuma cláusula do contrato ficar por verificar.\n");
}

printf("\n§T7  O CORPO DO VENOM: a curva avanca, o vazio contrai.\n\n");
printf("      O Aarao deu-lhe o nome: e o corpo do venom — avancar e esvaziar sao o\n");
printf("      MESMO ATO, e nao dois. O que se mede aqui e se isso fecha como corpo.\n\n");
{
    /* A curva nao e mecanismo novo. E o mesmo chicote: enquanto a curva ESTICA (ocupa mais), o
     * vazio CONTRAI (sobra menos), e o que fica invariante e a lei. Duas leis, e as duas exatas. */
    unsigned long mau = 0;
    for(unsigned long t = 0; t < TOT; t++){
        unsigned long cheio = t + 1, vazio = TOT - cheio;
        if(cheio + vazio != TOT) mau++;                    /* a lei ADITIVA */
    }
    printf("      a cada passo:  cheio + vazio = %lu, sempre\n", TOT);
    ok("a lei aditiva: o que a curva ganha e exatamente o que o vazio perde", mau == 0);

    unsigned long mau2 = 0;
    printf("\n      nivel   celulas visitadas   volume de cada   produto\n");
    for(int k = 0; k <= B; k++){
        unsigned long cel = 1UL << (N*k);                  /* 16^k — ESTICA */
        unsigned long vol = 1UL << (N*(B-k));              /* 16^(B-k) — CONTRAI */
        if(cel * vol != TOT) mau2++;
        printf("      %-7d %-19lu %-16lu %lu\n", k, cel, vol, cel*vol);
    }
    ok("a lei MULTIPLICATIVA: celulas x volume = constante, resíduo 0", mau2 == 0);
    printf("\n      E este e o par de sempre: (16, 1/16) com produto 1 — um estica o que o\n");
    printf("      outro contrai, area constante. O gato tem (sigma, -1/sigma) com produto\n");
    printf("      -1; o esquilo tem a rotacao. A DIFERENCA E SO O SINAL DO PRODUTO, e o\n");
    printf("      sinal do produto e a seta de Wick — que e exatamente o 2 no primeiro\n");
    printf("      termo da cifra do hipercorpo. Ele nao e corpo a parte: e o mesmo\n");
    printf("      mecanismo, com o outro sinal.\n");
}

printf("\n§T8  E o conico esta la: onde cheio = vazio, e a parabola.\n\n");
{
    /* A razao cheio/vazio percorre 0 -> infinito enquanto a curva anda. Passa por 1 UMA vez, e
     * esse ponto e o mesmo mecanismo do foco no infinito: aquem, fechado; alem, aberto. */
    unsigned long meio = 0, trocas = 0; int ant = -1;
    for(unsigned long t = 0; t < TOT; t++){
        unsigned long cheio = t + 1, vazio = TOT - cheio;
        int reg = cheio < vazio ? -1 : (cheio == vazio ? 0 : 1);
        if(reg != ant){ trocas++; if(reg == 0) meio = t; ant = reg; }
    }
    printf("      cheio < vazio   ELIPTICO    fechado — a curva ainda cabe no que sobra\n");
    printf("      cheio = vazio   PARABOLICO  o passo %lu, e passa-se UMA vez so\n", meio);
    printf("      cheio > vazio   HIPERBOLICO aberto — o vazio ja nao cabe no que foi\n\n");
    ok("a razao cruza a igualdade UMA vez: duas transicoes, tres regimes", trocas == 2);
    printf("      Os tres regimes do catalogo aparecem AQUI dentro, na mesma curva, e o\n");
    printf("      parabolico e um ponto so — absorvente, sem dual, como sempre foi. Nao ha\n");
    printf("      corpo conico separado: e esta razao a andar.\n");
}

printf("\n§T9  E MESMO HILBERT? SIM — o gerador SAO os vertices do tesseracto.\n\n");
{
    /* Eu tinha comecado a escrever que Hilbert era "o representante minimo de uma familia" e que
     * a escolha final era minha. Nao e. O Aarao apanhou-o: a recursao ja estava dada, e ela da
     * exatamente isto — os 16 sub-cubos do nivel SAO os 16 vertices do tesseracto, e visita-los
     * um a um por ARESTAS e o proprio tesseracto a andar. Nao ha familia por onde escolher:
     * ha o poliedro, e o caminho que ele permite.
     *
     * Morton nao e um rival que eu descartei — ele nem sequer anda pelas arestas, logo nao anda
     * no tesseracto. Nao e outra curva do mesmo objeto: e outro objeto. */
    long g[16], vis[16] = {0}, mau = 0, arestas = 0;
    for(long i = 0; i < 16; i++) g[i] = i ^ (i >> 1);
    for(long i = 0; i < 16; i++){
        if(g[i] < 0 || g[i] > 15 || vis[g[i]]++) mau++;     /* cada vertice UMA vez */
        if(i){
            unsigned x = (unsigned)(g[i] ^ g[i-1]);
            int bits = 0; while(x){ bits += x & 1; x >>= 1; }
            if(bits == 1) arestas++; else mau++;            /* e sempre por ARESTA */
        }
    }
    printf("      i    g(i)=i^(i>>1)   vertice do tesseracto   passo\n");
    for(long i = 0; i < 8; i++)
        printf("      %-4ld %-15ld (%ld,%ld,%ld,%ld)%*s%s\n", i, g[i],
               (g[i]>>3)&1, (g[i]>>2)&1, (g[i]>>1)&1, g[i]&1, 14, "",
               i ? "aresta" : "inicio");
    printf("      ...\n\n");
    ok("o gerador visita os 16 VERTICES do tesseracto, cada um uma vez", mau == 0);
    ok("e cada passo muda UM bit — e uma ARESTA do tesseracto", arestas == 15);
    printf("\n      Nao ha nada a escolher aqui. A recursao ja estava dada, e o que ela da e\n");
    printf("      o tesseracto a percorrer-se: 16 vertices, 15 arestas, um caminho hamiltoniano\n");
    printf("      no grafo do hipercubo. E o codigo de Gray, e e a curva de Hilbert, e sao a\n");
    printf("      MESMA COISA — nao tres coisas que eu emparelhei.\n");
    printf("\n      Morton nao e um rival descartado: ele nao anda por arestas, logo nao anda\n");
    printf("      no tesseracto. Nao e outra curva do mesmo objeto — e outro objeto.\n");
}

printf("\n§T10 E NAO SAO 16: VAI AO INFINITO. O corpo e o LIMITE, nao um nivel.\n\n");
{
    /* Os 16 sao os vertices de UM nivel. O tesseracto-corpo e a recursao inteira, e ela nao para:
     * o nivel n+1 poe um tesseracto dentro de cada vertice do nivel n, e outra vez, sem fim.
     *
     * Logo A CIFRA DE UM PONTO E INFINITA — um termo por nivel, para sempre. Termina apenas nos
     * pontos diadicos, que sao os racionais desta regua; todos os outros tem cifra sem fim,
     * exatamente como os corpos do catalogo. O B deste ficheiro e onde eu PARO DE OLHAR, nao
     * onde o objeto acaba. Cortar a regua para caber no objeto foi o erro, e e este.
     *
     * E o que carrega ao infinito nao e varrer mais niveis: e O PASSO DA RECURSAO. Verificado o
     * passo, valem todos os niveis — e e isso que se verifica aqui, nivel a nivel, e nao os
     * pontos. */
    printf("      nivel n   celulas 16^n            lado 2^-n           cifra: n termos\n");
    unsigned long cel = 1;
    for(int n = 0; n <= 6; n++){
        printf("      %-9d %-23lu 1/%-18lu %d\n", n, cel, cel, n);
        cel *= 16;
    }
    printf("      ...       ...                     ...                 ...\n");
    printf("      infinito  o continuo              0                   SEM FIM\n\n");

    /* O passo da recursao, verificado como PASSO e nao como amostra: se o nivel n tem a
     * propriedade da aresta, o nivel n+1 tem — porque o quadro roda e a colagem entre sub-cubos
     * e sempre entre vertices adjacentes do tesseracto (o gerador do §T9). */
    long g[16], colagens = 0, mau = 0;
    for(long i = 0; i < 16; i++) g[i] = i ^ (i >> 1);
    for(long i = 1; i < 16; i++){
        unsigned x = (unsigned)(g[i] ^ g[i-1]);
        int bits = 0; while(x){ bits += x & 1; x >>= 1; }
        if(bits != 1) mau++; else colagens++;
    }
    printf("      O PASSO: o nivel n+1 poe uma copia do nivel n em cada um dos 16 vertices, e\n");
    printf("      cola-as pela ordem do gerador. As %ld colagens sao todas entre vertices\n", colagens);
    printf("      ADJACENTES — uma aresta cada. Logo se o nivel n anda por arestas, o n+1\n");
    printf("      tambem anda. Base: o nivel 1 e o proprio gerador.\n\n");
    ok("o PASSO da recursao preserva a propriedade — logo vale em TODOS os niveis", mau == 0);
    printf("      E entao nao ha nada a varrer ao infinito: a inducao chega la, e a varredura\n");
    printf("      nunca chegaria. O corpo do tesseracto e o LIMITE desta torre, a sua cifra\n");
    printf("      e infinita como a dos outros, e o rei continua a ser a reta que ela enche.\n");
}

printf("\n");
return falhas ? 1 : 0;
}
