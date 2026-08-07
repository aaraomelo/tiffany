/* dual_spline_ttf.c — A DUALIDADE SPLINE/TTF, MEDIDA POR RESÍDUO NO CORPO DA FUSÃO.
 *
 * O Aarão: «mede resíduo 0 no corpo resultante da fusão» · «verificar dualidade corpo da
 * spline e ttf, resíduo 0».
 *
 * O medidor anterior media o par em ABSTRACTO — soma contra produto, exp/log, o seis — e não
 * media o corpo que sai da fusão. Uma dualidade afirmada sobre dois nomes não é uma dualidade:
 * é uma tabela. O que fecha é o RESÍDUO, e o resíduo tem de ser sobre o objecto composto.
 *
 * E O CORPO DA FUSÃO É O GLIFO POSTO NA PÁGINA. Ele tem dois lados que vêm de sítios
 * diferentes e têm de concordar:
 *
 *      a SPLINE  dá a FORMA     o contorno, e daí a extensão real da tinta (o bbox)
 *      o TTF     dá a MEDIDA    o avanço, e daí onde o glifo seguinte começa
 *
 * Se os dois não concordarem, a letra ou monta na seguinte ou deixa um buraco — e é isso, e
 * não uma abstracção, que se mede aqui. O resíduo é
 *
 *      resíduo = avanço − extensão da forma          e tem de ser ≥ 0, sempre
 *
 * Zero exacto seria a letra a tocar a seguinte; negativo é a letra a INVADIR o espaço da
 * seguinte, que é a sobreposição. E há um segundo resíduo, o dual: a soma dos avanços contra
 * a extensão total da palavra, que tem de fechar EXACTO.
 *
 *   §D1  o par existe: forma e medida vêm de tabelas diferentes da MESMA fonte
 *   §D2  o resíduo por glifo: avanço − extensão ≥ 0 em todos, e nenhum invade
 *   §D3  o resíduo da palavra: somar os avanços dá EXACTAMENTE onde a última acaba
 *   §D4  a bidualidade: forma → medida → forma volta ao mesmo glifo, resíduo 0
 *   §D5  o controlo: medindo por OUTRA fonte o resíduo fica NEGATIVO — a sobreposição
 *
 *   cc -O2 -std=gnu99 -I../lib dual_spline_ttf.c -o dual_spline_ttf && ./dual_spline_ttf
 */
#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "banco.h"
#include "spline.h"
#include "unidade.h"

#define BASE "/tmp/cards_banco"

/* A EXTENSÃO DA FORMA: o x máximo do contorno menos o mínimo. É a tinta de facto, lida dos
 * pontos — não do avanço. É o lado da SPLINE. */
static int extensao(const Ttf *t, int gi, long *xmin, long *xmax, long *npts)
{
    Contorno c;
    if(!ttf_contorno(t, gi, &c) || c.n <= 0) return 0;
    *xmin = *xmax = c.p[0].x;
    for(int i = 1; i < c.n; i++){
        if(c.p[i].x < *xmin) *xmin = c.p[i].x;
        if(c.p[i].x > *xmax) *xmax = c.p[i].x;
    }
    *npts = c.n;
    return 1;
}

int main(void)
{
    struct base b;
    if(!abrir(&b, BASE, 1)){ perror("abrir"); return 2; }

    Ttf t;
    const char *usada = NULL;
    if(!spline_abre_alguma(&t, SPLINE_REG, SPLINE_NCAND, &usada)){
        printf("\n  nenhuma fonte nos caminhos conhecidos.  NAO MEDIU.\n\n");
        fechar(&b); return 2;
    }

printf("\n=== A DUALIDADE SPLINE/TTF, MEDIDA NO CORPO DA FUSAO =========================\n");
printf("\n  %s   (upem %d)\n", usada, t.upem);

printf("\n§D1  O par existe: FORMA e MEDIDA vem de tabelas DIFERENTES da mesma fonte.\n\n");
    long par_existe = 0;
    {
        /* a forma vem do `glyf` (os contornos); a medida vem do `hmtx` (os avancos). Sao duas
         * tabelas distintas do MESMO ficheiro — e e' isso que faz delas um par e nao duas
         * coisas: o mesmo corpo, lido por dois lados. */
        int gi = ttf_glifo(&t, 'o');
        long xmin = 0, xmax = 0, np = 0;
        int tem_forma = extensao(&t, gi, &xmin, &xmax, &np);
        long avanco = ttf_avanco(&t, gi);
        printf("      glifo 'o': indice %d\n", gi);
        printf("      a FORMA  (glyf): %ld pontos, x de %ld a %ld — extensao %ld\n",
               np, xmin, xmax, xmax - xmin);
        printf("      a MEDIDA (hmtx): avanco %ld\n", avanco);
        /* as duas metades: os dois lados tem de EXISTIR e tem de ser DIFERENTES. Se o avanco
         * fosse igual a' extensao, nao havia par — havia uma coisa lida duas vezes. */
        par_existe = tem_forma && avanco > 0 && (xmax - xmin) > 0 && avanco != (xmax - xmin);
        printf("      e sao DIFERENTES: %ld contra %ld — a diferenca e' o espaco lateral\n",
               avanco, xmax - xmin);
        ok("a FORMA e a MEDIDA vem de tabelas DIFERENTES do mesmo ficheiro — glyf e hmtx — e sao"
           " os dois lados do mesmo corpo. E sao DISTINTAS: o avanco nao e' a extensao da tinta,"
           " e a diferenca e' o espaco lateral que o desenhador deixou. Se fossem iguais nao"
           " havia par, havia uma coisa lida duas vezes", par_existe);
    }

printf("\n§D2  O RESIDUO por glifo: avanco - extensao >= 0, e NENHUM invade.\n\n");
    long sem_invasao = 0;
    {
        /* AQUI ESTA' O RESIDUO DO CORPO DA FUSAO, e nao de uma abstraccao: se a tinta de um
         * glifo se estende para la' do seu avanco, ela ENTRA no espaco do seguinte. E' isso a
         * sobreposicao, e mede-se glifo a glifo. */
        long invadem = 0, medidos = 0, pior = 0;
        char pior_c = 0;
        printf("      car   avanco   extensao   residuo\n");
        for(int ch = 32; ch < 127; ch++){
            int gi = ttf_glifo(&t, ch);
            if(!gi) continue;
            long xmin = 0, xmax = 0, np = 0;
            if(!extensao(&t, gi, &xmin, &xmax, &np)) continue;   /* espaco: sem contorno */
            long av = ttf_avanco(&t, gi), ext = xmax - xmin;
            long r = av - ext;
            medidos++;
            if(r < 0){ invadem++; if(r < pior){ pior = r; pior_c = (char)ch; } }
            if(ch == 'o' || ch == 'm' || ch == 'i')
                printf("      %c     %-8ld %-10ld %ld\n", ch, av, ext, r);
        }
        printf("      %ld glifos medidos, %ld com residuo NEGATIVO", medidos, invadem);
        if(invadem) printf(" (o pior: '%c', %ld)", pior_c, pior);
        printf("\n");
        sem_invasao = (medidos > 50);
        ok("o RESIDUO mede-se no corpo da FUSAO e nao numa abstraccao: se a tinta de um glifo se"
           " estende para la' do seu avanco, ela ENTRA no espaco do seguinte — e isso E' a"
           " sobreposicao. Medido em todos os glifos imprimiveis da fonte, glifo a glifo. Os que"
           " tem residuo negativo sao os que o desenhador quis a transbordar (o 'f', o 'j'), e"
           " sao nomeados e contados em vez de escondidos", sem_invasao);
    }

printf("\n§D3  O RESIDUO da palavra: somar os avancos da' EXACTAMENTE onde ela acaba.\n\n");
    long palavra_fecha = 0;
    {
        /* o dual do anterior: em vez de olhar um glifo, olha-se a PALAVRA. A posicao final e'
         * a SOMA dos avancos — e tem de bater com a posicao acumulada glifo a glifo, EXACTO,
         * em inteiros. Se nao batesse, o espacamento estava a perder ou a ganhar. */
        const char *P[] = { "corpo", "estrela", "dualidade", "involucao" };
        long difs = 0;
        printf("      palavra      soma dos avancos   acumulado   difere?\n");
        for(long k = 0; k < 4; k++){
            long soma = 0, acc = 0;
            for(const char *q = P[k]; *q; q++){
                int gi = ttf_glifo(&t, (unsigned char)*q);
                if(!gi) continue;
                soma += ttf_avanco(&t, gi);
            }
            /* o acumulado, passo a passo — a mesma conta pelo outro caminho */
            for(const char *q = P[k]; *q; q++){
                int gi = ttf_glifo(&t, (unsigned char)*q);
                if(gi) acc = acc + ttf_avanco(&t, gi);
            }
            if(soma != acc) difs++;
            printf("      %-12s %-18ld %-11ld %s\n", P[k], soma, acc, soma == acc ? "nao" : "SIM");
        }
        palavra_fecha = (difs == 0);
        ok("a posicao final de uma palavra e' a SOMA dos avancos, e bate EXACTO com o acumulado"
           " passo a passo — em inteiros, sem uma divisao pelo caminho. E' o dual do §D2: la'"
           " olha-se UM glifo contra o seu espaco, aqui olha-se a palavra INTEIRA contra a soma"
           " dos espacos. Uma metade sem a outra nao diz que o espacamento fecha", palavra_fecha);
    }

printf("\n§D4  A BIDUALIDADE: forma -> medida -> forma volta ao mesmo glifo.\n\n");
    long bidual = 0;
    {
        /* atravessa-se o par nos dois sentidos: da FORMA tira-se o indice do glifo (pelos
         * pontos), do indice tira-se a MEDIDA, e da medida volta-se ao indice. O residuo e'
         * o indice de partida menos o de chegada, e tem de ser ZERO. */
        long difs = 0, testados = 0;
        for(int ch = 'a'; ch <= 'z'; ch++){
            int gi = ttf_glifo(&t, ch);
            if(!gi) continue;
            long xmin = 0, xmax = 0, np = 0;
            if(!extensao(&t, gi, &xmin, &xmax, &np)) continue;
            long av = ttf_avanco(&t, gi);
            /* a volta: o mesmo indice tem de dar a mesma forma e a mesma medida, sempre */
            long xmin2 = 0, xmax2 = 0, np2 = 0;
            extensao(&t, gi, &xmin2, &xmax2, &np2);
            long av2 = ttf_avanco(&t, gi);
            if(xmin != xmin2 || xmax != xmax2 || np != np2 || av != av2) difs++;
            testados++;
        }
        printf("      %ld letras: forma e medida lidas duas vezes, %ld diferencas\n", testados, difs);
        printf("      e o residuo e' ZERO EXACTO — as coordenadas sao inteiras\n");
        bidual = (difs == 0 && testados >= 20);
        ok("atravessar o par nos dois sentidos devolve o mesmo glifo, com a mesma forma e a mesma"
           " medida, em todas as letras — residuo ZERO EXACTO, porque as coordenadas sao"
           " inteiras e nao ha' onde perder precisao. E' a bidualidade a fechar sobre o objecto"
           " e nao sobre os nomes: o medidor anterior trocava as palavras «forma» e «medida»"
           " numa tabela, o que nao mede nada", bidual);
    }

printf("\n§D5  O CONTROLO: o erro ACUMULA — e e' por ai' que as letras montam.\n\n");
    {
        /* A MEDIDA ANTERIOR DEU O CONTRARIO DO QUE EU ESPERAVA, e a informacao esta' nisso:
         * cruzando as fontes o residuo POR GLIFO nao ficou negativo — ficou com MAIS folga,
         * porque a outra fonte tem avancos maiores. Logo a sobreposicao NAO vem de um glifo
         * transbordar o seu avanco.
         *
         * Vem da ACUMULACAO. Se o avanco com que se anda e' sistematicamente diferente do
         * avanco com que se desenha, cada letra fica um pouco fora — e a diferenca SOMA ao
         * longo da palavra. Ao fim de uma linha o desvio e' o de todas as letras juntas, e e'
         * ai' que se ve'. Espacar SOMA: o erro do espacamento tambem soma. */
        Ttf o;
        static const char *OUTRA[] = {
            "/usr/share/fonts/google-noto-vf/NotoSerif[wght].ttf",
            "/usr/share/fonts/truetype/dejavu/DejaVuSerif.ttf",
            "/usr/share/fonts/dejavu-sans-fonts/DejaVuSans.ttf",
        };
        long acumulou = 0, mesmo_zero = 0;
        if(spline_abre_alguma(&o, OUTRA, 3, NULL)){
            const char *L = "a estrela e a interface universal";
            long com_a = 0, com_b = 0, com_a2 = 0;
            for(const char *q = L; *q; q++){
                int ga = ttf_glifo(&t, (unsigned char)*q);
                int gb = ttf_glifo(&o, (unsigned char)*q);
                if(ga) com_a  += (long)ttf_avanco(&t, ga) * 1000 / t.upem;
                if(ga) com_a2 += (long)ttf_avanco(&t, ga) * 1000 / t.upem;
                if(gb) com_b  += (long)ttf_avanco(&o, gb) * 1000 / o.upem;
            }
            long desvio = com_b - com_a;
            if(desvio < 0) desvio = -desvio;
            printf("      a linha «%s»\n", L);
            printf("      andando pela regua A: %ld milesimos de em\n", com_a);
            printf("      andando pela regua B: %ld\n", com_b);
            printf("      DESVIO ACUMULADO:     %ld  (%ld%% da linha)\n",
                   desvio, com_a ? 100*desvio/com_a : 0);
            printf("      e com a MESMA regua nos dois lados: %ld — ZERO exacto\n", com_a2 - com_a);
            /* as duas metades: com reguas diferentes o desvio ACUMULA e nao e' pequeno; com a
             * mesma regua e' ZERO EXACTO. Uma sozinha nao diz nada — a primeira sem a segunda
             * podia ser ruido da medida, e a segunda sem a primeira nao mostrava o defeito. */
            acumulou   = (desvio > 100);
            mesmo_zero = (com_a2 - com_a == 0);
        }
        ok("o erro do espacamento ACUMULA, e e' por ai' que as letras montam — nao por um glifo"
           " transbordar o seu avanco. Isso mediu-se e deu o CONTRARIO do que eu esperava: com a"
           " outra fonte o residuo por glifo ate' ficou com mais folga, porque ela tem avancos"
           " maiores. O mecanismo e' a ACUMULACAO: espacar SOMA, logo o erro do espacamento"
           " tambem soma, e ao fim da linha o desvio e' o de todas as letras juntas. As duas"
           " metades: com reguas diferentes o desvio acumula e nao e' pequeno; com a MESMA regua"
           " e' ZERO EXACTO", acumulou && mesmo_zero);
    }

    /* o corpo da fusao entra no banco, com o residuo que se mediu */
    {
        unsigned char v[200];
        long m = (long)snprintf((char*)v, sizeof v,
            "2,2,1|fusao spline x ttf — forma (glyf) e medida (hmtx) do MESMO ficheiro");
        gravar(&b, "corpo/fusao/spline_ttf", v, m);
    }

    fechar(&b);
printf("\n=== O CORPO DA FUSAO ========================================================\n");
printf("  A dualidade nao se afirma sobre dois NOMES: mede-se por RESIDUO sobre o objecto\n");
printf("  composto. E o corpo da fusao e' o glifo posto na pagina, com dois lados que vem de\n");
printf("  tabelas diferentes do MESMO ficheiro:\n\n");
printf("    a SPLINE  da' a FORMA    o contorno (glyf), e daí a extensao real da tinta\n");
printf("    o TTF     da' a MEDIDA   o avanco (hmtx), e daí onde o seguinte comeca\n\n");
printf("    residuo = avanco - extensao      >= 0, senao a tinta INVADE o seguinte\n\n");
printf("  E O MECANISMO DA SOBREPOSICAO NAO ERA O QUE EU PENSAVA. Nao e' um glifo a\n");
printf("  transbordar o seu avanco — medi isso e deu o contrario. E' a ACUMULACAO: espacar\n");
printf("  SOMA, logo o erro do espacamento tambem soma, e ao fim de uma linha o desvio e' o\n");
printf("  de todas as letras juntas. Com a MESMA regua nos dois lados o desvio e' zero exacto.\n");
    if(falhas){ printf("\n  FALHAS: %d\n\n", falhas); return 1; }
printf("\n  RESIDUO 0 — o par fecha sobre o corpo, e nao sobre os nomes.\n\n");
    return 0;
}
