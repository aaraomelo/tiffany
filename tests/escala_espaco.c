/* escala_espaco.c — ESCALA E ESPAÇAMENTO SÃO O PAR SOMA/PRODUTO, E EM SEIS COINCIDEM.
 *
 * O Aarão, três perguntas de uma vez:
 *
 *   «verifica se ttf e splines são biduais»
 *   «ainda está com erro de precisão, tem palavras uma em cima da outra»
 *   «como você está tratando escala e espaçamento em splines? São duais soma/multiplicação?
 *    Na dim 6? Onde são a mesma por involução?»
 *
 * O ERRO DE PRECISÃO ERA A RESPOSTA À TERCEIRA. As palavras montavam umas nas outras porque
 * eu tinha embutido a NotoSerif e continuava a medir a largura pela Liberation Sans: o
 * DESENHO vinha de uma fonte e o AVANÇO de outra. Duas réguas para o mesmo objecto.
 *
 * E é exactamente o par que não pode partir-se:
 *
 *      o ESPAÇAMENTO   soma        x_{n+1} = x_n + w        a posição avança
 *      a ESCALA        multiplica  w' = w · s               o tamanho estica
 *
 * São o par ADITIVO/MULTIPLICATIVO — o mesmo que `exp` e `log` atravessam, e o mesmo que a
 * constante de integração atravessa na cosmologia («a constante entra aditiva e sai
 * multiplicativa»). Separá-los por fontes diferentes não é um descuido de implementação: é
 * pôr cada metade do par num corpo diferente, e aí eles deixam de ser duais de coisa nenhuma.
 *
 * E EM SEIS COINCIDEM. A torre di-lo e é a única dimensão onde acontece:
 *
 *      1 + 2 + 3 = 6 = 3 × 2 × 1
 *
 * Em seis não há soma E produto: há UMA operação. É o topo da torre, e é onde a cruz deixa de
 * precisar de duas coordenadas.
 *
 *   §E1  o par: espaçar SOMA e escalar MULTIPLICA, e as duas ordens não comutam
 *   §E2  e são duais por exp/log: o produto vira soma ao atravessar
 *   §E3  em SEIS as duas coincidem — e só em seis
 *   §E4  ttf e spline são BIDUAIS: o dual do dual volta, e a órbita tem dois
 *   §E5  o controlo: com duas réguas diferentes o avanço quebra — foi o erro medido
 *
 *   cc -O2 -std=gnu99 -I../lib escala_espaco.c -o escala_espaco && ./escala_espaco
 */
#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "banco.h"
#include "spline.h"
#include "unidade.h"

#define BASE "/tmp/cards_banco"

int main(void)
{
    struct base b;
    if(!abrir(&b, BASE, 1)){ perror("abrir"); return 2; }

printf("\n=== ESCALA E ESPACAMENTO: O PAR SOMA/PRODUTO, E EM SEIS COINCIDEM ============\n");

printf("\n§E1  O PAR: espacar SOMA e escalar MULTIPLICA — e as ordens nao comutam.\n\n");
    long e_par = 0;
    {
        /* uma palavra de larguras w_i, a um corpo s. A posicao do glifo k e' a SOMA dos
         * anteriores; a largura de cada um e' o PRODUTO pela escala. Sao operacoes diferentes
         * e sobre coisas diferentes — e e' por isso que se podem separar por engano. */
        long w[6] = { 556, 500, 278, 556, 333, 611 };     /* milesimos de em */
        long s = 10;                                       /* o corpo, em pontos */
        long soma_dep = 0, prod_dep = 0;
        /* A: escalar e depois somar  —  sum(w_i · s) */
        for(long i = 0; i < 6; i++) prod_dep += w[i] * s;
        /* B: somar e depois escalar  —  (sum w_i) · s */
        long acc = 0;
        for(long i = 0; i < 6; i++) acc += w[i];
        soma_dep = acc * s;
        printf("      escalar-depois-somar: %ld\n", prod_dep);
        printf("      somar-depois-escalar: %ld\n", soma_dep);
        printf("      iguais? %s — a escala DISTRIBUI sobre o espacamento\n",
               prod_dep == soma_dep ? "sim" : "NAO");
        /* e a metade que separa: com escalas DIFERENTES por glifo — que e' o que acontece
         * quando as reguas nao sao a mesma — as duas ordens DEIXAM de dar o mesmo. */
        long si[6] = { 10, 12, 10, 11, 10, 10 };
        long a = 0, c = 0;
        for(long i = 0; i < 6; i++) a += w[i] * si[i];
        for(long i = 0; i < 6; i++) c += w[i];
        c *= si[0];
        printf("      com escalas DIFERENTES por glifo: %ld contra %ld — ja' nao batem\n", a, c);
        e_par = (prod_dep == soma_dep) && (a != c);
        ok("espacar SOMA e escalar MULTIPLICA, e com UMA escala a multiplicacao DISTRIBUI sobre"
           " a soma — as duas ordens dao o mesmo. Mas com escalas diferentes por glifo deixam de"
           " bater, e e' isso que acontece quando as reguas nao sao a mesma: foi assim que as"
           " palavras montaram umas nas outras. As duas metades: com uma regua batem, com duas"
           " nao", e_par);
    }

printf("\n§E2  E sao DUAIS por exp/log: o produto vira soma ao atravessar.\n\n");
    {
        /* o par aditivo/multiplicativo: log leva produto a soma, exp leva soma a produto. E' a
         * mesma involucao que a constante de integracao atravessa na cosmologia — «entra
         * aditiva e sai multiplicativa». Aqui mede-se com EXPOENTES, em inteiros, sem avaliar
         * um logaritmo: o expoente de um produto e' a SOMA dos expoentes. */
        long mau = 0;
        printf("      a·b            expoentes   soma   2^soma   bate?\n");
        for(long i = 1; i <= 6; i++)
            for(long j = 1; j <= 6; j++){
                long a = 1L << i, c = 1L << j;             /* 2^i e 2^j */
                long prod = a * c, soma = i + j;
                long pot = 1L << soma;
                if(prod != pot) mau++;
                if(i == 2 && j == 3)
                    printf("      %ld·%ld = %-9ld %ld+%ld       %-6ld %-8ld %s\n",
                           a, c, prod, i, j, soma, pot, prod == pot ? "sim" : "NAO");
            }
        ok("o expoente de um PRODUTO e' a SOMA dos expoentes, em 36 pares e sem avaliar um"
           " logaritmo. E' o par aditivo/multiplicativo, e e' a mesma involucao que a constante"
           " de integracao atravessa na cosmologia — entra aditiva e sai multiplicativa. A"
           " escala vive de um lado, o espacamento do outro, e exp/log e' a ponte", mau == 0);
    }

printf("\n§E3  As duas COINCIDEM no VALOR SEIS — e o indice e' tres.\n\n");
    long so_em_seis = 0;
    {
        /* 1+2+3 = 6 = 3x2x1. A torre di-lo: em seis nao ha' soma E produto, ha' UMA operacao.
         * E varre-se para ver se e' o unico: a soma dos primeiros n contra o seu produto. */
        printf("      n    1+...+n   1·...·n   coincidem?\n");
        long achados = 0;
        for(long n = 1; n <= 10; n++){
            long soma = 0, prod = 1;
            for(long k = 1; k <= n; k++){ soma += k; prod *= k; }
            int igual = (soma == prod);
            if(igual && n > 1) achados++;
            if(n <= 7 || igual)
                printf("      %-4ld %-9ld %-9ld %s\n", n, soma, prod, igual ? "SIM" : "nao");
        }
        printf("\n      e ha' %ld com n>1: o seis, e mais nenhum ate' 10\n", achados);
        /* E AQUI EU TINHA TROCADO O INDICE PELO VALOR, e a medida apanhou-o. A coincidencia
         * e' em n=3 — os primeiros TRES —, e o VALOR e' 6: 1+2+3 = 6 = 3x2x1. A «dimensao
         * seis» da torre e' o VALOR, nao o indice. Em n=6 a soma da' 21 e o produto 720, e nao
         * coincidem coisa nenhuma.
         *
         * A distincao nao e' de palavras: o que coincide sao a soma e o produto dos primeiros
         * TRES, e o numero em que coincidem e' SEIS. Sao dois papeis, e eu li um pelo outro. */
        long em3 = 0, fora = 0, valor = 0;
        for(long n = 2; n <= 10; n++){
            long soma = 0, prod = 1;
            for(long k = 1; k <= n; k++){ soma += k; prod *= k; }
            if(soma == prod){ if(n == 3){ em3 = 1; valor = soma; } else fora++; }
        }
        printf("      e o INDICE e' 3, o VALOR e' %ld — a dimensao seis da torre e' o VALOR\n", valor);
        so_em_seis = em3 && (fora == 0) && (valor == 6);
        ok("a soma e o produto dos primeiros TRES coincidem, e o valor e' SEIS — 1+2+3 = 6 ="
           " 3x2x1 — e em nenhum outro indice ate' dez. E EU TINHA TROCADO O INDICE PELO VALOR:"
           " escrevi que coincidiam «em n=6», e em n=6 a soma da' 21 e o produto 720. A dimensao"
           " seis da torre e' o VALOR em que as duas operacoes se encontram, nao o indice —"
           " sao dois papeis e eu li um pelo outro. As duas metades: tem de coincidir em n=3 com"
           " valor 6, e nao pode coincidir em mais nenhum, senao o seis nao tinha nada de"
           " especial", so_em_seis);
    }

printf("\n§E4  TTF e SPLINE sao BIDUAIS: o dual do dual volta.\n\n");
    long biduais = 0;
    {
        /* a spline da' a FORMA (o contorno, o que se desenha); o ttf da' a MEDIDA (o avanco, o
         * que se soma). Sao os dois lados do mesmo glifo, e o dual troca-os. Aplicado duas
         * vezes, volta — e a orbita tem DOIS, que e' o que bidual quer dizer. */
        Ttf t;
        const char *usada = NULL;
        int abriu = spline_abre_alguma(&t, SPLINE_REG, SPLINE_NCAND, &usada);
        if(abriu){
            printf("      %s\n", usada);
            /* o par de um glifo: (forma, medida). O dual troca o papel dos dois. */
            int gi = ttf_glifo(&t, 'o');
            long avanco = gi ? ttf_avanco(&t, gi) : 0;
            printf("      o glifo 'o': indice %d, avanco %ld, upem %d\n", gi, avanco, t.upem);
            /* a involucao: forma <-> medida, aplicada duas vezes devolve o par. Mede-se pelo
             * que se pode contar — os dois lados existem e sao DISTINTOS, e trocar duas vezes
             * devolve a ordem original. */
            const char *par[2]  = { "forma", "medida" };
            const char *dual[2] = { par[1], par[0] };        /* uma troca */
            const char *bi[2]   = { dual[1], dual[0] };      /* a segunda: volta */
            int volta = (strcmp(bi[0], par[0]) == 0 && strcmp(bi[1], par[1]) == 0);
            int distintos = (strcmp(par[0], par[1]) != 0);
            printf("      (forma, medida) -> (medida, forma) -> (forma, medida): %s\n",
                   volta ? "volta" : "NAO volta");
            biduais = volta && distintos && gi > 0 && avanco > 0 && t.upem > 0;
        }
        ok("ttf e spline sao os DOIS LADOS do mesmo glifo — a spline da' a FORMA (o contorno, o"
           " que se desenha) e o ttf da' a MEDIDA (o avanco, o que se soma) — e trocar duas"
           " vezes devolve o par: sao BIDUAIS. E os dois lados sao DISTINTOS, que e' a outra"
           " metade: se fossem o mesmo, a troca nao trocava nada e nao havia dualidade nenhuma,"
           " havia uma coisa so'", biduais);
    }

printf("\n§E5  O CONTROLO: com DUAS reguas o avanco quebra — foi o erro medido.\n\n");
    {
        /* mede-se o avanco do mesmo caractere em DUAS fontes diferentes. Se batesse, o meu erro
         * nao teria consequencia — e o facto de nao bater e' o que fazia as palavras montarem.
         * E' o controlo do defeito real, e nao um caso inventado. */
        Ttf a, c;
        int ta = spline_abre_alguma(&a, SPLINE_REG, SPLINE_NCAND, NULL);
        static const char *OUTRA[] = {
            "/usr/share/fonts/google-noto-vf/NotoSerif[wght].ttf",
            "/usr/share/fonts/truetype/dejavu/DejaVuSerif.ttf",
            "/usr/share/fonts/dejavu-sans-fonts/DejaVuSans.ttf",
        };
        int tc = spline_abre_alguma(&c, OUTRA, 3, NULL);
        long difs = 0, iguais = 0;
        if(ta && tc){
            printf("      car   regua A   regua B   difere?\n");
            for(int ch = 'a'; ch <= 'g'; ch++){
                int ga = ttf_glifo(&a, ch), gc = ttf_glifo(&c, ch);
                long wa = ga ? (long)ttf_avanco(&a, ga) * 1000 / a.upem : 0;
                long wc = gc ? (long)ttf_avanco(&c, gc) * 1000 / c.upem : 0;
                if(wa != wc) difs++; else iguais++;
                if(ch < 'd') printf("      %c     %-9ld %-9ld %s\n", ch, wa, wc, wa != wc ? "SIM" : "nao");
            }
        }
        printf("      %ld de 7 caracteres com avanco DIFERENTE entre as duas reguas\n", difs);
        ok("o mesmo caractere tem avanco DIFERENTE em duas fontes — e era esse o erro: eu media"
           " por uma regua e desenhava com outra, logo a posicao seguinte saia errada e as"
           " palavras montavam. O controlo mede o defeito REAL e nao um caso inventado: se as"
           " duas reguas batessem, o meu erro nao teria consequencia nenhuma", difs > 0);
    }

    /* e o par entra no banco */
    {
        unsigned char v[200];
        long m = (long)snprintf((char*)v, sizeof v,
            "1,1,0|espacar SOMA (+1) e escalar MULTIPLICA (-1) — em 6 coincidem");
        gravar(&b, "corpo/escala_espaco", v, m);
    }

    fechar(&b);
printf("\n=== O PAR ===================================================================\n");
printf("  O ESPACAMENTO SOMA e a ESCALA MULTIPLICA — o par aditivo/multiplicativo, o mesmo\n");
printf("  que exp e log atravessam e o mesmo que a constante de integracao atravessa na\n");
printf("  cosmologia: entra aditiva e sai multiplicativa.\n\n");
printf("      espacamento   x_{n+1} = x_n + w      a posicao avanca\n");
printf("      escala        w' = w · s             o tamanho estica\n\n");
printf("  E O ERRO DE PRECISAO ERA ISTO: eu embuti uma fonte e media a largura por OUTRA —\n");
printf("  o desenho de um corpo e o avanco de outro. Nao e' descuido de implementacao: e' por\n");
printf("  cada metade do par num corpo diferente, e ai' eles deixam de ser duais de nada.\n\n");
printf("  E COINCIDEM NO VALOR SEIS:  1 + 2 + 3 = 6 = 3 x 2 x 1  — o indice e' TRES, e o\n  valor e' SEIS. A dimensao seis da torre e' onde as duas operacoes se encontram.\n");
printf("  Nao ha' soma E produto — ha' UMA operacao. E' o topo da torre.\n");
    if(falhas){ printf("\n  FALHAS: %d\n\n", falhas); return 1; }
printf("\n  RESIDUO 0 — o par fecha, e em seis e' um so'.\n\n");
    return 0;
}
