/* numerica.c — A EXPRESSÃO NUMÉRICA: a precedência cai da ORDEM DAS DOBRAS.
 *
 * O Aarão: "vamos para as expressões numéricas, a assistente resolve passo a passo e explica;
 * começa simples, com parênteses, colchetes e chaves, soma e multiplicação."
 *
 * E não há máquina nova a inventar. Uma expressão aninhada É uma árvore, e resolver é dobrar de
 * dentro para fora — o OP_FOLD do banco, com números no lugar das folhas.
 *
 *   §X1  a fita entra no banco, e a expressão mal fechada é RECUSADA
 *   §X2  o passo a passo não se programa: cada dobra É um passo
 *   §X3  a precedência é a ORDEM das dobras, e mede-se que ela decide
 *   §X4  parêntese, colchete e chave são a MESMA coisa — manda a profundidade
 *   §X5  e o resultado bate com a conta à mão, em toda a bateria
 *   §X6  a DISTRIBUTIVA: os dois caminhos fecham no mesmo, e ela tem dual — fatorar
 *   §X7  e onde ela não vale (o x sobre o x) é RECUSADA, com o motivo
 *
 *   cc -O2 -std=c99 numerica.c -o numerica && ./numerica
 */
#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include "expr.h"
#include "unidade.h"

static int abre_fita(const char *nome){
    unlink(nome);
    return open(nome, O_RDWR|O_CREAT|O_TRUNC, 0644);
}

/* resolve e devolve o valor; se mostrar, imprime cada passo */
static int resolve(const char *nome, const char *e, long *out, int mostrar){
    int fd = abre_fita(nome);
    long n = ct_leia(fd, e);
    if(n < 0){ close(fd); unlink(nome); return (int)n; }
    char buf[2048], porque[256];
    if(mostrar){ ct_mostra(fd, n, buf, sizeof buf); printf("      %s\n", buf); }
    int passos = 0;
    while(ct_passo(fd, n, porque, sizeof porque)){
        passos++;
        if(mostrar){
            ct_mostra(fd, n, buf, sizeof buf);
            printf("      = %-26s   (%s)\n", buf, porque);
        }
        if(passos > 10000) break;
    }
    int bom = ct_valor(fd, n, out);
    close(fd); unlink(nome);
    return bom ? passos : -9;
}

int main(void){
printf("\n=== A EXPRESSÃO NUMÉRICA — resolvida por DOBRA, no banco ==================\n");
printf("    Não há máquina nova: uma expressão aninhada é uma árvore, e resolver é\n");
printf("    dobrar de dentro para fora. É o OP_FOLD, com números em vez de folhas.\n");

printf("\n§X1  A fita entra no banco, e o que não fecha é RECUSADO.\n\n");
{
    long v;
    struct { const char *e; int esperado; const char *porque; } mau[] = {
        { "2 + (3 x 4",   -4, "abriu e não fechou" },
        { "2 + 3) x 4",   -1, "fechou sem ter aberto" },
        { "2 + (3 x 4]",  -2, "fechou com a roupa errada" },
        { "2 + 3 - 4",    -3, "símbolo que não é desta conta" },
    };
    int mal = 0;
    for(size_t k = 0; k < sizeof mau/sizeof *mau; k++){
        int r = resolve("/tmp/.expr_t", mau[k].e, &v, 0);
        printf("      %-14s -> recusa %d   (%s)\n", mau[k].e, r, mau[k].porque);
        if(r != mau[k].esperado) mal++;
    }
    printf("\n");
    ok("a expressão mal formada é recusada, e o motivo vai dito", mal == 0);
    printf("      Fail-closed: não se adivinha o que o aluno quis. E veja-se o terceiro —\n");
    printf("      ']' não fecha '(' MESMO com a contagem equilibrada. A roupa não decide qual\n");
    printf("      é o nível, mas tem de casar com a que abriu, senão não há volta única.\n");
}

printf("\n§X2  O passo a passo não se programa: cada dobra É um passo.\n\n");
{
    long v;
    printf("      2 + 3 x 4\n");
    int p = resolve("/tmp/.expr_t", "2 + 3 x 4", &v, 1);
    printf("\n");
    ok("resolve 2 + 3 x 4 = 14 em duas dobras, e mostra o caminho", v == 14 && p == 2);
    printf("      Os passos SÃO as dobras, e a explicação é a fita depois de cada uma. Não há\n");
    printf("      um modo que resolve e outro que explica — é o mesmo caminho, visto de fora.\n");
}

printf("\n§X3  A precedência é a ORDEM das dobras — e mede-se que ela decide.\n\n");
{
    long a, b;
    resolve("/tmp/.expr_t", "2 + 3 x 4", &a, 0);
    resolve("/tmp/.expr_t", "(2 + 3) x 4", &b, 0);
    printf("      2 + 3 x 4     = %ld     (o x é dobrado primeiro)\n", a);
    printf("      (2 + 3) x 4   = %ld     (o parêntese é mais fundo, e dobra antes)\n\n", b);
    ok("as duas dão valores diferentes, e é a ordem que os separa", a == 14 && b == 20);
    printf("      Não há tabela de precedência no código: há uma varredura que acha o nível\n");
    printf("      mais fundo e, dentro dele, o x antes do +. A regra escolar CAI daqui, em\n");
    printf("      vez de ser imposta por cima — que é como o resto do sistema trabalha.\n");
}

printf("\n§X4  Parêntese, colchete e chave são a MESMA coisa — manda a profundidade.\n\n");
{
    long a, b, c;
    resolve("/tmp/.expr_t", "2 x [3 + (4 x 5)]", &a, 0);
    resolve("/tmp/.expr_t", "2 x (3 + [4 x 5])", &b, 0);
    resolve("/tmp/.expr_t", "2 x {3 + {4 x 5}}", &c, 0);
    printf("      2 x [3 + (4 x 5)]  = %ld\n", a);
    printf("      2 x (3 + [4 x 5])  = %ld    <- trocada a roupa, o mesmo valor\n", b);
    printf("      2 x {3 + {4 x 5}}  = %ld    <- a mesma roupa nos dois níveis, o mesmo valor\n\n", c);
    ok("trocar a roupa dos delimitadores não muda nada — manda a profundidade",
       a == 46 && b == 46 && c == 46);
    printf("      É o critério do Aarão a aplicar-se aqui: não interessa com que roupa se veste\n");
    printf("      a estrutura, interessa se ela FECHA. A convenção escolar de abrir (, depois\n");
    printf("      [, depois { é hábito de escrita — o aninhamento já diz sozinho a ordem.\n");

    printf("\n      E o passo a passo de um aninhado a três:\n\n");
    long v;
    printf("      {2 x [3 + (4 + 5)]} + 1\n");
    resolve("/tmp/.expr_t", "{2 x [3 + (4 + 5)]} + 1", &v, 1);
    printf("\n");
    ok("{2 x [3 + (4 + 5)]} + 1 = 25", v == 25);
}

printf("\n§X5  E o resultado bate com a conta à mão, em toda a bateria.\n\n");
{
    struct { const char *e; long v; } t[] = {
        { "7",                         7 },
        { "2 + 3",                     5 },
        { "2 x 3",                     6 },
        { "2 + 3 x 4",                14 },
        { "2 x 3 + 4",                10 },
        { "(2 + 3) x 4",              20 },
        { "2 x (3 + 4)",              14 },
        { "1 + 2 + 3 + 4",            10 },
        { "2 x 3 x 4",                24 },
        { "1 + 2 x 3 + 4",            11 },
        { "[2 + 3] x [4 + 5]",        45 },
        { "2 x [3 + (4 x 5)]",        46 },
        { "{2 x [3 + (4 + 5)]} + 1",  25 },
        { "{(1 + 1) x (2 + 2)} x 3",  24 },
        { "10 x 10 + 5 x 5",         125 },
        { "((((7))))",                 7 },
        { "0 x (1 + 2 + 3)",           0 },
        { "100 + 200 x 3",           700 },
    };
    int mal = 0, n = (int)(sizeof t/sizeof *t);
    printf("      expressão                    dá      esperado   passos\n");
    for(int k = 0; k < n; k++){
        long v = -1;
        int p = resolve("/tmp/.expr_t", t[k].e, &v, 0);
        printf("      %-28s %-7ld %-10ld %d%s\n", t[k].e, v, t[k].v, p,
               v == t[k].v ? "" : "   <- ERRO");
        if(v != t[k].v || p < 0) mal++;
    }
    printf("\n");
    ok("as expressões da bateria dão exatamente a conta à mão — resíduo 0", mal == 0);
    printf("      Repare-se em ((((7)))): quatro níveis e operação nenhuma. Cada dobra tira um\n");
    printf("      par e o número atravessa inteiro — a roupa sai e o valor fica, que é o que se\n");
    printf("      pede a qualquer coisa deste sistema.\n");
}

printf("\n§X6  A DISTRIBUTIVA — e ela é a prova de que a ordem não decide o valor.\n\n");
{
    /* a(b+c) = ab + ac. Aqui ela nao e mais uma regra na lista: e a reescrita que PROVA que o
     * resultado nao depende da ordem por que se dobra. E tem dual — fatorar. */
    struct { const char *e; long v; } t[] = {
        { "2 x (3 + 4)",        14 },
        { "(3 + 4) x 2",        14 },
        { "2 x (3 + 4 + 5)",    24 },
        { "1 + 2 x (3 + 4)",    15 },
        { "2 x [3 + (4 x 5)]",  46 },
        { "(1 + 2) x (3 + 4)",  21 },
        { "5 x (1 + 1) x 2",    20 },
        { "10 x (2 + 3)",       50 },
    };
    int mal = 0, difere = 0, n = (int)(sizeof t/sizeof *t);
    printf("      expressão              distribuída                    dobrando  distribuindo\n");
    for(int k = 0; k < n; k++){
        long direto = -1, pela_lei = -2;
        resolve("/tmp/.expr_t", t[k].e, &direto, 0);        /* caminho 1: dobrar por dentro */

        int fd = abre_fita("/tmp/.expr_t");                 /* caminho 2: distribuir primeiro */
        long m = ct_leia(fd, t[k].e); char b[512], pq[256];
        long d = ct_distribui(fd, m, pq, sizeof pq);
        if(d > 0){
            ct_mostra(fd, d, b, sizeof b);
            while(ct_passo(fd, d, pq, sizeof pq)) ;
            ct_valor(fd, d, &pela_lei);
        } else snprintf(b, sizeof b, "(não distribuiu)");
        close(fd); unlink("/tmp/.expr_t");

        printf("      %-21s %-30s %-9ld %ld%s\n", t[k].e, b, direto, pela_lei,
               direto == pela_lei ? "" : "   <- DIFEREM");
        if(direto != t[k].v) mal++;
        if(direto != pela_lei) difere++;
    }
    printf("\n");
    ok("os DOIS caminhos dão o mesmo valor — e é isso que a distributiva afirma", difere == 0);
    ok("e os dois batem com a conta à mão", mal == 0);
    printf("      Dobrar por dentro dá 2 x 7 = 14; distribuir dá 6 + 8 = 14. Não é coincidência\n");
    printf("      que fechem no mesmo: é exatamente o que a lei diz, e aqui está medida em vez\n");
    printf("      de citada. Repare-se em \"5 x (1 + 1) x 2\": a distribuição põe parênteses à\n");
    printf("      volta do que abriu, senão o x seguinte agarrava só a última parcela.\n");

    printf("\n      E o DUAL: distribuir abre, fatorar fecha.\n\n");
    struct { const char *e; const char *ida; const char *volta; } dd[] = {
        { "2 x (3 + 4)",     "(2 x 3 + 2 x 4)",         "(2 x (3 + 4))" },
        { "10 x (2 + 3)",    "(10 x 2 + 10 x 3)",       "(10 x (2 + 3))" },
        { "1 + 2 x (3 + 4)", "1 + (2 x 3 + 2 x 4)",     "1 + (2 x (3 + 4))" },
    };
    int mau_dual = 0;
    for(size_t k = 0; k < sizeof dd/sizeof *dd; k++){
        int fd = abre_fita("/tmp/.expr_t");
        long m = ct_leia(fd, dd[k].e); char b1[512], b2[512], pq[256];
        long d = ct_distribui(fd, m, pq, sizeof pq);
        ct_mostra(fd, d, b1, sizeof b1);
        long f = ct_fatora(fd, d, pq, sizeof pq);
        ct_mostra(fd, f, b2, sizeof b2);
        printf("      %-18s -> %-24s -> %s\n", dd[k].e, b1, b2);
        if(strcmp(b1, dd[k].ida) || strcmp(b2, dd[k].volta)) mau_dual++;
        close(fd); unlink("/tmp/.expr_t");
    }
    printf("\n");
    ok("distribuir e depois fatorar devolve a expressão de partida", mau_dual == 0);
    printf("      Volta com um par de parênteses a mais — o que a distribuição pôs para segurar\n");
    printf("      a precedência, e que o dual não tem por onde saber que era supérfluo. O valor\n");
    printf("      é o mesmo e a estrutura também; sobra roupa, e isso digo-o em vez de o calar.\n");

    printf("\n§X7  E ONDE ELA NÃO VALE, é recusada.\n\n");
    {
        int fd = abre_fita("/tmp/.expr_t");
        long m = ct_leia(fd, "2 x (3 x 4)"); char pq[256];
        long r = ct_distribui(fd, m, pq, sizeof pq);
        printf("      2 x (3 x 4)  ->  %s\n", r < 0 ? "RECUSA" : "distribuiu");
        printf("      %s\n\n", pq);
        long v1 = -1, v2 = -1;
        close(fd); unlink("/tmp/.expr_t");
        resolve("/tmp/.expr_t", "2 x (3 x 4)", &v1, 0);
        resolve("/tmp/.expr_t", "(2 x 3) x (2 x 4)", &v2, 0);
        printf("      2 x (3 x 4)        = %ld\n", v1);
        printf("      (2 x 3) x (2 x 4)  = %ld    <- o que sairia se ela valesse ali (o dobro)\n\n", v2);
        ok("o x não distribui sobre o x, e a recusa vem com o motivo", r == -1);
        ok("e os números mostram porquê: 24 contra 48", v1 == 24 && v2 == 48);
        printf("      A distributiva é entre DUAS operações diferentes: o x sobre o +. Sobre si\n");
        printf("      própria não vale, e vê-se PORQUÊ nos números: distribuir ali aplicaria o\n");
        printf("      fator 2 a CADA parcela, e as parcelas multiplicam-se entre si — o 2 entra\n");
        printf("      duas vezes e sai 2 vezes 24. Sobre o + isso não acontece porque as\n");
        printf("      parcelas somam-se, e o fator sai em evidência inteiro. É a mesma\n");
        printf("      disciplina do corpus científico: a lei vale no corpo declarado, e dizer só\n");
        printf("      \"vale a distributiva\" sem dizer de que sobre que é dizer meia lei.\n");
    }
}

printf("\n");
return falhas ? 1 : 0;
}
