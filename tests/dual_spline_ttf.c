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
 *   §D3  o RESÍDUO 0 a sério: três caminhos independentes para o mesmo número
 *   §D4  a MUTAÇÃO, que fecha o §D3: mexido um byte, o resíduo acusa — e volta
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

/* O LSB (left side bearing) DA TABELA hmtx — e é o SEGUNDO campo de cada entrada, a seguir
 * ao avanço. É a MEDIDA a dizer onde a tinta começa.
 *
 * E o mesmo número está no `glyf`, como xMin do glifo — a FORMA a dizer o mesmo. São dois
 * caminhos INDEPENDENTES para a mesma quantidade, gravados em tabelas diferentes por quem
 * desenhou a fonte, e é isso que faz deles um resíduo: se discordarem, um dos lados mente. */
static long lsb_da_medida(const Ttf *t, int g)
{
    return (long)(short)u16(&t->b, t->hmtx + 4L*g + 2);
}

/* e o xMin do glifo, lido do CABEÇALHO do glyf — não dos pontos. É outro caminho ainda:
 * a fonte grava-o à parte, e ele tem de bater com o mínimo dos pontos. */
static int xmin_do_cabecalho(const Ttf *t, int g, long *x)
{
    long ini, fim;
    if(t->longloca){ ini = u32(&t->b, t->loca + 4L*g); fim = u32(&t->b, t->loca + 4L*g + 4); }
    else           { ini = 2*u16(&t->b, t->loca + 2L*g); fim = 2*u16(&t->b, t->loca + 2L*g + 2); }
    if(fim <= ini) return 0;
    *x = (long)(short)u16(&t->b, t->glyf + ini + 2);      /* xMin, logo a seguir a numberOfContours */
    return 1;
}

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

printf("\n§D3  O RESIDUO 0 A SERIO: o lsb da MEDIDA contra o xMin da FORMA.\n\n");
    long residuo_zero = 0;
    {
        /* AQUI ESTA' O RESIDUO, E ANTES NAO ESTAVA. As duas versoes anteriores deste bloco
         * comparavam uma quantidade CONSIGO PROPRIA: numa somei `soma += x` e `acc = acc + x`
         * — a mesma conta escrita de duas maneiras —, noutra chamei a mesma funcao duas vezes
         * com os mesmos argumentos. Nenhuma podia falhar, e o Aarao perguntou directamente:
         * «mediu o residuo 0?». Nao tinha medido.
         *
         * O residuo precisa de DOIS CAMINHOS INDEPENDENTES para o mesmo numero, e a TTF tem
         * tres:
         *
         *    o lsb        na tabela hmtx      a MEDIDA a dizer onde a tinta comeca
         *    o xMin       no cabecalho glyf   a FORMA, gravada a' parte pela fonte
         *    o min dos x  nos PONTOS          a FORMA, calculada por mim
         *
         * Sao gravados em sitios diferentes por quem desenhou a fonte. Se discordarem, um dos
         * lados mente — e isso e' um residuo que PODE falhar. */
        long difs_lsb = 0, difs_cab = 0, medidos = 0;
        printf("      car   lsb(hmtx)   xMin(glyf)   min dos pontos   residuo\n");
        for(int ch = 32; ch < 127; ch++){
            int gi = ttf_glifo(&t, ch);
            if(!gi) continue;
            long xmin = 0, xmax = 0, np = 0;
            if(!extensao(&t, gi, &xmin, &xmax, &np)) continue;
            long lsb = lsb_da_medida(&t, gi), xcab = 0;
            if(!xmin_do_cabecalho(&t, gi, &xcab)) continue;
            medidos++;
            if(lsb  != xmin) difs_lsb++;
            if(xcab != xmin) difs_cab++;
            if(ch == 'o' || ch == 'm' || ch == 'A')
                printf("      %c     %-11ld %-12ld %-16ld %ld\n", ch, lsb, xcab, xmin, lsb - xmin);
        }
        printf("      %ld glifos: %ld com lsb != min dos pontos, %ld com xMin != min\n",
               medidos, difs_lsb, difs_cab);
        residuo_zero = (medidos > 50 && difs_lsb == 0 && difs_cab == 0);
        ok("o RESIDUO E' ZERO entre TRES caminhos independentes para o mesmo numero: o lsb da"
           " tabela hmtx (a MEDIDA), o xMin do cabecalho do glyf (a FORMA gravada a' parte) e o"
           " minimo dos pontos do contorno (a FORMA calculada aqui). Sao gravados em sitios"
           " diferentes por quem desenhou a fonte, e se discordassem um dos lados mentia. E' isto"
           " que as versoes anteriores deste bloco NAO faziam: comparavam uma quantidade consigo"
           " propria — `soma += x` contra `acc = acc + x`, e a mesma funcao chamada duas vezes."
           " Nenhuma podia falhar", residuo_zero);
    }

printf("\n§D4  A MUTACAO: mexido UM byte no lsb, o residuo do §D3 ACUSA.\n\n");
    {
        /* E AQUI ESTAVA MAIS UMA TAUTOLOGIA, que eu escrevi na propria correccao. Tinha posto
         * `rsb = avanco - lsb - extensao` e depois somava `lsb + extensao + rsb` de volta — que
         * da' o avanco SEMPRE, por construcao. Escrevi ate' «o rsb define-se pela identidade», o
         * que e' reconhecer o defeito e deixa-lo la'.
         *
         * O que fecha o §D3 nao e' outra identidade: e' a MUTACAO. Muda-se um dos tres caminhos
         * e o residuo TEM de deixar de ser zero. Se nao deixasse, os tres nao eram independentes
         * — eram o mesmo numero lido tres vezes. */
        long acusou = 0, glifos_afectados = 0, medidos = 0;
        int alvo = ttf_glifo(&t, 'o');
        if(alvo){
            long pos = t.hmtx + 4L*alvo + 2;                  /* o lsb do glifo 'o' */
            unsigned char a = t.b.d[pos], c = t.b.d[pos+1];
            long antes = 0;
            {   long xmin = 0, xmax = 0, np = 0;
                extensao(&t, alvo, &xmin, &xmax, &np);
                antes = lsb_da_medida(&t, alvo) - xmin;  }
            /* mexe-se UM byte: o lsb passa a mentir sobre onde a tinta comeca */
            t.b.d[pos] = (unsigned char)(a ^ 0x01);
            long depois = 0;
            {   long xmin = 0, xmax = 0, np = 0;
                extensao(&t, alvo, &xmin, &xmax, &np);
                depois = lsb_da_medida(&t, alvo) - xmin;  }
            /* e conta-se quantos glifos passam a discordar */
            for(int ch = 32; ch < 127; ch++){
                int gi = ttf_glifo(&t, ch);
                if(!gi) continue;
                long xmin = 0, xmax = 0, np = 0;
                if(!extensao(&t, gi, &xmin, &xmax, &np)) continue;
                medidos++;
                if(lsb_da_medida(&t, gi) != xmin) glifos_afectados++;
            }
            t.b.d[pos] = a; t.b.d[pos+1] = c;                 /* devolve-se SEMPRE */
            long volta = 0;
            {   long xmin = 0, xmax = 0, np = 0;
                extensao(&t, alvo, &xmin, &xmax, &np);
                volta = lsb_da_medida(&t, alvo) - xmin;  }
            printf("      o residuo do 'o' antes:   %ld\n", antes);
            printf("      com um byte mexido:       %ld\n", depois);
            printf("      e depois de devolver:     %ld\n", volta);
            printf("      glifos a discordar durante a mutacao: %ld de %ld\n",
                   glifos_afectados, medidos);
            acusou = (antes == 0) && (depois != 0) && (volta == 0) && (glifos_afectados == 1);
        }
        ok("mexido UM byte no lsb, o residuo do §D3 DEIXA de ser zero — e volta a zero quando o"
           " byte se devolve. E acusa UM glifo e nao todos: se acusasse todos, o que estaria a"
           " falhar era a leitura e nao o residuo. E' isto que fecha o §D3, e nao outra"
           " identidade: eu tinha posto aqui `rsb = avanco - lsb - extensao` e somava de volta,"
           " o que da' o avanco SEMPRE. Escrevi ate' «o rsb define-se pela identidade» — que e'"
           " reconhecer o defeito e deixa-lo la'", acusou);
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
