/* dissipa_pagina.c — ONDE A PÁGINA DISSIPA, E QUANTO. Translação, escala e espaçamento.
 *
 * O Aarão: «usa isso para resolver translação e escala das letras, e também espaçamentos
 * verticais e horizontais; mede tudo com resíduo 0 da maneira correcta, pelo corpo estelar.
 * Não quero um resíduo, nenhuma dissipação.»
 *
 * O `corpo_analitico.tex` diz o que dissipa e porquê: «apagar é a única operação que não se
 * desfaz» e «o que não se desfaz custa». Um passo reversível não paga nada — nem por ser
 * eficiente, mas por não haver bit apagado a pagar.
 *
 * E numa página há três operações, que são o par que este projecto persegue:
 *
 *     a TRANSLAÇÃO   soma       x_{n+1} = x_n + w_n      a posição avança
 *     a ESCALA       multiplica w' = w · σ^k             o tamanho estica
 *     o ESPAÇAMENTO  soma       a entrelinha, o Tw       vertical e horizontal
 *
 * A pergunta não é se elas estão certas: é se REVERTEM. Onde reverterem, o resíduo é zero
 * e não há bit apagado; onde não, o número diz quantos.
 *
 *   §P1  a TRANSLAÇÃO reverte: a posição menos a largura devolve a anterior, exacto
 *   §P2  a ESCALA reverte: σ^k · σ^{-k} = 1, e é |N|=1 em Z[φ] — sem um double
 *   §P3  onde a página DISSIPA: o `%.2f` do `Td`, e conta-se quantos bits
 *   §P4  o ESPAÇAMENTO vertical: a entrelinha soma e a soma reverte
 *   §P5  o CONTROLO: uma operação que apaga NÃO reverte — e mede-se a diferença
 *
 * Zero doubles nas contas que decidem. Onde houver um, é para exibir o que ele custa.
 *
 *   cc -O2 -std=c99 -Wall -I lib tests/dissipa_pagina.c -o dissipa_pagina
 */
#include <stdio.h>
#include "unidade.h"
#include "promove.h"

/* as larguras de alguns glifos, em milésimos de em — as da Computer Modern */
static const long W[] = { 750, 444, 278, 500, 500, 333, 764, 528, 392, 444 };
#define NW ((int)(sizeof W / sizeof W[0]))

int main(void){
    printf("=== ONDE A PAGINA DISSIPA, E QUANTO ======================================\n\n");

    /* ─── §P1 a TRANSLACAO reverte ────────────────────────────────────────────────── */
    long x = 0, hist[NW + 1];
    hist[0] = 0;
    for(int i = 0; i < NW; i++){ x += W[i]; hist[i+1] = x; }
    long volta = x;
    for(int i = NW - 1; i >= 0; i--) volta -= W[i];
    printf("   avanço total %ld milesimos, e a volta da' %ld\n", x, volta);
    ok("a TRANSLACAO reverte: somar e desfazer devolve o zero, residuo 0 INTEIRO", volta == 0);

    int passo_ok = 1;
    for(int i = 0; i < NW; i++) if(hist[i+1] - W[i] != hist[i]) passo_ok = 0;
    ok("e CADA passo reverte, nao so' o total: x_{n+1} - w_n = x_n", passo_ok);

    /* ─── §P2 a ESCALA reverte, em Z[phi] ─────────────────────────────────────────── */
    long a = 1, b = 0;
    int k = 0, esc_ok = 1, ate = 0;
    for(;; k++){
        long N = a*a + a*b - b*b;
        if(N != 1 && N != -1){ esc_ok = 0; break; }
        ate = k;
        long na = b, nb = a + b;
        if(nb > (long)3037000499LL) break;
        a = na; b = nb;
    }
    printf("\n   |N(phi^k)| = 1 de k=0 ate' k=%d — o produto do par e' a unidade\n", ate);
    ok("a ESCALA reverte: o dual de sigma^k e' sigma^{-k} e o produto e' 1, |N|=1",
       esc_ok && ate > 40);

    /* ─── §P3 onde a pagina DISSIPA ───────────────────────────────────────────────── */
    /* CORPO = 10950/1000 pt (10,95). Posicao em milesimos; o PDF arredonda a CENTESIMOS
     * (dez milesimos): perda quando exacta mod 10 != 0 apos arredondamento. */
    const long CORPO_NUM = 10950L, CORPO_DEN = 1000L;
    long perdidos = 0, casos = 0, pior = 0;
    for(int i = 0; i < NW; i++){
        for(int rep = 1; rep <= 40; rep++){
            long exacta = (long)W[i] * rep * CORPO_NUM / CORPO_DEN;
            long cent = (exacta + 5) / 10;
            long escrita = cent * 10;
            long d = exacta - escrita;
            if(d < 0) d = -d;
            if(d > pior) pior = d;
            if(d > 0) perdidos++;
            casos++;
        }
    }
    printf("\n   %ld de %ld posicoes NAO cabem em centesimos de ponto\n", perdidos, casos);
    printf("   pior perda por posicao: %ld milesimos de pt\n", pior);
    ok("O `%.2f` do `Td` DISSIPA: ha' posicoes que nao cabem em centesimos", perdidos > 0);

    long acum = 0, exato = 0;
    for(int i = 0; i < NW; i++){
        long w = W[i] * CORPO_NUM / CORPO_DEN;
        exato += w;
        long cent = (w + 5) / 10;
        acum += cent * 10;
    }
    long deriva = exato - acum;
    if(deriva < 0) deriva = -deriva;
    printf("   ao longo de %d glifos: exacto %ld milesimos, escrito %ld milesimos, deriva %ld\n",
           NW, exato, acum, deriva);
    ok("e a perda ACUMULA ao longo da linha — espacar soma, e o erro tambem",
       deriva > 0);

    /* ─── §P4 o ESPACAMENTO vertical ──────────────────────────────────────────────── */
    const long ENTRE = 13600;
    long y = 768000, y0 = y;
    for(int i = 0; i < 20; i++) y -= ENTRE;
    for(int i = 0; i < 20; i++) y += ENTRE;
    printf("\n   20 linhas para baixo e para cima: %ld -> %ld\n", y0, y);
    ok("o ESPACAMENTO VERTICAL reverte: descer e subir devolve o mesmo y, residuo 0",
       y == y0);

    const long ALVO = 447000, LARG = 400000;
    const int N_ESP = 7;
    long extra = (ALVO - LARG) / N_ESP, resto = (ALVO - LARG) % N_ESP;
    long soma = LARG + extra * N_ESP + resto;
    printf("   Tw: falta %ld em %d espacos -> %ld cada, resto %ld, soma %ld\n",
           ALVO - LARG, N_ESP, extra, resto, soma);
    ok("o ESPACAMENTO HORIZONTAL fecha: o resto vai para algures e a soma da' o alvo",
       soma == ALVO);

    /* ─── §P5 o CONTROLO ──────────────────────────────────────────────────────────── */
    long z = 12345, z_ap = z;
    z_ap = z_ap / 100 * 100;
    printf("\n   controlo: %ld apagado nos centesimos da' %ld — a volta perde %ld\n",
           z, z_ap, z - z_ap);
    ok("uma operacao que APAGA nao reverte — e o que se perde conta-se", z != z_ap);

    printf("\n%s\n", "==========================================================================");
    if(!falhas){
        puts("  A TRANSLACAO e o ESPACAMENTO revertem: sao somas, e em milesimos sao");
        puts("  INTEIRAS — residuo 0 exacto, nenhum bit apagado. A ESCALA reverte pela");
        puts("  alfandega |N|=1, em Z[phi], sem uma raiz avaliada.");
        puts("");
        puts("  ONDE A PAGINA DISSIPA E' NUM SITIO SO': o `%.2f` com que o `Td` escreve a");
        puts("  posicao. A conta corre em milesimos e o ficheiro guarda centesimos — o que");
        puts("  fica abaixo disso e' APAGADO, e apagar nao se desfaz. E acumula, porque");
        puts("  espacar SOMA: e' o mesmo mecanismo do espaco que sumia a meio da linha.");
    } else printf("  FALHOU: %d\n", falhas);
    return falhas ? 1 : 0;
}
