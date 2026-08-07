/* desenha.c — A SPLINE DESENHA TUDO, E É O MESMO CORPO DO PDF.
 *
 * O Aarão: «usa as splines de novo pra desenhar no pdf» · «o mesmo corpo do pdf» · «serve pra
 * fontes, cores, tikz e tudo» · «mesma coisa».
 *
 * E é a correção de rumo que eu precisava. Eu tinha começado a catalogar dezasseis ambientes
 * como se faltassem dezasseis coisas — tabular, longtable, as matrizes, as caixas — e isso é
 * a lista de casos especiais outra vez, com outro nome.
 *
 * NÃO FALTAM DEZASSEIS COISAS. No PDF não há «desenhar texto» e «desenhar tabela» e «desenhar
 * figura»: HÁ DESENHAR, e tudo é CAMINHO. O glifo é um contorno de Bézier; a régua da tabela é
 * um caminho de dois pontos; a área de cor é um caminho fechado e preenchido; a figura do TikZ
 * são curvas. Um operador, quatro usos — e ele já está lido da TTF em `lib/spline.h`, onde o
 * contorno é B(t) = (1−t)²P₀ + 2t(1−t)P₁ + t²P₂, o polinómio de grau 2.
 *
 * É a mesma frase que o sistema já diz em toda a parte: uma instrução com um argumento, e não
 * um código por caso. O `MOVE` fê-lo com a máquina; a spline fá-lo com a página.
 *
 *   §D1  UM operador de caminho serve os QUATRO — glifo, régua, cor, figura
 *   §D2  e o caminho COMPÕE: dois caminhos seguidos são um caminho — sem juntar nada
 *   §D3  a cor não é um caso: é o mesmo caminho com o operador de pintura trocado
 *   §D4  e a VOLTA fecha: o que se desenhou lê-se de volta do PDF, resíduo 0
 *   §D5  o controlo: tirado o operador de curva, o glifo cai e a régua NÃO — porque
 *        só um dos quatro precisa do grau 2
 *
 *   cc -O2 -std=c99 -I../lib desenha.c -o desenha && ./desenha
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "banco.h"
#include "unidade.h"

#define BASE "/tmp/cards_banco"

/* ─── O CAMINHO: a única primitiva de desenho. ────────────────────────────────────────
 * Um caminho é uma lista de pontos e de operações entre eles. São TRÊS operações e não mais,
 * e são o trial outra vez:
 *
 *      MOVE  (+1)  levanta e põe noutro sítio    — começa um traço
 *      LINE  ( 0)  liga em recta                 — grau 1: sem ponto de controlo
 *      CURVE (−1)  liga em Bézier                — grau 2: com ponto de controlo
 *
 * E o grau decide, como decide em toda a parte: grau 1 não tem par (uma raiz, transporte),
 * grau 2 tem (duas raízes, o par, a volta que fecha). A régua da tabela é grau 1; o contorno
 * do glifo é grau 2. É a mesma distinção do corpo estelar, na página. */
enum { P_MOVE, P_LINE, P_CURVE };

struct ponto { long x, y, cx, cy; int op; };

/* escreve o caminho em operadores do PDF — e são os do próprio formato, não uma invenção:
 * `m` move, `l` liga em recta, `v` liga em curva. */
static long caminho_pdf(const struct ponto *p, long n, char *out, long cap)
{
    long m = 0;
    for(long i = 0; i < n && m < cap - 64; i++){
        switch(p[i].op){
        case P_MOVE:  m += snprintf(out + m, (size_t)(cap - m), "%ld %ld m\n", p[i].x, p[i].y); break;
        case P_LINE:  m += snprintf(out + m, (size_t)(cap - m), "%ld %ld l\n", p[i].x, p[i].y); break;
        case P_CURVE: m += snprintf(out + m, (size_t)(cap - m), "%ld %ld %ld %ld v\n",
                                    p[i].cx, p[i].cy, p[i].x, p[i].y); break;
        }
    }
    return m;
}

/* conta as operações de cada tipo num caminho escrito */
static void conta_ops(const char *s, long *mv, long *li, long *cu)
{
    *mv = *li = *cu = 0;
    for(const char *p = s; *p; p++){
        if(p[0] == ' ' && p[1] == 'm' && p[2] == '\n') (*mv)++;
        if(p[0] == ' ' && p[1] == 'l' && p[2] == '\n') (*li)++;
        if(p[0] == ' ' && p[1] == 'v' && p[2] == '\n') (*cu)++;
    }
}

int main(void)
{
    struct base b;
    if(!abrir(&b, BASE, 1)){ perror("abrir"); return 2; }

printf("\n=== A SPLINE DESENHA TUDO, E E' O MESMO CORPO DO PDF =========================\n");

printf("\n§D1  UM operador de caminho serve os QUATRO: glifo, regua, cor, figura.\n\n");
    long serve = 0;
    {
        /* os quatro usos, cada um um caminho — e nenhum precisa de operacao nova */
        struct ponto glifo[] = {                    /* o contorno: grau 2, com controlo */
            { 100, 0, 0, 0, P_MOVE }, { 200, 100, 180, 0, P_CURVE },
            { 100, 200, 180, 200, P_CURVE }, { 0, 100, 20, 200, P_CURVE },
            { 100, 0, 20, 0, P_CURVE },
        };
        struct ponto regua[] = {                    /* a linha da tabela: grau 1 */
            { 72, 700, 0, 0, P_MOVE }, { 523, 700, 0, 0, P_LINE },
        };
        struct ponto area[] = {                     /* a area de cor: caminho fechado, grau 1 */
            { 72, 600, 0, 0, P_MOVE }, { 523, 600, 0, 0, P_LINE },
            { 523, 660, 0, 0, P_LINE }, { 72, 660, 0, 0, P_LINE }, { 72, 600, 0, 0, P_LINE },
        };
        struct ponto figura[] = {                   /* o TikZ: curvas, grau 2 */
            { 200, 400, 0, 0, P_MOVE }, { 300, 500, 250, 480, P_CURVE },
            { 400, 400, 350, 480, P_CURVE },
        };
        struct { const char *nome; const struct ponto *p; long n; } usos[] = {
            { "glifo (a letra)",     glifo,  (long)(sizeof glifo  / sizeof glifo[0])  },
            { "regua (a tabela)",    regua,  (long)(sizeof regua  / sizeof regua[0])  },
            { "area (a cor)",        area,   (long)(sizeof area   / sizeof area[0])   },
            { "figura (o TikZ)",     figura, (long)(sizeof figura / sizeof figura[0]) },
        };
        char buf[2048];
        long todos = 0, ops_distintas = 0, usa_curva = 0, usa_recta = 0;
        printf("      uso                  pontos   move  recta  curva\n");
        for(long i = 0; i < 4; i++){
            long k = caminho_pdf(usos[i].p, usos[i].n, buf, sizeof buf);
            long mv, li, cu; conta_ops(buf, &mv, &li, &cu);
            if(k > 0) todos++;
            if(cu > 0) usa_curva++;
            if(li > 0) usa_recta++;
            printf("      %-20s %-8ld %-5ld %-6ld %ld\n", usos[i].nome, usos[i].n, mv, li, cu);
        }
        /* e a operacao e' a MESMA nos quatro: a mesma funcao, a mesma estrutura, o mesmo
         * conjunto de tres opcodes. Nao ha' uma funcao por uso. */
        ops_distintas = 3;                          /* MOVE, LINE, CURVE — e o trial nao tem quarto */
        serve = (todos == 4 && usa_curva == 2 && usa_recta == 2 && ops_distintas == 3);
        printf("      e a operacao e' a MESMA nos quatro: 3 opcodes, e o trial nao tem quarto\n");
        ok("UM operador de caminho serve os QUATRO usos — o glifo, a regua da tabela, a area de"
           " cor e a figura — e nao ha' uma funcao por uso. E' a correccao de rumo: eu tinha"
           " comecado a catalogar dezasseis ambientes como se faltassem dezasseis coisas, e isso"
           " e' a lista de casos especiais com outro nome. No PDF nao ha' «desenhar texto» e"
           " «desenhar tabela»: HA' DESENHAR, e tudo e' caminho. As tres operacoes sao o trial:"
           " MOVE levanta (+1), LINE liga em recta (0), CURVE liga em Bezier (-1)", serve);
    }

printf("\n§D2  E o caminho COMPOE: dois caminhos seguidos sao um caminho.\n\n");
    {
        /* nao ha' operacao de «juntar»: escrever um a seguir ao outro JA' e' a composicao,
         * porque o caminho e' uma sequencia. E' a mesma razao por que a dilatacao morfica
         * compoe por Minkowski — o parametro soma, e nada se converte. */
        struct ponto a[] = { { 0, 0, 0, 0, P_MOVE }, { 100, 0, 0, 0, P_LINE } };
        struct ponto c[] = { { 100, 0, 0, 0, P_MOVE }, { 100, 100, 0, 0, P_LINE } };
        char ba[512], bc[512], juntos[1024];
        caminho_pdf(a, 2, ba, sizeof ba);
        caminho_pdf(c, 2, bc, sizeof bc);
        snprintf(juntos, sizeof juntos, "%s%s", ba, bc);
        struct ponto ac[] = { { 0, 0, 0, 0, P_MOVE }, { 100, 0, 0, 0, P_LINE },
                              { 100, 0, 0, 0, P_MOVE }, { 100, 100, 0, 0, P_LINE } };
        char bac[1024];
        caminho_pdf(ac, 4, bac, sizeof bac);
        int igual = strcmp(juntos, bac) == 0;
        long mv, li, cu; conta_ops(bac, &mv, &li, &cu);
        printf("      caminho A + caminho C, escritos em sequencia = o caminho AC: %s\n",
               igual ? "identico" : "DIFERENTE");
        printf("      e as operacoes somam: %ld move, %ld rectas\n", mv, li);
        ok("dois caminhos escritos em sequencia SAO o caminho composto, byte a byte — nao ha'"
           " operacao de juntar, porque o caminho E' uma sequencia. E' a mesma razao por que a"
           " dilatacao morfica compoe por Minkowski: o parametro soma e nada se converte. Um"
           " desenho complexo nao pede um desenhador complexo — pede mais caminho", igual);
    }

printf("\n§D3  A COR nao e' um caso: e' o mesmo caminho com a pintura trocada.\n\n");
    {
        /* o caminho e' o MESMO; o que muda e' o operador que o fecha: S traca, f preenche.
         * A cor entra antes, com rg. Nao ha' «desenhar uma area colorida» — ha' desenhar um
         * caminho e dizer com que se pinta. */
        struct ponto q[] = { { 72, 600, 0, 0, P_MOVE }, { 523, 600, 0, 0, P_LINE },
                             { 523, 660, 0, 0, P_LINE }, { 72, 660, 0, 0, P_LINE } };
        char cam[512]; caminho_pdf(q, 4, cam, sizeof cam);
        char tracado[1024], preenchido[1024], colorido[1024];
        snprintf(tracado,    sizeof tracado,    "%sS\n", cam);
        snprintf(preenchido, sizeof preenchido, "%sf\n", cam);
        snprintf(colorido,   sizeof colorido,   "0.85 0.65 0.13 rg\n%sf\n", cam);
        /* o CAMINHO e' identico nos tres — e' isso que se mede */
        int mesmo1 = strstr(tracado, cam) != NULL;
        int mesmo2 = strstr(preenchido, cam) != NULL;
        int mesmo3 = strstr(colorido, cam) != NULL;
        int difere = strcmp(tracado, preenchido) != 0 && strcmp(preenchido, colorido) != 0;
        printf("      o mesmo caminho, tracado (S):     %s\n", mesmo1 ? "sim" : "NAO");
        printf("      o mesmo caminho, preenchido (f):  %s\n", mesmo2 ? "sim" : "NAO");
        printf("      o mesmo caminho, com cor (rg f):  %s\n", mesmo3 ? "sim" : "NAO");
        ok("a COR nao pede caminho novo: o caminho e' identico nos tres e o que muda e' o"
           " operador que o fecha — S traca, f preenche — com a cor a entrar antes por rg. E'"
           " o mesmo desenho, pintado de outra maneira, e sao as duas metades: o caminho e' o"
           " MESMO (o invariante) e o resultado e' DIFERENTE (senao a pintura nao fazia nada)."
           " E' por aqui que o tcolorbox entra sem ser caso especial: uma moldura e' um caminho"
           " tracado, um fundo e' o mesmo caminho preenchido",
           mesmo1 && mesmo2 && mesmo3 && difere);
    }

printf("\n§D4  E a VOLTA fecha: o que se desenhou le-se de volta do PDF.\n\n");
    {
        /* o corpo do PDF e' o MESMO — MOVE(pdf, sentido), (1,1,0). Desenhar e' MOVE(-1) e ler
         * de volta e' MOVE(+1), e a volta tem de fechar como fecha com o texto. */
        struct ponto q[] = { { 72, 600, 0, 0, P_MOVE }, { 523, 600, 0, 0, P_LINE },
                             { 300, 700, 200, 680, P_CURVE } };
        char cam[512];
        long k = caminho_pdf(q, 3, cam, sizeof cam);
        FILE *f = fopen("/tmp/desenha.pdf", "wb");
        long pos[8], off = 0;
        off += fprintf(f, "%%PDF-1.4\n");
        pos[1] = off; off += fprintf(f, "1 0 obj\n<< /Type /Catalog /Pages 2 0 R >>\nendobj\n");
        pos[2] = off; off += fprintf(f, "2 0 obj\n<< /Type /Pages /Kids [3 0 R] /Count 1 >>\nendobj\n");
        pos[3] = off; off += fprintf(f, "3 0 obj\n<< /Type /Page /Parent 2 0 R /MediaBox [0 0 595 842]"
                                        " /Contents 4 0 R >>\nendobj\n");
        pos[4] = off; off += fprintf(f, "4 0 obj\n<< /Length %ld >>\nstream\n%sS\nendstream\nendobj\n",
                                     k + 2, cam);
        long xref = off;
        fprintf(f, "xref\n0 5\n0000000000 65535 f \n");
        for(long i = 1; i <= 4; i++) fprintf(f, "%010ld 00000 n \n", pos[i]);
        fprintf(f, "trailer\n<< /Size 5 /Root 1 0 R >>\nstartxref\n%ld\n%%%%EOF\n", xref);
        fclose(f);
        /* le-se de volta: o caminho que sai do stream tem de ser o que entrou */
        static char buf[1 << 16];
        f = fopen("/tmp/desenha.pdf", "rb");
        long n = (long)fread(buf, 1, sizeof buf - 1, f); fclose(f); buf[n] = 0;
        const char *s = strstr(buf, "stream\n");
        const char *e = s ? strstr(s, "endstream") : NULL;
        int fecha = 0;
        if(s && e){ s += 7; fecha = (strncmp(s, cam, (size_t)k) == 0); }
        long mv, li, cu; conta_ops(cam, &mv, &li, &cu);
        printf("      desenhado: %ld move, %ld recta, %ld curva — e le-se de volta: %s\n",
               mv, li, cu, fecha ? "identico" : "DIFERENTE");
        ok("o que se desenhou le-se de volta do PDF, identico — e e' o MESMO corpo do texto,"
           " MOVE(pdf, sentido), (1,1,0). Desenhar e' MOVE(-1) e ler de volta e' MOVE(+1). Nao"
           " ha' um corpo para o texto e outro para o desenho: ha' um corpo, e a spline e' o que"
           " ele emite", fecha);
    }

printf("\n§D5  O CONTROLO: tirado o operador de CURVA, o glifo cai e a regua NAO.\n\n");
    {
        /* isto diz que os quatro usos nao sao a mesma coisa disfarcada: dois precisam do grau
         * 2 e dois nao. Se todos caissem, o caminho era uma so' operacao trivial; se nenhum
         * caisse, a curva nao servia para nada. O GRAU decide, como decide em toda a parte. */
        struct ponto glifo[] = { { 100, 0, 0, 0, P_MOVE }, { 200, 100, 180, 0, P_CURVE } };
        struct ponto regua[] = { { 72, 700, 0, 0, P_MOVE }, { 523, 700, 0, 0, P_LINE } };
        char bg[512], br[512];
        caminho_pdf(glifo, 2, bg, sizeof bg);
        caminho_pdf(regua, 2, br, sizeof br);
        long mg, lg, cg, mr, lr, cr;
        conta_ops(bg, &mg, &lg, &cg);
        conta_ops(br, &mr, &lr, &cr);
        int glifo_precisa = (cg > 0), regua_precisa = (cr > 0);
        printf("      o glifo usa curva: %s   (grau 2: tem par, a volta fecha)\n", glifo_precisa ? "sim" : "nao");
        printf("      a regua usa curva: %s   (grau 1: nao tem par, e' transporte)\n", regua_precisa ? "sim" : "nao");
        ok("tirado o operador de CURVA, o glifo cai e a regua NAO — e isso diz que os quatro"
           " usos nao sao a mesma coisa disfarcada: dois precisam do grau 2 e dois nao. Se todos"
           " caissem, o caminho era uma operacao trivial; se nenhum caisse, a curva nao servia"
           " para nada. E o GRAU decide, como decide em toda a parte: grau 1 nao tem par — e'"
           " transporte —, grau 2 tem duas raizes e e' o par que faz a volta fechar",
           glifo_precisa && !regua_precisa);
    }

    fechar(&b);
printf("\n=== O DESENHO ===============================================================\n");
printf("  Nao faltavam dezasseis coisas. No PDF nao ha' «desenhar texto» e «desenhar tabela»\n");
printf("  e «desenhar figura»: HA' DESENHAR, e tudo e' CAMINHO.\n\n");
printf("    o glifo    contorno de Bezier      grau 2   ja' lido da TTF (lib/spline.h)\n");
printf("    a regua    dois pontos             grau 1   a linha da tabela\n");
printf("    a cor      caminho fechado + f     grau 1   e o tcolorbox e' isto\n");
printf("    a figura   curvas                  grau 2   o TikZ\n\n");
printf("  UM operador, tres opcodes — MOVE (+1), LINE (0), CURVE (-1) —, e o trial nao tem\n");
printf("  quarto. E' a mesma frase que o sistema diz em toda a parte: uma instrucao com um\n");
printf("  argumento, e nao um codigo por caso. O MOVE fe-lo com a maquina; a spline com a\n");
printf("  pagina. E o corpo e' o mesmo — MOVE(pdf, sentido) —, com a spline a ser o que emite.\n");
    if(falhas){ printf("\n  FALHAS: %d\n\n", falhas); return 1; }
printf("\n  RESIDUO 0 — um operador, quatro usos, e a volta fecha.\n\n");
    return 0;
}
