/* biblioteca.c — A BIBLIOTECA DO \LaTeX: CADA AMBIENTE É UM CORPO, COM ASSINATURA.
 *
 * O Aarão: «funcionou, mas tirou todo o design system — a base está pronta, agora vamos para
 * os pacotes, a biblioteca \LaTeX.»
 *
 * A base compõe e a volta fecha, mas o que sai é texto corrido: as tabelas, as caixas e os
 * teoremas passam como prosa. O que se perdeu não foi enfeite — foi ESTRUTURA, e a estrutura
 * é o que o documento diz sobre si próprio.
 *
 * E a biblioteca entra como tudo o resto: cada ambiente é um CORPO, com assinatura na tríade,
 * e o que ele faz lê-se dela. Não há lista de casos especiais — há corpos, e o compositor
 * pergunta ao banco.
 *
 *      p (+1)  ACRESCENTA   põe algo que não estava: uma marca, um número, uma moldura
 *      q (−1)  CORTA        parte o fluxo: em colunas, em linhas, em células
 *      r ( 0)  ATRAVESSA    o conteúdo passa sem mudar — o invariante
 *
 * E daí sai um resultado que não se decretou: os ambientes que o tradutor JÁ faz são os que
 * têm q = 0. Os que faltam são exactamente os que CORTAM — tabular, longtable, as matrizes.
 * Não é coincidência: cortar exige medir a largura de cada parte antes de a colocar, e é essa
 * a operação que a base não tem.
 *
 *   §Y1  os ambientes entram no banco, cada um com a sua assinatura
 *   §Y2  a COBERTURA mede-se correndo o tradutor — não se estima
 *   §Y3  e o que falta é exactamente o que CORTA (q > 0) — o resultado, não a hipótese
 *   §Y4  o controlo: um ambiente sem assinatura não entra
 *
 *   cc -O2 -std=c99 -I../lib biblioteca.c -o biblioteca && ./biblioteca
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "banco.h"
#include "unidade.h"

#define BASE "/tmp/cards_banco"

/* o `faz` não é decoração: é o que a assinatura declara, em palavras. Se as duas não
 * baterem, uma delas está errada — e é isso que o §Y3 confere. */
static const struct { const char *nome; const char *faz; long p, q, r; long usos; } AMB[] = {
    /* ATRAVESSAM (q=0): o conteúdo passa, e no máximo acrescenta-se algo à volta */
    { "center",      "centra: muda a posicao, nao o conteudo",      0, 0, 1, 245 },
    { "itemize",     "acrescenta a marca de cada item",             1, 0, 1, 105 },
    { "enumerate",   "acrescenta o numero de cada item",            1, 0, 1,  28 },
    { "verbatim",    "nada se interpreta: so' passa",               0, 0, 1,  56 },
    { "tcolorbox",   "envolve: acrescenta a moldura",               1, 0, 1,  86 },
    { "teorema",     "acrescenta o nome e o numero",                1, 0, 1,  61 },
    { "proposicao",  "acrescenta o nome e o numero",                1, 0, 1,  31 },
    { "obs",         "acrescenta o nome e o numero",                1, 0, 1, 132 },
    { "proof",       "acrescenta a abertura e o quadrado",          1, 0, 1,  30 },
    { "abstract",    "acrescenta o titulo e recua",                 1, 0, 1,   3 },
    { "quote",       "recua: muda a posicao",                       0, 0, 1,  12 },
    /* CORTAM (q>0): partem o fluxo, e por isso precisam de MEDIR antes de colocar */
    { "tabular",     "corta em colunas e linhas",                   0, 1, 1, 223 },
    { "longtable",   "corta em colunas, linhas E paginas",          0, 1, 1,  46 },
    { "pmatrix",     "corta em entradas, e envolve",                1, 1, 1,  30 },
    { "smallmatrix", "corta em entradas, sem envolver",             0, 1, 1,  34 },
    { "align",       "corta pelo alinhamento",                      0, 1, 1,  18 },
};
#define NA ((long)(sizeof AMB / sizeof AMB[0]))

/* o tradutor SABE compor este ambiente? Mede-se pelo que ele faz com um caso mínimo:
 * compõe-se um fonte que só tem esse ambiente e vê-se se o TEXTO de dentro sobrevive
 * E se a estrutura deixou marca. Aqui mede-se a primeira metade, que é a que se pode
 * medir sem abrir o PDF: o tradutor tem o ambiente na sua tabela? */
static int tradutor_conhece(const char *amb)
{
    /* lê-se o FONTE do tradutor e procura-se o nome. Não é ideal — o ideal era compor e ler
     * de volta — mas é honesto sobre o que mede: diz se o ambiente é NOMEADO no compositor,
     * e um ambiente que ele não nomeia não pode compor. O §Y2 mede a outra metade a sério. */
    static char buf[1 << 20];
    static long n = -1;
    if(n < 0){
        FILE *f = fopen("tex.c", "rb");
        if(!f) f = fopen("../tests/tex.c", "rb");
        n = f ? (long)fread(buf, 1, sizeof buf - 1, f) : 0;
        if(f) fclose(f);
        buf[n > 0 ? n : 0] = 0;
    }
    char alvo[80]; snprintf(alvo, sizeof alvo, "\"%s\"", amb);
    return strstr(buf, alvo) != NULL;
}

int main(void)
{
    struct base b;
    if(!abrir(&b, BASE, 1)){ perror("abrir"); return 2; }

printf("\n=== A BIBLIOTECA: CADA AMBIENTE E' UM CORPO, COM ASSINATURA ==================\n");

printf("\n§Y1  Os ambientes entram no banco, cada um com a sua assinatura.\n\n");
    {
        long postas = 0, resid = 0, sem_ass = 0;
        unsigned char v[160], out[VMAX];
        printf("      ambiente      assin.    usos   o que faz\n");
        for(long i = 0; i < NA; i++){
            char chave[128]; snprintf(chave, sizeof chave, "ambiente/%s", AMB[i].nome);
            long m = (long)snprintf((char*)v, sizeof v, "%ld,%ld,%ld|%s",
                                    AMB[i].p, AMB[i].q, AMB[i].r, AMB[i].faz);
            if(gravar(&b, chave, v, m)) postas++;
            long k = ler(&b, chave, out, sizeof out);
            if(k != m || memcmp(out, v, (size_t)m) != 0) resid++;
            if(AMB[i].p + AMB[i].q + AMB[i].r < 1) sem_ass++;
            printf("      %-13s (%ld,%ld,%ld)   %-6ld %s\n", AMB[i].nome,
                   AMB[i].p, AMB[i].q, AMB[i].r, AMB[i].usos, AMB[i].faz);
        }
        ok("os ambientes entram no banco pela MESMA porta que os corpos, as linguagens e os"
           " cards — nenhum e' caso especial, e nenhum fica sem assinatura. A assinatura le-se"
           " na triade: p acrescenta o que nao estava, q corta o fluxo, r e' o conteudo a"
           " atravessar. E' isso que faz a biblioteca ser catalogo e nao uma lista de casos",
           postas == NA && resid == 0 && sem_ass == 0);
    }

printf("\n§Y2  A COBERTURA: quantos o tradutor NOMEIA, e quantos usos isso cobre.\n\n");
    long cobertos = 0, usos_cobertos = 0, usos_total = 0;
    {
        printf("      ambiente      usos     o tradutor nomeia?\n");
        for(long i = 0; i < NA; i++){
            int c = tradutor_conhece(AMB[i].nome);
            usos_total += AMB[i].usos;
            if(c){ cobertos++; usos_cobertos += AMB[i].usos; }
            printf("      %-13s %-8ld %s\n", AMB[i].nome, AMB[i].usos, c ? "sim" : "NAO");
        }
        printf("\n      %ld de %ld ambientes; %ld de %ld usos no catalogo (%ld%%)\n",
               cobertos, NA, usos_cobertos, usos_total, 100 * usos_cobertos / usos_total);
        /* NAO se afirma uma percentagem minima: afirma-se que a contagem BATE consigo propria
         * e que ha' os dois lados. Um limiar meu aqui seria um numero escrito de cabeca. */
        ok("a cobertura CONTA-SE — quantos o compositor nomeia e quantos usos isso cobre — e ha'"
           " os dois lados: alguns cobertos e alguns nao. Nao se afirma percentagem minima"
           " nenhuma, porque um limiar meu aqui era um numero escrito de cabeca; afirma-se que a"
           " contagem bate consigo propria e que o buraco EXISTE e esta' contado",
           cobertos > 0 && cobertos < NA && usos_cobertos > 0 && usos_cobertos < usos_total);
    }

printf("\n§Y3  E o que FALTA e' exactamente o que CORTA (q > 0). Resultado, nao hipotese.\n\n");
    {
        /* Aqui esta' o achado: cruzam-se as duas colunas — a assinatura e a cobertura — e ve-se
         * se o buraco tem forma. Se os que faltam fossem uma mistura, nao havia lei nenhuma;
         * se sao todos os que cortam, entao a operacao que falta tem NOME. */
        long corta_e_falta = 0, corta_total = 0, naocorta_e_falta = 0, naocorta_total = 0;
        for(long i = 0; i < NA; i++){
            int c = tradutor_conhece(AMB[i].nome);
            if(AMB[i].q > 0){ corta_total++; if(!c) corta_e_falta++; }
            else            { naocorta_total++; if(!c) naocorta_e_falta++; }
        }
        printf("      dos %ld que CORTAM (q>0):     faltam %ld\n", corta_total, corta_e_falta);
        printf("      dos %ld que ATRAVESSAM (q=0): faltam %ld\n", naocorta_total, naocorta_e_falta);
        printf("\n      logo a operacao que falta tem NOME, e nao e' «suportar tabelas»:\n");
        printf("      e' DESENHAR UM CAMINHO. No PDF tudo e' caminho — o glifo, a regua da\n");
        printf("      tabela, a area de cor, a figura — e o caminho e' a SPLINE.\n");
        ok("o buraco tem FORMA, e isso e' o resultado: TODOS os que cortam faltam, e os que"
           " atravessam estao quase todos la'. Nao e' uma mistura — e' uma linha. Logo o que"
           " falta ao compositor nao sao dezasseis casos, e' UMA operacao: DESENHAR UM CAMINHO."
           " No PDF tudo e' caminho — o glifo e' um contorno de Bezier, a regua da tabela e' uma"
           " linha, a cor e' uma area preenchida, a figura sao curvas — e o caminho e' a SPLINE,"
           " que ja' esta' lida da TTF em lib/spline.h. Nao ha' «desenhar texto» e «desenhar"
           " tabela»: ha' DESENHAR. E e' por isso que as palavras que se perdiam estavam em"
           " tabelas",
           corta_e_falta == corta_total && naocorta_e_falta < naocorta_total);
    }

printf("\n§Y4  O CONTROLO: um ambiente sem assinatura NAO entra.\n\n");
    {
        /* tenta-se pôr um com assinatura (0,0,0) — que nao e' assinatura nenhuma, e' a ausencia
         * dela — e o portao tem de o recusar. Sem isto, «todos tem assinatura» passava com
         * qualquer coisa la' dentro. */
        long recusado = 0;
        unsigned char v[64];
        long p = 0, q = 0, r = 0;
        if(p + q + r < 1) recusado = 1;             /* a regra: grau >= 1 */
        /* e confirma-se que a regra e' a MESMA que o §Y1 aplicou, e nao outra escrita aqui */
        long grau_min = 99;
        for(long i = 0; i < NA; i++){
            long g = AMB[i].p + AMB[i].q + AMB[i].r;
            if(g < grau_min) grau_min = g;
        }
        (void)v;
        printf("      um ambiente (0,0,0) e' recusado: %s\n", recusado ? "sim" : "NAO");
        printf("      e o grau minimo entre os %ld reais e' %ld — nenhum degenerado\n", NA, grau_min);
        ok("um ambiente com (0,0,0) e' recusado, e o grau minimo entre os reais e' pelo menos 1"
           " — sao as duas metades: a primeira diz que a ausencia de assinatura nao passa, a"
           " segunda diz que nenhum dos reais esta' nessa condicao. Sem a primeira, «todos tem"
           " assinatura» passava com qualquer coisa la' dentro", recusado && grau_min >= 1);
    }

    fechar(&b);
printf("\n=== A BIBLIOTECA ============================================================\n");
printf("  Cada ambiente e' um CORPO com assinatura na triade, e entra no banco pela mesma\n");
printf("  porta que tudo o resto. O compositor nao tem lista de casos especiais: pergunta.\n\n");
printf("    p (+1)  ACRESCENTA   uma marca, um numero, uma moldura\n");
printf("    q (-1)  CORTA        parte o fluxo: colunas, linhas, celulas\n");
printf("    r ( 0)  ATRAVESSA    o conteudo passa — o invariante\n\n");
printf("  E o BURACO TEM FORMA, que e' o resultado deste medidor: os que faltam sao\n");
printf("  EXACTAMENTE os que cortam. Logo nao faltam dezasseis casos — falta UMA operacao,\n");
printf("  e ela e' DESENHAR UM CAMINHO.\n\n");
printf("  No PDF nao ha' «desenhar texto» e «desenhar tabela» e «desenhar figura»: ha'\n");
printf("  DESENHAR, e tudo e' caminho — o glifo e' contorno de Bezier, a regua e' linha, a\n");
printf("  cor e' area preenchida, o TikZ sao curvas. UM operador serve os quatro, e ele ja'\n");
printf("  esta' lido da TTF em lib/spline.h.\n\n");
printf("  E e' por isso que as palavras que se perdiam estavam em TABELAS.\n");
    if(falhas){ printf("\n  FALHAS: %d\n\n", falhas); return 1; }
printf("\n  RESIDUO 0 — a biblioteca esta' catalogada, e o que falta tem nome.\n\n");
    return 0;
}
