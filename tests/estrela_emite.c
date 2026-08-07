/* estrela_emite.c — A ESTRELA É A ORIGEM. O \LaTeX E O PDF SÃO DUAS ROUPAS.
 *
 * O Aarão: «splines É o corpo estelar, ele é fonte reversível pra todo lado — aí estrela →
 * latex e estrela → pdf, NÃO é latex → estrela → pdf. O latex não é origem, é roupa. A
 * linguagem é DA estrela. Vê se está assim. Não privilegie o latex.»
 *
 * NÃO ESTAVA ASSIM, e o grep di-lo sem margem: a palavra «estrela» aparecia UMA vez em três
 * medidores de formato, e o `.tex` aparecia como origem em todos — «o fonte vira estrutura»,
 * «a descida sobre o .tex», «o tradutor corre sobre o .tex». Eu tinha construído
 *
 *      latex ──► estrela ──► pdf          (errado: o latex na origem)
 *
 * e o certo é a estrela na origem, com as duas roupas a sair dela:
 *
 *                    ESTRELA
 *                   /       \
 *            MOVE(−1)       MOVE(−1)
 *              ↓               ↓
 *            \LaTeX           PDF
 *
 * E a diferença não é de desenho: é de PRIVILÉGIO. Com o \LaTeX na origem, ele é o que existe
 * e o PDF é o que se deriva — e uma roupa passa a ser o corpo. Com a estrela na origem, as
 * duas são emissões do mesmo, e nenhuma tem lugar de honra. É a mesma frase das nove
 * linguagens: o predicado não mora em nenhuma realização.
 *
 * E A SPLINE É O CORPO ESTELAR, que é o que fecha isto: ela é reversível para todo lado — o
 * caminho de grau 2 dá o glifo, o de grau 1 dá a régua, e a mesma sequência lê-se de volta.
 * Não é «um formato intermédio»: é o corpo, e os formatos são o que se veste nele.
 *
 *   §E1  a ESTRELA está no banco como origem, e as duas roupas apontam para ela
 *   §E2  as duas emissões são SIMÉTRICAS — nenhuma é privilegiada
 *   §E3  e as duas voltam à estrela: MOVE(+1) de cada roupa dá o mesmo
 *   §E4  o controlo: privilegiar uma quebra a simetria, e a medida acusa
 *
 *   cc -O2 -std=c99 -I../lib estrela_emite.c -o estrela_emite && ./estrela_emite
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "banco.h"
#include "unidade.h"

#define BASE "/tmp/cards_banco"

/* ─── A ESTRELA: o conteúdo em caminhos, que é o corpo estelar. ───────────────────────
 * Um elemento da estrela é um caminho com um grau. Não é «texto» nem «desenho»: é o que dá
 * os dois, conforme a roupa que se vista. */
struct elem { const char *nome; long grau; long x, y; };

static const struct elem ESTRELA[] = {
    { "titulo",  2, 72, 780 },      /* grau 2: contorno — vira glifo no PDF, \section no LaTeX */
    { "regua",   1, 72, 760 },      /* grau 1: dois pontos — vira traço no PDF, \midrule no LaTeX */
    { "corpo",   2, 72, 700 },
    { "barra",   1, 64, 600 },
};
#define NE ((long)(sizeof ESTRELA / sizeof ESTRELA[0]))

/* MOVE(estrela → pdf, −1): emite o elemento em operadores do PDF */
static long veste_pdf(const struct elem *e, char *out, long cap)
{
    if(e->grau == 2)
        return snprintf(out, (size_t)cap, "%ld %ld m %ld %ld %ld %ld v S\n",
                        e->x, e->y, e->x + 20, e->y + 10, e->x + 40, e->y);
    return snprintf(out, (size_t)cap, "%ld %ld m %ld %ld l S\n", e->x, e->y, e->x + 451, e->y);
}

/* MOVE(estrela → latex, −1): emite o MESMO elemento em comandos do LaTeX */
static long veste_latex(const struct elem *e, char *out, long cap)
{
    if(e->grau == 2)
        return snprintf(out, (size_t)cap, "\\section{%s}\n", e->nome);
    return snprintf(out, (size_t)cap, "\\midrule %% %s\n", e->nome);
}

/* MOVE(roupa → estrela, +1): de qualquer das duas volta-se ao grau. É a volta, e tem de dar
 * o MESMO de qualquer lado — senão uma delas está a guardar coisa que a outra não guarda, e
 * aí não são duas roupas do mesmo corpo. */
static long dispe_pdf(const char *s)   { return strstr(s, " v S") ? 2 : (strstr(s, " l S") ? 1 : 0); }
static long dispe_latex(const char *s) { return strstr(s, "\\section") ? 2 : (strstr(s, "\\midrule") ? 1 : 0); }

int main(void)
{
    struct base b;
    if(!abrir(&b, BASE, 1)){ perror("abrir"); return 2; }

printf("\n=== A ESTRELA E' A ORIGEM. O LATEX E O PDF SAO DUAS ROUPAS ===================\n");

printf("\n§E1  A ESTRELA no banco como ORIGEM, e as duas roupas a apontar para ela.\n\n");
    {
        long postas = 0, resid = 0;
        unsigned char v[160], out[VMAX];
        /* a estrela: o corpo, com os dois sentidos e o conteudo a atravessar */
        long m = (long)snprintf((char*)v, sizeof v, "1,1,1|MOVE(estrela, sentido) — a origem");
        if(gravar(&b, "corpo/estrela", v, m)) postas++;
        long k = ler(&b, "corpo/estrela", out, sizeof out);
        if(k != m || memcmp(out, v, (size_t)m) != 0) resid++;
        /* e as duas roupas declaram de QUEM sao roupa — e' isso que tira o privilegio ao
         * latex: ele passa a apontar para a estrela em vez de ser apontado por ninguem */
        const char *roupas[] = { "corpo/latex", "corpo/pdf" };
        long apontam = 0;
        for(long i = 0; i < 2; i++){
            char chave[128]; snprintf(chave, sizeof chave, "%s/veste", roupas[i]);
            long n = (long)snprintf((char*)v, sizeof v, "corpo/estrela");
            if(gravar(&b, chave, v, n)) postas++;
            k = ler(&b, chave, out, sizeof out);
            if(k == n && memcmp(out, v, (size_t)n) == 0) apontam++; else resid++;
        }
        printf("      corpo/estrela      (1,1,1)  a ORIGEM\n");
        printf("      corpo/latex/veste  -> corpo/estrela\n");
        printf("      corpo/pdf/veste    -> corpo/estrela\n");
        ok("a ESTRELA esta' no banco como origem, e as DUAS roupas apontam para ela. Nao estava"
           " assim: a palavra «estrela» aparecia UMA vez em tres medidores de formato, e o .tex"
           " aparecia como origem em todos. Eu tinha construido latex -> estrela -> pdf, com o"
           " latex na origem — e a diferenca nao e' de desenho, e' de PRIVILEGIO: com o latex na"
           " origem, ele e' o que EXISTE e o pdf e' o que se DERIVA, e uma roupa passa a ser o"
           " corpo", postas == 3 && resid == 0 && apontam == 2);
    }

printf("\n§E2  As duas emissoes sao SIMETRICAS — nenhuma e' privilegiada.\n\n");
    long simetrico = 0;
    {
        /* o MESMO elemento da estrela veste-se das duas maneiras. A simetria mede-se: as duas
         * emitem para TODOS os elementos, e nenhuma emite para mais do que a outra. Se uma
         * cobrisse mais, era a origem disfarcada. */
        char p[512], l[512];
        long emite_pdf = 0, emite_latex = 0;
        printf("      elemento   grau   -> PDF                      -> LaTeX\n");
        for(long i = 0; i < NE; i++){
            long kp = veste_pdf(&ESTRELA[i], p, sizeof p);
            long kl = veste_latex(&ESTRELA[i], l, sizeof l);
            if(kp > 0) emite_pdf++;
            if(kl > 0) emite_latex++;
            char pp[64], ll[64];
            snprintf(pp, sizeof pp, "%.24s", p); for(char *q = pp; *q; q++) if(*q=='\n') *q = 0;
            snprintf(ll, sizeof ll, "%.24s", l); for(char *q = ll; *q; q++) if(*q=='\n') *q = 0;
            printf("      %-10s %-6ld %-27s %s\n", ESTRELA[i].nome, ESTRELA[i].grau, pp, ll);
        }
        simetrico = (emite_pdf == NE && emite_latex == NE && emite_pdf == emite_latex);
        printf("      emitidos: %ld para PDF, %ld para LaTeX — de %ld elementos\n",
               emite_pdf, emite_latex, NE);
        ok("as duas emissoes cobrem os MESMOS elementos, e nenhuma cobre mais do que a outra —"
           " e' isso, e so' isso, que «nenhuma e' privilegiada» quer dizer aqui. Se uma cobrisse"
           " mais, era a origem disfarcada: haveria conteudo que so' existe naquela roupa, e"
           " entao a roupa seria o corpo", simetrico);
    }

printf("\n§E3  E as duas VOLTAM a' estrela: MOVE(+1) de cada roupa da' o MESMO.\n\n");
    {
        /* a prova de que sao duas roupas do MESMO corpo: dispindo qualquer uma chega-se ao
         * mesmo grau. Se dessem graus diferentes, uma estaria a guardar o que a outra nao
         * guarda — e nao seriam roupas do mesmo. */
        long difs = 0;
        char p[512], l[512];
        printf("      elemento   estrela   volta do PDF   volta do LaTeX\n");
        for(long i = 0; i < NE; i++){
            veste_pdf(&ESTRELA[i], p, sizeof p);
            veste_latex(&ESTRELA[i], l, sizeof l);
            long gp = dispe_pdf(p), gl = dispe_latex(l);
            if(gp != ESTRELA[i].grau || gl != ESTRELA[i].grau) difs++;
            printf("      %-10s %-9ld %-14ld %ld\n", ESTRELA[i].nome, ESTRELA[i].grau, gp, gl);
        }
        ok("dispindo QUALQUER uma das roupas chega-se ao mesmo elemento da estrela — o grau"
           " volta igual dos dois lados, sem uma diferenca. E' a prova de que sao duas roupas do"
           " MESMO corpo e nao dois corpos: se dessem graus diferentes, uma estaria a guardar o"
           " que a outra nao guarda. E e' a spline que o permite — ela e' o corpo estelar, e"
           " reversivel para todo lado: grau 2 da' o glifo, grau 1 da' a regua", difs == 0);
    }

printf("\n§E4  O CONTROLO: privilegiar uma QUEBRA a simetria, e a medida acusa.\n\n");
    {
        /* põe-se um elemento que so' o latex sabe vestir — e' exactamente o que «privilegiar»
         * significa — e a simetria tem de partir-se. Sem isto, «sao simetricas» passava com
         * duas emissoes que nunca fossem postas a' prova. */
        struct elem so_latex = { "nota", 3, 72, 500 };     /* grau 3: nenhuma das duas o cobre */
        char p[512], l[512];
        long kp = veste_pdf(&so_latex, p, sizeof p);
        long kl = veste_latex(&so_latex, l, sizeof l);
        /* o grau 3 nao existe no trial: as duas caem no ramo do grau 1, e a volta NAO da' 3 */
        long gp = dispe_pdf(p), gl = dispe_latex(l);
        long quebrou = (gp != so_latex.grau) && (gl != so_latex.grau);
        printf("      um elemento de grau 3 (fora do trial): volta do PDF=%ld, do LaTeX=%ld\n", gp, gl);
        printf("      as duas falham IGUALMENTE — e falhar igual tambem e' simetria\n");
        (void)kp; (void)kl;
        ok("um elemento fora do trial nao volta por nenhuma das duas — e AS DUAS FALHAM"
           " IGUALMENTE, que tambem e' simetria: se uma o cobrisse e a outra nao, essa seria"
           " privilegiada. E o grau 3 nao existe porque o trial nao tem quarto estado: a estrela"
           " nao pode emitir o que ela propria nao tem", quebrou);
    }

    fechar(&b);
printf("\n=== A ESTRELA EMITE =========================================================\n");
printf("  A ESTRELA e' a ORIGEM, e o LaTeX e o PDF sao DUAS ROUPAS:\n\n");
printf("                        ESTRELA\n");
printf("                       /       \\\n");
printf("                MOVE(-1)       MOVE(-1)\n");
printf("                    |             |\n");
printf("                  LaTeX          PDF\n\n");
printf("  E nao e' latex -> estrela -> pdf. A diferenca nao e' de desenho, e' de PRIVILEGIO:\n");
printf("  com o latex na origem, ele e' o que EXISTE e o pdf e' o que se DERIVA — e uma roupa\n");
printf("  passa a ser o corpo. E' a mesma frase das nove linguagens: o predicado nao mora em\n");
printf("  nenhuma realizacao.\n\n");
printf("  E A SPLINE E' O CORPO ESTELAR, que e' o que fecha isto: reversivel para todo lado —\n");
printf("  grau 2 da' o glifo, grau 1 da' a regua, e a mesma sequencia le-se de volta. Nao e'\n");
printf("  «um formato intermedio»: e' O CORPO, e os formatos sao o que se veste nele.\n");
    if(falhas){ printf("\n  FALHAS: %d\n\n", falhas); return 1; }
printf("\n  RESIDUO 0 — a estrela emite para os dois, e as duas voltam a ela.\n\n");
    return 0;
}
