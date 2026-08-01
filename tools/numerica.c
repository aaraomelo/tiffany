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
 *   §X8  subtração e divisão: associam à ESQUERDA, e a divisão não fecha em Z
 *   §X9  e a distributiva da divisão é de um lado só
 *   §X10 a POTÊNCIA associa à DIREITA, e o que não fecha para e diz o corpo
 *   §X11 e ela distribui sobre o produto, nunca sobre a soma
 *   §X12 o FATORIAL é pósfixo — o único — e o MÓDULO é o Z/n do corpus
 *   §X13 e com ele a máquina VERIFICA as afirmações do corpus científico
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
    while(ct_passo(fd, n, porque, sizeof porque) == 1){
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
        { "2 + 3 & 4",    -3, "símbolo que não é desta conta" },
        /* Aqui estava "2 + 3 - 4", depois "2 + 3 ^ 4", e as DUAS passaram a resolver — a
         * linguagem cresceu e o que era recusa virou conta, duas vezes seguidas. O teste caiu
         * vermelho nas duas, que é para o que ele serve. Ficou o '&', que não é operação
         * aritmética nenhuma e por isso não envelhece com o próximo alargamento. */
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

printf("\n§X15 OS DECIMAIS SÃO NOTAÇÃO — e a dízima é da BASE, não do número.\n\n");
{
    /* Nao ha aritmetica nova nenhuma: 0,25 entra como 25/100 e reduz por Euclides a 1/4. O
     * que a maquina ganhou foi uma PORTA, e a conta continua a ser a de Q. */
    struct { const char *e; long p, q; const char *dec; long pre, per; } t[] = {
        { "0,25",        1,  4, "0,25",            2, 0 },
        { "0,5 + 0,25",  3,  4, "0,75",            2, 0 },
        { "1 / 4",       1,  4, "0,25",            2, 0 },
        { "1 / 3",       1,  3, "0,(3)...",        0, 1 },
        { "1 / 7",       1,  7, "0,(142857)...",   0, 6 },
        { "1 / 6",       1,  6, "0,1(6)...",       1, 1 },
        { "1 / 8",       1,  8, "0,125",           3, 0 },
        { "1 / 11",      1, 11, "0,(09)...",       0, 2 },
        { "1 / 13",      1, 13, "0,(076923)...",   0, 6 },
        { "22 / 7",     22,  7, "3,(142857)...",   0, 6 },
        { "0,125 x 8",   1,  1, "1",               0, 0 },
    };
    int mal = 0;
    printf("      expressão        fração   decimal              pré  período\n");
    for(size_t k = 0; k < sizeof t/sizeof *t; k++){
        int fd = abre_fita("/tmp/.expr_t");
        long m = ct_leia(fd, t[k].e); char pq[512] = ""; long p = -9, q = -9;
        while(ct_passo(fd, m, pq, sizeof pq) == 1) ;
        int bom = ct_valorq(fd, m, &p, &q);
        close(fd); unlink("/tmp/.expr_t");
        char f[64], d[160]; long pre = -1, per = -1;
        if(bom){ ct_escreve(p, q, f, sizeof f); ct_decimal(p, q, d, sizeof d, &pre, &per); }
        else { snprintf(f, sizeof f, "?"); snprintf(d, sizeof d, "?"); }
        printf("      %-16s %-8s %-20s %-4ld %ld\n", t[k].e, f, d, pre, per);
        if(!bom || p != t[k].p || q != t[k].q || strcmp(d, t[k].dec)
           || pre != t[k].pre || per != t[k].per) mal++;
    }
    printf("\n");
    ok("o decimal entra como fração, e a dízima sai com o período certo", mal == 0);
    printf("      A REGRA é exata: p/q reduzida tem decimal FINITO em base b se e só se todo o\n");
    printf("      primo de q divide b. Em base 10, q só pode ter 2 e 5 — e o número de casas é\n");
    printf("      o maior dos dois expoentes. Se sobrar outro fator, a dízima é infinita e o\n");
    printf("      comprimento do período é a ORDEM de 10 módulo o que sobrou. É por isso que\n");
    printf("      1/7 tem período 6 e 1/11 tem período 2: 10^6 = 1 mod 7, e 10^2 = 1 mod 11.\n");
    printf("      O módulo, que entrou há dois passos, é o que mede isto.\n");

    printf("\n      E o corpus tinha duas entradas à espera desta:\n\n");
    {
        int fd = abre_fita("/tmp/.expr_t");
        long m = ct_leia(fd, "0,1 + 0,2"); char pq[512] = ""; long p, q;
        while(ct_passo(fd, m, pq, sizeof pq) == 1) ;
        int bom = ct_valorq(fd, m, &p, &q);
        char d[160]; long pre, per; ct_decimal(p, q, d, sizeof d, &pre, &per);
        close(fd); unlink("/tmp/.expr_t");
        printf("      \"0,1 mais 0,2 é 0,3\" -> o corpus: em binary64 dá 0,30000000000000004\n");
        printf("      a máquina, em Q:        0,1 + 0,2 = %ld/%ld = %s\n\n", p, q, d);
        ok("e em Q dá 0,3 EXATO — o defeito era da representação, não da aritmética",
           bom && p == 3 && q == 10 && !strcmp(d, "0,3"));
        printf("      \"um terço em decimal\" -> o corpus: 0,333... em base 10, 0,1 exato em\n");
        printf("      base 3, e a dízima é da BASE. A máquina diz o mesmo, e diz PORQUÊ: o 3 não\n");
        printf("      divide 10. Outra vez as duas metades a concordar sem terem sido ligadas.\n");
    }
}

printf("\n§X14 AS FRAÇÕES — e a máquina deixou de ser Z e passou a ser Q.\n\n");
{
    /* Nao houve sintaxe nova: o '/' ja era lido como operador, e o que mudou foi que ele
     * deixa de PARAR e passa a construir. Uma fracao e um par (p,q) com q != 0, e a
     * igualdade nao e de pares: 2/4 e 1/2 sao o MESMO numero. E o representante canonico
     * obtem-se por EUCLIDES — o mesmo algoritmo que gera a cifra do rei. */
    struct { const char *e; long p, q; const char *nota; } t[] = {
        { "7 / 2",            7, 2, "em Z parava; em Q fecha" },
        { "2 / 4",            1, 2, "reduzida por Euclides — o mesmo da cifra" },
        { "1 / 2 + 1 / 3",    5, 6, "o comum é o produto reduzido pelo mdc" },
        { "1 / 2 + 1 / 2",    1, 1, "e fecha de volta em Z: Z está DENTRO de Q" },
        { "2 / 3 x 3 / 4",    1, 2, "" },
        { "1 / 2 / (1 / 4)",  2, 1, "dividir por uma fração é multiplicar pelo inverso" },
        { "2 ^ -1",           1, 2, "o expoente negativo pede o INVERSO — o dual que Z não tinha" },
        { "(1/2) ^ 3",        1, 8, "a potência entra em cima e em baixo" },
        { "raiz (4 / 9)",     2, 3, "fecha porque 4 e 9 são ambos quadrados" },
        { "3 / 6 + 1 / 6",    2, 3, "" },
        { "6 / 3",            2, 1, "" },
    };
    int mal = 0;
    printf("      expressão              dá       nota\n");
    for(size_t k = 0; k < sizeof t/sizeof *t; k++){
        int fd = abre_fita("/tmp/.expr_t");
        long m = ct_leia(fd, t[k].e); char pq[512] = ""; long p = -9, q = -9;
        while(ct_passo(fd, m, pq, sizeof pq) == 1) ;
        int bom = ct_valorq(fd, m, &p, &q);
        close(fd); unlink("/tmp/.expr_t");
        char r[64]; if(bom) ct_escreve(p, q, r, sizeof r); else snprintf(r, sizeof r, "?");
        printf("      %-22s %-8s %s\n", t[k].e, r, t[k].nota);
        if(!bom || p != t[k].p || q != t[k].q) mal++;
    }
    printf("\n");
    ok("as frações fecham, reduzem-se, e voltam a Z quando o dão", mal == 0);
    printf("      Q é Z com o dual da multiplicação. Em Z só o 1 e o -1 têm inverso; em Q todo o\n");
    printf("      não nulo tem, e é SÓ isso que separa os dois. Repare-se em 2^-1: em Z parava,\n");
    printf("      aqui dá 1/2 — a mesma conta, o mesmo símbolo, e o corpo é que mudou.\n");

    printf("\n      E o que continua a não fechar, agora com Q inteiro à disposição:\n\n");
    struct { const char *e; } p2[] = { {"raiz 2"}, {"raiz (2 / 9)"}, {"7 / 2 mod 3"},
                                       {"(1/2) !"}, {"1 / 0"} };
    int paradas = 0;
    for(size_t k = 0; k < sizeof p2/sizeof *p2; k++){
        int fd = abre_fita("/tmp/.expr_t");
        long m = ct_leia(fd, p2[k].e); char pq[512] = ""; int st;
        while((st = ct_passo(fd, m, pq, sizeof pq)) == 1) ;
        close(fd); unlink("/tmp/.expr_t");
        printf("      %-14s %.150s\n", p2[k].e, pq);
        if(st < 0) paradas++;
    }
    printf("\n");
    ok("e as cinco continuam a parar, cada uma pela sua razão", paradas == 5);
    printf("      A PRIMEIRA É QUE INTERESSA. A máquina JÁ ESTÁ em Q — o 7/2 fecha, o 2^-1 fecha\n");
    printf("      — e a raiz de 2 continua fora. Não é falta de alcance da máquina: é o que\n");
    printf("      IRRACIONAL quer dizer, e quer dizer exatamente isto. Ampliar o corpo resolveu\n");
    printf("      a divisão e não resolveu esta, e a diferença entre as duas é o assunto.\n");
    printf("\n      E o resto tem cada um a sua razão, que não é a mesma: o módulo não existe em\n");
    printf("      Q porque em Q toda a divisão fecha — o resto é o que sobra de NÃO fechar, e\n");
    printf("      onde nada sobra não há resto. O fatorial de 1/2 sai do produto e vira integral.\n");
}

printf("\n§X12 O FATORIAL É PÓSFIXO, E O MÓDULO É O Z/n DO CORPUS.\n\n");
{
    struct { const char *e; long v; const char *nota; } t[] = {
        { "5 !",             120, "" },
        { "0 !",               1, "o produto VAZIO, e a recursão obriga" },
        { "3 ! !",           720, "(3!)! — o pósfixo associa à esquerda sozinho" },
        { "2 ^ 3 !",          64, "é 2^(3!) — o fatorial liga mais forte que a potência" },
        { "20 !", 2432902008176640000L, "o último que cabe num inteiro da máquina" },
        { "7 mod 3",           1, "" },
        { "-7 mod 3",          2, "o representante NÃO negativo; em C daria -1" },
        { "7 % 3",             1, "o mesmo, com a outra roupa" },
        { "10 mod 4 mod 3",    2, "à esquerda, como o x e o /" },
        { "5 ! mod 7",         1, "os dois juntos" },
    };
    int mal = 0;
    printf("      expressão            dá                     nota\n");
    for(size_t k = 0; k < sizeof t/sizeof *t; k++){
        long v = -999; resolve("/tmp/.expr_t", t[k].e, &v, 0);
        printf("      %-20s %-22ld %s\n", t[k].e, v, t[k].nota);
        if(v != t[k].v) mal++;
    }
    printf("\n");
    ok("o fatorial e o módulo dão a conta à mão — e o fatorial liga mais forte", mal == 0);
    printf("      O fatorial é o ÚNICO operador pósfixo desta máquina, e por isso é o único que\n");
    printf("      não precisa de se decidir a associatividade: 3!! só pode ser (3!)!. Onde a\n");
    printf("      potência precisou de uma varredura ao contrário, este não precisou de nada.\n");

    printf("\n      E o que não fecha para, cada um com a sua razão:\n\n");
    struct { const char *e; } p[] = { {"21 !"}, {"-3 !"}, {"5 mod 0"} };
    int paradas = 0;
    for(size_t k = 0; k < sizeof p/sizeof *p; k++){
        int fd = abre_fita("/tmp/.expr_t");
        long m = ct_leia(fd, p[k].e); char pq[400] = ""; int st;
        while((st = ct_passo(fd, m, pq, sizeof pq)) == 1) ;
        close(fd); unlink("/tmp/.expr_t");
        printf("      %-9s %s\n", p[k].e, pq);
        if(st < 0) paradas++;
    }
    printf("\n");
    ok("as três param, e a de 21! é da MÁQUINA e não da matemática", paradas == 3);
}

printf("\n§X13 E AGORA A MÁQUINA VERIFICA O CORPUS.\n\n");
{
    /* Isto e o fecho de duas semanas de trabalho em lados diferentes. O corpus cientifico
     * afirma coisas sobre Z/n; ate hoje eram texto. Com o modulo, a maquina CONTA. */
    long a1, a2, b1, b2, c1;
    resolve("/tmp/.expr_t", "(3 x 3) mod 3", &a1, 0);
    resolve("/tmp/.expr_t", "(3 + 3) mod 3", &a2, 0);
    resolve("/tmp/.expr_t", "(3 x 3) mod 7", &b1, 0);
    resolve("/tmp/.expr_t", "(3 x 3) mod 11", &b2, 0);
    resolve("/tmp/.expr_t", "(5 x 5) mod 11", &c1, 0);
    printf("      o corpus diz: \"em Z/3, 3 vezes 3 é 3 mais 3\"\n");
    printf("        (3 x 3) mod 3 = %ld    (3 + 3) mod 3 = %ld\n\n", a1, a2);
    ok("e a máquina conta, e confirma: os dois dão 0 em Z/3", a1 == 0 && a2 == 0 && a1 == a2);

    printf("\n      o corpus diz: \"a raiz de 2 existe em Z/7, e vale 3\"\n");
    printf("        (3 x 3) mod 7 = %ld\n\n", b1);
    ok("e a máquina conta, e confirma: 3 ao quadrado é 2 em Z/7", b1 == 2);

    printf("\n      o corpus diz: \"a raiz de 3 existe em Z/11, e vale 5\"\n");
    printf("        (5 x 5) mod 11 = %ld    (e (3x3) mod 11 = %ld, que não é 3)\n\n", c1, b2);
    ok("e confirma outra vez — 5 ao quadrado é 3 em Z/11", c1 == 3);

    printf("      Estas três frases estavam no corpus como TEXTO, escritas noutro dia e por\n");
    printf("      outro motivo. Com o módulo, a máquina conta-as. E o que fecha o círculo é\n");
    printf("      que nada foi ligado de propósito: o corpus declarou o corpo porque esse é o\n");
    printf("      critério, o resolvedor declarou o corpo pelo mesmo critério, e agora um\n");
    printf("      verifica o outro. A primeira delas foi o Aarão a corrigir-me — eu tinha dito\n");
    printf("      que 3x3 = 3+3 não passava, assumindo Z sem o declarar. Aqui está a contar.\n");
}

printf("\n§X10 A POTÊNCIA ASSOCIA À DIREITA — ao contrário de tudo o resto aqui.\n\n");
{
    struct { const char *e; long v; const char *nota; } t[] = {
        { "2 ^ 3",          8, "" },
        { "3 ^ 2",          9, "e 2^3 é 8: a potência NÃO comuta" },
        { "2 ^ 3 ^ 2",    512, "é 2^(3^2), à DIREITA" },
        { "(2 ^ 3) ^ 2",   64, "e à esquerda daria isto — oito vezes menos" },
        { "raiz 9",         3, "" },
        { "raiz 16 + raiz 9", 7, "" },
        { "2 ^ 0",          1, "" },
        { "raiz 0",         0, "" },
        { "(2 + 3) ^ 2",   25, "e 2^2 + 3^2 é 13 — falta o cruzado" },
        { "2 ^ 2 + 3 ^ 2", 13, "" },
    };
    int mal = 0;
    printf("      expressão            dá     esperado   nota\n");
    for(size_t k = 0; k < sizeof t/sizeof *t; k++){
        long v = -999; resolve("/tmp/.expr_t", t[k].e, &v, 0);
        printf("      %-20s %-6ld %-10ld %s\n", t[k].e, v, t[k].v, t[k].nota);
        if(v != t[k].v) mal++;
    }
    printf("\n");
    ok("a potência associa à DIREITA, e o valor confirma-o: 512 e não 64", mal == 0);
    printf("      A subtração dobra o PRIMEIRO da esquerda; a potência dobra o ÚLTIMO da direita.\n");
    printf("      É a mesma varredura no sentido contrário, e é só isso que separa 512 de 64. A\n");
    printf("      associatividade não é um facto sobre o símbolo: é a direção em que se lê.\n");

    printf("\n      E o que NÃO fecha para, e diz onde fecha:\n\n");
    struct { const char *e; const char *o_que; } p[] = {
        { "raiz 2",  "o corpus já dizia isto, e a máquina chega lá sozinha" },
        { "raiz -4", "em R não; em C sim" },
        { "0 ^ 0",   "depende do que se está a fazer" },
        { "2 ^ 100", "e este é da MÁQUINA, não da matemática" },
    };
    /* o "2 ^ -1" estava nesta lista e SAIU: com as frações passou a fechar e vale 1/2. É o
     * segundo teste desta série a mudar de resposta por o corpo ter crescido. */
    int paradas = 0;
    for(size_t k = 0; k < sizeof p/sizeof *p; k++){
        int fd = abre_fita("/tmp/.expr_t");
        long m = ct_leia(fd, p[k].e); char pq[400] = ""; int st;
        while((st = ct_passo(fd, m, pq, sizeof pq)) == 1) ;
        close(fd); unlink("/tmp/.expr_t");
        printf("      %-10s %s\n", p[k].e, pq);
        if(st < 0) paradas++;
    }
    printf("\n");
    ok("as quatro param e declaram o corpo, em vez de arredondar ou inventar",
       paradas == (int)(sizeof p/sizeof *p));
    printf("      Repare-se na raiz de 2: a máquina diz que em Z/7 existe, porque 3 x 3 = 9 = 2.\n");
    printf("      É a MESMA resposta que o corpus científico dá à fala \"a raiz de 2 é racional\",\n");
    printf("      e eu não liguei as duas — chegaram lá pelo mesmo critério, que é declarar o\n");
    printf("      corpo. E a última é de outra natureza: 2^100 existe, a caixa é que acaba, e\n");
    printf("      dizer que \"não dá\" sem separar isso seria confundir a máquina com a álgebra.\n");
}

printf("\n§X11 A POTÊNCIA DISTRIBUI SOBRE O PRODUTO, E NÃO SOBRE A SOMA.\n\n");
{
    struct { const char *e; long v; int distribui; } t[] = {
        { "(2 x 3) ^ 2",      36, 1 },
        { "(2 x 3 x 5) ^ 2", 900, 1 },
        { "(2 + 3) ^ 2",      25, 0 },
        { "(10 - 4) ^ 2",     36, 0 },
    };
    int mal = 0;
    for(size_t k = 0; k < sizeof t/sizeof *t; k++){
        int fd = abre_fita("/tmp/.expr_t");
        long m = ct_leia(fd, t[k].e); char b[256], pq[400] = ""; long v = -999;
        long d = ct_distribui(fd, m, pq, sizeof pq);
        if(d > 0){ ct_mostra(fd, d, b, sizeof b);
                   while(ct_passo(fd, d, pq, sizeof pq) == 1) ; ct_valor(fd, d, &v); }
        else snprintf(b, sizeof b, "RECUSA");
        close(fd); unlink("/tmp/.expr_t");
        long direto = -998; resolve("/tmp/.expr_t", t[k].e, &direto, 0);
        printf("      %-16s -> %-24s   direto %ld\n", t[k].e, b, direto);
        if(t[k].distribui){ if(d <= 0 || v != direto || direto != t[k].v) mal++; }
        else              { if(d >= 0 || direto != t[k].v) mal++; }
    }
    printf("\n");
    ok("distribui sobre o x e RECUSA sobre o + — e os valores conferem", mal == 0);
    long a1 = -1, a2 = -1;
    resolve("/tmp/.expr_t", "(2 + 3) ^ 2", &a1, 0);
    resolve("/tmp/.expr_t", "2 ^ 2 + 3 ^ 2", &a2, 0);
    printf("      (2 + 3) ^ 2      = %ld\n", a1);
    printf("      2 ^ 2 + 3 ^ 2    = %ld    <- e a diferença, %ld, é o termo cruzado 2ab\n\n",
           a2, a1 - a2);
    ok("a diferença é exatamente 2ab = 12, e não um resto qualquer",
       a1 == 25 && a2 == 13 && a1 - a2 == 2*2*3);
    printf("      Sobre o produto vale porque a potência É multiplicação repetida e o produto\n");
    printf("      comuta: os fatores separam-se e cada um leva o expoente. Sobre a soma não há\n");
    printf("      nada que separe, e o que sobra tem NOME e valor — 2ab, que aqui é 12. O engano\n");
    printf("      não é escrever a^n + b^n: é achar que o que falta não estava lá.\n");
}

printf("\n§X8  SUBTRAÇÃO E DIVISÃO — e cada uma traz um problema que era só delas.\n\n");
{
    /* A SUBTRACAO NAO E ASSOCIATIVA, e isso quase me escapou: fazer todos os '+' e so depois
     * os '-' daria, em "10 - 2 + 3", primeiro 2+3=5 e depois 10-5=5, quando o certo e 11. Cada
     * passada trata os DOIS operadores do seu nivel juntos, na ordem em que aparecem. */
    struct { const char *e; long v; const char *nota; } t[] = {
        { "10 - 3 - 2",      5, "à esquerda: (10-3)-2, e não 10-(3-2) que daria 9" },
        { "10 - 2 + 3",     11, "e NÃO 5 — o + e o - dobram na ordem em que aparecem" },
        { "8 / 2 x 2",       8, "e NÃO 2 — o mesmo, no nível de cima" },
        { "100 / 5 / 2",    10, "à esquerda também: (100/5)/2" },
        { "3 - 5",          -2, "em Z fecha; em N não estaria lá" },
        { "-3 + 5",          2, "o sinal unário entra" },
        { "2 + 3 x 4 - 5",   9, "os dois níveis, e a ordem dentro de cada um" },
        { "(10 - 4) / 2",    3, "o parêntese primeiro, como sempre" },
    };
    int mal = 0;
    printf("      expressão            dá     esperado   porquê\n");
    for(size_t k = 0; k < sizeof t/sizeof *t; k++){
        long v = -999; resolve("/tmp/.expr_t", t[k].e, &v, 0);
        printf("      %-20s %-6ld %-10ld %s\n", t[k].e, v, t[k].v, t[k].nota);
        if(v != t[k].v) mal++;
    }
    printf("\n");
    ok("a subtração e a divisão associam à ESQUERDA, e os pares dobram juntos", mal == 0);
    printf("      Se as passadas fossem uma por operador, \"10 - 2 + 3\" dava 5. A associatividade\n");
    printf("      à esquerda não é uma regra escrita: sai da varredura, que dobra o primeiro do\n");
    printf("      conjunto que encontra. É o mesmo mecanismo da precedência, um nível abaixo.\n");

    printf("\n      E a DIVISÃO: o 7/2 já não para, e é isso que interessa dizer.\n\n");
    {
        /* ESTE TESTE MUDOU DE RESPOSTA, e mudou por o CORPO ter crescido. Enquanto a máquina
         * era Z, 7/2 parava e declarava que não fechava — e essa era a resposta certa LÁ. Com
         * as frações, fecha. A afirmação antiga não era falsa: era relativa, e a régua mudou. */
        int fd = abre_fita("/tmp/.expr_t");
        long n1 = ct_leia(fd, "7 / 2"); char pq1[512] = ""; long p1 = 0, q1 = 0;
        int s1; while((s1 = ct_passo(fd, n1, pq1, sizeof pq1)) == 1) ;
        int b1 = ct_valorq(fd, n1, &p1, &q1);
        close(fd); unlink("/tmp/.expr_t");
        fd = abre_fita("/tmp/.expr_t");
        long n2 = ct_leia(fd, "5 / 0"); char pq2[512] = "";
        int s2; while((s2 = ct_passo(fd, n2, pq2, sizeof pq2)) == 1) ;
        close(fd); unlink("/tmp/.expr_t");
        printf("      7 / 2  ->  %s\n", pq1);
        printf("      5 / 0  ->  %s\n\n", pq2);
        ok("7/2 fecha agora e vale 7/2 — porque a máquina passou de Z a Q",
           s1 >= 0 && b1 && p1 == 7 && q1 == 2);
        ok("e a divisão por zero continua a parar: essa não é falta de corpo", s2 < 0);
        printf("      A resposta certa a 7/2 nos inteiros não era 3 nem 3,5 — era que ali não\n");
        printf("      fecha. Em Q fecha, e as duas estão certas, cada uma no seu corpo. Já a\n");
        printf("      divisão por zero não muda com corpo nenhum: se 0 vezes x fosse 1, a\n");
        printf("      estrutura colapsava. Uma é falta de corpo; a outra é impossível em todos,\n");
        printf("      e o teste que não separasse as duas estaria a medir a coisa errada.\n");
    }
}

printf("\n§X9  A DISTRIBUTIVA DA DIVISÃO É DE UM LADO SÓ.\n\n");
{
    /* (a+b)/c = a/c + b/c, mas c/(a+b) != c/a + c/b. E a razao e a mesma da nao-comutatividade:
     * a divisao nao e simetrica, logo a lei dela tambem nao pode ser. */
    struct { const char *e; long v; int distribui; } t[] = {
        { "(4 + 2) / 2",   3, 1 },
        { "(12 - 4) / 4",  2, 1 },
        { "12 / (2 + 4)",  2, 0 },
    };
    int mal = 0;
    for(size_t k = 0; k < sizeof t/sizeof *t; k++){
        int fd = abre_fita("/tmp/.expr_t");
        long n = ct_leia(fd, t[k].e); char b[256], pq[256] = "";
        long d = ct_distribui(fd, n, pq, sizeof pq);
        long v = -999;
        if(d > 0){ ct_mostra(fd, d, b, sizeof b);
                   while(ct_passo(fd, d, pq, sizeof pq) == 1) ; ct_valor(fd, d, &v); }
        else snprintf(b, sizeof b, "RECUSA");
        close(fd); unlink("/tmp/.expr_t");
        long direto = -998; resolve("/tmp/.expr_t", t[k].e, &direto, 0);
        printf("      %-16s -> %-22s   direto %ld\n", t[k].e, b, direto);
        if(t[k].distribui){ if(d <= 0 || v != direto || direto != t[k].v) mal++; }
        else              { if(d >= 0 || direto != t[k].v) mal++; }
    }
    printf("\n");
    ok("a divisão distribui pela DIREITA e recusa pela esquerda", mal == 0);
    long a1 = -1, a2 = -1;
    resolve("/tmp/.expr_t", "12 / (2 + 4)", &a1, 0);
    resolve("/tmp/.expr_t", "12 / 2 + 12 / 4", &a2, 0);
    printf("      12 / (2 + 4)      = %ld\n", a1);
    printf("      12 / 2 + 12 / 4   = %ld    <- o que sairia se distribuísse pela esquerda\n\n", a2);
    ok("e os números mostram porquê: 2 contra 9", a1 == 2 && a2 == 9);
    printf("      É a mesma assimetria que faz a divisão não comutar. Uma lei que vale de um lado\n");
    printf("      e não do outro não é meia lei: é uma lei com o lado DITO — e calar o lado é que\n");
    printf("      seria o erro. A multiplicação vale dos dois porque É simétrica.\n");
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
            while(ct_passo(fd, d, pq, sizeof pq) == 1) ;
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
        long m = ct_leia(fd, "2 x (3 x 4)"); char pq[512];
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
