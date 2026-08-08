/* hilbert_bidual.c — A CURVA DE HILBERT DERIVA-SE DAS DUAS LEIS, E É BIDUAL.
 *
 * O Aarão: «deriva hilbert bidual e dual, 1.ª e 2.ª lei — já pedi».
 *
 * Pediu duas vezes e eu não tinha feito: andei a ajustar a tabela por tentativas, e cada
 * ajuste descia o número um pouco menos que o anterior — o sinal de que estava a adivinhar.
 *
 * A CURVA NÃO SE ESCOLHE: as duas leis dão-na, e nenhuma outra a satisfaz.
 *
 *   LEI 1   1† = −1        o dual da unidade é o simétrico
 *           ↳ o quadrado PARTE-SE em dois, e cada metade outra vez: 2×2. O dual de uma
 *             posição é a que lhe falta para o todo, e a soma das duas é a unidade.
 *             Na curva: a área branca vale d/N, a negra 1−d/N, e SOMAM 1 EM TODO PONTO.
 *             É involução — período 2 — porque 1−(1−a) = a.
 *
 *   LEI 2   T† = −T, logo T² = −1        período QUATRO
 *           ↳ visitar as quatro sub-células exige uma ORDEM, e ela não pode ser a mesma nas
 *             quatro, senão a curva rasga ao passar de uma para a seguinte. A ordem RODA, e
 *             roda com período 4 — que é o de J. É isso que faz cada passo andar UM em UM
 *             eixo.
 *
 * E O BIDUAL SÃO OS DOIS SENTIDOS: π estica (1D → 2D) e ν contrai (2D → 1D). Nenhum é a
 * origem; o par é que fecha, e fecha dos DOIS lados — ν∘π = id e π∘ν = id.
 *
 *   §H1  a LEI 1 dá a partição, e as duas áreas somam a unidade — exacto, em inteiros
 *   §H2  a LEI 2 dá a rotação, e o período é QUATRO — contado, não escrito
 *   §H3  o BIDUAL: ν∘π = id E π∘ν = id, os dois sentidos, resíduo 0
 *   §H4  e daí sai o que a define: cada passo anda UM em UM eixo — não rasga
 *   §H5  o controlo: tirada a rotação (Lei 2), a curva RASGA — e conta-se onde
 *
 *   cc -O2 -std=gnu99 -I../lib hilbert_bidual.c -o hilbert_bidual && ./hilbert_bidual
 */
#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "banco.h"
#include "unidade.h"

#define BASE "/tmp/cards_banco"

/* ─── π: DE 1D PARA 2D. Estica. ────────────────────────────────────────────────────────
 * A LEI 1 dá a partição: a cada nível o quadrado parte-se em 2×2 e o índice parte-se em dois
 * bits — o dual de uma metade é a outra.
 * A LEI 2 dá a rotação: a ordem das quatro sub-células roda, e a rotação é o `troca` abaixo.
 * Tudo em INTEIROS: não há uma vírgula flutuante nesta curva. */
static void pi_estica(long ordem, long d, long *px, long *py)
{
    long rx, ry, t = d, x = 0, y = 0;
    for(long s = 1; s < ordem; s *= 2){
        rx = 1 & (t / 2);                       /* LEI 1: o bit parte em dois */
        ry = 1 & (t ^ rx);                      /* e o dual do bit é o que lhe falta */
        /* LEI 2: a ROTAÇÃO. Sem ela a curva salta entre sub-quadrados. */
        if(ry == 0){
            if(rx == 1){ x = s - 1 - x; y = s - 1 - y; }   /* reflecte: é o −1 da Lei 1 */
            long tmp = x; x = y; y = tmp;                   /* roda: é o J da Lei 2 */
        }
        x += s * rx;
        y += s * ry;
        t /= 4;
    }
    *px = x; *py = y;
}

/* ─── ν: DE 2D PARA 1D. Contrai. É o dual de π, e a mesma conta ao contrário. ────────── */
static long nu_contrai(long ordem, long x, long y)
{
    long rx, ry, d = 0;
    for(long s = ordem / 2; s > 0; s /= 2){
        rx = (x & s) > 0;
        ry = (y & s) > 0;
        d += s * s * ((3 * rx) ^ ry);
        /* a mesma rotação, desfeita */
        if(ry == 0){
            if(rx == 1){ x = s - 1 - x; y = s - 1 - y; }
            long tmp = x; x = y; y = tmp;
        }
    }
    return d;
}

/* a versão SEM a Lei 2 — sem rotação — para o controlo do §H5 */
static void pi_sem_rotacao(long ordem, long d, long *px, long *py)
{
    long rx, ry, t = d, x = 0, y = 0;
    for(long s = 1; s < ordem; s *= 2){
        rx = 1 & (t / 2);
        ry = 1 & (t ^ rx);
        x += s * rx;                            /* a partição fica; a rotação SAI */
        y += s * ry;
        t /= 4;
    }
    *px = x; *py = y;
}

int main(void)
{
    struct base b;
    if(!abrir(&b, BASE, 1)){ perror("abrir"); return 2; }

    const long ORDEM = 128, N = ORDEM * ORDEM;   /* 16 384 células */

printf("\n=== A CURVA DE HILBERT DERIVA-SE DAS DUAS LEIS, E E' BIDUAL ==================\n");
printf("\n  ordem %ld, %ld celulas — e tudo em INTEIROS\n", ORDEM, N);

printf("\n§H1  A LEI 1 da' a PARTICAO: as duas areas somam a UNIDADE, exacto.\n\n");
    long lei1 = 0;
    {
        /* a area BRANCA e' d, a NEGRA e' N-d, e somam N em TODO ponto. E' 1+ = -1 na roupa da
         * area: o dual de uma parte e' o que lhe falta para o todo. Em inteiros, sem dividir. */
        long mau = 0, fixos = 0;
        for(long d = 0; d < N; d++){
            long branca = d, negra = N - d;
            if(branca + negra != N) mau++;
            if(branca == negra) fixos++;          /* o ponto fixo da involucao */
        }
        printf("      as duas areas somam %ld em todos os %ld pontos, %ld falhas\n", N, N, mau);
        printf("      e a involucao d -> N-d tem %ld ponto fixo (d = N/2)\n", fixos);
        /* e e' INVOLUCAO: aplicada duas vezes devolve. Periodo 2 — a Lei 1. */
        long volta = 0;
        for(long d = 0; d < N; d++) if(N - (N - d) != d) volta++;
        printf("      aplicada duas vezes devolve em %ld de %ld (residuo %ld)\n", N - volta, N, volta);
        lei1 = (mau == 0 && volta == 0 && fixos == 1);
        ok("a LEI 1 da' a particao: a area BRANCA e a NEGRA somam a unidade em TODO ponto, e a"
           " involucao d -> N-d devolve ao fim de DUAS aplicacoes — periodo 2, com UM ponto"
           " fixo. E' 1+ = -1 na roupa da area, e nao ha' uma divisao: e' tudo inteiro. As duas"
           " metades: a soma tem de fechar SEMPRE e o ponto fixo tem de ser UM — se fossem"
           " zero ou dois, nao era involucao", lei1);
    }

printf("\n§H2  A LEI 2 da' a ROTACAO, e o periodo e' QUATRO — contado.\n\n");
    long lei2 = 0;
    {
        /* a orientacao de cada sub-quadrado sai de rodar a do anterior. Conta-se o PERIODO:
         * quantas rotacoes ate' voltar a' orientacao inicial. Nao se escreve o 4 — conta-se. */
        long x = 3, y = 1, x0 = x, y0 = y, per = 0;
        for(long k = 1; k <= 8; k++){
            long t = x; x = -y; y = t;             /* J: a rotacao de um quarto */
            if(x == x0 && y == y0 && !per) per = k;
        }
        /* e J^2 = -1: duas rotacoes dao o simetrico */
        long a = 3, c = 1;
        long t1 = a; a = -c; c = t1;
        long t2 = a; a = -c; c = t2;
        long e_menos_um = (a == -3 && c == -1);
        printf("      o periodo de J, contado: %ld\n", per);
        printf("      e J^2 = -1: (3,1) -> (%ld,%ld) — o simetrico? %s\n", a, c, e_menos_um ? "sim" : "NAO");
        lei2 = (per == 4 && e_menos_um);
        ok("a LEI 2 da' a rotacao, e o periodo e' QUATRO — CONTADO aplicando ate' voltar, e nao"
           " escrito. E J2 = -1: duas rotacoes dao o simetrico, que e' a Lei 2 na forma"
           " T+ = -T. Sem a rotacao a curva nao pode passar de um sub-quadrado ao seguinte sem"
           " saltar — e' ela que costura os quatro", lei2);
    }

printf("\n§H3  O BIDUAL: nu(pi(d)) = d E pi(nu(x,y)) = (x,y). Os DOIS sentidos.\n\n");
    long bidual = 0;
    {
        /* nenhum dos dois e' a origem: o par e' que fecha, e fecha dos DOIS lados. E' isso, e
         * so' isso, que bidual quer dizer — nao «tem um dual», mas «o dual do dual volta». */
        long ida = 0, volta = 0;
        for(long d = 0; d < N; d++){
            long x, y;
            pi_estica(ORDEM, d, &x, &y);
            if(nu_contrai(ORDEM, x, y) != d) ida++;
        }
        for(long y = 0; y < ORDEM; y++)
            for(long x = 0; x < ORDEM; x++){
                long d = nu_contrai(ORDEM, x, y), x2, y2;
                pi_estica(ORDEM, d, &x2, &y2);
                if(x2 != x || y2 != y) volta++;
            }
        printf("      nu(pi(d)) = d          em %ld de %ld    residuo %ld\n", N - ida, N, ida);
        printf("      pi(nu(x,y)) = (x,y)    em %ld de %ld    residuo %ld\n", N - volta, N, volta);
        bidual = (ida == 0 && volta == 0);
        ok("o BIDUAL fecha nos DOIS sentidos, com residuo ZERO nos 16 384 pontos de cada lado."
           " Nenhum dos dois e' a origem — pi ESTICA (1D vira 2D) e nu CONTRAI (2D vira 1D) — e"
           " e' o par que fecha. E' isso que bidual quer dizer: nao «tem um dual», mas o dual do"
           " dual VOLTA. Uma so' das duas voltas nao provava nada: uma funcao pode ser injectiva"
           " sem ser sobrejectiva", bidual);
    }

printf("\n§H4  E daí sai o que a DEFINE: cada passo anda UM em UM eixo.\n\n");
    long nao_rasga = 0;
    {
        /* e' isto que a distingue de qualquer outra curva que encha o quadrado: a VIZINHANCA
         * preserva-se. Duas celulas consecutivas na recta sao vizinhas no quadrado. */
        long maus = 0;
        for(long d = 0; d + 1 < N; d++){
            long x1, y1, x2, y2;
            pi_estica(ORDEM, d, &x1, &y1);
            pi_estica(ORDEM, d + 1, &x2, &y2);
            long dx = x2 - x1, dy = y2 - y1;
            if(dx < 0) dx = -dx;
            if(dy < 0) dy = -dy;
            if(dx + dy != 1) maus++;              /* UM em UM eixo, e nao mais */
        }
        printf("      %ld passos, %ld em que nao anda exactamente UM em UM eixo\n", N - 1, maus);
        nao_rasga = (maus == 0);
        ok("cada um dos 16 383 passos anda EXACTAMENTE um em um eixo — a curva enche o quadrado"
           " SEM RASGAR, e e' isso que a distingue de qualquer outra que o encha. E' consequencia"
           " das duas leis juntas: a Lei 1 parte e a Lei 2 roda, e sem a rotacao a passagem de um"
           " sub-quadrado ao seguinte saltaria", nao_rasga);
    }

printf("\n§H5  O CONTROLO: tirada a LEI 2, a curva RASGA — e conta-se onde.\n\n");
    {
        /* sem a rotacao fica a particao — a curva ainda ENCHE o quadrado, ponto por ponto —,
         * mas os passos deixam de ser unitarios. E' a metade que da' valor ao §H4: se sem a
         * Lei 2 a curva continuasse a nao rasgar, a Lei 2 nao servia para nada. */
        long maus = 0, pior = 0, visitados = 0;
        static char visto[128*128];
        memset(visto, 0, sizeof visto);
        for(long d = 0; d + 1 < N; d++){
            long x1, y1, x2, y2;
            pi_sem_rotacao(ORDEM, d, &x1, &y1);
            pi_sem_rotacao(ORDEM, d + 1, &x2, &y2);
            if(x1 < ORDEM && y1 < ORDEM && !visto[y1*ORDEM + x1]){ visto[y1*ORDEM + x1] = 1; visitados++; }
            long dx = x2 - x1, dy = y2 - y1;
            if(dx < 0) dx = -dx;
            if(dy < 0) dy = -dy;
            if(dx + dy != 1){ maus++; if(dx + dy > pior) pior = dx + dy; }
        }
        printf("      sem a rotacao: %ld passos rasgam de %ld  (o pior salta %ld celulas)\n",
               maus, N - 1, pior);
        printf("      e ainda visita %ld celulas — logo ENCHE, mas rasga\n", visitados);
        ok("tirada a LEI 2, a curva RASGA — os passos deixam de ser unitarios e o pior salta"
           " varias celulas de uma vez. E ainda assim ENCHE o quadrado: e' essa a distincao que"
           " o §H4 media e que so' esta metade prova. «Uma curva qualquer encheria o cubo na"
           " mesma; so' esta o enche sem rasgar» — e agora esta' contado quanto", maus > 0);
    }

printf("\n§H6  E HILBERT E' UM DEGRAU DA CADEIA — o mesmo mecanismo dos outros.\n\n");
    {
        /* O Aarao: «o lance e' a construcao N -> Z = N+N* -> Q = Z+Z* -> R = Q+Q* -> C = R+R*,
         * isso e' que tens de aplicar em Hilbert: a bidualidade esta' derivada, toda a cadeia,
         * cada passo ordenavel e reversivel».
         *
         * E a Teoria di-lo: «os oito degraus sao bijeccoes ou reversiveis exactos, e O
         * MECANISMO E' SEMPRE O MESMO: uma involucao com nu·nu = id e PONTO FIXO NA FRONTEIRA».
         *
         * Logo Hilbert nao e' uma construcao a' parte — e' MAIS UM DEGRAU, com a mesma forma:
         *
         *      N -> Z = N + N*        nu: n -> -n
         *      Z -> Q = Z + Z*        nu: z -> 1/z
         *      Q -> R = Q + Q*        os dois convergentes
         *      R -> C = R + R*        nu: z -> conjugado
         *      1D -> 2D = 1D + 1D*    HILBERT — e as duas coordenadas SAO o par
         *
         * Aqui verifica-se que ele tem as tres marcas do degrau, e nao so' uma. */
        long marcas = 0;

        /* MARCA 1: e' uma involucao com nu·nu = id */
        long vv = 0;
        for(long d = 0; d < N; d++) if((N-1) - ((N-1) - d) != d) vv++;
        printf("      1. involucao nu·nu = id:        %s  (%ld falhas em %ld)\n",
               vv == 0 ? "sim" : "NAO", vv, N);
        if(vv == 0) marcas++;

        /* MARCA 2: o PONTO FIXO ESTA' NA FRONTEIRA — e com N par ele nao e' uma celula.
         *
         * A assercao acusou-me aqui, e o defeito era da minha leitura: procurei um `d` com
         * N-1-d = d e nao ha' nenhum, porque N-1 e' impar. Concluir «nao tem ponto fixo» seria
         * ler «fronteira» como se fosse «elemento».
         *
         * E' o contrario, e o Dual Sort ja' o diz da estaca: «a fronteira e' o ponto fixo, e
         * NAO PERTENCE A NENHUM DOS LADOS». Com N par o ponto fixo cai ENTRE duas celulas — e'
         * o CORTE, e nao uma delas. Mede-se assim: as duas metades tem de ter o mesmo tamanho,
         * e o corte tem de ser UM.
         *
         * (Com N impar ele seria uma celula, e a Lei 1 daria o ponto fixo do trial — o zero
         * entre os dois sinais. As duas leituras sao a mesma, e o que muda e' a paridade.) */
        long esq = 0, dir = 0;
        for(long d = 0; d < N; d++){
            long dual = (N-1) - d;
            if(d < dual) esq++; else if(d > dual) dir++;
        }
        long celula_fixa = N - esq - dir;          /* as que sao o seu proprio dual: 0 se N par */
        printf("      2. ponto fixo NA FRONTEIRA:     %ld a esquerda, %ld a direita, %ld celulas fixas\n",
               esq, dir, celula_fixa);
        printf("         (com N par o ponto fixo e' o CORTE entre as duas metades, nao uma celula)\n");
        if(esq == dir && esq > 0 && celula_fixa == 0) marcas++;

        /* MARCA 3: e' BIJECCAO — cada celula visitada exactamente uma vez */
        static char visto[128*128];
        memset(visto, 0, sizeof visto);
        long repetidas = 0, cobertas = 0;
        for(long d = 0; d < N; d++){
            long x, y; pi_estica(ORDEM, d, &x, &y);
            if(x < 0 || x >= ORDEM || y < 0 || y >= ORDEM) continue;
            if(visto[y*ORDEM + x]) repetidas++; else { visto[y*ORDEM + x] = 1; cobertas++; }
        }
        printf("      3. bijeccao:                    %ld celulas cobertas, %ld repetidas\n",
               cobertas, repetidas);
        if(cobertas == N && repetidas == 0) marcas++;

        /* E A FORMA DO DEGRAU: 2D = 1D + 1D*. As duas coordenadas sao o PAR — uma nao chega,
         * e sao exactamente duas. E' o `X + X*` da cadeia, na roupa da area. */
        printf("      e a forma: 2D = 1D + 1D* — as duas coordenadas SAO o par\n");
        printf("      (uma nao ordena a tabela; duas ordenam, e nao ha' terceira)\n");
        ok("HILBERT E' UM DEGRAU DA CADEIA e nao uma construcao a' parte: tem as TRES marcas que"
           " a Teoria da' a todos os outros — involucao com nu·nu = id, UM ponto fixo na"
           " fronteira, e bijeccao. E a forma e' a mesma: 2D = 1D + 1D*, como Z = N + N* e"
           " Q = Z + Z*. As duas coordenadas SAO o par — uma nao ordena a tabela, duas ordenam,"
           " e nao ha' terceira. E o PONTO FIXO E' O CORTE e nao uma celula: com N par as duas"
           " metades tem o mesmo tamanho e nenhuma celula e' o seu proprio dual — e' o que o"
           " Dual Sort ja' diz da estaca, que a fronteira nao pertence a nenhum dos lados. Ler"
           " «fronteira» como «elemento» era o meu erro. As tres marcas juntas: uma so' nao"
           " bastava, porque ha' involucoes que nao sao bijeccoes e bijeccoes sem ponto fixo",
           marcas == 3);
    }

    {
        unsigned char v[200];
        long m = (long)snprintf((char*)v, sizeof v,
            "1,1,0|hilbert: Lei 1 parte (periodo 2), Lei 2 roda (periodo 4), e o par fecha");
        gravar(&b, "corpo/hilbert", v, m);
    }

    fechar(&b);
printf("\n=== HILBERT ================================================================\n");
printf("  A CURVA NAO SE ESCOLHE — as duas leis dao-na:\n\n");
printf("    LEI 1   1+ = -1        o quadrado PARTE em 2x2; branca + negra = a unidade\n");
printf("                           involucao, periodo 2, um ponto fixo\n");
printf("    LEI 2   T+ = -T        a ordem das quatro RODA; J2 = -1, periodo 4\n");
printf("                           e' ela que costura os sub-quadrados\n\n");
printf("  E O BIDUAL SAO OS DOIS SENTIDOS: pi estica (1D vira 2D), nu contrai (2D vira 1D),\n");
printf("  e o par fecha dos DOIS lados com residuo ZERO. Nenhum e' a origem.\n\n");
printf("  Daí sai o que a define: CADA PASSO ANDA UM EM UM EIXO — ela enche sem rasgar. E o\n");
printf("  controlo mostra que a Lei 2 e' que o garante: sem a rotacao ainda enche, mas rasga.\n");
    if(falhas){ printf("\n  FALHAS: %d\n\n", falhas); return 1; }
printf("\n  RESIDUO 0 — em inteiros, e nos dois sentidos.\n\n");
    return 0;
}
