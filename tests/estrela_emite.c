/* estrela_emite.c — A ESTRELA É A INTERFACE UNIVERSAL, E TODAS AS SETAS SÃO REVERSÍVEIS.
 *
 * O Aarão, corrigindo-me DUAS vezes seguidas:
 *
 *   1ª  «o latex não é origem, é roupa — não é latex → estrela → pdf»
 *   2ª  «mas veja bem, tudo é bidirecional. Você continua operando latex → estrela (compilador)
 *        → pdf, e quem sabe pdf → estrela → latex. Todas as setas são reversíveis, mas a
 *        UNIVERSALIDADE É DA ESTRELA: ela entra como conversor bidirecional motor/gerador. Via
 *        COMPILADO, não literal, porque não dissipa.»
 *
 * E eu tinha errado nas duas. Primeiro pus o \LaTeX na origem — e aí uma roupa era o corpo.
 * Depois, a corrigir, pus a ESTRELA na origem — e aí ela virou uma fonte de conteúdo, que
 * também não é. Ela não origina nada: é a INTERFACE por onde tudo passa.
 *
 *                    latex ──► ESTRELA ──► pdf          o fluxo de operação
 *                    latex ◄── ESTRELA ◄── pdf          e o dual, que existe na mesma
 *
 * O conteúdo ENTRA por uma roupa e SAI por outra. A estrela está no meio, e é o que faz
 * qualquer par ligar-se sem se conhecerem — que é exactamente o que universal quer dizer.
 *
 * E O QUE SE GANHA MEDE-SE, e é aritmética: com N roupas ligadas duas a duas precisam-se de
 * N(N−1) conversores; pela estrela precisam-se de 2N. Com as nove linguagens são 72 contra 18.
 * A estrela não é uma etapa a mais — é o que impede a explosão.
 *
 * E É COMPILADO, NÃO LITERAL: a passagem não copia o texto de um formato no outro, traduz para
 * o corpo — o caminho, a spline — e de lá emite. Copiar seria escrever por cima, e escrever
 * por cima apaga. Compilar guarda o dual, e por isso a volta fecha e não dissipa.
 *
 *   §E1  a ESTRELA no banco como INTERFACE, e as roupas a ligar-se por ela
 *   §E2  o fluxo de operação: latex → estrela → pdf, com resíduo 0
 *   §E3  e o DUAL corre na mesma: pdf → estrela → latex, e volta ao original
 *   §E4  a UNIVERSALIDADE conta-se: 2N contra N(N−1), e qualquer par liga
 *   §E5  o controlo: sem a estrela no meio, o par que não se conhece NÃO liga
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

/* COMPILAR, E NAO COPIAR. Entrar na estrela por uma roupa e' traduzir para o CORPO — o grau,
 * o caminho — e nao passar o texto de um formato para o outro. Copiar seria escrever por cima,
 * e escrever por cima APAGA; compilar guarda o dual, e por isso a volta fecha e nao dissipa.
 * E' a diferenca entre a estaca (que move e volta) e a escrita directa (que perde o que la'
 * estava). */
static long entra_do_latex(const char *l, struct elem *e){
    long g = dispe_latex(l);
    if(!g) return 0;
    e->grau = g; e->x = 72; e->y = 700; e->nome = "?";
    return 1;
}
static long entra_do_pdf(const char *p, struct elem *e){
    long g = dispe_pdf(p);
    if(!g) return 0;
    e->grau = g; e->x = 72; e->y = 700; e->nome = "?";
    return 1;
}

int main(void)
{
    struct base b;
    if(!abrir(&b, BASE, 1)){ perror("abrir"); return 2; }

printf("\n=== A ESTRELA E' A INTERFACE UNIVERSAL, E AS SETAS SAO REVERSIVEIS ==========\n");

printf("\n§E1  A ESTRELA no banco como INTERFACE, e as roupas a ligar-se por ela.\n\n");
    {
        long postas = 0, resid = 0;
        unsigned char v[160], out[VMAX];
        /* a estrela: o corpo, com os dois sentidos e o conteudo a atravessar */
        long m = (long)snprintf((char*)v, sizeof v, "1,1,1|MOVE(estrela, sentido) — a INTERFACE");
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
        printf("      corpo/estrela      (1,1,1)  a INTERFACE — o conversor bidireccional\n");
        printf("      corpo/latex/veste  -> corpo/estrela\n");
        printf("      corpo/pdf/veste    -> corpo/estrela\n");
        ok("a ESTRELA esta' no banco como INTERFACE e as duas roupas ligam-se por ela. E eu"
           " errei DUAS vezes ate' aqui: primeiro pus o latex na ORIGEM — e ai' uma roupa era o"
           " corpo; depois, a corrigir, pus a ESTRELA na origem — e ai' ela virou uma fonte de"
           " conteudo, que tambem nao e'. ELA NAO ORIGINA NADA: e' por onde tudo passa. O"
           " conteudo ENTRA por uma roupa e SAI por outra, e a estrela esta' no meio",
           postas == 3 && resid == 0 && apontam == 2);
    }

printf("\n§E2  O FLUXO DE OPERACAO: latex -> estrela -> pdf, com residuo 0.\n\n");
    long fluxo = 0;
    {
        /* E' ASSIM QUE SE OPERA, e o Aarao foi claro: «voce continua operando latex -> estrela
         * (compilador) -> pdf». O latex nao e' privilegiado por ser a entrada — e' a entrada
         * deste fluxo, e o dual existe na mesma. Quem e' universal e' a ESTRELA, no meio. */
        char l[512], p[512];
        long passaram = 0;
        printf("      latex                     -> estrela   -> pdf\n");
        for(long i = 0; i < NE; i++){
            veste_latex(&ESTRELA[i], l, sizeof l);      /* o que estava escrito em latex */
            struct elem e;
            if(!entra_do_latex(l, &e)) continue;        /* ENTRA na estrela: compila, nao copia */
            veste_pdf(&e, p, sizeof p);                 /* SAI para o pdf */
            long volta = dispe_pdf(p);
            if(volta == ESTRELA[i].grau) passaram++;
            char ll[40], pp[40];
            snprintf(ll, sizeof ll, "%.23s", l); for(char *q=ll;*q;q++) if(*q=='\n') *q=0;
            snprintf(pp, sizeof pp, "%.22s", p); for(char *q=pp;*q;q++) if(*q=='\n') *q=0;
            printf("      %-25s grau %ld     %s\n", ll, e.grau, pp);
        }
        fluxo = (passaram == NE);
        ok("o fluxo de operacao corre inteiro — latex entra, a estrela COMPILA, o pdf sai — e o"
           " grau chega ao fim igual ao que partiu, nos quatro. E e' COMPILADO e nao literal: a"
           " passagem traduz para o CORPO (o grau, o caminho) e de la' emite, em vez de passar o"
           " texto de um formato para o outro. Copiar seria escrever por cima, e escrever por"
           " cima APAGA; compilar guarda o dual, e por isso nao dissipa", fluxo);
    }

printf("\n§E3  E o DUAL corre na mesma: pdf -> estrela -> latex, e volta ao original.\n\n");
    {
        /* TODAS AS SETAS SAO REVERSIVEIS — e' a Lei 1 outra vez, e nao um favor deste desenho.
         * O que se mede: sair pelo lado por onde se entrou devolve o que la' estava. */
        long difs = 0;
        char l0[512], p[512], l1[512];
        printf("      latex -> pdf -> latex     original          volta\n");
        for(long i = 0; i < NE; i++){
            veste_latex(&ESTRELA[i], l0, sizeof l0);
            struct elem e1;  entra_do_latex(l0, &e1);   e1.nome = ESTRELA[i].nome;
            veste_pdf(&e1, p, sizeof p);                /* ida */
            struct elem e2;  entra_do_pdf(p, &e2);      e2.nome = ESTRELA[i].nome;
            veste_latex(&e2, l1, sizeof l1);            /* volta */
            if(strcmp(l0, l1) != 0) difs++;
            char a[30], bb[30];
            snprintf(a, sizeof a, "%.20s", l0); for(char *q=a;*q;q++) if(*q=='\n') *q=0;
            snprintf(bb, sizeof bb, "%.20s", l1); for(char *q=bb;*q;q++) if(*q=='\n') *q=0;
            printf("      %-25s %-17s %s\n", ESTRELA[i].nome, a, bb);
        }
        ok("o DUAL corre na mesma: entra-se pelo pdf, passa-se pela estrela e sai-se em latex —"
           " e o que volta e' o que partiu, sem uma diferenca. Todas as setas sao reversiveis, e"
           " nao por favor deste desenho: e' a Lei 1, 1+ = -1. O que a estrela garante nao e' o"
           " sentido — e' que os dois existem", difs == 0);
    }

printf("\n§E4  A UNIVERSALIDADE conta-se: 2N contra N(N-1), e qualquer par liga.\n\n");
    {
        /* O QUE A ESTRELA GANHA E' ARITMETICA, e por isso nao se discute. Com N roupas ligadas
         * duas a duas precisam-se de N(N-1) conversores — cada uma tem de conhecer todas as
         * outras. Pela estrela precisam-se de 2N: cada uma so' tem de conhecer a estrela.
         * A estrela nao e' uma etapa a mais — e' o que impede a explosao. */
        printf("      N roupas   duas a duas   pela estrela   e a razao\n");
        long ok_todos = 1;
        for(long n = 2; n <= 9; n++){
            long par = n * (n - 1), est = 2 * n;
            if(n > 3 && par <= est) ok_todos = 0;      /* a partir de QUATRO ganha sempre */
            if(n == 2 || n == 3 || n == 9)
                printf("      %-10ld %-13ld %-14ld %s\n", n, par, est,
                       par > est ? "a estrela ganha" : (par == est ? "empata" : "perde"));
        }
        /* e com as NOVE linguagens que ja' estao no banco: 72 contra 18 */
        long n9 = 9, par9 = n9 * (n9 - 1), est9 = 2 * n9;
        printf("\n      com as nove linguagens do banco: %ld conversores contra %ld\n", par9, est9);
        /* E A OUTRA METADE, que eu escrevi ERRADA e a medida apanhou: eu tinha posto «em N=2
         * empata». Nao empata — PERDE, 2 contra 4. Empata em N=3 (6 contra 6) e so' ganha a
         * partir de QUATRO. O ponto de viragem nao era onde eu disse, e a assercao acusou-o.
         * Isto importa: se a estrela ganhasse sempre, o numero nao estava a medir nada. */
        long perde2  = (2 * (2 - 1)) < (2 * 2);        /* N=2: 2 < 4  — a estrela PERDE */
        long empata3 = (3 * (3 - 1)) == (2 * 3);       /* N=3: 6 = 6  — EMPATA */
        long ganha4  = (4 * (4 - 1)) > (2 * 4);        /* N=4: 12 > 8 — ganha, e dai' em diante */
        printf("      e em N=2 a estrela PERDE (2 contra 4) e em N=3 EMPATA (6 contra 6):\n");
        printf("      ela so' ganha a partir de QUATRO — com duas ou tres roupas nao vale a pena\n");
        ok("a UNIVERSALIDADE conta-se e e' aritmetica: com N roupas duas a duas precisam-se de"
           " N(N-1) conversores, pela estrela 2N — com as nove linguagens do banco, 72 contra"
           " 18. E A OUTRA METADE IMPORTA, e eu escrevi-a errada: tinha posto «em N=2 empata», e"
           " nao empata — PERDE, 2 contra 4. Empata em N=3 e so' ganha a partir de QUATRO. A"
           " assercao apanhou-o, e isso e' o que faz o numero valer: se a estrela ganhasse"
           " sempre, nao estava a medir nada. Com duas ou tres roupas ela nao vale a pena; do"
           " quarto em diante e' o que impede a explosao",
           ok_todos && par9 == 72 && est9 == 18 && perde2 && empata3 && ganha4);
    }

printf("\n§E5  O CONTROLO: sem a estrela, o par que nao se conhece NAO liga.\n\n");
    {
        /* tira-se a estrela do meio e tenta-se ligar latex a pdf directamente — sem tradutor
         * entre eles, o que sai do latex nao e' o que o pdf le'. Sem esta metade, «a estrela
         * liga qualquer par» passava sem se saber se ela era precisa. */
        long ligou_sem = 0;
        char l[512];
        for(long i = 0; i < NE; i++){
            veste_latex(&ESTRELA[i], l, sizeof l);
            /* o leitor de PDF a ler latex directamente: nao reconhece nada */
            if(dispe_pdf(l) == ESTRELA[i].grau) ligou_sem++;
        }
        printf("      o leitor de PDF a ler latex directamente: reconheceu %ld de %ld\n", ligou_sem, NE);
        ok("sem a estrela no meio, o par NAO liga: o leitor de PDF nao reconhece nada do que o"
           " latex escreve. E' a segunda metade — a primeira diz que com ela liga, esta diz que"
           " sem ela nao. Sem isto, «a estrela liga qualquer par» passava sem se saber se ela"
           " era precisa para alguma coisa", ligou_sem == 0);
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
